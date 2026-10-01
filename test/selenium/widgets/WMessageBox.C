/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <map>
#include <vector>

using namespace Wt;
using namespace Selenium;

namespace {
  class WMessageBoxTestApp : public WApplication
  {
  public:
    struct ButtonAction {
      static constexpr StandardButton Delete = StandardButton::Yes;
      static constexpr StandardButton Hide = StandardButton::No;
      static constexpr StandardButton AnimateHide = StandardButton::Cancel;
    };

    WMessageBoxTestApp(const WEnvironment& env)
      : WApplication(env),
        testModalMessageBox_(nullptr),
        testNonModalMessageBox_(nullptr)
    {
      auto triggerModalButton = root()->addWidget(std::make_unique<WPushButton>("Open Modal Popup"));
      triggerModalButton->setId("trigger-modal-button");
      triggerModalButton->clicked().connect([=] {
        showMessageBox(true);
      });

      auto triggerNonModalButton = root()->addWidget(std::make_unique<WPushButton>("Open Non-Modal Popup"));
      triggerNonModalButton->setId("trigger-non-modal-button");
      triggerNonModalButton->clicked().connect([=] {
        showMessageBox(false);
      });

      auto uselessButton = root()->addWidget(std::make_unique<WPushButton>("Useless Button"));
      uselessButton->setId("useless-button");
    }

  private:
    void showMessageBox(bool modal)
    {
      WMessageBox*& messageBox = modal ?
        testModalMessageBox_ :
        testNonModalMessageBox_;

      if (!messageBox) {
        messageBox = root()->addChild(std::make_unique<WMessageBox>(
          "Test Message: ",
          "Accept or I'll just hide",
          Wt::Icon::Information,
          ButtonAction::Delete | ButtonAction::Hide | ButtonAction::AnimateHide
        ));

        std::string idPrefix = modal ? "modal" : "non-modal";
        messageBox->button(ButtonAction::Delete)->setId(idPrefix + "-delete-button");
        messageBox->button(ButtonAction::Hide)->setId(idPrefix + "-hide-button");
        messageBox->button(ButtonAction::AnimateHide)->setId(idPrefix + "-animate-hide-button");
        messageBox->setModal(modal);

        messageBox->buttonClicked().connect([&] {
          onActionButtonClick(messageBox);
        });
      }

      messageBox->show();
      messageBox->setFirstFocus();
    }

    void onActionButtonClick(WMessageBox*& messageBox) {
      switch (messageBox->buttonResult()) {
      case ButtonAction::Delete:
        root()->removeChild(messageBox);
        messageBox = nullptr;
        break;
      case ButtonAction::AnimateHide:
        messageBox->animateHide(WAnimation(AnimationEffect::Pop, TimingFunction::Linear, 500));
        break;
      case ButtonAction::Hide:
        messageBox->hide();
        break;
      default:
        break;
      }
    }

    WMessageBox* testModalMessageBox_;
    WMessageBox* testNonModalMessageBox_;
  };
}

BOOST_AUTO_TEST_SUITE(selenium_wmessagebox)

SELENIUM_TEST(wmessagebox_modal_give_back_focus, WMessageBoxTestApp)
  auto triggerModalButton = api.getElement(SeleniumAPI::FindBy::ID, "trigger-modal-button");
  BOOST_REQUIRE(triggerModalButton.has_value());

  triggerModalButton->click();

  std::map<std::string, std::optional<Element>> buttons{
    {"modal-delete-button", std::nullopt},
    {"modal-hide-button", std::nullopt},
    {"modal-animate-hide-button", std::nullopt}
  };
  const std::vector<std::string> buttonIds {
    "modal-hide-button",
    "modal-animate-hide-button",
    "modal-delete-button"
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

    // reset for the next iteration
    if (buttonId != "modal-delete-button") {
      triggerModalButton->click();
      for (const auto& visibleButtonId : buttonIds) {
        BOOST_REQUIRE(wait.until([&]{ return buttons[visibleButtonId]->isVisible(); }));
      }
      BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerModalButton; }));
    }
  }

END_SELENIUM_TEST

SELENIUM_TEST(wmessagebox_non_modal_give_back_focus_to_trigger_button, WMessageBoxTestApp)
  auto triggerNonModalButton = api.getElement(SeleniumAPI::FindBy::ID, "trigger-non-modal-button");
  BOOST_REQUIRE(triggerNonModalButton.has_value());

  triggerNonModalButton->click();

  std::map<std::string, std::optional<Element>> buttons {
    {"non-modal-delete-button", std::nullopt},
    {"non-modal-hide-button", std::nullopt},
    {"non-modal-animate-hide-button", std::nullopt}
  };
  const std::vector<std::string> buttonIds {
    "non-modal-hide-button",
    "non-modal-animate-hide-button",
    "non-modal-delete-button"
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

    // reset for the next iteration
    if (buttonId != "non-modal-delete-button") {
      triggerNonModalButton->click();
      for (const auto& visibleButtonId : buttonIds)
        BOOST_REQUIRE(wait.until([&]{ return buttons[visibleButtonId]->isVisible(); }));
      BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerNonModalButton; }));
    }
  }

END_SELENIUM_TEST

SELENIUM_TEST(wmessagebox_non_modal_give_back_focus_to_last_focused_element, WMessageBoxTestApp)
  auto triggerNonModalButton = api.getElement(SeleniumAPI::FindBy::ID, "trigger-non-modal-button");
  BOOST_REQUIRE(triggerNonModalButton.has_value());
  auto uselessButton = api.getElement(SeleniumAPI::FindBy::ID, "useless-button");
  BOOST_REQUIRE(uselessButton.has_value());

  triggerNonModalButton->click();

  std::map<std::string, std::optional<Element>> buttons {
    {"non-modal-delete-button", std::nullopt},
    {"non-modal-hide-button", std::nullopt},
    {"non-modal-animate-hide-button", std::nullopt}
  };
  const std::vector<std::string> buttonIds {
    "non-modal-hide-button",
    "non-modal-animate-hide-button",
    "non-modal-delete-button"
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
    BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() ==
      buttons["non-modal-hide-button"]; }));

    buttons[buttonId]->click();
    BOOST_TEST(wait.until([&]{ return api.getActiveElement() == uselessButton; }));

    if (buttonId == "non-modal-animate-hide-button") {
      BOOST_TEST(buttons[buttonId]->isVisible());
      BOOST_REQUIRE(wait.until([&]{ return !buttons[buttonId]->isVisible(); }, std::chrono::seconds(1)));
    }

    // reset for the next iteration
    if (buttonId != "non-modal-delete-button") {
      triggerNonModalButton->click();
      for (const auto& visibleButtonId : buttonIds)
        BOOST_REQUIRE(wait.until([&]{ return buttons[visibleButtonId]->isVisible(); }));
      BOOST_REQUIRE(wait.until([&]{ return api.getActiveElement() != triggerNonModalButton; }));
    }
  }

END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
