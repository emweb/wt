/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>
#include <Wt/WApplication.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WDialog.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>

namespace {

class FocusDialog : public Wt::WDialog
{
public:
  using Wt::WDialog::render;
};

class SecondFieldFocusContainer : public Wt::WContainerWidget
{
public:
  bool setFirstFocus() override
  {
    return widget(1)->setFirstFocus();
  }
};

class SuppressFocusContainer : public Wt::WContainerWidget
{
public:
  bool setFirstFocus() override
  {
    return false;
  }
};

}

BOOST_AUTO_TEST_CASE(test_WDialog_disable_title_on_layout_in_titlebar)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto dialog = std::make_unique<Wt::WDialog>();

  dialog->titleBar()->setLayout(std::make_unique<Wt::WHBoxLayout>());
  dialog->setWindowTitle("Dialog with layouted title bar");

  BOOST_TEST(dialog->windowTitle().empty());
}

BOOST_AUTO_TEST_CASE(test_WDialog_disable_closable_on_layout_in_titlebar)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto dialog = std::make_unique<Wt::WDialog>();

  dialog->titleBar()->setLayout(std::make_unique<Wt::WHBoxLayout>());
  dialog->setClosable(true);

  BOOST_TEST(!dialog->closable());
}

BOOST_AUTO_TEST_CASE(test_WDialog_remove_title_on_layout_in_titlebar)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto dialog = std::make_unique<Wt::WDialog>();
  dialog->setWindowTitle("Dialog with layouted title bar");

  // Ensure the title is set before adding the layout.
  BOOST_REQUIRE(dialog->windowTitle().toUTF8() == "Dialog with layouted title bar");

  dialog->titleBar()->setLayout(std::make_unique<Wt::WHBoxLayout>());

  BOOST_TEST(dialog->windowTitle().empty());
}

BOOST_AUTO_TEST_CASE(test_WDialog_remove_closable_on_layout_in_titlebar)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto dialog = std::make_unique<Wt::WDialog>();
  dialog->setClosable(true);

  // Ensure the dialog is closable before adding the layout.
  BOOST_REQUIRE(dialog->closable());

  dialog->titleBar()->setLayout(std::make_unique<Wt::WHBoxLayout>());

  BOOST_TEST(!dialog->closable());
}

BOOST_AUTO_TEST_CASE(test_WDialog_focus_selection)
{
  for (bool modal : {true, false}) {
    BOOST_TEST_CONTEXT("modal = " << modal) {
      Wt::Test::WTestEnvironment testEnv;
      Wt::WApplication app(testEnv);
      Wt::WDialog dialog;
      dialog.setModal(modal);
      auto title = dialog.titleBar()->addNew<Wt::WLineEdit>();
      auto body = dialog.contents()->addNew<Wt::WLineEdit>();
      auto footer = dialog.footer()->addNew<Wt::WPushButton>("OK");
      dialog.show();

      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(title->hasFocus());

      title->hide();
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(body->hasFocus());

      body->setDisabled(true);
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(footer->hasFocus());

      // A previously focused fallback must not precede descendants.
      footer->hide();
      BOOST_REQUIRE(dialog.setFirstFocus());
      footer->show();
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(footer->hasFocus());
    }
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_focus_footer_skips_unavailable_children)
{
  for (bool modal : {true, false}) {
    BOOST_TEST_CONTEXT("modal = " << modal) {
      Wt::Test::WTestEnvironment testEnv;
      Wt::WApplication app(testEnv);
      Wt::WDialog dialog;
      dialog.setModal(modal);
      auto hidden = dialog.contents()->addNew<Wt::WContainerWidget>();
      hidden->addNew<Wt::WLineEdit>();
      hidden->hide();
      auto disabled = dialog.contents()->addNew<Wt::WContainerWidget>();
      disabled->addNew<Wt::WLineEdit>();
      disabled->setDisabled(true);
      dialog.footer()->addNew<Wt::WPushButton>("Hidden")->hide();
      dialog.footer()->addNew<Wt::WPushButton>("Disabled")->setDisabled(true);
      auto button = dialog.footer()->addNew<Wt::WPushButton>("OK");
      dialog.show();

      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(button->hasFocus());
    }
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_focus_virtual_selection_before_footer_buttons)
{
  for (bool modal : {true, false}) {
    Wt::Test::WTestEnvironment testEnv;
    Wt::WApplication app(testEnv);
    Wt::WDialog dialog;
    dialog.setModal(modal);
    dialog.titleBar()->addNew<Wt::WPushButton>("Close");
    auto bodyContainer = dialog.contents()->addNew<Wt::WContainerWidget>();
    auto body = bodyContainer->addNew<Wt::WLineEdit>();
    auto button = dialog.footer()->addNew<Wt::WPushButton>("OK");
    auto nested = dialog.footer()->addNew<Wt::WContainerWidget>();
    nested->addNew<Wt::WPushButton>("Nested");
    auto footerInput = nested->addNew<Wt::WLineEdit>();
    dialog.footer()->addNew<Wt::WPushButton>("Cancel");
    dialog.show();

    BOOST_REQUIRE(dialog.setFirstFocus());
    BOOST_TEST(body->hasFocus());
    body->hide();
    BOOST_REQUIRE(dialog.setFirstFocus());
    BOOST_TEST(footerInput->hasFocus());
    footerInput->hide();
    BOOST_REQUIRE(dialog.setFirstFocus());
    BOOST_TEST(button->hasFocus());
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_focus_custom_second_field)
{
  for (bool modal : {true, false}) {
    for (bool automatic : {true, false}) {
      BOOST_TEST_CONTEXT("modal = " << modal << ", automatic = " << automatic) {
        Wt::Test::WTestEnvironment testEnv;
        Wt::WApplication app(testEnv);
        FocusDialog dialog;
        dialog.setModal(modal);
        auto body = dialog.contents()->addNew<SecondFieldFocusContainer>();
        auto first = body->addNew<Wt::WLineEdit>();
        auto second = body->addNew<Wt::WLineEdit>();
        dialog.footer()->addNew<Wt::WPushButton>("OK");
        dialog.show();

        if (automatic) {
          dialog.render(Wt::RenderFlag::Full);
        } else {
          BOOST_REQUIRE(dialog.setFirstFocus());
        }
        BOOST_TEST(second->hasFocus());
        BOOST_TEST(!first->hasFocus());
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_focus_custom_suppression)
{
  for (bool modal : {true, false}) {
    for (bool automatic : {true, false}) {
      for (bool withFooter : {true, false}) {
        BOOST_TEST_CONTEXT("modal = " << modal << ", automatic = " << automatic
                           << ", withFooter = " << withFooter) {
          Wt::Test::WTestEnvironment testEnv;
          Wt::WApplication app(testEnv);
          FocusDialog dialog;
          dialog.setModal(modal);
          auto body = dialog.contents()->addNew<SuppressFocusContainer>();
          auto input = body->addNew<Wt::WLineEdit>();
          Wt::WPushButton *button = nullptr;
          if (withFooter) {
            button = dialog.footer()->addNew<Wt::WPushButton>("OK");
          }
          dialog.show();

          if (automatic) {
            dialog.render(Wt::RenderFlag::Full);
          } else {
            BOOST_REQUIRE(dialog.setFirstFocus());
          }
          BOOST_TEST(!input->hasFocus());
          if (button) {
            BOOST_TEST(button->hasFocus());
          } else {
            auto focused = dialog.findById(app.focus());
            BOOST_REQUIRE(focused);
            BOOST_TEST(focused->canReceiveFocus());
            BOOST_TEST(focused->hasStyleClass("dialog-layout"));
          }
        }
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_focus_fallback)
{
  for (bool modal : {true, false}) {
    BOOST_TEST_CONTEXT("modal = " << modal) {
      Wt::Test::WTestEnvironment testEnv;
      Wt::WApplication app(testEnv);
      Wt::WDialog dialog;
      dialog.setModal(modal);
      dialog.show();

      BOOST_REQUIRE(dialog.setFirstFocus());
      auto focused = dialog.findById(app.focus());
      BOOST_REQUIRE(focused);
      BOOST_TEST(focused->canReceiveFocus());

      // Repeated selection still succeeds after focusing the fallback.
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(focused->hasFocus());
    }
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_autofocus_preserves_child_focus)
{
  for (bool modal : {true, false}) {
    Wt::Test::WTestEnvironment testEnv;
    Wt::WApplication app(testEnv);
    FocusDialog dialog;
    dialog.setModal(modal);
    dialog.contents()->addNew<Wt::WLineEdit>();
    auto button = dialog.footer()->addNew<Wt::WPushButton>("OK");
    button->setFocus();
    dialog.show();
    dialog.render(Wt::RenderFlag::Full);

    BOOST_TEST(button->hasFocus());
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_autofocus_disabled)
{
  for (bool modal : {true, false}) {
    Wt::Test::WTestEnvironment testEnv;
    Wt::WApplication app(testEnv);
    auto outside = app.root()->addNew<Wt::WLineEdit>();
    outside->setFocus();
    FocusDialog dialog;
    dialog.setModal(modal);
    dialog.setAutoFocus(false);
    auto input = dialog.contents()->addNew<Wt::WLineEdit>();
    dialog.show();
    dialog.render(Wt::RenderFlag::Full);

    BOOST_TEST(outside->hasFocus());
    BOOST_REQUIRE(dialog.setFirstFocus());
    BOOST_TEST(input->hasFocus());
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_autofocus_skips_unavailable_focus)
{
  for (bool modal : {true, false}) {
    for (bool hidden : {true, false}) {
      Wt::Test::WTestEnvironment testEnv;
      Wt::WApplication app(testEnv);
      FocusDialog dialog;
      dialog.setModal(modal);
      auto unavailable = dialog.contents()->addNew<Wt::WLineEdit>();
      auto available = dialog.contents()->addNew<Wt::WLineEdit>();
      unavailable->setFocus();
      if (hidden) {
        unavailable->hide();
      } else {
        unavailable->setDisabled(true);
      }
      dialog.show();
      dialog.render(Wt::RenderFlag::Full);

      BOOST_TEST(available->hasFocus());
    }
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_focus_unavailable_dialog)
{
  for (bool modal : {true, false}) {
    Wt::Test::WTestEnvironment testEnv;
    Wt::WApplication app(testEnv);
    auto outside = app.root()->addNew<Wt::WLineEdit>();
    outside->setFocus();
    Wt::WDialog dialog;
    dialog.setModal(modal);
    dialog.contents()->addNew<Wt::WLineEdit>();

    BOOST_TEST(!dialog.setFirstFocus());
    BOOST_TEST(outside->hasFocus());

    dialog.show();
    dialog.setDisabled(true);
    BOOST_TEST(!dialog.setFirstFocus());
    BOOST_TEST(outside->hasFocus());
  }
}

BOOST_AUTO_TEST_CASE(test_WDialog_close_icon_fallback)
{
  for (bool bootstrap : {false, true}) {
    for (bool modal : {false, true}) {
      Wt::Test::WTestEnvironment testEnv;
      Wt::WApplication app(testEnv);
      if (bootstrap) {
        app.setTheme(std::make_shared<Wt::WBootstrap5Theme>());
      }
      Wt::WDialog dialog;
      dialog.setModal(modal);
      dialog.setClosable(true);
      auto children = dialog.titleBar()->children();
      auto close = bootstrap ? children.back() : children.front();
      auto body = dialog.contents()->addNew<Wt::WLineEdit>();
      auto footer = dialog.footer()->addNew<Wt::WPushButton>("OK");
      dialog.show();
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(body->hasFocus());
      body->hide();
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(footer->hasFocus());
      footer->hide();
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(close->hasFocus());
      BOOST_TEST(close->canReceiveFocus());
      body->show();
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(body->hasFocus());
      body->hide();
      close->setDisabled(true);
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(!close->hasFocus());
      close->setDisabled(false);
      dialog.setTitleBarEnabled(false);
      BOOST_REQUIRE(dialog.setFirstFocus());
      BOOST_TEST(!close->hasFocus());
    }
  }
}
