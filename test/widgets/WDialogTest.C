/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>
#include <Wt/WApplication.h>
#include <Wt/WDialog.h>
#include <Wt/WHBoxLayout.h>

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
