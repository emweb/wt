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
#include "Wt/WLogger"
#include "Wt/WMessageResources"
#include "Wt/WString"
#include "Wt/WApplication"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

using namespace Wt;
using namespace rapidxml;

namespace {
  void encode_utf8(unsigned long code, char *&out)
  {
    if (code < 0x80) { // 1 byte sequence
      out[0] = static_cast<unsigned char>(code);
      out += 1;
    } else if (code < 0x800) {  // 2 byte sequence
      out[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
      out[0] = static_cast<unsigned char>(code | 0xC0);
      out += 2;
    } else if (code < 0x10000) { // 3 byte sequence
      out[2] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
      out[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
      out[0] = static_cast<unsigned char>(code | 0xE0);
      out += 3;
    } else if (code < 0x110000) { // 4 byte sequence
      out[3] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
      out[2] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
      out[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
      out[0] = static_cast<unsigned char>(code | 0xF0);
      out += 4;
    } else
      // impossible since UTF16 is already checked ?
      throw std::runtime_error("Invalid UTF-16 stream");
  }

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
  
  std::map<std::string, std::string>::const_iterator it;

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

  KeyValueMap::const_iterator j;

  j = local_.find(key);
  if (j != local_.end()) {
    result = j->second;
    return true;
  }

  j = defaults_.find(key);
  if (j != defaults_.end()) {
    result = j->second;
    return true;
  }

  return false;
}

bool WMessageResources::readResourceFile(const std::string& locale,
				         KeyValueMap& valueMap)
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
					   KeyValueMap& valueMap,
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

	  encode_utf8(cp, out);

	  firstWord = 0;
	} else if (ch >= 0xD800 && ch <= 0xDBFF) {
	  // first word of multi-word
	  firstWord = ch;
	} else {
	  // single-word
	  encode_utf8(ch, out);

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

    // factor 2 in case we expanded <span/> to <span></span>
    boost::scoped_array<char> buf(new char[length * 2]);

    for (xml_node<> *x_message = x_root->first_node("message");
	 x_message; x_message = x_message->next_sibling("message")) {
      if (std::strncmp(x_message->name(), "message", x_message->name_size()) != 0)
	throw parse_error("Expected <message>", x_message->value());

      xml_attribute<> *x_id = x_message->first_attribute("id");
      if (!x_id)
	throw parse_error("Missing message id", x_message->value());

      std::string id(x_id->value(), x_id->value_size());

      char *ptr = buf.get();

      for (xml_node<> *x_child = x_message->first_node();
	   x_child; x_child = x_child->next_sibling()) {
	fixSelfClosingTags(x_child);
	ptr = print(ptr, *x_child, print_no_indenting);
      }

      valueMap[id] = std::string(buf.get(), ptr - buf.get());
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

