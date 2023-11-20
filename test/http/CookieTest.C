/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <web/WebRenderer.h>
#include <Wt/WApplication.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>
#include <Wt/Http/Cookie.h>
#include <Wt/Test/WTestEnvironment.h>

BOOST_AUTO_TEST_CASE( cookie_construction_test )
{
  Wt::Http::Cookie cookie("test");

  BOOST_TEST(cookie.name() == "test");
  BOOST_CHECK(cookie.value().empty());
  BOOST_CHECK(cookie.expires().isNull());
  BOOST_CHECK(cookie.domain().empty());
  BOOST_CHECK(cookie.path().empty());
  BOOST_TEST(cookie.secure() == false);
  BOOST_TEST(cookie.httpOnly() == true);
  BOOST_CHECK(cookie.sameSite() == Wt::Http::Cookie::SameSite::Lax);

  // session cookie
  cookie = Wt::Http::Cookie("test", "value");

  BOOST_TEST(cookie.name() == "test");
  BOOST_CHECK(cookie.value() == "value");
  BOOST_CHECK(cookie.expires().isNull());
  BOOST_CHECK(cookie.domain().empty());
  BOOST_CHECK(cookie.path().empty());
  BOOST_TEST(cookie.secure() == false);
  BOOST_TEST(cookie.httpOnly() == true);
  BOOST_CHECK(cookie.sameSite() == Wt::Http::Cookie::SameSite::Lax);

  // cookie with expiration
  cookie = Wt::Http::Cookie("test", "value", Wt::WDateTime(Wt::WDate(2022, 10, 12)));

  BOOST_TEST(cookie.name() == "test");
  BOOST_CHECK(cookie.value() == "value");
  BOOST_CHECK(cookie.expires() == Wt::WDateTime(Wt::WDate(2022, 10, 12)));
  BOOST_CHECK(cookie.domain().empty());
  BOOST_CHECK(cookie.path().empty());
  BOOST_TEST(cookie.secure() == false);
  BOOST_TEST(cookie.httpOnly() == true);
  BOOST_CHECK(cookie.sameSite() == Wt::Http::Cookie::SameSite::Lax);

  cookie = Wt::Http::Cookie("test", "value", std::chrono::minutes(10));
  auto expires = Wt::WDateTime::currentDateTime().addSecs(600);

  BOOST_TEST(cookie.name() == "test");
  BOOST_CHECK(cookie.value() == "value");
  BOOST_TEST(std::abs(cookie.expires().secsTo(expires)) <= 1);
  BOOST_CHECK(cookie.domain().empty());
  BOOST_CHECK(cookie.path().empty());
  BOOST_TEST(cookie.secure() == false);
  BOOST_TEST(cookie.httpOnly() == true);
  BOOST_CHECK(cookie.sameSite() == Wt::Http::Cookie::SameSite::Lax);
}

BOOST_AUTO_TEST_CASE( cookie_render_test )
{
  // Tests whether the cookie is rendered correctly with the right attributes.
  // This tests whether the defaults are correct, and if certain values (like Path)
  // are always present.

  Wt::Http::Cookie cookie("test", "value");
  Wt::Test::WTestEnvironment env;
  Wt::WebSession& session = *env.theSession_.get();

  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Path=/; httponly; SameSite=Lax;");
  cookie.setDomain("test.example.com");
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Domain=test.example.com; Path=/; httponly; SameSite=Lax;");
  cookie.setPath("/docs");
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Domain=test.example.com; Path=/docs; httponly; SameSite=Lax;");
  cookie.setSecure(true);
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Domain=test.example.com; Path=/docs; httponly; secure; SameSite=Lax;");
  cookie.setSameSite(Wt::Http::Cookie::SameSite::None);
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Domain=test.example.com; Path=/docs; httponly; secure; SameSite=None;");
  cookie.setSameSite(Wt::Http::Cookie::SameSite::Strict);
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Domain=test.example.com; Path=/docs; httponly; secure; SameSite=Strict;");
  cookie.setExpires(Wt::WDateTime(Wt::WDate(2023, 1, 1)));
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Expires=Sun, 01 Jan 2023 00:00:00 GMT; Domain=test.example.com; Path=/docs; httponly; secure; SameSite=Strict;");
  cookie.setExpires(Wt::WDateTime(Wt::WDate(2023, 1, 1), Wt::WTime(14, 41, 11)));
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Expires=Sun, 01 Jan 2023 14:41:11 GMT; Domain=test.example.com; Path=/docs; httponly; secure; SameSite=Strict;");
  // Test removal
  cookie.setValue("deleted");
  cookie.setMaxAge(std::chrono::seconds(0));
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=deleted; Version=1; Expires=Sun, 01 Jan 2023 14:41:11 GMT; Max-Age=0; Domain=test.example.com; Path=/docs; httponly; secure; SameSite=Strict;");

  cookie = Wt::Http::Cookie("test", "value", std::chrono::minutes(10));
  auto expires  = Wt::WDateTime::currentDateTime().addSecs(600);
  std::string expected = "test=value; Version=1; Max-Age=600; Path=/; httponly; SameSite=Lax;";
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == expected);
  // Test removal
  cookie.setValue("deleted");
  cookie.setMaxAge(std::chrono::seconds(0));
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=deleted; Version=1; Max-Age=0; Path=/; httponly; SameSite=Lax;");

  // Test "Path" for an application with a deployment path
  env.setDeploymentPath("/app/");
  Wt::WApplication app(env);
  cookie = Wt::Http::Cookie("test", "value");
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=value; Version=1; Path=/app/; httponly; SameSite=Lax;");
  // Test removal
  cookie.setValue("deleted");
  cookie.setMaxAge(std::chrono::seconds(0));
  BOOST_TEST(Wt::WebRenderer::renderCookieHttpHeader(cookie, session) == "test=deleted; Version=1; Max-Age=0; Path=/app/; httponly; SameSite=Lax;");
}
