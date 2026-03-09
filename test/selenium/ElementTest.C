/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "framework/SeleniumTest.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WText.h>

using namespace Wt;
using namespace Selenium;

namespace {
  class ElementTestApp : public WApplication
  {
  public:
    ElementTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      // Create a WLineEdit with various attributes
      auto lineEdit = root()->addWidget(std::make_unique<WLineEdit>());
      lineEdit->setId("test-input");
      lineEdit->setStyleClass("form-control test-class");
      lineEdit->setAttributeValue("name", "testInput");
      lineEdit->setText("Initial Value");
      lineEdit->setPlaceholderText("Enter text here");

      // Create a WText with content and styling
      auto text = root()->addWidget(std::make_unique<WText>("Hello World"));
      text->setId("test-text");
      text->setStyleClass("text-widget");
      text->setAttributeValue("name", "testText");
      text->setPadding(WLength(10, LengthUnit::Pixel), Side::Left | Side::Right);
      text->setTextAlignment(AlignmentFlag::Center);

      auto hiddenText = root()->addWidget(std::make_unique<WText>("I'm hiding!"));
      hiddenText->setId("hidden-text");
      hiddenText->setHidden(true);
    }
  };
}

BOOST_AUTO_TEST_SUITE(selenium_element)

SELENIUM_TEST(element_id, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());
  BOOST_TEST(inputElement->id() == "test-input");

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());
  BOOST_TEST(textElement->id() == "test-text");

  auto hiddenTextElement = api.getElement(SeleniumAPI::FindBy::ID, "hidden-text");
  BOOST_REQUIRE(hiddenTextElement.has_value());
  BOOST_TEST(hiddenTextElement->id() == "hidden-text");
END_SELENIUM_TEST

SELENIUM_TEST(element_tagName, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());
  BOOST_TEST(inputElement->tagName() == "input");

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());
  BOOST_TEST(textElement->tagName() == "span");
END_SELENIUM_TEST

SELENIUM_TEST(element_className, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());
  BOOST_TEST(inputElement->className().find("form-control") != std::string::npos);
  BOOST_TEST(inputElement->className().find("test-class") != std::string::npos);

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());
  BOOST_TEST(textElement->className().find("text-widget") != std::string::npos);
END_SELENIUM_TEST

SELENIUM_TEST(element_name, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());
  BOOST_TEST(inputElement->name() == "testInput");

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());
  BOOST_TEST(textElement->name() == "testText");
END_SELENIUM_TEST

SELENIUM_TEST(element_value, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());
  BOOST_TEST(inputElement->value() == "Initial Value");

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());
  BOOST_TEST(textElement->value().empty());
END_SELENIUM_TEST

SELENIUM_TEST(element_text, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());
  BOOST_TEST(inputElement->text().empty());

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());
  BOOST_TEST(textElement->text() == "Hello World");
END_SELENIUM_TEST

SELENIUM_TEST(element_getCssValue, ElementTestApp)
  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());

  auto paddingLeft = textElement->getCssValue("padding-left");
  BOOST_TEST(paddingLeft == "10px");

  auto paddingRight = textElement->getCssValue("padding-right");
  BOOST_TEST(paddingRight == "10px");

  auto textAlign = textElement->getCssValue("text-align");
  BOOST_TEST(textAlign == "center");
END_SELENIUM_TEST

SELENIUM_TEST(element_isVisible, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());
  BOOST_TEST(inputElement->isVisible());

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());
  BOOST_TEST(textElement->isVisible());

  auto hiddenTextElement = api.getElement(SeleniumAPI::FindBy::ID, "hidden-text");
  BOOST_REQUIRE(hiddenTextElement.has_value());
  BOOST_TEST(!hiddenTextElement->isVisible());
END_SELENIUM_TEST

SELENIUM_TEST(element_size, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());

  BOOST_TEST(inputElement->width() > 0);
  BOOST_TEST(inputElement->height() > 0);

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());

  BOOST_TEST(textElement->width() > 0);
  BOOST_TEST(textElement->height() > 0);

  auto hiddenTextElement = api.getElement(SeleniumAPI::FindBy::ID, "hidden-text");
  BOOST_REQUIRE(hiddenTextElement.has_value());
  // Note: test on sizing is dependent on the browser, so has been omitted.
END_SELENIUM_TEST

SELENIUM_TEST(element_location, ElementTestApp)
  auto inputElement = api.getElement(SeleniumAPI::FindBy::ID, "test-input");
  BOOST_REQUIRE(inputElement.has_value());

  BOOST_TEST(inputElement->x() >= 0);
  BOOST_TEST(inputElement->y() >= 0);

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(textElement.has_value());

  BOOST_TEST(textElement->x() >= 0);
  BOOST_TEST(textElement->y() >= 0);

  auto hiddenTextElement = api.getElement(SeleniumAPI::FindBy::ID, "hidden-text");
  BOOST_REQUIRE(hiddenTextElement.has_value());

  BOOST_TEST(textElement->x() > inputElement->x());
  BOOST_TEST(textElement->y() >= inputElement->y());
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
