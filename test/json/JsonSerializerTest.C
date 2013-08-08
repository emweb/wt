/*
 * Copyright (C) 2013 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>

#include <Wt/Json/Parser>
#include <Wt/Json/Serializer>
#include <Wt/Json/Object>
#include <Wt/Json/Array>

#include <fstream>
#include <streambuf>

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
	      "  \"fifth\" : 2.7182818,"
	      "  \"sixth\" : 1.54e99,"
	      "  \"seventh\" : 9.87E88,"
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
	      "  3.141592,"
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

#endif // JSON_PARSER
