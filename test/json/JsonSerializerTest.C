/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>

#include <Wt/Json/Parser.h>
#include <Wt/Json/Serializer.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Array.h>

#include <fstream>
#include <streambuf>
#include <iostream>

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104100
#  define JSON_PARSER
#endif

#ifdef JSON_PARSER

using namespace Wt;

BOOST_AUTO_TEST_CASE( json_generate_object )
{
  Json::Value initial;
  Json::parse("{"
	      "  \"first\" : 1,"
	      "  \"second\" : true,"
	      "  \"third\" : null,"
	      "  \"fourth\" : false,"
	      "  \"fifth\" : 1.25,"
#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
              // We lose precision in earlier versions of boost
	      "  \"sixth\" : 2.418980221897202e90,"
	      "  \"seventh\" : 2.713877091499598e75,"
#endif
	      "  \"eight\" : \"a string type value\","
	      "  \"ninth\" : {"
	      "    \"sub-first\" : 1,"
	      "    \"sub-second\" : 2"
	      "  },"
	      "  \"tenth\" : ["
	      "    true,"
	      "    false,"
	      "    null,"
	      "    666"
	      "  ]"
	      "}"
	      ,
  	      initial);
  
  Json::Object obj = initial;
  std::string generated = Json::serialize(obj);
  
  Json::Value reconstructed;
  Json::parse(generated, reconstructed);

  BOOST_REQUIRE(initial == reconstructed);
}

BOOST_AUTO_TEST_CASE( json_generate_array )
{
  Json::Value initial;
  Json::parse("["
	      "  \"string1\","
	      "  \"string2 (after string1)\","
	      "  true,"
	      "  false,"
	      "  null,"
	      "  10,"
	      "  -3.141592,"
	      "  0.0000369,"
	      "  1.23e4,"
	      "  {"
	      "    \"first\" : \"it works\","
	      "    \"second\" : true"
	      "  },"
	      "  ["
	      "    10,"
	      "    20,"
	      "    30"
	      "  ]"
	      "]"
	      ,
  	      initial);

  Json::Array arr = initial;
  std::string generated = Json::serialize(arr);
  
  Json::Value reconstructed;
  Json::parse(generated, reconstructed);

  BOOST_REQUIRE(initial == reconstructed);
}

BOOST_AUTO_TEST_CASE( json_generate_UTF8 )
{
  std::ifstream t("json/UTF-8-test2.json", std::ios::in | std::ios::binary);
  BOOST_REQUIRE(t.good());
  std::string str((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

  Json::Object initial;
  Json::parse(str, initial);

  std::string generated = Json::serialize(initial);

  Json::Object reconstructed;
  Json::parse(generated, reconstructed);

  BOOST_REQUIRE(initial == reconstructed);
}

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
// We lose precision in earlier versions of boost
BOOST_AUTO_TEST_CASE( json_test_double_dim )
{
  Json::Value v(2.0487042606859837E-309);  
  Json::Object obj;
  obj["test"] = v;
  std::string generated = Json::serialize(obj);

  Json::Object reconstructed;
  Json::parse(generated, reconstructed);

  BOOST_REQUIRE(obj == reconstructed);
}
#endif

BOOST_AUTO_TEST_CASE( json_test_nan )
{
  Json::Value v(std::numeric_limits<double>::quiet_NaN());
  Json::Object obj;
  obj["test"] = v;
  std::string generated = Json::serialize(obj);

  Json::Object reconstructed;
  Json::parse(generated, reconstructed);

  Json::Object obj2;
  obj2["test"] = Json::Value::Null;

  BOOST_REQUIRE(obj2 == reconstructed);
}

BOOST_AUTO_TEST_CASE( json_test_infinity )
{
  Json::Value v(std::numeric_limits<double>::infinity());
  Json::Object obj;
  obj["test"] = v;
  std::string generated = Json::serialize(obj);

  Json::Object reconstructed;
  Json::parse(generated, reconstructed);

  Json::Object obj2;
  obj2["test"] = Json::Value::Null;

  BOOST_REQUIRE(obj2 == reconstructed);
}

BOOST_AUTO_TEST_CASE( json_test_negative_infinity )
{
  Json::Value v(-std::numeric_limits<double>::infinity());
  Json::Object obj;
  obj["test"] = v;
  std::string generated = Json::serialize(obj);

  Json::Object reconstructed;
  Json::parse(generated, reconstructed);

  Json::Object obj2;
  obj2["test"] = Json::Value::Null;

  BOOST_REQUIRE(obj2 == reconstructed);
}

#endif // JSON_PARSER
