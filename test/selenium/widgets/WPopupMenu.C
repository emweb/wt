/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"
#include "../framework/ActionChains.h"

#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WMenuItem.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

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

  class BootstrapKeyboardTestApp : public WApplication
  {
  public:
    BootstrapKeyboardTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      setTheme(std::make_shared<WBootstrap5Theme>());

      auto popup = std::make_unique<WPopupMenu>();
      popup->setId("popup");
      popup->addItem("Top-level item")->setId("top-item");
      auto submenu = std::make_unique<WPopupMenu>();
      submenu->setId("submenu");
      submenu->addItem("Submenu item")->setId("sub-item");
      auto parent = popup->addItem("Submenu");
      parent->setId("sub-parent");
      parent->setMenu(std::move(submenu));
      auto dismissals = root()->addNew<WText>("0");
      dismissals->setId("dismissal-count");
      popup->aboutToHide().connect([this, dismissals]() {
        dismissals->setText(std::to_string(++dismissals_));
      });

      auto open = root()->addNew<WPushButton>("Open popup");
      open->setId("open-popup");
      open->setMenu(std::move(popup));

      auto status = root()->addNew<WText>("0");
      status->setId("server-count");
      auto ping = root()->addNew<WPushButton>("Contact server");
      ping->setId("ping");
      ping->clicked().connect([this, status]() {
        status->setText(std::to_string(++count_));
      });
    }

    void addNativeDropdown()
    {
      root()->addNew<WText>(R"(
        <div class="dropdown">
          <button id="native-toggle" class="btn btn-secondary dropdown-toggle"
                  data-bs-toggle="dropdown" aria-expanded="false">
            Native dropdown
          </button>
          <ul id="native-menu" class="dropdown-menu">
            <li><a id="native-first" class="dropdown-item" href="#">First</a></li>
            <li><a id="native-last" class="dropdown-item" href="#">Last</a></li>
          </ul>
        </div>
      )", TextFormat::UnsafeXHTML);
    }

  private:
    int count_ = 0;
    int dismissals_ = 0;
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

SELENIUM_TEST(bootstrap_keyboard, BootstrapKeyboardTestApp)
  BOOST_REQUIRE(wait.until([&]() {
    return api.executeScript("return String(!!window.bootstrap && !!bootstrap.Dropdown);") == "true";
  }));

  // Capture exceptions even if the browser continues event dispatch.
  api.executeScript(R"(
    window.popupErrors = [];
    window.addEventListener('error', event => popupErrors.push(event.message));
    window.addEventListener('unhandledrejection', event => {
      popupErrors.push(String(event.reason));
    });
    window.popupKeys = [];
    document.addEventListener('keydown', event => {
      popupKeys.push([event.key, event.defaultPrevented]);
    });
    return '';
  )");

  auto open = api.getElement(SeleniumAPI::FindBy::ID, "open-popup");
  auto ping = api.getElement(SeleniumAPI::FindBy::ID, "ping");
  auto count = api.getElement(SeleniumAPI::FindBy::ID, "server-count");
  auto dismissals = api.getElement(SeleniumAPI::FindBy::ID, "dismissal-count");
  BOOST_REQUIRE(open && ping && count && dismissals);

  // Escape on a focused, detached submenu is the original failure. The
  // arrow keys must pass through Bootstrap without being cancelled.
  const std::string keys[] = { "\uE00C", "\uE013", "\uE015", "\uE00C" };
  const std::string names[] = { "Escape", "ArrowUp", "ArrowDown", "Escape" };
  for (int i = 0; i < 4; ++i) {
    BOOST_TEST_CONTEXT("Wt popup key " << names[i] << ", submenu " << (i < 3)) {
      BOOST_REQUIRE(open->click());
      std::optional<Element> popup;
      BOOST_REQUIRE(wait.until([&]() {
        popup = api.getElement(SeleniumAPI::FindBy::ID, "popup");
        return popup && popup->isVisible();
      }));

      if (i < 3) {
        auto parent = api.getElement(SeleniumAPI::FindBy::ID, "sub-parent");
        BOOST_REQUIRE(parent);
        ActionChains(api).moveToElement(*parent).perform();
        BOOST_REQUIRE(wait.until([&]() {
          auto submenu = api.getElement(SeleniumAPI::FindBy::ID, "submenu");
          return submenu && submenu->isVisible();
        }));
      }

      const std::string selector = i < 3 ? "#sub-item a" : "#top-item a";
      auto item = api.getElement(SeleniumAPI::FindBy::CSS_SELECTOR, selector);
      BOOST_REQUIRE(item);
      BOOST_REQUIRE(item->focus());
      BOOST_REQUIRE(wait.until([&]() { return api.getActiveElement() == item; }));
      BOOST_REQUIRE(item->sendKeys(keys[i]));

      BOOST_CHECK_EQUAL(api.executeScript("return JSON.stringify(popupErrors);"), "[]");
      BOOST_CHECK_EQUAL(api.executeScript("return JSON.stringify(popupKeys.slice(-1)[0]);"),
                        "[\"" + names[i] + "\",false]");

      if (names[i] != "Escape") {
        BOOST_TEST(popup->isVisible());
        BOOST_REQUIRE(item->sendKeys("\uE00C"));
      }
      BOOST_REQUIRE(wait.until([&]() { return !popup->isVisible(); }));
      BOOST_REQUIRE(wait.until(*dismissals, &Element::text, std::to_string(i + 1)));
      BOOST_REQUIRE(ping->click());
      BOOST_REQUIRE(wait.until(*count, &Element::text, std::to_string(i + 1)));
      BOOST_CHECK_EQUAL(api.executeScript("return JSON.stringify(popupErrors);"), "[]");
    }
  }

END_SELENIUM_TEST

SELENIUM_TEST(bootstrap_native_keyboard, BootstrapKeyboardTestApp)
  api.executeScript(R"(
    window.popupErrors = [];
    window.addEventListener('error', event => popupErrors.push(event.message));
    window.addEventListener('unhandledrejection', event => {
      popupErrors.push(String(event.reason));
    });
    return '';
  )");

  // Bootstrap must still navigate and dismiss menus with a toggle.
  updateApplication([](BootstrapKeyboardTestApp* app) { app->addNativeDropdown(); });
  BOOST_REQUIRE(wait.until([&]() {
    return api.getElement(SeleniumAPI::FindBy::ID, "native-toggle").has_value();
  }));
  auto toggle = api.getElement(SeleniumAPI::FindBy::ID, "native-toggle");
  auto menu = api.getElement(SeleniumAPI::FindBy::ID, "native-menu");
  auto first = api.getElement(SeleniumAPI::FindBy::ID, "native-first");
  auto last = api.getElement(SeleniumAPI::FindBy::ID, "native-last");
  BOOST_REQUIRE(toggle && menu && first && last);
  BOOST_REQUIRE(toggle->sendKeys("\uE015"));
  BOOST_REQUIRE(wait.until([&]() { return menu->isVisible(); }));
  BOOST_CHECK(wait.until([&]() { return api.getActiveElement() == first; }));
  BOOST_REQUIRE(first->sendKeys("\uE015"));
  BOOST_CHECK(wait.until([&]() { return api.getActiveElement() == last; }));
  BOOST_REQUIRE(last->sendKeys("\uE013"));
  BOOST_CHECK(wait.until([&]() { return api.getActiveElement() == first; }));
  BOOST_REQUIRE(first->sendKeys("\uE00C"));
  BOOST_REQUIRE(wait.until([&]() { return !menu->isVisible(); }));
  BOOST_CHECK(wait.until([&]() { return api.getActiveElement() == toggle; }));
  BOOST_CHECK(api.getElement(SeleniumAPI::FindBy::CSS_SELECTOR, "#native-toggle[aria-expanded='false']").has_value());
  BOOST_CHECK_EQUAL(api.executeScript("return JSON.stringify(popupErrors);"), "[]");
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
