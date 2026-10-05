/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

#include <thread>

using namespace Wt;
using namespace Selenium;

namespace {
  class WWebWidgetTestApp : public WApplication
  {
  public:
    WWebWidgetTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      text1_ = root()->addWidget(std::make_unique<WText>("Text 1"));
      text1_->setId("text-1");
      text1_->setCanReceiveFocus(true);

      text2_ = root()->addWidget(std::make_unique<WText>("Text 2"));
      text2_->setId("text-2");
      text2_->setCanReceiveFocus(true);
    };

    WText* text1() const { return text1_; }
    WText* text2() const { return text2_; }

  private:
    WText* text1_;
    WText* text2_;
  };
}

BOOST_AUTO_TEST_SUITE(selenium_wwebwidget)

SELENIUM_TEST(wwebwidget_track_focus_keep_focus_synchronized, WWebWidgetTestApp)
  auto text1 = api.getElement(SeleniumAPI::FindBy::ID, "text-1");
  BOOST_REQUIRE(text1.has_value());
  auto text2 = api.getElement(SeleniumAPI::FindBy::ID, "text-2");
  BOOST_REQUIRE(text2.has_value());
  WWidget* text2Ptr = nullptr;

  updateApplication([&](WWebWidgetTestApp* app) {
    app->text1()->setFocus();
    app->setTrackFocus(true);
  });

  BOOST_REQUIRE(wait.until([&]() { return text1 == api.getActiveElement(); }));

  BOOST_REQUIRE(
    executeInApplication([&](const WWebWidgetTestApp* app) {
      BOOST_TEST(app->focus() == "text-1");
      BOOST_TEST(app->text1() == app->focusedWidget());
      BOOST_REQUIRE(app->focus() != "text-2");
      text2Ptr = app->text2();
      BOOST_REQUIRE(text2Ptr != nullptr);
    })
  );

  text2->focus();

  BOOST_REQUIRE(wait.until([&]() { return text2 == api.getActiveElement(); }));

  std::string focusId;
  WWidget* focusedWidget = nullptr;

  wait.until([&]() {
    return checkApplicationState([&](const WWebWidgetTestApp* app) {
      focusId = app->focus();
      focusedWidget = app->focusedWidget();
      return focusedWidget == text2Ptr && focusId == "text-2";
    });
  });

  BOOST_TEST(focusId == "text-2");
  BOOST_TEST(focusedWidget == text2Ptr);

END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
