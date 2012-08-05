/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Json/Parser>
#include <Wt/Json/Object>
#include <Wt/Json/Array>

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104100
#  define JSON_PARSER
#endif

#ifdef JSON_PARSER

#define JS(...) #__VA_ARGS__

using namespace Wt;

BOOST_AUTO_TEST_CASE( json_parse_empty_test )
{
  Json::Value result;
  Json::parse("{}", result);

  BOOST_REQUIRE(result.type() == Json::ObjectType);
  BOOST_REQUIRE(((const Json::Object&) result).empty());
}

BOOST_AUTO_TEST_CASE( json_parse_object1_test )
{
  Json::Object result;
  Json::parse("{ "
	      "  \"a\": \"That's great\", "
	      "  \"b\": true "
	      "}",
	      result);
  BOOST_REQUIRE(result.size() == 2);

  WString a = result.get("a");
  bool b = result.get("b");

  BOOST_REQUIRE(a == "That's great");
  BOOST_REQUIRE(b == true);
}

BOOST_AUTO_TEST_CASE( json_parse_object2_test )
{
  Json::Object result;
  Json::parse("{ \"a\": 5 }", result);

  BOOST_REQUIRE(result.size() == 1);

  int i = result.get("a");

  BOOST_REQUIRE(i == 5);
}

BOOST_AUTO_TEST_CASE( json_parse_strings_test )
{
  Json::Object result;
  Json::parse(JS({
	"s1": "simple",
	"s2": "escaped: \\ \t \n \b \r",
	"s3": "unicode: \u0194"
	  }), result);

  BOOST_REQUIRE(result.size() == 3);

  const WString& s1 = result.get("s1");
  BOOST_REQUIRE(s1 == "simple");
  const WString& s2 = result.get("s2");
  BOOST_REQUIRE(s2 == "escaped: \\ \t \n \b \r");
  const WString& s3 = result.get("s3");
  BOOST_REQUIRE(s3 == L"unicode: \x0194");
}

BOOST_AUTO_TEST_CASE( json_structure_test )
{
  Json::Object result;
  Json::parse(JS({
     "firstName": "John",
     "lastName": "Smith",
     "age": 25,
     "address":
     {
         "streetAddress": "21 2nd Street",
         "city": "New York",
         "state": "NY",
         "postalCode": "10021"
     },
     "phoneNumber":
     [
         {
           "type": "home",
           "number": "212 555-1234"
         },
         {
           "type": "fax",
           "number": "646 555-4567"
         }
     ]
	  }), result);

  BOOST_REQUIRE(result.size() == 5);

  const Json::Array& phoneNumbers = result.get("phoneNumber");
  BOOST_REQUIRE(phoneNumbers.size() == 2);

  const Json::Object& p1 = phoneNumbers[0];
  WString t1 = p1.get("type");
  WString n1 = p1.get("number");
  BOOST_REQUIRE(t1 == "home");
  BOOST_REQUIRE(n1 == "212 555-1234");
}

#endif // JSON_PARSER
