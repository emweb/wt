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

// Regression test: setContents() on the currently-selected item must not
// change the active selection.
BOOST_AUTO_TEST_CASE(WMenuItem_setContents_keeps_current_selection)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);

  menu->addItem("Item 0", std::make_unique<Wt::WText>("Content 0"));
  Wt::WMenuItem *item1 = menu->addItem("Item 1", std::make_unique<Wt::WText>("Content 1"));
  menu->addItem("Item 2", std::make_unique<Wt::WText>("Content 2"));

  menu->select(item1);
  BOOST_REQUIRE(menu->currentIndex() == 1);

  item1->setContents(std::make_unique<Wt::WText>("New content 1"));

  BOOST_TEST(menu->currentItem() == item1);
  BOOST_TEST(menu->currentIndex() == 1);
}

