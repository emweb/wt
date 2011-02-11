/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CNOR
#include <fstream>
#include <stdexcept>
#include <cstring>

#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>

#include "DomElement.h"
#include "EscapeOStream.h"
#include "Utils.h"
#include "Wt/WLogger"
#include "Wt/WMessageResources"
#include "Wt/WString"
#include "Wt/WApplication"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

using namespace Wt;
using namespace rapidxml;

namespace {
  void fixSelfClosingTags(xml_node<> *x_node)
  {
    for (xml_node<> *x_child = x_node->first_node(); x_child;
	 x_child = x_child->next_sibling())
      fixSelfClosingTags(x_child);

    if (!x_node->first_node()
	&& x_node->value_size() == 0
	&& !Wt::DomElement::isSelfClosingTag
	(std::string(x_node->name(), x_node->name_size()))) {
      // We need to add an emtpy data node since <div /> is illegal HTML
      // (but valid XML / XHTML)
      xml_node<> *empty
	= x_node->document()->allocate_node(node_data, 0, 0, 0, 0);
      x_node->append_node(empty);
    }
  }

  std::string readElementContent(xml_node<> *x_parent, 
				 boost::scoped_array<char> &buf) 
  {
    char *ptr = buf.get();
    
    for (xml_node<> *x_child = x_parent->first_node();
	 x_child; x_child = x_child->next_sibling()) {
      fixSelfClosingTags(x_child);
      ptr = print(ptr, *x_child, print_no_indenting);
    }
    return std::string(buf.get(), ptr - buf.get());
  }

  int attributeValueToInt(xml_attribute<> *x_attribute)
  {
    return boost::lexical_cast<int>(std::string(x_attribute->value(), 
						x_attribute->value_size()));
  }
}

namespace Wt {

WMessageResources::WMessageResources(const std::string& path,
				     bool loadInMemory)
  : loadInMemory_(loadInMemory),
    loaded_(false),
    path_(path)
{}

WMessageResources::WMessageResources(const char *data)
  : loadInMemory_(true),
    loaded_(false),
    path_("")
{
  std::istringstream s(data,  std::ios::in | std::ios::binary);
  readResourceStream(s, defaults_, "<internal resource bundle>");
}

std::set<std::string> 
WMessageResources::keys(WFlags<WMessageResourceBundle::Scope> scope) const
{
  std::set<std::string> keys;
  
  KeyValuesMap::const_iterator it;

  if (scope & WMessageResourceBundle::Local)
    for (it = local_.begin() ; it != local_.end(); it++)
      keys.insert((*it).first);

  if (scope & WMessageResourceBundle::Default)
    for (it = defaults_.begin() ; it != defaults_.end(); it++)
      keys.insert((*it).first);

  return keys;
}

void WMessageResources::refresh()
{
  if (!path_.empty()) {
    defaults_.clear();
    readResourceFile("", defaults_);

    local_.clear();
    WApplication *app = WApplication::instance();
    std::string locale = app ? app->locale() : std::string();

    if (!locale.empty())
      for(;;) {
        if (readResourceFile(locale, local_))
          break;

        /* try a lesser specified variant */
        std::string::size_type l = locale.rfind('-');
        if (l != std::string::npos)
          locale.erase(l);
        else
          break;
      }

      loaded_ = true;
  }
}

void WMessageResources::hibernate()
{
  if (!loadInMemory_) {
    defaults_.clear();
    local_.clear();
    loaded_ = false;
  }
}

bool WMessageResources::resolveKey(const std::string& key, std::string& result)
{
  if (!loaded_)
    refresh();

  KeyValuesMap::const_iterator j;

  j = local_.find(key);
  if (j != local_.end()) {
    if (j->second.size() > 1 )
      return false;
    result = j->second[0];
    return true;
  }

  j = defaults_.find(key);
  if (j != defaults_.end()) {
    if (j->second.size() > 1 )
      return false;
    result = j->second[0];
    return true;
  }

  return false;
}

std::string WMessageResources::findCase(const std::vector<std::string> &cases, 
					::uint64_t amount)
{
  int c = Utils::calculatePluralCase(pluralExpression_, amount);

  if (c > (int)cases.size() - 1 || c < 0) {
    SStream error;
    error << "Expression '" << pluralExpression_ << "' evaluates to '" 
	  << c << "' for n=" << boost::lexical_cast<std::string>(amount);
    
    if (c < 0) 
      error << " and values smaller than 0 are not allowed.";
    else
      error << " which is greater than the list of cases (size=" 
	    << (int)cases.size() << ").";
    
    throw std::logic_error(error.c_str());
  }

  return cases[c];
}

bool WMessageResources::resolvePluralKey(const std::string& key, 
					 std::string& result, 
					 ::uint64_t amount)
{
  if (!loaded_)
    refresh();

  KeyValuesMap::const_iterator j;

  j = local_.find(key);
  if (j != local_.end()) {
    if (j->second.size() != pluralCount_ )
      return false;
    result = findCase(j->second, amount);
    return true;
  }

  j = defaults_.find(key);
  if (j != defaults_.end()) {
    if (j->second.size() != pluralCount_)
      return false;
    result = findCase(j->second, amount);
    return true;
  }

  return false;
}

bool WMessageResources::readResourceFile(const std::string& locale,
				         KeyValuesMap& valueMap)
{
  if (!path_.empty()) {
    std::string fileName
      = path_ + (locale.length() > 0 ? "_" : "") + locale + ".xml";

    std::ifstream s(fileName.c_str(), std::ios::binary);
    return readResourceStream(s, valueMap, fileName);
  } else {
    return false;
  }
}

bool WMessageResources::readResourceStream(std::istream &s,
					   KeyValuesMap& valueMap,
                                           const std::string &fileName)
{
  if (!s)
    return false;

  s.seekg(0, std::ios::end);
  int length = s.tellg();
  s.seekg(0, std::ios::beg);

  enum { UTF8, UTF16LE, UTF16BE } encoding = UTF8;

  // See if we have UTF16 BOM
  if (length >= 2) {
    unsigned char m1, m2;
    m1 = s.get();
    m2 = s.get();

    if (m1 == 0xFE && m2 == 0xFF)
      encoding = UTF16BE;
    else if (m1 == 0xFF && m2 == 0xFE)
      encoding = UTF16LE;
    else {
      s.unget();
      s.unget();
    }
  }

  boost::scoped_array<char> text
    (new char[encoding == UTF8 ? length + 1 : (length-2)*2 + 1]);

  if (encoding != UTF8) {
    // Transcode from UTF16 stream to UTF8 text
    const int BUFSIZE = 2048;
    unsigned char buf[BUFSIZE];

    unsigned long firstWord = 0;
    char *out = text.get();

    for (;;) {
      s.read((char *)buf, BUFSIZE);
      int read = s.gcount();

      for (int i = 0; i < read; i += 2) {
	unsigned long ch;

	// read next 2-byte char
	if (encoding == UTF16LE) {
	  ch = buf[i+1];
	  ch = (ch << 8) | buf[i];
	} else {
	  ch = buf[i];
	  ch = (ch << 8) | buf[i+1];
	}

	if (firstWord) {
	  // second word of multi-word
	  if (ch < 0xDC00 || ch > 0xDFFF) {
	    read = 0;
	    break;
	  }

	  unsigned long cp = 0x10000 + (((firstWord & 0x3FF) << 10)
					| (ch & 0x3FF));

	  rapidxml::xml_document<>::insert_coded_character<0>(out, cp);

	  firstWord = 0;
	} else if (ch >= 0xD800 && ch <= 0xDBFF) {
	  // first word of multi-word
	  firstWord = ch;
	} else {
	  // single-word
	  rapidxml::xml_document<>::insert_coded_character<0>(out, ch);

	  firstWord = 0;
	}
      }

      if (read != BUFSIZE)
	break;
    }

    length = out - text.get();
  } else {
    s.read(text.get(), length);
  }

  text[length] = 0;

  try {
    xml_document<> doc;
    doc.parse<parse_no_string_terminators
      | parse_comment_nodes
      | parse_xhtml_entity_translation
      | parse_validate_closing_tags>(text.get());

    xml_node<> *x_root = doc.first_node("messages");
    if (!x_root)
      throw parse_error("Expected <messages> root element", text.get());

    xml_attribute<> *x_nplurals = x_root->first_attribute("nplurals");
    xml_attribute<> *x_plural = x_root->first_attribute("plural");
    if (x_nplurals && !x_plural)
      throw parse_error("Expected 'plural' attribute in <messages>",
			x_root->value());
    if (x_plural && !x_nplurals)
      throw parse_error("Expected 'nplurals' attribute in <messages>",
			x_root->value());
    if (x_nplurals && x_plural) {
      pluralCount_ = attributeValueToInt(x_nplurals);
      pluralExpression_ 
	= std::string(x_plural->value(), x_plural->value_size());
    } else {
      pluralCount_ = 0;
    }

    // factor 2 in case we expanded <span/> to <span></span>
    boost::scoped_array<char> buf(new char[length * 2]);

    for (xml_node<> *x_message = x_root->first_node("message");
	 x_message; x_message = x_message->next_sibling("message")) {
      if (std::strncmp(x_message->name(), "message", x_message->name_size())
	  != 0)
	throw parse_error("Expected <message>", x_message->value());

      xml_attribute<> *x_id = x_message->first_attribute("id");
      if (!x_id)
	throw parse_error("Missing message id", x_message->value());

      std::string id(x_id->value(), x_id->value_size());

      xml_node<> *x_plural = x_message->first_node("plural");
      if (x_plural) {
	if (pluralCount_ == 0)
	  throw parse_error("Expected 'nplurals' attribute in <message>",
			    x_plural->value());

	valueMap[id] = std::vector<std::string>();
	valueMap[id].reserve(pluralCount_);
	
	std::vector<bool> visited;
	visited.reserve(pluralCount_);
	
	for (unsigned i = 0; i < pluralCount_; i++) {
	  valueMap[id].push_back(std::string());
	  visited.push_back(false);
	}
	
	for (; x_plural; x_plural = x_plural->next_sibling("plural")) {
	  xml_attribute<> *x_case = x_plural->first_attribute("case");
	  int c = attributeValueToInt(x_case);
	  if (c >= (int)pluralCount_)
	    throw parse_error("The attribute 'case' used in <plural> is greater"
			      " than the nplurals <messages> attribute.", 
			      x_plural->value());
	  visited[c] = true;
	  valueMap[id][c] = readElementContent(x_plural, buf);
	}

	for (unsigned i = 0; i < pluralCount_; i++)
	  if (!visited[i])
	    throw parse_error("Missing plural case in <message>", 
			      x_message->value());
      } else {
	valueMap[id] = std::vector<std::string>();
	valueMap[id].reserve(1);
	valueMap[id].push_back(readElementContent(x_message, buf));
      }
    }
  } catch (parse_error& e) {
    WApplication::instance()->log("error")
      << "Error reading " << fileName
      << ": at character " << e.where<char>() - text.get()
      << ": " << e.what();
  }

  return true;
}

}
#endif // WT_CNOR

