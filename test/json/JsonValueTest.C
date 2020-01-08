/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>
#include <iostream>

#include <Wt/Json/Parser.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Array.h>



#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104100
#  define JSON_PARSER
#endif

#ifdef JSON_PARSER

#define JS(...) #__VA_ARGS__

using namespace Wt;

BOOST_AUTO_TEST_CASE( json_compare_string_test )
{
  Json::Value v1 = Json::Value("value1");
  Json::Value v2 = Json::Value("value2");

  BOOST_REQUIRE(v1 != v2);
}

BOOST_AUTO_TEST_CASE( json_compare_int_test )
{
  Json::Value v1 = Json::Value(1);
  Json::Value v2 = Json::Value(2);

  BOOST_REQUIRE(v1 != v2);
}


#endif // JSON_PARSER
