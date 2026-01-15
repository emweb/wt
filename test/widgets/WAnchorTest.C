/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>

#include <web/DomElement.h>

#include <Wt/WAnchor.h>
#include <Wt/WLink.h>

BOOST_AUTO_TEST_CASE( WAnchor_external_link_render )
{
  // Tests whether an external URL is correctly rendered
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink("https://www.emweb.be"));
  auto anchorPtr = anchor.get();
  app.root()->addWidget(std::move(anchor));

  // Simulate UI update (and calls updateDom)
  auto domElement = anchorPtr->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("href") == "https://www.emweb.be");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WAnchor_internal_path_link_render )
{
  // Test whether an internal path link is correctly rendered
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath, "/app"));
  auto anchorPtr = anchor.get();
  app.root()->addWidget(std::move(anchor));

  // Simulate UI update (and calls updateDom)
  auto domElement = anchorPtr->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("href") == "app");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WAnchor_internal_path_same_page_link_render )
{
  // Tests whether a page anchor is correctly rendered
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath, "#app"));
  auto anchorPtr = anchor.get();
  app.root()->addWidget(std::move(anchor));

  // Simulate UI update (and calls updateDom)
  auto domElement = anchorPtr->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("href") == "app");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WAnchor_internal_path_same_page_slash_link_render )
{
  // Tests whether a "relative" page anchor is correctly rendered
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath, "/#app"));
  auto anchorPtr = anchor.get();
  app.root()->addWidget(std::move(anchor));

  // Simulate UI update (and calls updateDom)
  auto domElement = anchorPtr->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("href") == "#app");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WAnchor_external_link_with_parameters_render )
{
  // Tests whether an external URL with parameters is correctly rendered
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink("https://www.emweb.be?param1=1&param2=2"));
  auto anchorPtr = anchor.get();
  app.root()->addWidget(std::move(anchor));

  // Simulate UI update (and calls updateDom)
  auto domElement = anchorPtr->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("href") == "https://www.emweb.be?param1=1&param2=2");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WAnchor_internal_path_link_with_parameters_render )
{
  // Test whether an internal path link with parameters is correctly rendered
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath, "/app?param1=1&param2=2"));
  auto anchorPtr = anchor.get();
  app.root()->addWidget(std::move(anchor));

  // Simulate UI update (and calls updateDom)
  auto domElement = anchorPtr->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("href") == "app%3fparam1%3d1%26param2%3d2");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WAnchor_external_link_with_parameters_and_session_id_render )
{
  // Test whether an external URL with parameters is correctly rendered with session ID in the URL
  Wt::Test::WTestEnvironment environment;
  environment.setSessionIdInUrl(true);
  Wt::WApplication app(environment);

  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink("https://www.emweb.be?param1=1&param2=2"));
  auto anchorPtr = anchor.get();
  app.root()->addWidget(std::move(anchor));

  // Simulate UI update (and calls updateDom)
  auto domElement = anchorPtr->createSDomElement(&app);

  // This "&" will be handled by the RefEncoder
  BOOST_TEST(domElement->getAttribute("href") == "?request=redirect&url=https%3a%2f%2fwww.emweb.be%3fparam1%3d1%26param2%3d2&hash=bVKA5XzzgVcoduCJyhjk6w%3d%3d");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WAnchor_internal_path_link_with_parameters_and_session_id_render )
{
  // Test whether an internal path link with parameters is correctly rendered with session ID in the URL
  Wt::Test::WTestEnvironment environment;
  environment.setSessionIdInUrl(true);
  Wt::WApplication app(environment);

  auto anchor = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath, "/app?param1=1&param2=2"));
  auto anchorPtr = anchor.get();
  app.root()->addWidget(std::move(anchor));

  // Simulate UI update (and calls updateDom)
  auto domElement = anchorPtr->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("href") == "app%3fparam1%3d1%26param2%3d2");

  delete domElement;
}
