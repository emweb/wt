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

BOOST_AUTO_TEST_CASE(WMenu_select_propagation_to_parent_and_submenus)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);

  Wt::WMenuItem *item0 = menu->addItem("Item 0", std::make_unique<Wt::WText>("Content 0"));

  auto subMenuPtr_ = std::make_unique<Wt::WMenu>(stack);
  Wt::WMenu* subMenu = subMenuPtr_.get();
  subMenu->addItem("Item 1.1", std::make_unique<Wt::WText>("Content 1.1"));

  auto subSubMenuPtr_ = std::make_unique<Wt::WMenu>(stack);
  Wt::WMenu* subSubMenu = subSubMenuPtr_.get();
  subSubMenu->addItem("Item 1.2.1", std::make_unique<Wt::WText>("Content 1.2.1"));
  subSubMenu->addItem("Item 1.2.2", std::make_unique<Wt::WText>("Content 1.2.2"));
  Wt::WMenuItem *item1_2_3 = subSubMenu->addItem("Item 1.2.3", std::make_unique<Wt::WText>("Content 1.2.3"));

  Wt::WMenuItem *item1_2 = subMenu->addMenu("Item 1.2", std::move(subSubMenuPtr_));
  subMenu->addItem("Item 1.3", std::make_unique<Wt::WText>("Content 1.3"));

  Wt::WMenuItem *item1 = menu->addMenu("Item 1", std::move(subMenuPtr_));
  menu->addItem("Item 2", std::make_unique<Wt::WText>("Content 2"));

  // By default, the first item of the top menu is selected.
  BOOST_TEST(menu->currentItem() == item0);
  BOOST_TEST(subMenu->currentItem() == nullptr);
  BOOST_TEST(subSubMenu->currentItem() == nullptr);
  BOOST_REQUIRE_EQUAL(menu->currentIndex(), 0);
  BOOST_REQUIRE_EQUAL(subMenu->currentIndex(), -1);
  BOOST_REQUIRE_EQUAL(subSubMenu->currentIndex(), -1);

  // Select should be propagated to the parent menus.
  subSubMenu->select(2);
  BOOST_TEST(menu->currentItem() == item1);
  BOOST_TEST(subMenu->currentItem() == item1_2);
  BOOST_TEST(subSubMenu->currentItem() == item1_2_3);
  BOOST_REQUIRE_EQUAL(menu->currentIndex(), 1);
  BOOST_REQUIRE_EQUAL(subMenu->currentIndex(), 1);
  BOOST_REQUIRE_EQUAL(subSubMenu->currentIndex(), 2);

  // Select should not be propagated to the submenus.
  menu->select(0);
  BOOST_TEST(menu->currentItem() == item0);
  BOOST_TEST(subMenu->currentItem() == item1_2);
  BOOST_TEST(subSubMenu->currentItem() == item1_2_3);
  BOOST_REQUIRE_EQUAL(menu->currentIndex(), 0);
  BOOST_REQUIRE_EQUAL(subMenu->currentIndex(), 1);
  BOOST_REQUIRE_EQUAL(subSubMenu->currentIndex(), 2);

  // Select should be propagated to the parent menu even when the index did not change.
  subSubMenu->select(2);
  BOOST_TEST(menu->currentItem() == item1);
  BOOST_TEST(subMenu->currentItem() == item1_2);
  BOOST_TEST(subSubMenu->currentItem() == item1_2_3);
  BOOST_TEST(menu->currentIndex() == 1);
  BOOST_TEST(subMenu->currentIndex() == 1);
  BOOST_TEST(subSubMenu->currentIndex() == 2);

}

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

  auto downloadItem = menu->addItem("download", std::make_unique<Wt::WText>("Download Page"));
  downloadItem->setPathComponent("download/");

  BOOST_TEST(menu->currentIndex() == 0);

  app.setInternalPath("/contact/us",  true);

  BOOST_TEST(menu->currentIndex() == 1);

  // Check that path components ending with a slash are correctly matched
  app.setInternalPath("/download/file",  true);

  BOOST_TEST(menu->currentIndex() == 2);
}

BOOST_AUTO_TEST_CASE(WMenuItem_setContents_at_current_keeps_selection)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);
  menu->setInternalPathEnabled("/");

  Wt::WMenuItem *item0 = menu->addItem("Item 0", std::make_unique<Wt::WText>("Content 0"), Wt::ContentLoading::Eager);
  Wt::WMenuItem *item1 = menu->addItem("Item 1", std::make_unique<Wt::WText>("Content 1"), Wt::ContentLoading::Eager);
  Wt::WMenuItem *item2 = menu->addItem("Item 2", std::make_unique<Wt::WText>("Content 2"), Wt::ContentLoading::Eager);

  menu->select(1);
  BOOST_REQUIRE(menu->currentIndex() == 1);

  item0->triggered().connect([&]() {
    BOOST_TEST(false, "Item 0 should not be triggered");
  });
  item1->triggered().connect([&]() {
    BOOST_TEST(false, "Item 1 should not be triggered");
  });
  item2->triggered().connect([&]() {
    BOOST_TEST(false, "Item 2 should not be triggered");
  });
  menu->itemSelected().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(false, "itemSelected should not be emitted");
  });
  menu->itemSelectRendered().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(false, "itemSelectRendered should not be emitted");
  });
  app.internalPathChanged().connect([&](const std::string &path) {
    BOOST_TEST(false, "Internal path should not change");
  });
  stack->currentWidgetChanged().connect([&](Wt::WWidget *widget) {
    Wt::WText* textWidget = dynamic_cast<Wt::WText*>(widget);
    BOOST_TEST(false, "StackedWidget currentWidgetChanged should not be emitted for widget " << (textWidget ? textWidget->text() : "null"));
  });

  item1->setContents(std::make_unique<Wt::WText>("New Content"), Wt::ContentLoading::Eager);
  BOOST_TEST(menu->currentIndex() == 1);
  BOOST_TEST(menu->currentItem() == item1);
  BOOST_TEST(stack->currentWidget() != nullptr);
  BOOST_TEST(stack->currentWidget() == item1->contents());
}

BOOST_AUTO_TEST_CASE(WMenuItem_setContents_before_current_keeps_selection)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);
  menu->setInternalPathEnabled("/");

  Wt::WMenuItem *item0 = menu->addItem("Item 0", std::make_unique<Wt::WText>("Content 0"), Wt::ContentLoading::Eager);
  Wt::WMenuItem *item1 = menu->addItem("Item 1", std::make_unique<Wt::WText>("Content 1"), Wt::ContentLoading::Eager);
  Wt::WMenuItem *item2 = menu->addItem("Item 2", std::make_unique<Wt::WText>("Content 2"), Wt::ContentLoading::Eager);

  menu->select(2);
  BOOST_REQUIRE(menu->currentIndex() == 2);

  item0->triggered().connect([&]() {
    BOOST_TEST(false, "Item 0 should not be triggered");
  });
  item1->triggered().connect([&]() {
    BOOST_TEST(false, "Item 1 should not be triggered");
  });
  item2->triggered().connect([&]() {
    BOOST_TEST(false, "Item 2 should not be triggered");
  });
  menu->itemSelected().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(false, "itemSelected should not be emitted");
  });
  menu->itemSelectRendered().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(false, "itemSelectRendered should not be emitted");
  });
  app.internalPathChanged().connect([&](const std::string &path) {
    BOOST_TEST(false, "Internal path should not change");
  });
  stack->currentWidgetChanged().connect([&](Wt::WWidget *widget) {
    Wt::WText* textWidget = dynamic_cast<Wt::WText*>(widget);
    BOOST_TEST(false, "StackedWidget currentWidgetChanged should not be emitted for widget " << (textWidget ? textWidget->text() : "null"));
  });

  item1->setContents(std::make_unique<Wt::WText>("New Content"), Wt::ContentLoading::Eager);
  BOOST_TEST(menu->currentIndex() == 2);
  BOOST_TEST(menu->currentItem() == item2);
  BOOST_TEST(stack->currentWidget() != nullptr);
  BOOST_TEST(stack->currentWidget() == item2->contents());
}

BOOST_AUTO_TEST_CASE(WMenuItem_setContents_with_submenus_keeps_selection)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  bool canTriggerEvents = true;

  Wt::WStackedWidget *stack = app.root()->addNew<Wt::WStackedWidget>();
  Wt::WMenu *menu = app.root()->addNew<Wt::WMenu>(stack);
  menu->setInternalPathEnabled("/");

  Wt::WMenuItem *item0 = menu->addItem("Item 0", std::make_unique<Wt::WText>("Content 0"), Wt::ContentLoading::Eager);

  auto subMenuPtr_ = std::make_unique<Wt::WMenu>(stack);
  Wt::WMenu* subMenu = subMenuPtr_.get();
  Wt::WMenuItem *item1_1 = subMenu->addItem("Item 1.1", std::make_unique<Wt::WText>("Content 1.1"), Wt::ContentLoading::Eager);

  auto subSubMenuPtr_ = std::make_unique<Wt::WMenu>(stack);
  Wt::WMenu* subSubMenu = subSubMenuPtr_.get();
  Wt::WMenuItem *item1_2_1 = subSubMenu->addItem("Item 1.2.1", std::make_unique<Wt::WText>("Content 1.2.1"), Wt::ContentLoading::Eager);
  Wt::WMenuItem *item1_2_2 = subSubMenu->addItem("Item 1.2.2", std::make_unique<Wt::WText>("Content 1.2.2"), Wt::ContentLoading::Eager);
  Wt::WMenuItem *item1_2_3 = subSubMenu->addItem("Item 1.2.3", std::make_unique<Wt::WText>("Content 1.2.3"), Wt::ContentLoading::Eager);

  subSubMenu->setInternalPathEnabled("/");

  Wt::WMenuItem *item1_2 = subMenu->addMenu("Item 1.2", std::move(subSubMenuPtr_));
  Wt::WMenuItem *item1_3 = subMenu->addItem("Item 1.3", std::make_unique<Wt::WText>("Content 1.3"), Wt::ContentLoading::Eager);
  subMenu->setInternalPathEnabled("/");

  Wt::WMenuItem *item1 = menu->addMenu("Item 1", std::move(subMenuPtr_));
  Wt::WMenuItem *item2 = menu->addItem("Item 2", std::make_unique<Wt::WText>("Content 2"), Wt::ContentLoading::Eager);

  subSubMenu->select(2);
  BOOST_REQUIRE(menu->currentIndex() == 1);
  BOOST_REQUIRE(subMenu->currentIndex() == 1);
  BOOST_REQUIRE(subSubMenu->currentIndex() == 2);

  item0->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 0 should not be triggered");
  });
  item1->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 1 should not be triggered");
  });
  item1_1->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 1.1 should not be triggered");
  });
  item1_2->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 1.2 should not be triggered");
  });
  item1_2_1->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 1.2.1 should not be triggered");
  });
  item1_2_2->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 1.2.2 should not be triggered");
  });
  item1_2_3->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 1.2.3 should not be triggered");
  });
  item1_3->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 1.3 should not be triggered");
  });
  item2->triggered().connect([&]() {
    BOOST_TEST(canTriggerEvents, "Item 2 should not be triggered");
  });
  menu->itemSelected().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(canTriggerEvents, "itemSelected should not be emitted");
  });
  menu->itemSelectRendered().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(canTriggerEvents, "itemSelectRendered should not be emitted for item " << item->text());
  });
  subMenu->itemSelected().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(canTriggerEvents, "submenu itemSelected should not be emitted");
  });
  subMenu->itemSelectRendered().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(canTriggerEvents, "submenu itemSelectRendered should not be emitted for item " << item->text());
  });
  subSubMenu->itemSelected().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(canTriggerEvents, "subsubmenu itemSelected should not be emitted");
  });
  subSubMenu->itemSelectRendered().connect([&](Wt::WMenuItem *item) {
    BOOST_TEST(canTriggerEvents, "subsubmenu itemSelectRendered should not be emitted for item " << item->text());
  });
  app.internalPathChanged().connect([&](const std::string &path) {
    BOOST_TEST(canTriggerEvents, "Internal path should not change");
  });
  stack->currentWidgetChanged().connect([&](Wt::WWidget *widget) {
    Wt::WText* textWidget = dynamic_cast<Wt::WText*>(widget);
    BOOST_TEST(canTriggerEvents, "StackedWidget currentWidgetChanged should not be emitted for widget " << (textWidget ? textWidget->text() : "null"));
  });

  canTriggerEvents = false;

  item0->setContents(std::make_unique<Wt::WText>("New Content"), Wt::ContentLoading::Eager);
  BOOST_TEST(menu->currentIndex() == 1);
  BOOST_TEST(menu->currentItem() == item1);
  BOOST_TEST(subMenu->currentIndex() == 1);
  BOOST_TEST(subMenu->currentItem() == item1_2);
  BOOST_TEST(subSubMenu->currentIndex() == 2);
  BOOST_TEST(subSubMenu->currentItem() == item1_2_3);
  BOOST_TEST(stack->currentWidget() != nullptr);
  BOOST_TEST(stack->currentWidget() == item1_2_3->contents());

  canTriggerEvents = true;

  subSubMenu->select(2);
  BOOST_REQUIRE(menu->currentIndex() == 1);
  BOOST_REQUIRE(subMenu->currentIndex() == 1);
  BOOST_REQUIRE(subSubMenu->currentIndex() == 2);

  canTriggerEvents = false;

  item1_2->setContents(std::make_unique<Wt::WText>("New Content"), Wt::ContentLoading::Eager);
  BOOST_TEST(menu->currentIndex() == 1);
  BOOST_TEST(menu->currentItem() == item1);
  BOOST_TEST(subMenu->currentIndex() == 1);
  BOOST_TEST(subMenu->currentItem() == item1_2);
  BOOST_TEST(subSubMenu->currentIndex() == 2);
  BOOST_TEST(subSubMenu->currentItem() == item1_2_3);
  BOOST_TEST(stack->currentWidget() != nullptr);
  BOOST_TEST(stack->currentWidget() == item1_2_3->contents());

  canTriggerEvents = true;

  subSubMenu->select(2);
  BOOST_REQUIRE(menu->currentIndex() == 1);
  BOOST_REQUIRE(subMenu->currentIndex() == 1);
  BOOST_REQUIRE(subSubMenu->currentIndex() == 2);

  canTriggerEvents = false;

  item1_2_1->setContents(std::make_unique<Wt::WText>("New Content"), Wt::ContentLoading::Eager);
  BOOST_TEST(menu->currentIndex() == 1);
  BOOST_TEST(menu->currentItem() == item1);
  BOOST_TEST(subMenu->currentIndex() == 1);
  BOOST_TEST(subMenu->currentItem() == item1_2);
  BOOST_TEST(subSubMenu->currentIndex() == 2);
  BOOST_TEST(subSubMenu->currentItem() == item1_2_3);
  BOOST_TEST(stack->currentWidget() != nullptr);
  BOOST_TEST(stack->currentWidget() == item1_2_3->contents());

  canTriggerEvents = true;

  subMenu->select(2);
  BOOST_REQUIRE(menu->currentIndex() == 1);
  BOOST_REQUIRE(subMenu->currentIndex() == 2);

  canTriggerEvents = false;

  item1_2_2->setContents(std::make_unique<Wt::WText>("New Content"), Wt::ContentLoading::Eager);
  BOOST_TEST(menu->currentIndex() == 1);
  BOOST_TEST(menu->currentItem() == item1);
  BOOST_TEST(subMenu->currentIndex() == 2);
  BOOST_TEST(subMenu->currentItem() == item1_3);
  BOOST_TEST(stack->currentWidget() != nullptr);
  BOOST_TEST(stack->currentWidget() == item1_3->contents());

  canTriggerEvents = true;

  menu->select(2);
  BOOST_REQUIRE(menu->currentIndex() == 2);

  canTriggerEvents = false;

  item1_2_2->setContents(std::make_unique<Wt::WText>("New New Content"), Wt::ContentLoading::Eager);
  BOOST_TEST(menu->currentIndex() == 2);
  BOOST_TEST(menu->currentItem() == item2);
  BOOST_TEST(stack->currentWidget() != nullptr);
  BOOST_TEST(stack->currentWidget() == item2->contents());
}

