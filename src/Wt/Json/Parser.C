/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Json/Array.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Parser.h"
#include "Wt/Json/Value.h"
#include "Wt/WStringStream.h"

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

namespace {

static constexpr int MAX_RECURSION_DEPTH = 1000;

}

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
      result_(result),
      recursionDepth_(0)
  {
    create();

    state_.push_back(State::InObject);
    currentValue_ = &result_;
  }

  typedef boost::iterator_range<std::string::const_iterator> StrValue;

  void startObject(bool &pass)
  {
    refCurrent();

    *currentValue_ = Value(Type::Object);
    objectStack_.push_back(&((Object&)(*currentValue_)));
    state_.push_back(State::InObject);

    ++recursionDepth_;

    pass = recursionDepth_ <= MAX_RECURSION_DEPTH;
  }

  void startArray(bool &pass)
  {
    refCurrent();

    *currentValue_ = Value(Type::Array);
    arrayStack_.push_back(&((Array&)(*currentValue_)));
    state_.push_back(State::InArray);

    ++recursionDepth_;

    pass = recursionDepth_ <= MAX_RECURSION_DEPTH;
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

    const auto endObject = [this](){
      state_.pop_back();
      objectStack_.pop_back();

      --recursionDepth_;
    };
    
    object
      =  lit('{')[boost::bind(&Self::startObject, this, _3)]
      >> -(member % ',')
      >> lit('}')[endObject]
      ;

    const auto setMemberName = [this](const StrValue &value)
    {
      assert(state() == State::InObject);

      currentValue_ = &((currentObject())[s_.str()] = Value::Null);
      s_.clear();
    };
                
    member
      = raw[string][setMemberName]
      >> lit(':') 
      >> value
      ;

    const auto endArray = [this]()
    {
      state_.pop_back();
      arrayStack_.pop_back();

      --recursionDepth_;
    };
                
    array 
      = lit('[')[boost::bind(&Self::startArray, this, _3)]
      >> -(value % ',')
      >> lit(']')[endArray]
      ;

    const auto setStringValue = [this](const StrValue &value)
    {
      refCurrent();

      *currentValue_ = Value(WString::fromUTF8(s_.str()));
      s_.clear();
      currentValue_ = nullptr;
    };

    const auto setNumberValue = [this](double d)
    {
      // FIXME save as long long or int too ?
      refCurrent();
      *currentValue_ = Value(d);
      currentValue_ = nullptr;
    };

    const auto setTrueValue = [this]()
    {
      refCurrent();
      *currentValue_ = Value::True;
      currentValue_ = nullptr;
    };

    const auto setFalseValue = [this]()
    {
      refCurrent();
      *currentValue_ = Value::False;
      currentValue_ = nullptr;
    };

    const auto setNullValue = [this]()
    {
      refCurrent();
      *currentValue_ = Value::Null;
      currentValue_ = nullptr;
    };

    value 
      = raw[string][setStringValue]
      | double_[setNumberValue]
      | lit("true")[setTrueValue]
      | lit("false")[setFalseValue]
      | lit("null")[setNullValue]
      | object
      | array
      ;

    string
      = lexeme['"' > *character > '"']
      ;

    const auto addChar = [this](char c)
    {
      s_ << c;
    };

    character
      = (char_ - '\\' - '"')[addChar]
      | (lit('\\') > escape)
      ;

    const auto addEscapedChar = [this](char c)
    {
      switch (c) {
      case 'b': s_ << '\b'; break;
      case 'f': s_ << '\f'; break;
      case 'n': s_ << '\n'; break;
      case 'r': s_ << '\r'; break;
      case 't': s_ << '\t'; break;
      default:  s_ << c;
      }
    };

    const auto addUnicodeChar = [this](unsigned long code)
    {
      char buf[4];
      char *end = buf;
      Wt::rapidxml::xml_document<>::insert_coded_character<0>(end, code);
      for (char *b = buf; b != end; ++b)
        s_ << *b;
    };

    escape
      = char_("\"\\/bfnrt")[addEscapedChar]
      | ('u' > uint_parser<unsigned long, 16, 4, 4>()
         [addUnicodeChar])
      ;
  }

  qi::rule<Iterator, ascii::space_type>
    root, object, member, array, value, string;

  qi::rule<Iterator> character, escape;

private:
  Value& result_;
  Value *currentValue_;
  int recursionDepth_;

  std::list<Object *> objectStack_;
  std::list<Array *> arrayStack_;

  enum class State { InObject, InArray };

  std::vector<State> state_;

  WStringStream s_;

  State state() { return state_.back(); }
  Object& currentObject() { return *(objectStack_.back()); }
  Array& currentArray() { return *(arrayStack_.back()); }

  void refCurrent()
  {
    switch (state()) {
    case State::InObject:
      assert(currentValue_);
      break;
    case State::InArray:
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
    bool success = false;

    try {
      success = qi::phrase_parse(begin, end, g, ascii::space);
    } catch (const std::exception &e) {
      throw ParseError(e.what());
    }

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
  } catch (const ParseError& e) {
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
  } catch (const ParseError& e) {
    error.setError(e.what());
    return false;
  }
}

void parse(const std::string& input, Array& result, bool validateUTF8)
{
  Value value;

  parse(input, value, validateUTF8);

  Array& parsedObject = value;

  parsedObject.swap(result);
}

bool parse(const std::string& input, Array& result, ParseError& error, bool validateUTF8)
{
  try {
    parse(input, result, validateUTF8);
    return true;
  } catch (const ParseError& e) {
    error.setError(e.what());
    return false;
  }
}


  }
}
