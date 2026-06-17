/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>
#include <Wt/WApplication.h>
#include <Wt/WMenu.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>

BOOST_AUTO_TEST_CASE(WMenu_addItem_change_index_for_internal_path_match)
{
  Wt::Test::WTestEnvironment testEnv;
  testEnv.setInternalPath("/links");
  Wt::WApplication app(testEnv);

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);
  menu->setInternalPathEnabled("/");

  BOOST_TEST(menu->currentIndex() == -1);

  menu->addItem("dashboard", std::make_unique<Wt::WText>("Dashboard"));

  BOOST_TEST(menu->currentIndex() == 0);

  // Check that adding an item with the internal path changes the index
  menu->addItem("links", std::make_unique<Wt::WText>("Links"));

  BOOST_TEST(menu->currentIndex() == 1);

  // Check that adding an item with an empty path component does not change the index
  int previousIndex = menu->currentIndex();
  menu->addItem("", std::make_unique<Wt::WText>("Account Settings Page"));

  BOOST_TEST(menu->currentIndex() == previousIndex);
}

BOOST_AUTO_TEST_CASE(WMenu_internal_path_matching_segment_boundary)
{
  Wt::Test::WTestEnvironment testEnv;
  testEnv.setInternalPath("/contact-us");
  Wt::WApplication app(testEnv);

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);
  menu->setInternalPathEnabled("/");

  menu->addItem("dashboard", std::make_unique<Wt::WText>("Dashboard"));

  BOOST_TEST(menu->currentIndex() == 0);

  menu->addItem("contact", std::make_unique<Wt::WText>("Contact Page"));

  BOOST_TEST(menu->currentIndex() == 0);

  app.setInternalPath("/contact/us",  true);

  BOOST_TEST(menu->currentIndex() == 1);
}

