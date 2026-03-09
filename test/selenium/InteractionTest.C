/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "framework/SeleniumTest.h"

#include <Wt/WApplication.h>
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
      // Adds an input
      auto nameEdit = root()->addWidget(std::make_unique<WLineEdit>());
      nameEdit->setId("editor");

      // Adds a button
      auto button = root()->addWidget(std::make_unique<WPushButton>("Click Me"));

      // Adds a span
      auto greeting = root()->addWidget(std::make_unique<WText>());
      greeting->setId("greeting");

      button->clicked().connect([=]() {
        greeting->setText("Hello, " + nameEdit->text() + "!");
      });
    }
  };
}

BOOST_AUTO_TEST_SUITE(selenium_interaction)

SELENIUM_TEST(selenium_click, SimpleTestApp)
  auto button = api.getElement(SeleniumAPI::FindBy::TAG_NAME, "button");
  BOOST_REQUIRE(button.has_value());
  BOOST_TEST(button->click());
END_SELENIUM_TEST

SELENIUM_TEST(selenium_sendKey, SimpleTestApp)
  auto input = api.getElement(SeleniumAPI::FindBy::ID, "editor");
  BOOST_REQUIRE(input.has_value());
  BOOST_TEST(input->sendKeys("NAME!"));
END_SELENIUM_TEST

SELENIUM_TEST(selenium_sendKey_with_result, SimpleTestApp)
  auto input = api.getElement(SeleniumAPI::FindBy::ID, "editor");
  BOOST_REQUIRE(input.has_value());
  BOOST_REQUIRE(input->sendKeys("NAME!"));
  BOOST_TEST(input->value() == "NAME!");
END_SELENIUM_TEST

SELENIUM_TEST(selenium_combined, SimpleTestApp)
  auto input = api.getElement(SeleniumAPI::FindBy::ID, "editor");
  BOOST_REQUIRE(input.has_value());
  BOOST_REQUIRE(input->sendKeys("NAME!"));
  BOOST_TEST(input->value() == "NAME!");

  auto button = api.getElement(SeleniumAPI::FindBy::TAG_NAME, "button");
  BOOST_REQUIRE(button.has_value());
  BOOST_REQUIRE(button->click());

  auto output = api.getElement(SeleniumAPI::FindBy::TAG_NAME, "span");
  BOOST_REQUIRE(output.has_value());

  // Needs waiting for change
  wait.until([output] { return output.value().text() != ""; });
  BOOST_TEST(output->text() == "Hello, NAME!!");
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
