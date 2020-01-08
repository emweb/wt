/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/Test/WTestEnvironment.h>

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

  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./foo") == "/foo");
  BOOST_REQUIRE(app.resolveRelativeUrl("?a=b") == "/app?a=b");
  BOOST_REQUIRE(app.resolveRelativeUrl("bar/i") == "/bar/i");
}

BOOST_AUTO_TEST_CASE( urlmanip_test3 )
{
  UrlManipTestEnvironment env("/app/bla");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/app/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./foo") == "/app/foo");
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

BOOST_AUTO_TEST_CASE( urlmanip_test5 )
{
  UrlManipTestEnvironment env("/foo/bar");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/foo/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./") == "/foo/");
  BOOST_REQUIRE(app.resolveRelativeUrl("../") == "/foo/../");
  BOOST_REQUIRE(app.resolveRelativeUrl("./../") == "/foo/../");

  app.setInternalPath("/internal/path");
  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/foo/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./") == "/foo/");
  BOOST_REQUIRE(app.resolveRelativeUrl("../") == "/foo/../");
  BOOST_REQUIRE(app.resolveRelativeUrl("./../") == "/foo/../");
}

BOOST_AUTO_TEST_CASE( urlmanip_test6 )
{
  UrlManipTestEnvironment env("/foo/bar/");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/foo/bar/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./") == "/foo/bar/");
  BOOST_REQUIRE(app.resolveRelativeUrl("../") == "/foo/bar/../");
  BOOST_REQUIRE(app.resolveRelativeUrl("./../") == "/foo/bar/../");
  
  app.setInternalPath("/internal/path");
  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/foo/bar/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./") == "/foo/bar/");
  BOOST_REQUIRE(app.resolveRelativeUrl("../") == "/foo/bar/../");
  BOOST_REQUIRE(app.resolveRelativeUrl("./../") == "/foo/bar/../");
}

BOOST_AUTO_TEST_CASE( urlmanip_test7 )
{
  // Examples adapted from the examples of RFC 3986 section 5.4
  UrlManipTestEnvironment env("/b/c/d");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl("g:h") == "g:h");
  BOOST_REQUIRE(app.resolveRelativeUrl("g") == "/b/c/g");
  BOOST_REQUIRE(app.resolveRelativeUrl("./g") == "/b/c/g");
  BOOST_REQUIRE(app.resolveRelativeUrl("g/") == "/b/c/g/");
  BOOST_REQUIRE(app.resolveRelativeUrl("/g") == "/g");
  BOOST_REQUIRE(app.resolveRelativeUrl("//g") == "//g");
  BOOST_REQUIRE(app.resolveRelativeUrl("?y") == "/b/c/d?y");
  BOOST_REQUIRE(app.resolveRelativeUrl("g?y") == "/b/c/g?y");
  BOOST_REQUIRE(app.resolveRelativeUrl("#s") == "#s");
  BOOST_REQUIRE(app.resolveRelativeUrl("g#s") == "/b/c/g#s");
  BOOST_REQUIRE(app.resolveRelativeUrl("g?y#s") == "/b/c/g?y#s");
  BOOST_REQUIRE(app.resolveRelativeUrl("") == "/b/c/d");
  BOOST_REQUIRE(app.resolveRelativeUrl(".") == "/b/c/");
  BOOST_REQUIRE(app.resolveRelativeUrl("./") == "/b/c/");
  BOOST_REQUIRE(app.resolveRelativeUrl("..") == "/b/c/..");
  BOOST_REQUIRE(app.resolveRelativeUrl("../") == "/b/c/../");
  BOOST_REQUIRE(app.resolveRelativeUrl("../g") == "/b/c/../g");
  BOOST_REQUIRE(app.resolveRelativeUrl("../..") == "/b/c/../..");
  BOOST_REQUIRE(app.resolveRelativeUrl("../../") == "/b/c/../../");
  BOOST_REQUIRE(app.resolveRelativeUrl("../../g") == "/b/c/../../g");
}

BOOST_AUTO_TEST_CASE( urlmanip_test8 )
{
  UrlManipTestEnvironment env("/b/c/d");
  WApplication app(env);

  BOOST_REQUIRE(app.resolveRelativeUrl("?a=b") == "/b/c/d?a=b");
  BOOST_REQUIRE(app.resolveRelativeUrl(".?a=b") == "/b/c/?a=b");
}
