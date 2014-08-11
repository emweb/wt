/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Json/Array"
#include "Wt/Json/Object"
#include "Wt/Json/Parser"
#include "Wt/Json/Value"
#include "Wt/WStringStream"

#include <boost/version.hpp>

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104100
#  define JSON_PARSER
#endif

#ifdef JSON_PARSER

#include "3rdparty/rapidxml/rapidxml.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#if BOOST_VERSION < 105600
#include <boost/spirit/home/phoenix/statement/throw.hpp>
#else
#include <boost/phoenix.hpp>
#endif
#include <boost/bind.hpp>

#endif // JSON_PARSER

namespace Wt {
  namespace Json {

ParseError::ParseError()
  : WException(std::string())
{ }

ParseError::ParseError(const std::string& message)
  : WException(message)
{ }

void ParseError::setError(const std::string& message)
{
  setMessage(message);
}

#ifdef JSON_PARSER

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::standard;
namespace phoenix = boost::phoenix;

template <typename Iterator>
struct json_grammar : public qi::grammar<Iterator, ascii::space_type>
{
  typedef json_grammar<Iterator> Self;

  json_grammar(Value& result)
    : json_grammar::base_type(root),
      result_(result)
  {
    create();

    state_.push_back(InObject);
    currentValue_ = &result_;
  }

  void create()
  {
    using qi::lit;
    using qi::double_;
    using qi::lexeme;
    using qi::no_case;
    using qi::raw;
    using qi::on_error;
    using qi::fail;
    using qi::uint_parser;
    using ascii::char_;
    using ascii::graph;

    using phoenix::construct;
    using phoenix::val;
    using phoenix::throw_;
        
    root
      = object | array;
    
    object
      =  lit('{')[boost::bind(&Self::startObject, this)]
      >> -(member % ',')
      >> lit('}')[boost::bind(&Self::endObject, this)]
      ;
                
    member
      = raw[string][boost::bind(&Self::setMemberName, this, _1)] 
      >> lit(':') 
      >> value
      ;
                
    array 
      = lit('[')[boost::bind(&Self::startArray, this)]
      >> -(value % ',')
      >> lit(']')[boost::bind(&Self::endArray, this)]
      ;

    value 
      = raw[string][boost::bind(&Self::setStringValue, this, _1)] 
      | double_[boost::bind(&Self::setNumberValue, this, _1)]
      | lit("true")[boost::bind(&Self::setTrueValue, this)]
      | lit("false")[boost::bind(&Self::setFalseValue, this)]
      | lit("null")[boost::bind(&Self::setNullValue, this)]
      | object
      | array
      ;

    string
      = lexeme['"' > *character > '"']
      ;

    character
      = (char_ - '\\' - '"')[boost::bind(&Self::addChar, this, _1)]
      | (lit('\\') > escape)
      ;

    escape
      = char_("\"\\/bfnrt")[boost::bind(&Self::addEscapedChar, this, _1)]
      | ('u' > uint_parser<unsigned long, 16, 4, 4>()
	 [boost::bind(&Self::addUnicodeChar, this, _1)])
      ;
  }

  qi::rule<Iterator, ascii::space_type>
    root, object, member, array, value, string;

  qi::rule<Iterator> character, escape;

  typedef boost::iterator_range<std::string::const_iterator> StrValue;

  void startObject()
  {
    refCurrent();

    *currentValue_ = Value(ObjectType);
    objectStack_.push_back(&((Object&) (*currentValue_)));
    state_.push_back(InObject);
  }

  void endObject()
  {
    state_.pop_back();
    objectStack_.pop_back();
  }

  void setMemberName(const StrValue& value)
  {
    assert(state() == InObject);

    currentValue_ = &((currentObject())[s_.str()] = Value::Null);
    s_.clear();
  }

  void startArray()
  {
    refCurrent();

    *currentValue_ = Value(ArrayType);
    arrayStack_.push_back(&((Array&) (*currentValue_)));
    state_.push_back(InArray);
  }

  void endArray()
  {
    state_.pop_back();
    arrayStack_.pop_back();
  }

  void setStringValue(const StrValue& value)
  {
    refCurrent();

    *currentValue_ = Value(WString::fromUTF8(s_.str()));
    s_.clear();
    currentValue_  = 0;
  }

  void setNumberValue(double d)
  {
    // FIXME save as long long or int too ?
    refCurrent();
    *currentValue_ = Value(d);
    currentValue_  = 0;
  }

  void addChar(char c)
  {
    s_ << c;
  }

  void addUnicodeChar(unsigned code)
  {
    char buf[4];
    char *end = buf;
    Wt::rapidxml::xml_document<>::insert_coded_character<0>(end, code);
    for (char *b = buf; b != end; ++b)
      s_ << *b;
  }

  void addEscapedChar(char c)
  { 
    switch (c) {
    case 'b': s_ << '\b'; break;
    case 'f': s_ << '\f'; break;
    case 'n': s_ << '\n'; break;
    case 'r': s_ << '\r'; break;
    case 't': s_ << '\t'; break;
    default:  s_ << c;
    }
  }

  void setTrueValue()
  {
    refCurrent();
    *currentValue_ = Value::True;
    currentValue_  = 0;
  }

  void setFalseValue()
  {
    refCurrent();
    *currentValue_ = Value::False;
    currentValue_  = 0;
  }

  void setNullValue()
  {
    refCurrent();
    *currentValue_ = Value::Null;
    currentValue_  = 0;
  }

private:
  Value& result_;
  Value *currentValue_;

  std::list<Object *> objectStack_;
  std::list<Array *> arrayStack_;

  enum State { InObject, InArray };

  std::vector<State> state_;

  WStringStream s_;

  State state() { return state_.back(); }
  Object& currentObject() { return *(objectStack_.back()); }
  Array& currentArray() { return *(arrayStack_.back()); }

  void refCurrent()
  {
    switch (state()) {
    case InObject:
      assert(currentValue_);
      break;
    case InArray:
      currentArray().push_back(Value());
      currentValue_ = &currentArray().back();
      break;
    }
  }
};

namespace {
  void parseJson(const std::string &str, Value& result, bool validateUTF8)
  {
    // security sanitization of input UTF-8
    std::string validated_string = str;
    if (validateUTF8)
      WString::checkUTF8Encoding(validated_string);

    json_grammar<std::string::const_iterator> g(result);

    std::string::const_iterator begin = validated_string.begin();
    std::string::const_iterator end = validated_string.end();
    bool success = qi::phrase_parse(begin, end, g, ascii::space);

    if (success) {
      if (begin != end)
	throw ParseError("Error parsing json: Expected end here:\""
			 + std::string(begin, end) + "\"");
    } else
      throw ParseError("Error parsing json: \"" + std::string(begin, end)
		       + "\"");
  }
}

#else
  void parseJson(const std::string &str, Value& result, bool validateUTF8)
  {
    throw ParseError("Wt::Json::parse requires boost version 1.41 or later");
  }  

#endif // JSON_PARSER

void parse(const std::string& input, Value& result, bool validateUTF8)
{
  parseJson(input, result, validateUTF8);
}

bool parse(const std::string& input, Value& result, ParseError& error, bool validateUTF8)
{
  try {
    parseJson(input, result, validateUTF8);
    return true;
  } catch (ParseError& e) {
    error.setError(e.what());
    return false;
  }
}

void parse(const std::string& input, Object& result, bool validateUTF8)
{
  Value value;

  parse(input, value, validateUTF8);

  Object& parsedObject = value;

  parsedObject.swap(result);
}

bool parse(const std::string& input, Object& result, ParseError& error, bool validateUTF8)
{
  try {
    parse(input, result, validateUTF8);
    return true;
  } catch (std::exception& e) {
    error.setError(e.what());
    return false;
  }
}

  }
}
