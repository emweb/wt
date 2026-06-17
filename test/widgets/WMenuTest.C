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

// Regression test: removeItem() must clear selection (not select the previous
// item) when the currently-selected item is the one being removed.
BOOST_AUTO_TEST_CASE(WMenu_removeItem_clears_selection_of_current_item)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);

  Wt::WMenuItem *item0 = menu->addItem("Item 0", std::make_unique<Wt::WText>("Content 0"));
  Wt::WMenuItem *item1 = menu->addItem("Item 1", std::make_unique<Wt::WText>("Content 1"));
  Wt::WMenuItem *item2 = menu->addItem("Item 2", std::make_unique<Wt::WText>("Content 2"));

  // Select the middle item
  menu->select(item1);
  BOOST_REQUIRE(menu->currentIndex() == 1);

  // Remove the currently-selected item — selection must be cleared, not moved
  auto removed = menu->removeItem(item1);
  BOOST_TEST(menu->currentIndex() == -1);
  BOOST_TEST(menu->currentItem() == nullptr);

  // Remaining items must be accessible at correct indices
  BOOST_TEST(menu->count() == 2);
  BOOST_TEST(menu->itemAt(0) == item0);
  BOOST_TEST(menu->itemAt(1) == item2);
}

// Regression test: removeItem() must decrement current_ (not clear) when
// an item *before* the currently-selected item is removed.
BOOST_AUTO_TEST_CASE(WMenu_removeItem_before_current_keeps_selection)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);

  Wt::WMenuItem *item0 = menu->addItem("Item 0", std::make_unique<Wt::WText>("Content 0"));
  Wt::WMenuItem *item1 = menu->addItem("Item 1", std::make_unique<Wt::WText>("Content 1"));
  Wt::WMenuItem *item2 = menu->addItem("Item 2", std::make_unique<Wt::WText>("Content 2"));

  // Select item2 (index 2)
  menu->select(item2);
  BOOST_REQUIRE(menu->currentIndex() == 2);

  // Remove item0 (before current) — item2 must remain selected at new index 1
  auto removed = menu->removeItem(item0);
  BOOST_TEST(menu->currentItem() == item2);
  BOOST_TEST(menu->currentIndex() == 1);
}

