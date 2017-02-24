/*
 * Copyright (C) 2017 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication>
#include <Wt/Test/WTestEnvironment>

using namespace Wt;
using namespace Wt::Test;

class UrlManipTestEnvironment : public WTestEnvironment {
public:
  UrlManipTestEnvironment(const std::string &publicDeploymentPath)
    : WTestEnvironment()
  {
    publicDeploymentPath_ = publicDeploymentPath;
  }
};

BOOST_AUTO_TEST_CASE( urlmanip_test1 )
{
  UrlManipTestEnvironment env("/");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./foo") == "/foo");
  BOOST_REQUIRE(app.resolveRelativeUrl("?a=b") == "/?a=b");
  BOOST_REQUIRE(app.resolveRelativeUrl("bar/i") == "/bar/i");
}

BOOST_AUTO_TEST_CASE( urlmanip_test2 )
{
  UrlManipTestEnvironment env("/app");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/app");
  BOOST_REQUIRE(app.resolveRelativeUrl("./foo") == "/app/foo");
  BOOST_REQUIRE(app.resolveRelativeUrl("?a=b") == "/app?a=b");
  BOOST_REQUIRE(app.resolveRelativeUrl("bar/i") == "/bar/i");
}

BOOST_AUTO_TEST_CASE( urlmanip_test3 )
{
  UrlManipTestEnvironment env("/app/bla");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/app/bla");
  BOOST_REQUIRE(app.resolveRelativeUrl("./foo") == "/app/bla/foo");
  BOOST_REQUIRE(app.resolveRelativeUrl("?a=b") == "/app/bla?a=b");
  BOOST_REQUIRE(app.resolveRelativeUrl("bar/i") == "/app/bar/i");
}

BOOST_AUTO_TEST_CASE( urlmanip_test4 )
{
  UrlManipTestEnvironment env("/app/");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/app/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./foo") == "/app/foo");
  BOOST_REQUIRE(app.resolveRelativeUrl("?a=b") == "/app/?a=b");
  BOOST_REQUIRE(app.resolveRelativeUrl("bar/i") == "/app/bar/i");
}
