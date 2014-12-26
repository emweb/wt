/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CNOR
#include <fstream>
#include <cstring>

#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>

#include "Wt/WLocale"
#include "Wt/WLogger"
#include "Wt/WMessageResources"
#include "Wt/WStringStream"

#include "DomElement.h"

#include "3rdparty/rapidxml/rapidxml.hpp"
#include "3rdparty/rapidxml/rapidxml_print.hpp"

using namespace Wt;
using namespace Wt::rapidxml;

#ifndef WT_NO_SPIRIT

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#include <boost/spirit.hpp>
#include <boost/spirit/phoenix/binders.hpp>
#else
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/phoenix1_binders.hpp>
#endif

#include <boost/bind.hpp>

namespace {

#if BOOST_VERSION < 103600
  using namespace boost::spirit;
#else
  using namespace boost::spirit::classic;
#endif
  using namespace boost;

struct CExpressionParser : grammar<CExpressionParser>
{
  struct ParseState
  {
    bool condition_;
  };

  CExpressionParser(::int64_t n, int &result, ParseState &state) : 
    n_(n),
    result_(result),
    state_(state)
  {}

  struct value_closure : closure<value_closure, ::int64_t, ::int64_t>
  {
    member1 value;
    member2 condition;
  };

  template <typename ScannerT>
  struct definition
  {
    definition(CExpressionParser const& self)
    {
      using namespace boost::spirit;
      using namespace phoenix;

      group
        = '('
          >> expression[group.value = arg1]
          >> ')'
        ;

       // A statement can end at the end of the line, or with a semicolon.
      statement
        =   ( expression[bind(&CExpressionParser::set_result)(self, arg1)]
          )
        >> (end_p | ';')
        ;

      literal
        = uint_p[literal.value = arg1]
        ;

      factor
        = literal[factor.value = arg1]
        | group[factor.value = arg1]
        | ch_p('n')[factor.value = bind(&CExpressionParser::get_n)(self)]
        ;

      term
        = factor[term.value = arg1]
          >> *( ('*' >> factor[term.value *= arg1])
              | ('/' >> factor[term.value /= arg1])
	      | ('%' >> factor[term.value %= arg1])
            )
        ;

      additive_expression
        = term[additive_expression.value = arg1]
          >> *( ('+' >> term[additive_expression.value += arg1])
              | ('-' >> term[additive_expression.value -= arg1])
            )
        ;

      expression
	= or_expression[expression.value = arg1]
	               [expression.condition = arg1] 
	>> !( '?' 
	      >> expression[bind(&CExpressionParser::set_cond)
			    (self, expression.condition)]
	                   [bind(&CExpressionParser::ternary_op)
			    (self, expression.value, arg1)] 
	      >> ':' 
	      >> expression[bind(&CExpressionParser::set_not_cond)
			    (self, expression.condition)]
	                   [bind(&CExpressionParser::ternary_op)
			    (self, expression.value, arg1)]
			    )
	;

      or_expression
        = and_expression[or_expression.value = arg1]
	  >> *( "||" >> and_expression[bind(&CExpressionParser::or_op)
				       (self, or_expression.value, arg1)] )
        ;
      
      and_expression
        = eq_expression[and_expression.value = arg1]
          >> *( "&&" >> eq_expression[bind(&CExpressionParser::and_op)
				       (self, and_expression.value, arg1)] )
        ;

      eq_expression
        = relational_expression[eq_expression.value = arg1]
          >> *( ("==" >> relational_expression[bind(&CExpressionParser::eq_op)
					       (self, 
						eq_expression.value, 
						arg1)])
	      | ("!=" >> relational_expression[bind(&CExpressionParser::neq_op)
					       (self, 
						eq_expression.value, 
						arg1)])
            )
        ;
	
      relational_expression
        = additive_expression[relational_expression.value = arg1]
          >> *( (">" >> additive_expression[bind(&CExpressionParser::gt_op)
					    (self, 
					     relational_expression.value, 
					     arg1)])
	      | (">=" >> additive_expression[bind(&CExpressionParser::gte_op)
					     (self, 
					      relational_expression.value, 
					      arg1)])
	      | ("<" >> additive_expression[bind(&CExpressionParser::lt_op)
					    (self, 
					     relational_expression.value, 
					     arg1)])
	      | ("<=" >> additive_expression[bind(&CExpressionParser::lte_op)
					     (self, 
					      relational_expression.value, 
					      arg1)])
            )
        ;
    }

    rule<ScannerT> const&
    start() const { return statement; }

    rule<ScannerT> statement;
    rule<ScannerT, value_closure::context_t> expression, factor,
      group, literal, term, additive_expression, or_expression, and_expression,
      eq_expression, relational_expression;
  };

private: 
  ::int64_t get_n() const { return n_; }
  
  void eq_op(::int64_t &x, ::int64_t y) const { x = x == y; }
  void neq_op(::int64_t &x, ::int64_t y) const { x = x != y; }
  
  void lt_op(::int64_t &x, ::int64_t y) const { x = x < y;}
  void gt_op(::int64_t &x, ::int64_t y) const { x = x > y;}
  void lte_op(::int64_t &x, ::int64_t y) const { x = x <= y;}
  void gte_op(::int64_t &x, ::int64_t y) const { x = x >= y;}

  void ternary_op(::int64_t &result, ::int64_t y) const 
  { 
    if (state_.condition_) 
      result = y; 
  } 

  void set_cond(::int64_t condition) const 
  { 
    state_.condition_ = condition;
  } 

  void set_not_cond(::int64_t condition) const 
  { 
    state_.condition_ = !condition;
  } 
  
  void or_op(::int64_t &x, ::int64_t y) const { x = x || y; }
  void and_op(::int64_t &x, ::int64_t y) const { x = x && y; }

  void set_result(int result) const { result_ = result; }
private :
  ::int64_t n_;
  int &result_;
  ParseState &state_;

public :
  int result() { return result_; }
};

} // anonymous namespace

#endif //WT_NO_SPIRIT

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

  char *copy_chars(const char *begin, const char *end, char *out)
  {
    while (begin != end)
      *out++ = *begin++;
    return out;
  }

  std::string readElementContent(xml_node<> *x_parent, 
				 boost::scoped_array<char> &buf) 
  {
    char *ptr = buf.get();

    if (x_parent->type() == node_cdata) {
      return std::string(x_parent->value(), x_parent->value_size());
    } else {
      for (xml_node<> *x_child = x_parent->first_node();
	   x_child; x_child = x_child->next_sibling()) {
	if (x_child->type() == node_cdata) {
	  ptr = copy_chars(x_child->value(),
			   x_child->value() + x_child->value_size(), ptr);
	} else {
	  fixSelfClosingTags(x_child);
	  ptr = print(ptr, *x_child, print_no_indenting);
	}
      }

      return std::string(buf.get(), ptr - buf.get());
    }
  }

  int attributeValueToInt(xml_attribute<> *x_attribute)
  {
    return boost::lexical_cast<int>(std::string(x_attribute->value(), 
						x_attribute->value_size()));
  }
}

namespace Wt {

LOGGER("WMessageResources");

WMessageResources::WMessageResources(const std::string& path,
				     bool loadInMemory)
  : loadInMemory_(loadInMemory),
    loaded_(false),
    path_(path),
    builtin_(0)
{ }

WMessageResources::WMessageResources(const char *builtin)
  : loadInMemory_(true),
    loaded_(false),
    path_(""),
    builtin_(builtin)
{
  std::istringstream s(builtin,  std::ios::in | std::ios::binary);
  readResourceStream(s, defaults_, "<internal resource bundle>");
  loaded_ = true;
}

std::set<std::string> 
WMessageResources::keys(WFlags<WMessageResourceBundle::Scope> scope) const
{
  std::set<std::string> keys;
  
  KeyValuesMap::const_iterator it;

  if (scope & WMessageResourceBundle::Local)
    for (it = local_.map_.begin() ; it != local_.map_.end(); it++)
      keys.insert((*it).first);

  if (scope & WMessageResourceBundle::Default)
    for (it = defaults_.map_.begin() ; it != defaults_.map_.end(); it++)
      keys.insert((*it).first);

  return keys;
}

void WMessageResources::refresh()
{
  if (!path_.empty()) {
    defaults_.map_.clear();
    readResourceFile("", defaults_);

    local_.map_.clear();
    std::string locale = WLocale::currentLocale().name();

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
    defaults_.map_.clear();
    local_.map_.clear();
    loaded_ = false;
  }
}

bool WMessageResources::resolveKey(const std::string& key, std::string& result)
{
  if (!loaded_)
    refresh();

  KeyValuesMap::const_iterator j;

  j = local_.map_.find(key);
  if (j != local_.map_.end()) {
    if (j->second.size() > 1 )
      return false;
    result = j->second[0];
    return true;
  }

  j = defaults_.map_.find(key);
  if (j != defaults_.map_.end()) {
    if (j->second.size() > 1 )
      return false;
    result = j->second[0];
    return true;
  }

  return false;
}

std::string WMessageResources::findCase(const std::vector<std::string> &cases, 
					std::string pluralExpression,
					::uint64_t amount)
{
#ifdef WT_NO_SPIRIT
  throw WException("WString::trn() requires the spirit library.");
#else
  int c = evalPluralCase(pluralExpression, amount);

  if (c > (int)cases.size() - 1 || c < 0) {
    WStringStream error;
    error << "Expression '" << pluralExpression << "' evaluates to '" 
	  << c << "' for n=" << boost::lexical_cast<std::string>(amount);
    
    if (c < 0) 
      error << " and values smaller than 0 are not allowed.";
    else
      error << " which is greater than the list of cases (size=" 
	    << (int)cases.size() << ").";
    
    throw WException(error.c_str());
  }

  return cases[c];
#endif // WT_NO_SPIRIT
}

bool WMessageResources::resolvePluralKey(const std::string& key, 
					 std::string& result, 
					 ::uint64_t amount)
{
  if (!loaded_)
    refresh();

  KeyValuesMap::const_iterator j;

  j = local_.map_.find(key);
  if (j != local_.map_.end()) {
    if (j->second.size() != local_.pluralCount_ )
      return false;
    result = findCase(j->second, local_.pluralExpression_, amount);
    return true;
  }

  j = defaults_.map_.find(key);
  if (j != defaults_.map_.end()) {
    if (j->second.size() != defaults_.pluralCount_)
      return false;
    result = findCase(j->second, defaults_.pluralExpression_, amount);
    return true;
  }

  return false;
}

bool WMessageResources::readResourceFile(const std::string& locale,
				         Resource& resource)
{
  if (!path_.empty()) {
    std::string fileName
      = path_ + (locale.length() > 0 ? "_" : "") + locale + ".xml";

    std::ifstream s(fileName.c_str(), std::ios::binary);
    return readResourceStream(s, resource, fileName);
  } else {
    return false;
  }
}

bool WMessageResources::readResourceStream(std::istream &s,
					   Resource& resource,
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
      s.seekg(0, std::ios::beg);
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

	  Wt::rapidxml::xml_document<>::insert_coded_character<0>(out, cp);

	  firstWord = 0;
	} else if (ch >= 0xD800 && ch <= 0xDBFF) {
	  // first word of multi-word
	  firstWord = ch;
	} else {
	  // single-word
	  Wt::rapidxml::xml_document<>::insert_coded_character<0>(out, ch);

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
      resource.pluralCount_ = attributeValueToInt(x_nplurals);
      resource.pluralExpression_ 
	= std::string(x_plural->value(), x_plural->value_size());
    } else {
      resource.pluralCount_ = 0;
    }

    // factor 2 in case we expanded <span/> to <span></span>
    boost::scoped_array<char> buf(new char[length * 2]);

    for (xml_node<> *x_message = x_root->first_node("message");
	 x_message; x_message = x_message->next_sibling("message")) {
      xml_attribute<> *x_id = x_message->first_attribute("id");
      if (!x_id)
	throw parse_error("Missing message id", x_message->value());

      std::string id(x_id->value(), x_id->value_size());

      xml_node<> *x_plural = x_message->first_node("plural");
      if (x_plural) {
	if (resource.pluralCount_ == 0)
	  throw parse_error("Expected 'nplurals' attribute in <message>",
			    x_plural->value());

	resource.map_[id] = std::vector<std::string>();
	resource.map_[id].reserve(resource.pluralCount_);
	
	std::vector<bool> visited;
	visited.reserve(resource.pluralCount_);
	
	for (unsigned i = 0; i < resource.pluralCount_; i++) {
	  resource.map_[id].push_back(std::string());
	  visited.push_back(false);
	}
	
	for (; x_plural; x_plural = x_plural->next_sibling("plural")) {
	  xml_attribute<> *x_case = x_plural->first_attribute("case");
	  int c = attributeValueToInt(x_case);
	  if (c >= (int)resource.pluralCount_)
	    throw parse_error("The attribute 'case' used in <plural> is greater"
			      " than the nplurals <messages> attribute.", 
			      x_plural->value());
	  visited[c] = true;
	  resource.map_[id][c] = readElementContent(x_plural, buf);
	}

	for (unsigned i = 0; i < resource.pluralCount_; i++)
	  if (!visited[i])
	    throw parse_error("Missing plural case in <message>", 
			      x_message->value());
      } else {
	resource.map_[id] = std::vector<std::string>();
	resource.map_[id].reserve(1);
	resource.map_[id].push_back(readElementContent(x_message, buf));
      }
    }
  } catch (parse_error& e) {
    LOG_ERROR("Error reading " << fileName
	      << ": at character " << (int)(e.where<char>() - text.get())
	      << ": " << e.what());
  }

  return true;
}

int WMessageResources::evalPluralCase(const std::string &expression, ::uint64_t n)
{
  int result;

#ifndef WT_NO_SPIRIT
  CExpressionParser::ParseState state;
  CExpressionParser p(n, result, state);
  std::string tmp = expression;
  parse(tmp.begin(), tmp.end(), p, space_p);
#endif // WT_NO_SPIRIT

  return result;
}

}
#endif // WT_CNOR

