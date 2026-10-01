/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WAnimation.h>
#include <Wt/WApplication.h>
#include <Wt/WDialog.h>
#include <Wt/WPushButton.h>

#include <map>
#include <vector>

using namespace Wt;
using namespace Selenium;

namespace {
  class WDialogTestApp : public WApplication
  {
  public:
    WDialogTestApp(const WEnvironment& env)
      : WApplication(env),
        testModalDialog_(nullptr),
        testNonModalDialog_(nullptr)
    {
      auto triggerModalButton = root()->addWidget(std::make_unique<WPushButton>("Open Modal Popup"));
      triggerModalButton->setId("trigger-modal-button");
      triggerModalButton->clicked().connect([=] {
        showDialog(true);
      });

      auto triggerNonModalButton = root()->addWidget(std::make_unique<WPushButton>("Open Non-Modal Popup"));
      triggerNonModalButton->setId("trigger-non-modal-button");
      triggerNonModalButton->clicked().connect([=] {
        showDialog(false);
      });

      auto uselessButton = root()->addWidget(std::make_unique<WPushButton>("Useless Button"));
      uselessButton->setId("useless-button");
    }

  private:
    void showDialog(bool modal)
    {
      WDialog*& dialog = modal ? testModalDialog_ : testNonModalDialog_;

      if (!dialog) {
        dialog = root()->addChild(std::make_unique<WDialog>("Test Dialog"));

        std::string idPrefix = modal ? "modal" : "non-modal";
        auto deleteButton = dialog->contents()->addWidget(std::make_unique<WPushButton>("Delete"));
        deleteButton->setId(idPrefix + "-delete-button");
        auto hideButton = dialog->contents()->addWidget(std::make_unique<WPushButton>("Hide"));
        hideButton->setId(idPrefix + "-hide-button");
        auto animateHideButton = dialog->contents()->addWidget(std::make_unique<WPushButton>("Animate Hide"));
        animateHideButton->setId(idPrefix + "-animate-hide-button");
        dialog->setModal(modal);

        deleteButton->clicked().connect([&] {
          root()->removeChild(dialog);
          dialog = nullptr;
        });
        hideButton->clicked().connect([&] {
          dialog->hide();
        });
        animateHideButton->clicked().connect([&] {
          dialog->animateHide(WAnimation(AnimationEffect::Pop, TimingFunction::Linear, 500));
        });
      }

      dialog->show();
      dialog->setFirstFocus();
    }

    WDialog* testModalDialog_;
    WDialog* testNonModalDialog_;
  };
}

BOOST_AUTO_TEST_SUITE(selenium_wdialog)

SELENIUM_TEST(wdialog_modal_give_back_focus, WDialogTestApp)
  auto triggerModalButton = api.getElement(SeleniumAPI::FindBy::ID, "trigger-modal-button");
  BOOST_REQUIRE(triggerModalButton.has_value());

  triggerModalButton->click();

  std::map<std::string, std::optional<Element>> buttons{
    {"modal-delete-button", std::nullopt},
    {"modal-hide-button", std::nullopt},
    {"modal-animate-hide-button", std::nullopt}
  };
  const std::vector<std::string> buttonIds{
    "modal-hide-button", "modal-animate-hide-button", "modal-delete-button"
  };

  for (const auto& buttonId : buttonIds) {
    BOOST_REQUIRE(wait.until([&]{
      buttons[buttonId] = api.getElement(SeleniumAPI::FindBy::ID, buttonId);
      return buttons[buttonId].has_value();
    }));
  }
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerModalButton; }));

  for (const auto& buttonId : buttonIds) {
    buttons[buttonId]->click();
    BOOST_TEST(wait.until([&]{ return api.getActiveElement() == triggerModalButton; }));

    if (buttonId == "modal-animate-hide-button") {
      BOOST_TEST(buttons[buttonId]->isVisible());
      BOOST_REQUIRE(wait.until([&]{ return !buttons[buttonId]->isVisible(); }, std::chrono::seconds(1)));
    }

    if (buttonId != "modal-delete-button") {
      triggerModalButton->click();
      for (const auto& visibleButtonId : buttonIds)
        BOOST_REQUIRE(wait.until([&]{ return buttons[visibleButtonId]->isVisible(); }));
      BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerModalButton; }));
    }
  }

END_SELENIUM_TEST

SELENIUM_TEST(wdialog_non_modal_give_back_focus, WDialogTestApp)
  auto triggerNonModalButton = api.getElement(SeleniumAPI::FindBy::ID, "trigger-non-modal-button");
  BOOST_REQUIRE(triggerNonModalButton.has_value());

  triggerNonModalButton->click();

  std::map<std::string, std::optional<Element>> buttons{
    {"non-modal-delete-button", std::nullopt},
    {"non-modal-hide-button", std::nullopt},
    {"non-modal-animate-hide-button", std::nullopt}
  };
  const std::vector<std::string> buttonIds{
    "non-modal-hide-button", "non-modal-animate-hide-button", "non-modal-delete-button"
  };

  for (const auto& buttonId : buttonIds) {
    BOOST_REQUIRE(wait.until([&]{
      buttons[buttonId] = api.getElement(SeleniumAPI::FindBy::ID, buttonId);
      return buttons[buttonId].has_value();
    }));
  }
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerNonModalButton; }));

  for (const auto& buttonId : buttonIds) {
    buttons[buttonId]->click();
    BOOST_TEST(wait.until([&]{ return api.getActiveElement() == triggerNonModalButton; }));

    if (buttonId == "non-modal-animate-hide-button") {
      BOOST_TEST(buttons[buttonId]->isVisible());
      BOOST_REQUIRE(wait.until([&]{ return !buttons[buttonId]->isVisible(); }, std::chrono::seconds(1)));
    }

    if (buttonId != "non-modal-delete-button") {
      triggerNonModalButton->click();
      for (const auto& visibleButtonId : buttonIds)
        BOOST_REQUIRE(wait.until([&]{ return buttons[visibleButtonId]->isVisible(); }));
      BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerNonModalButton; }));
    }
  }

END_SELENIUM_TEST

SELENIUM_TEST(wdialog_non_modal_give_back_focus_to_last_focused, WDialogTestApp)
  auto triggerNonModalButton = api.getElement(SeleniumAPI::FindBy::ID, "trigger-non-modal-button");
  BOOST_REQUIRE(triggerNonModalButton.has_value());
  auto uselessButton = api.getElement(SeleniumAPI::FindBy::ID, "useless-button");
  BOOST_REQUIRE(uselessButton.has_value());

  triggerNonModalButton->click();

  std::map<std::string, std::optional<Element>> buttons{
    {"non-modal-delete-button", std::nullopt},
    {"non-modal-hide-button", std::nullopt},
    {"non-modal-animate-hide-button", std::nullopt}
  };
  const std::vector<std::string> buttonIds{
    "non-modal-hide-button", "non-modal-animate-hide-button", "non-modal-delete-button"
  };

  for (const auto& buttonId : buttonIds) {
    BOOST_REQUIRE(wait.until([&]{
      buttons[buttonId] = api.getElement(SeleniumAPI::FindBy::ID, buttonId);
      return buttons[buttonId].has_value();
    }));
  }
  BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerNonModalButton; }));

  for (const auto& buttonId : buttonIds) {
    BOOST_REQUIRE(uselessButton->focus());
    BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() == uselessButton; }));
    BOOST_REQUIRE(buttons["non-modal-hide-button"]->focus());
    BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() == buttons["non-modal-hide-button"]; }));

    buttons[buttonId]->click();
    BOOST_TEST(wait.until([&]{ return api.getActiveElement() == uselessButton; }));

    if (buttonId == "non-modal-animate-hide-button") {
      BOOST_TEST(buttons[buttonId]->isVisible());
      BOOST_REQUIRE(wait.until([&]{ return !buttons[buttonId]->isVisible(); }, std::chrono::seconds(1)));
    }

    if (buttonId != "non-modal-delete-button") {
      triggerNonModalButton->click();
      for (const auto& visibleButtonId : buttonIds)
        BOOST_REQUIRE(wait.until([&]{ return buttons[visibleButtonId]->isVisible(); }));
      BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerNonModalButton; }));
    }
  }

END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()