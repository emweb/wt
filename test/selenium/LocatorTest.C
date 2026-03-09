/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "framework/SeleniumTest.h"

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

using namespace Wt;
using namespace Selenium;

namespace {
  class SimpleTestApp : public WApplication
  {
  public:
    SimpleTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      setTitle("Selenium Test Application");

      // Will render inside the `Wt-domroot -> div (100% height) -> div (root)
      // Adds a div with an h1
      root()->addWidget(std::make_unique<WText>("<h1>Test Page</h1>"));
      // Adds a span
      root()->addWidget(std::make_unique<WText>("This is a simple test application."));
      // Adds a br
      root()->addWidget(std::make_unique<WBreak>());

      // Adds an input
      auto nameEdit = root()->addWidget(std::make_unique<WLineEdit>());
      nameEdit->setStyleClass("form-control");
      nameEdit->setId("nameInput");

      // Adds a button
      auto button = root()->addWidget(std::make_unique<WPushButton>("Click Me"));
      button->setAttributeValue("name", "testButton");

      // Adds a span
      auto greeting = root()->addWidget(std::make_unique<WText>());
      greeting->setId("greeting");

      button->clicked().connect([=]() {
        greeting->setText("Hello, " + nameEdit->text() + "!");
      });
    }
  };
}

BOOST_AUTO_TEST_SUITE(selenium_locator)

SELENIUM_TEST(selenium_locator_id, SimpleTestApp)
  auto nameInput = api.getElement(SeleniumAPI::FindBy::ID, "nameInput");
  BOOST_REQUIRE(nameInput.has_value());
  BOOST_TEST(nameInput->id() == "nameInput");
END_SELENIUM_TEST

SELENIUM_TEST(selenium_locator_name, SimpleTestApp)
  auto button = api.getElement(SeleniumAPI::FindBy::NAME, "testButton");
  BOOST_REQUIRE(button.has_value());
  BOOST_TEST(button->name() == "testButton");
END_SELENIUM_TEST

SELENIUM_TEST(selenium_locator_xpath, SimpleTestApp)
  auto greeting = api.getElement(SeleniumAPI::FindBy::XPATH, "//div[@class='Wt-domRoot']//div/span[@id='greeting']");
  BOOST_REQUIRE(greeting.has_value());
  BOOST_TEST(greeting->id() == "greeting");
END_SELENIUM_TEST

SELENIUM_TEST(selenium_locator_tag_name, SimpleTestApp)
  auto button = api.getElement(SeleniumAPI::FindBy::TAG_NAME, "button");
  BOOST_REQUIRE(button.has_value());
  BOOST_TEST(button->name() == "testButton");
END_SELENIUM_TEST

SELENIUM_TEST(selenium_locator_class_name, SimpleTestApp)
  auto nameInput = api.getElement(SeleniumAPI::FindBy::CLASS_NAME, "form-control");
  BOOST_REQUIRE(nameInput.has_value());
  BOOST_TEST(nameInput->className() == "form-control");
END_SELENIUM_TEST

SELENIUM_TEST(selenium_locator_css_selector, SimpleTestApp)
  auto greeting = api.getElement(SeleniumAPI::FindBy::CSS_SELECTOR, "div div span:nth-of-type(2)");
  BOOST_REQUIRE(greeting.has_value());
  BOOST_TEST(greeting->id() == "greeting");
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
