/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/WebController.h"

namespace {
  std::string sessionFromCookie(const char *cookies,
                                const std::string &scriptName,
                                int sessionIdLength)
  {
    return Wt::WebController::sessionFromCookie(cookies, scriptName, sessionIdLength);
  }

  bool isEitherOf(const std::string &input,
                  const std::string &one,
                  const std::string &two) {
    return input == one || input == two;
  }
}

BOOST_AUTO_TEST_CASE( SessionFromCookieTest )
{
  // nullptr as input
  BOOST_REQUIRE(sessionFromCookie(nullptr, "/", 16) == "");
  BOOST_REQUIRE(sessionFromCookie(nullptr, "", 10) == "");

  // wrong length
  BOOST_REQUIRE(sessionFromCookie("abc=def", "abc", 14) == "");
  BOOST_REQUIRE(sessionFromCookie("abc=def", "abc", 2) == "");

  // valid cookie headers
  BOOST_REQUIRE(sessionFromCookie("abc=def", "abc", 3) == "def");
  BOOST_REQUIRE(sessionFromCookie("%2f=def", "/", 3) == "def");
  BOOST_REQUIRE(sessionFromCookie("abc=opq; %2f=def", "/", 3) == "def");
  BOOST_REQUIRE(sessionFromCookie("      %2f=def  ", "/", 3) == "def");

  // valid cookie headers with double quote
  BOOST_REQUIRE(sessionFromCookie("abc=\"def\"", "abc", 3) == "def");
  BOOST_REQUIRE(sessionFromCookie("%2f=\"def\"", "/", 3) == "def");
  BOOST_REQUIRE(sessionFromCookie("abc=\"opq\"; %2f=\"def\"", "/", 3) == "def");
  BOOST_REQUIRE(sessionFromCookie("      %2f=\"def\"  ", "/", 3) == "def");

  // Malformed cookies, result is not so important as long as it doesn't crash,
  // or generate memory issues
  BOOST_REQUIRE(isEitherOf(sessionFromCookie("junk; %2f=def", "/", 3), "", "def"));
  BOOST_REQUIRE(isEitherOf(sessionFromCookie("abc=opq;%2f=def", "/", 3), "", "def"));
  BOOST_REQUIRE(sessionFromCookie(";=", "/", 3) == "");
  BOOST_REQUIRE(sessionFromCookie(";%2f=", "/", 3) == "");
  BOOST_REQUIRE(isEitherOf(sessionFromCookie("; ms%2f=abcdef", "ms/", 6), "", "abcdef"));
  BOOST_REQUIRE(isEitherOf(sessionFromCookie("=; ms%2f=abcdef", "ms/", 6), "", "abcdef"));

  // Unbalanced double quote
  BOOST_REQUIRE(sessionFromCookie("abc=\"def", "abc", 3) == "");
  BOOST_REQUIRE(sessionFromCookie("%2f=def\"", "/", 3) == "");
  BOOST_REQUIRE(sessionFromCookie("abc=\"opq; %2f=def\"", "/", 3) == "");
  BOOST_REQUIRE(sessionFromCookie("      %2f=def\"  ", "/", 3) == "");
}
