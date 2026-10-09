/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WAnimation.h>
#include <Wt/WApplication.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <map>
#include <vector>

using namespace Wt;
using namespace Selenium;

namespace {
  class WDialogFocusRestoreTestApp : public WApplication
  {
  public:
    WDialogFocusRestoreTestApp(const WEnvironment& env)
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

enum class FocusContent {
  Close, Title, Body, Footer, Unavailable, Empty, Explicit, Manual, NoAutoFocus,
  CustomSelection, CustomSuppression, CustomSuppressionFallback
};

class SecondFieldFocusContainer : public WContainerWidget
{
public:
  bool setFirstFocus() override
  {
    return widget(1)->setFirstFocus();
  }
};

class SuppressFocusContainer : public WContainerWidget
{
public:
  bool setFirstFocus() override
  {
    return false;
  }
};

class WDialogTestApp : public WApplication
{
public:
  WDialogTestApp(const WEnvironment& env)
    : WApplication(env)
  {
    outside_ = root()->addNew<WLineEdit>();
    outside_->setId("outside");
    outside_->setFocus();
  }

  void showDialog(bool modal, FocusContent content, bool animated, bool hidden = false)
  {
    if (dialog_) {
      dialog_->setAttributeValue("data-focus-test", "previous-dialog");
    }
    dialog_ = addChild(std::make_unique<WDialog>("Focus test"));
    dialog_->setAttributeValue("data-focus-test", "dialog");
    dialog_->setModal(modal);
    if (content == FocusContent::Close) {
      dialog_->setClosable(true);
      for (auto child : dialog_->titleBar()->children()) {
        if (child->hasStyleClass("closeicon") || child->hasStyleClass("btn-close")) {
          child->setAttributeValue("data-focus-test", "close-icon");
        }
      }
    }
    dialog_->contents()->addNew<WText>("Dialog contents");
    if (content == FocusContent::Title) {
      dialog_->titleBar()->addNew<WLineEdit>()->setAttributeValue("data-focus-test", "title-input");
    }
    if (content == FocusContent::Unavailable) {
      auto hidden = dialog_->contents()->addNew<WContainerWidget>();
      hidden->addNew<WLineEdit>();
      hidden->hide();
      auto disabled = dialog_->contents()->addNew<WContainerWidget>();
      disabled->addNew<WLineEdit>();
      disabled->setDisabled(true);
      dialog_->footer()->addNew<WPushButton>("Hidden")->hide();
      dialog_->footer()->addNew<WPushButton>("Disabled")->setDisabled(true);
    }
    if (content == FocusContent::Title || content == FocusContent::Body
        || content == FocusContent::Explicit || content == FocusContent::NoAutoFocus) {
      dialog_->contents()->addNew<WLineEdit>()->setAttributeValue("data-focus-test", "body-input");
    }
    if (content == FocusContent::CustomSelection) {
      auto body = dialog_->contents()->addNew<SecondFieldFocusContainer>();
      body->addNew<WLineEdit>()->setAttributeValue("data-focus-test", "first-input");
      body->addNew<WLineEdit>()->setAttributeValue("data-focus-test", "second-input");
    }
    if (content == FocusContent::CustomSuppression
        || content == FocusContent::CustomSuppressionFallback) {
      auto body = dialog_->contents()->addNew<SuppressFocusContainer>();
      body->addNew<WLineEdit>()->setAttributeValue("data-focus-test", "suppressed-input");
    }
    if (content != FocusContent::Close && content != FocusContent::Empty
        && content != FocusContent::CustomSuppressionFallback) {
      auto button = dialog_->footer()->addNew<WPushButton>("OK");
      button->setAttributeValue("data-focus-test", "footer-button");
      if (content == FocusContent::Explicit) {
        button->setFocus();
      }
    }
    dialog_->setAutoFocus(content != FocusContent::NoAutoFocus
                          && content != FocusContent::Manual);
    if (!hidden) {
      showAgain(animated);
    }
    if (content == FocusContent::Manual) {
      dialog_->setFirstFocus();
    }
  }

  void hideDialog()
  {
    dialog_->hide();
    outside_->setFocus();
  }

  void showAgain(bool animated)
  {
    dialog_->animateShow(animated
      ? WAnimation(AnimationEffect::Fade, TimingFunction::Linear, 100)
      : WAnimation());
  }

private:
  WLineEdit *outside_;

protected:
  WDialog *dialog_ = nullptr;
};

class InitialDialogTestApp : public WDialogTestApp
{
public:
  InitialDialogTestApp(const WEnvironment& env)
    : WDialogTestApp(env)
  {
    showDialog(true, FocusContent::Body, false);
  }
};

class HiddenDialogTestApp : public WDialogTestApp
{
public:
  HiddenDialogTestApp(const WEnvironment& env)
    : WDialogTestApp(env)
  {
    showDialog(true, FocusContent::Body, false, true);
  }
};

class StatelessDialogTestApp : public WDialogTestApp
{
public:
  StatelessDialogTestApp(const WEnvironment& env)
    : WDialogTestApp(env)
  {
    showDialog(false, FocusContent::Body, false, true);
    auto show = root()->addNew<WPushButton>("Show");
    show->setId("show-dialog");
    show->clicked().connect(dialog_, &WWidget::show);
    auto hide = root()->addNew<WPushButton>("Hide");
    hide->setId("hide-dialog");
    hide->clicked().connect(dialog_, &WWidget::hide);
  }
};

bool hasFocus(SeleniumAPI& api, const std::string& expected)
{
  std::string selector = "[data-focus-test=dialog] [data-focus-test='" + expected + "']";
  if (expected == "dialog") {
    selector = "[data-focus-test=dialog] .dialog-layout";
  } else if (expected == "outside") {
    selector = "#outside";
  } else if (expected.empty()) {
    selector = "body";
  }
  auto element = api.getElement(SeleniumAPI::FindBy::CSS_SELECTOR, selector);
  auto active = api.getActiveElement();
  return element && active && *element == *active;
}

bool dialogVisible(SeleniumAPI& api)
{
  auto dialog = api.getElement(SeleniumAPI::FindBy::CSS_SELECTOR,
                               "[data-focus-test=dialog]");
  return dialog && dialog->isVisible();
}

void checkFocus(SeleniumAPI& api, const std::string& expected)
{
  api.waitForAnimations();
  BOOST_TEST(hasFocus(api, expected));
}

}

BOOST_AUTO_TEST_SUITE(selenium_wdialog)

SELENIUM_TEST(wdialog_modal_give_back_focus, WDialogFocusRestoreTestApp)
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

SELENIUM_TEST(wdialog_non_modal_give_back_focus, WDialogFocusRestoreTestApp)
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

SELENIUM_TEST(wdialog_non_modal_give_back_focus_to_last_focused, WDialogFocusRestoreTestApp)
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

SELENIUM_TEST(wdialog_focus, WDialogTestApp)
  for (bool bootstrap : {false, true}) {
    if (bootstrap) {
      updateApplication([](WDialogTestApp *app) {
        app->setTheme(std::make_shared<WBootstrap5Theme>());
      });
    }
    for (bool modal : {true, false}) {
      for (bool animated : {false, true}) {
        for (auto content : {FocusContent::Close, FocusContent::Title, FocusContent::Body,
                            FocusContent::Footer, FocusContent::Unavailable,
                            FocusContent::Empty, FocusContent::Explicit,
                            FocusContent::Manual, FocusContent::NoAutoFocus}) {
          BOOST_TEST_CONTEXT("bootstrap = " << bootstrap << ", modal = " << modal
                            << ", animated = " << animated
                            << ", content = " << static_cast<int>(content)) {
            updateApplication([=](WDialogTestApp *app) {
              app->showDialog(modal, content, animated);
            });
            BOOST_REQUIRE(wait.until([&] { return dialogVisible(api); }));
            std::string expected = "footer-button";
            if (content == FocusContent::Close) {
              expected = "close-icon";
            } else if (content == FocusContent::Title) {
              expected = "title-input";
            } else if (content == FocusContent::Body) {
              expected = "body-input";
            } else if (content == FocusContent::Empty) {
              expected = "dialog";
            } else if (content == FocusContent::NoAutoFocus) {
              expected = modal ? "" : "outside";
            }
            checkFocus(api, expected);
            updateApplication([](WDialogTestApp *app) { app->hideDialog(); });
            BOOST_REQUIRE(wait.until([&] { return !dialogVisible(api) && hasFocus(api, "outside"); }));
          }
        }
      }
    }
  }
END_SELENIUM_TEST

SELENIUM_TEST(wdialog_focus_custom_selection, WDialogTestApp)
  for (bool modal : {true, false}) {
    for (auto content : {FocusContent::CustomSelection,
                         FocusContent::CustomSuppression,
                         FocusContent::CustomSuppressionFallback}) {
      BOOST_TEST_CONTEXT("modal = " << modal
                         << ", content = " << static_cast<int>(content)) {
        updateApplication([=](WDialogTestApp *app) {
          app->showDialog(modal, content, false);
        });
        BOOST_REQUIRE(wait.until([&] { return dialogVisible(api); }));
        std::string expected = "footer-button";
        if (content == FocusContent::CustomSelection) {
          expected = "second-input";
        } else if (content == FocusContent::CustomSuppressionFallback) {
          expected = "dialog";
        }
        checkFocus(api, expected);
      }
    }
  }
END_SELENIUM_TEST

SELENIUM_TEST(wdialog_close_icon_keyboard, WDialogTestApp)
  for (bool bootstrap : {false, true}) {
    if (bootstrap) {
      updateApplication([](WDialogTestApp *app) {
        app->setTheme(std::make_shared<WBootstrap5Theme>());
      });
    }
    for (bool modal : {false, true}) {
      for (const std::string key : {std::string("\xee\x80\x87"), std::string(" ")}) {
        updateApplication([=](WDialogTestApp *app) {
          app->showDialog(modal, FocusContent::Close, false);
        });
        BOOST_REQUIRE(wait.until([&] { return dialogVisible(api); }));
        checkFocus(api, "close-icon");
        auto close = api.getElement(SeleniumAPI::FindBy::CSS_SELECTOR,
                                    "[data-focus-test=dialog] [data-focus-test=close-icon]");
        BOOST_REQUIRE(close.has_value());
        BOOST_REQUIRE(close->sendKeys(key));
        BOOST_REQUIRE(wait.until([&] { return !dialogVisible(api); }));
      }
      updateApplication([](WDialogTestApp *app) { app->hideDialog(); });
      BOOST_REQUIRE(wait.until([&] { return !dialogVisible(api); }));
    }
  }
END_SELENIUM_TEST

SELENIUM_TEST(wdialog_focus_initial, InitialDialogTestApp)
  BOOST_REQUIRE(wait.until([&] { return dialogVisible(api); }));
  checkFocus(api, "body-input");
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
