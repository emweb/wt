/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

using namespace Wt;
using namespace Selenium;

namespace {
  class WTextTestApp : public WApplication
  {
  public:
    WTextTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      // Test widget that we'll manipulate
      testText_ = root()->addWidget(std::make_unique<WText>());
      testText_->setId("test-text");
      testText_->setText("Hello World");
      testText_->setPadding(WLength(10, LengthUnit::Pixel), Side::Left | Side::Right);
      testText_->setTextAlignment(AlignmentFlag::Left);
      testText_->setWordWrap(true);
    };

    WText* testText() { return testText_; }

  private:
    WText* testText_;
  };
}

BOOST_AUTO_TEST_SUITE(selenium_wtext)

SELENIUM_TEST(wtext_setText_initial, WTextTestApp)
  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(element.has_value());

  auto textContent = element->text();
  BOOST_TEST(textContent == "Hello World");
END_SELENIUM_TEST

SELENIUM_TEST(wtext_setText_updated, WTextTestApp)
  updateApplication([](WTextTestApp* app) {
      app->testText()->setText("Updated Text");
  });

  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(element.has_value());

  BOOST_REQUIRE(wait.until(element.value(), &Element::text, "Updated Text"));
  auto textContent = element->text();
  BOOST_TEST(textContent == "Updated Text");
END_SELENIUM_TEST

SELENIUM_TEST(wtext_setPadding_initial, WTextTestApp)
  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(element.has_value());

  auto paddingLeft = element->getCssValue("padding-left");
  BOOST_TEST(paddingLeft == "10px");

  auto paddingRight = element->getCssValue("padding-right");
  BOOST_TEST(paddingRight == "10px");
END_SELENIUM_TEST

SELENIUM_TEST(wtext_setPadding_updated, WTextTestApp)
  updateApplication([](WTextTestApp* app) {
    app->testText()->setPadding(WLength(20, LengthUnit::Pixel), Side::Left);
    app->testText()->setPadding(WLength(5, LengthUnit::Pixel), Side::Right);
  });

  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(element.has_value());

  BOOST_REQUIRE(wait.until(element.value(), &Element::getCssValue, "padding-left", "20px"));
  auto paddingLeft = element->getCssValue("padding-left");
  BOOST_TEST(paddingLeft == "20px");

  auto paddingRight = element->getCssValue("padding-right");
  BOOST_TEST(paddingRight == "5px");
END_SELENIUM_TEST

SELENIUM_TEST(wtext_setTextAlignment_initial, WTextTestApp)

  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(element.has_value());

  auto textAlign = element->getCssValue("text-align");
  BOOST_TEST(textAlign == "left");
END_SELENIUM_TEST

SELENIUM_TEST(wtext_setTextAlignment_updated, WTextTestApp)
  updateApplication([](WTextTestApp* app) {
    app->testText()->setTextAlignment(AlignmentFlag::Center);
  });

  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(element.has_value());

  BOOST_REQUIRE(wait.until(element.value(), &Element::getCssValue, "text-align", "center"));
  auto textAlign = element->getCssValue("text-align");
  BOOST_TEST(textAlign == "center");

  updateApplication([](WTextTestApp* app) {
    app->testText()->setTextAlignment(AlignmentFlag::Right);
  });

  BOOST_REQUIRE(wait.until(element.value(), &Element::getCssValue, "text-align", "right"));
  textAlign = element->getCssValue("text-align");
  BOOST_TEST(textAlign == "right");
END_SELENIUM_TEST

SELENIUM_TEST(wtext_setWordWrap_initial, WTextTestApp)
  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(element.has_value());

  auto whiteSpace = element->getCssValue("white-space");
  BOOST_TEST(whiteSpace == "normal");
END_SELENIUM_TEST

SELENIUM_TEST(wtext_setWordWrap_updated, WTextTestApp)
  updateApplication([](WTextTestApp* app) {
    app->testText()->setWordWrap(false);
  });

  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-text");
  BOOST_REQUIRE(element.has_value());

  BOOST_REQUIRE(wait.until(element.value(), &Element::getCssValue, "white-space", "nowrap"));
  auto whiteSpace = element->getCssValue("white-space");
  BOOST_TEST(whiteSpace == "nowrap");
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
