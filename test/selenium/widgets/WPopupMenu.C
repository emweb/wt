/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WMenuItem.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>

using namespace Wt;
using namespace Selenium;

namespace {
  class WPopupMenuTestApp : public WApplication
  {
  public:
    WPopupMenuTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      auto triggerButton = root()->addWidget(std::make_unique<WPushButton>("Open Popup Menu"));
      triggerButton->setId("trigger-button");

      auto popupMenu = std::make_unique<WPopupMenu>();
      auto removeItem = popupMenu->addItem("Remove");
      removeItem->anchor()->setId("remove-menu");
      auto hideItem = popupMenu->addItem("Hide");
      hideItem->anchor()->setId("hide-menu");
      popupMenu->triggered().connect([=](WMenuItem *item) {
        if (item == removeItem)
          triggerButton->setMenu(nullptr);
      });
      triggerButton->setMenu(std::move(popupMenu));

      auto uselessButton = root()->addWidget(std::make_unique<WPushButton>("Useless Button"));
      uselessButton->setId("useless-button");
    }
  };
}

BOOST_AUTO_TEST_SUITE(selenium_wpopupmenu)

SELENIUM_TEST(wpopupmenu_give_back_focus, WPopupMenuTestApp)
  auto triggerButton = api.getElement(SeleniumAPI::FindBy::ID, "trigger-button");
  BOOST_REQUIRE(triggerButton.has_value());

  triggerButton->click();

  std::optional<Element> removeItem;
  std::optional<Element> hideItem;

  BOOST_REQUIRE(wait.until([&]{
    removeItem = api.getElement(SeleniumAPI::FindBy::ID, "remove-menu");
    return removeItem.has_value();
  }));
  BOOST_REQUIRE(wait.until([&]{
    hideItem = api.getElement(SeleniumAPI::FindBy::ID, "hide-menu");
    return hideItem.has_value();
  }));
  BOOST_REQUIRE(wait.until([&]{ return hideItem->isVisible(); }));
  BOOST_REQUIRE(hideItem->focus());
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() == hideItem; }));

  hideItem->click();

  BOOST_TEST(wait.until([&]{ return api.getActiveElement() == triggerButton; }));

  triggerButton->click();

  BOOST_REQUIRE(wait.until([&]{ return removeItem->isVisible(); }));
  BOOST_REQUIRE(wait.until([&]{ return hideItem->isVisible(); }));
  BOOST_REQUIRE(removeItem->focus());
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() == removeItem; }));

  removeItem->click();

  BOOST_TEST(wait.until([&]{ return api.getActiveElement() == triggerButton; }));

END_SELENIUM_TEST

SELENIUM_TEST(wpopupmenu_give_back_focus_to_last_focused, WPopupMenuTestApp)
  auto triggerButton = api.getElement(SeleniumAPI::FindBy::ID, "trigger-button");
  BOOST_REQUIRE(triggerButton.has_value());
  auto uselessButton = api.getElement(SeleniumAPI::FindBy::ID, "useless-button");
  BOOST_REQUIRE(uselessButton.has_value());

  triggerButton->click();

  std::optional<Element> removeItem;
  std::optional<Element> hideItem;

  BOOST_REQUIRE(wait.until([&]{
    removeItem = api.getElement(SeleniumAPI::FindBy::ID, "remove-menu");
    return removeItem.has_value();
  }));
  BOOST_REQUIRE(wait.until([&]{
    hideItem = api.getElement(SeleniumAPI::FindBy::ID, "hide-menu");
    return hideItem.has_value();
  }));
  BOOST_REQUIRE(wait.until([&]{ return hideItem->isVisible(); }));

  BOOST_REQUIRE(uselessButton->focus());
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() == uselessButton; }));
  BOOST_REQUIRE(hideItem->focus());
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() == hideItem; }));

  hideItem->click();

  BOOST_TEST(wait.until([&]{ return api.getActiveElement() == uselessButton; }));

  triggerButton->click();

  BOOST_REQUIRE(wait.until([&]{ return removeItem->isVisible(); }));
  BOOST_REQUIRE(wait.until([&]{ return hideItem->isVisible(); }));

  BOOST_REQUIRE(uselessButton->focus());
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() == uselessButton; }));
  BOOST_REQUIRE(hideItem->focus());
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() == hideItem; }));

  removeItem->click();

  BOOST_TEST(wait.until([&]{ return api.getActiveElement() == uselessButton; }));

END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()