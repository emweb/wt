/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>

#include <Wt/Json/Parser.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Array.h>

#include <fstream>
#include <streambuf>

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

  BOOST_REQUIRE(result.type() == Json::Type::Object);
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

BOOST_AUTO_TEST_CASE( json_bad_test )
{
  bool caught = false;

  try {
    Json::Object result;
    Json::parse("{ \"Field1\": \"one\" \n \"Field2\" : \"two\" }", result);
  } catch (std::exception& e) {
    caught = true;
  }

  BOOST_REQUIRE(caught);
}

BOOST_AUTO_TEST_CASE( json_utf8_test )
{
  std::ifstream t("json/UTF-8-test.json", std::ios::in | std::ios::binary);
  BOOST_REQUIRE(t.good());
  std::string str((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

  Json::Object result;
  Json::parse(str, result);
  WString s1 = result.get("kosme");
  WString s2 = result.get("2 bytes (U-00000080)");
  WString s3 = result.get("3 bytes (U-00000800)");
  WString s4 = result.get("4 bytes (U-00010000)");
  WString s5 = result.get("1 byte  (U-0000007F)");
  WString s6 = result.get("2 bytes (U-000007FF)");
  WString s7 = result.get("3 bytes (U-0000FFFF)");
  WString s8 = result.get("U-0000D7FF = ed 9f bf");
  WString s9 = result.get("U-0000E000 = ee 80 80");
  WString s10 = result.get("U-0000FFFD = ef bf bd");
  WString s11 = result.get("U-0010FFFF = f4 8f bf bf");

  std::u32string u32s1 = s1.toUTF32();
  std::u32string u32s2 = s2.toUTF32();
  std::u32string u32s3 = s3.toUTF32();
  std::u32string u32s4 = s4.toUTF32();
  std::u32string u32s5 = s5.toUTF32();
  std::u32string u32s6 = s6.toUTF32();
  std::u32string u32s7 = s7.toUTF32();
  std::u32string u32s8 = s8.toUTF32();
  std::u32string u32s9 = s9.toUTF32();
  std::u32string u32s10 = s10.toUTF32();
  std::u32string u32s11 = s11.toUTF32();

  BOOST_REQUIRE(u32s1[0] == 954);
  BOOST_REQUIRE(u32s1[1] == 8057);
  BOOST_REQUIRE(u32s1[2] == 963);
  BOOST_REQUIRE(u32s1[3] == 956);
  BOOST_REQUIRE(u32s1[4] == 949);
  BOOST_REQUIRE(u32s2[0] == 128);
  BOOST_REQUIRE(u32s3[0] == 2048);
  BOOST_REQUIRE(u32s4[0] == 65533 || u32s4[0] == 65536);
  BOOST_REQUIRE(u32s5[0] == 127);
  BOOST_REQUIRE(u32s6[0] == 2047);
  BOOST_REQUIRE(u32s7[0] == 65535);
  BOOST_REQUIRE(u32s8[0] == 55295);
  BOOST_REQUIRE(u32s9[0] == 57344);
  BOOST_REQUIRE(u32s10[0] == 65533);
  BOOST_REQUIRE(u32s11[0] == '?'); // should this really be rejected?

  BOOST_REQUIRE(result.size() == 11);
}


#endif // JSON_PARSER
