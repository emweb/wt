/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Test/WTestEnvironment.h"

#include <Wt/WApplication.h>
#include <Wt/WBootstrap2Theme.h>
#include <Wt/WBootstrap3Theme.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WCssTheme.h>
#include <Wt/WDateEdit.h>
#include <Wt/WDialog.h>
#include <Wt/WInPlaceEdit.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMenu.h>
#include <Wt/WMessageBox.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WPanel.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WSplitButton.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTimeEdit.h>
#include <Wt/WToolBar.h>


#include "thirdparty/rapidxml/rapidxml.hpp"
#include "thirdparty/rapidxml/rapidxml_print.hpp"
#include "thirdparty/rapidxml/rapidxml_utils.hpp"

#include "web/DomElement.h"

#include <iterator>
#include <memory>
#include <sstream>
#include <tuple>

using namespace Wt;

namespace {

void resetStream(std::stringstream& stream)
{
  stream.str("");
  stream.clear();
}

void strip(std::string& s)
{
  s.erase(0, s.find_first_not_of(" \t\n\r"));
  s.erase(s.find_last_not_of(" \t\n\r") + 1);
}

std::unique_ptr<WDateEdit> createWDateEdit()
{
  return std::make_unique<WDateEdit>();
}

std::unique_ptr<WDialog> createWDialog()
{
  auto dialog = std::make_unique<WDialog>();
  dialog->setWindowTitle("title");
  dialog->setTitleBarEnabled(true);
  dialog->contents()->addWidget(std::make_unique<WText>("content"));
  dialog->footer()->addWidget(std::make_unique<WText>("footer"));
  dialog->setMovable(true);
  dialog->setClosable(true);
  return dialog;
}

std::unique_ptr<WInPlaceEdit> createWInPlaceEdit()
{
  auto inPlaceEdit = std::make_unique<WInPlaceEdit>();
  inPlaceEdit->setText("text");
  inPlaceEdit->setButtonsEnabled(true);
  return inPlaceEdit;
}

std::unique_ptr<WMenu> createWMenu()
{
  auto menu = std::make_unique<Wt::WMenu>();
  auto item1 = menu->addItem("item 1", nullptr, ContentLoading::Eager);
  item1->setCheckable(true);
  item1->setIcon("icon");
  item1->setCloseable(true);
  auto item2 = menu->addItem("item 2", nullptr, ContentLoading::Eager);
  item2->setCheckable(true);
  item2->setIcon("icon");
  item2->setCloseable(true);
  menu->select(1);
  return menu;
}

std::unique_ptr<WMessageBox> createWMessageBox()
{
  auto messageBox = std::make_unique<WMessageBox>();
  messageBox->setText("text");
  messageBox->setIcon(Wt::Icon::Information);
  return messageBox;
}

std::unique_ptr<WNavigationBar> createWNavigationBar()
{
  auto navBar = std::make_unique<WNavigationBar>();
  navBar->setResponsive(true);
  navBar->setTitle("Title", "https://www.google.com");

  auto leftMenu = std::make_unique<Wt::WMenu>();
  leftMenu->addItem("item 1");
  leftMenu->addItem("item 2");
  navBar->addMenu(std::move(leftMenu));

  navBar->addSearch(std::make_unique<Wt::WLineEdit>(), AlignmentFlag::Right);
  navBar->addFormField(std::make_unique<Wt::WLineEdit>(), AlignmentFlag::Right);

  return navBar;
}

std::unique_ptr<WPanel> createWPanel()
{
  auto panel = std::make_unique<WPanel>();
  panel->setTitle("Test Panel");
  panel->setCollapsible(true);
  panel->setCentralWidget(std::make_unique<WText>("c"));
  return panel;
}

std::unique_ptr<WPushButton> createWPushButton()
{
  auto popupPtr = std::make_unique<Wt::WPopupMenu>();
  auto popup = popupPtr.get();

  popup->addItem("item 1");
  popup->addItem("item 2");

  auto btn = std::make_unique<WPushButton>("btn");
  btn->setMenu(std::move(popupPtr));
  return btn;
}

std::unique_ptr<WSplitButton> createWSplitButton()
{
  auto popupPtr = std::make_unique<Wt::WPopupMenu>();
  auto popup = popupPtr.get();

  popup->addItem("item 1");
  popup->addItem("item 2");

  auto splitBtn = std::make_unique<WSplitButton>("btn");
  splitBtn->setMenu(std::move(popupPtr));

  return splitBtn;
}

std::unique_ptr<WTableView> createWTableView()
{
  auto model = std::make_unique<WStandardItemModel>(10, 10);

  auto tableView = std::make_unique<WTableView>();
  tableView->setModel(std::move(model));
  tableView->setAlternatingRowColors(true);

  return tableView;
}

std::unique_ptr<WTimeEdit> createWTimeEdit()
{
  return std::make_unique<WTimeEdit>();
}

std::unique_ptr<WToolBar> createWToolBar()
{
  auto toolbar = std::make_unique<WToolBar>();
  toolbar->addButton(std::make_unique<WPushButton>());
  toolbar->addButton(std::make_unique<WPushButton>());
  toolbar->addSeparator();
  toolbar->addButton(std::make_unique<WPushButton>());
  toolbar->setOrientation(Orientation::Vertical);

  return toolbar;
}

struct TestedWidgets
{
  WDateEdit* dateEdit;
  WDialog* dialog;
  WInPlaceEdit* inPlaceEdit;
  WMenu* menu;
  WMessageBox* messageBox;
  WNavigationBar* navBar;
  WPanel* panel;
  WPushButton* pushButton;
  WSplitButton* splitButton;
  WTableView* tableView;
  WTimeEdit* timeEdit;
  WToolBar* toolbar;

  DomElement* domElement;

  ~TestedWidgets()
  {
    delete domElement;
  }
};

TestedWidgets createAllWidgets(Wt::WApplication& testApp, bool themeEnabled)
{
  TestedWidgets tw;

  tw.dateEdit = testApp.root()->addWidget(createWDateEdit());
  tw.dateEdit->setThemeStyleEnabled(themeEnabled);
  tw.dialog = testApp.root()->addWidget(createWDialog());
  tw.dialog->setThemeStyleEnabled(themeEnabled);
  tw.dialog->show();
  tw.inPlaceEdit = testApp.root()->addWidget(createWInPlaceEdit());
  tw.inPlaceEdit->setThemeStyleEnabled(themeEnabled);
  tw.menu = testApp.root()->addWidget(createWMenu());
  tw.menu->setThemeStyleEnabled(themeEnabled);
  tw.menu->itemAt(0)->setThemeStyleEnabled(themeEnabled);
  tw.menu->itemAt(1)->setThemeStyleEnabled(themeEnabled);
  tw.messageBox = testApp.root()->addWidget(createWMessageBox());
  tw.messageBox->setThemeStyleEnabled(themeEnabled);
  tw.messageBox->show();
  tw.navBar = testApp.root()->addWidget(createWNavigationBar());
  tw.navBar->setThemeStyleEnabled(themeEnabled);
  tw.panel = testApp.root()->addWidget(createWPanel());
  tw.panel->setThemeStyleEnabled(themeEnabled);
  tw.pushButton = testApp.root()->addWidget(createWPushButton());
  tw.pushButton->setThemeStyleEnabled(themeEnabled);
  tw.splitButton = testApp.root()->addWidget(createWSplitButton());
  tw.splitButton->setThemeStyleEnabled(themeEnabled);
  tw.splitButton->dropDownButton()->setThemeStyleEnabled(themeEnabled);
  tw.tableView = testApp.root()->addWidget(createWTableView());
  tw.tableView->setThemeStyleEnabled(themeEnabled);
  tw.timeEdit = testApp.root()->addWidget(createWTimeEdit());
  tw.timeEdit->setThemeStyleEnabled(themeEnabled);
  tw.toolbar = testApp.root()->addWidget(createWToolBar());
  tw.toolbar->setThemeStyleEnabled(themeEnabled);

  tw.domElement = testApp.domRoot()->createSDomElement(&testApp);

  return tw;
}

void testCssThemeRequiredClass(const Wt::WApplication& testApp, const TestedWidgets& tw)
{
  //WDialog
  auto titleBarChilds = tw.dialog->titleBar()->children();
  bool dialogHasTitlebarCloseIcon = false;
  for (size_t i = 0; i < titleBarChilds.size(); ++i) {
    dialogHasTitlebarCloseIcon |= titleBarChilds[i]->hasStyleClass("closeicon");
  }
  BOOST_TEST(dialogHasTitlebarCloseIcon);

  //WMenuItems
  auto menuItems = tw.menu->items();
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    BOOST_TEST((*i)->hasStyleClass("Wt-closable"));

    auto itemChilds = (*i)->children();
    bool menuItemHasCloseButtonClass = false;
    bool menuItemHasCheckable = false;
    bool menuItemHasIcon = false;

    for (size_t j = 0; j < itemChilds.size(); ++j) {
      bool isCloseButton = itemChilds[j]->hasStyleClass("closeicon");
      menuItemHasCloseButtonClass |= isCloseButton;

      if (!isCloseButton) {
        auto itemChildChilds = itemChilds[j]->children();
        for (size_t k = 0; k < itemChildChilds.size(); ++k) {
          menuItemHasCheckable |= itemChildChilds[k]->hasStyleClass("Wt-chkbox");
          menuItemHasIcon |= itemChildChilds[k]->hasStyleClass("Wt-icon");
        }
      }
    }

    BOOST_TEST(menuItemHasCloseButtonClass);
    BOOST_TEST(menuItemHasCheckable);
    BOOST_TEST(menuItemHasIcon);
  }

  //WTableView
  std::string backgroundImage = testApp.theme()->resourcesUrl() + "stripes/stripe-"
        + std::to_string(static_cast<int>(tw.tableView->rowHeight().toPixels()))
        + "px.gif";

  auto tableViewTable = tw.tableView->table();
  BOOST_TEST(tableViewTable->decorationStyle().backgroundImage() == backgroundImage);

  auto headerColumnsTable = tw.tableView->headerColumnsTable();
  BOOST_TEST(headerColumnsTable->decorationStyle().backgroundImage() == backgroundImage);
}

void testBootstrap2ThemeRequiredClass(WT_MAYBE_UNUSED const Wt::WApplication& testApp, const TestedWidgets& tw)
{
  //WDialog
  auto titleBarChilds = tw.dialog->titleBar()->children();
  bool dialogHasTitlebarCloseIcon = false;
  for (size_t i = 0; i < titleBarChilds.size(); ++i) {
    dialogHasTitlebarCloseIcon |= titleBarChilds[i]->hasStyleClass("close");
  }
  BOOST_TEST(dialogHasTitlebarCloseIcon);

  //WMenuItems
  auto menuItems = tw.menu->items();
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    auto itemChilds = (*i)->children();
    bool menuItemHasCloseButtonClass = false;
    bool menuItemHasCheckable = false;
    bool menuItemHasIcon = false;

    for (size_t j = 0; j < itemChilds.size(); ++j) {
      bool isCloseButton = itemChilds[j]->hasStyleClass("close");
      menuItemHasCloseButtonClass |= isCloseButton;

      if (!isCloseButton) {
        auto itemChildChilds = itemChilds[j]->children();
        for (size_t k = 0; k < itemChildChilds.size(); ++k) {
          menuItemHasCheckable |= itemChildChilds[k]->hasStyleClass("Wt-chkbox");
          menuItemHasIcon |= itemChildChilds[k]->hasStyleClass("Wt-icon");
        }
      }
    }

    BOOST_TEST(menuItemHasCloseButtonClass);
    BOOST_TEST(menuItemHasCheckable);
    BOOST_TEST(menuItemHasIcon);
  }

  //WTableView
  BOOST_TEST(tw.tableView->table()->hasStyleClass("Wt-striped"));
  BOOST_TEST(tw.tableView->headerColumnsTable()->hasStyleClass("Wt-striped"));
}

void testBootstrap3ThemeRequiredClass(const Wt::WApplication& testApp, const TestedWidgets& tw)
{
  testBootstrap2ThemeRequiredClass(testApp, tw);
}

void testBootstrap5ThemeRequiredClass(WT_MAYBE_UNUSED const Wt::WApplication& testApp, const TestedWidgets& tw)
{
  //WDialog
  auto titleBarChilds = tw.dialog->titleBar()->children();
  bool dialogHasTitlebarCloseIcon = false;
  for (size_t i = 0; i < titleBarChilds.size(); ++i) {
    dialogHasTitlebarCloseIcon |= titleBarChilds[i]->hasStyleClass("btn-close");
  }
  BOOST_TEST(dialogHasTitlebarCloseIcon);

  //WMenuItems
  auto menuItems = tw.menu->items();
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    auto itemChilds = (*i)->children();
    bool menuItemHasCloseButtonClass = false;
    bool menuItemHasCheckable = false;
    bool menuItemHasIcon = false;

    for (size_t j = 0; j < itemChilds.size(); ++j) {
      bool isCloseButton = itemChilds[j]->hasStyleClass("Wt-close-icon");
      menuItemHasCloseButtonClass |= isCloseButton;

      if (!isCloseButton) {
        auto itemChildChilds = itemChilds[j]->children();
        for (size_t k = 0; k < itemChildChilds.size(); ++k) {
          menuItemHasCheckable |= itemChildChilds[k]->hasStyleClass("Wt-chkbox");
          menuItemHasIcon |= itemChildChilds[k]->hasStyleClass("Wt-icon");
        }
      }
    }

    BOOST_TEST(menuItemHasCloseButtonClass);
    BOOST_TEST(menuItemHasCheckable);
    BOOST_TEST(menuItemHasIcon);
  }

  //WTableView
  BOOST_TEST(tw.tableView->table()->hasStyleClass("Wt-striped"));
  BOOST_TEST(tw.tableView->headerColumnsTable()->hasStyleClass("Wt-striped"));
}

} // namespace

BOOST_AUTO_TEST_CASE(disabled_css_theme_required_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WCssTheme>("default"));

  auto tw = createAllWidgets(testApp, false);
  testCssThemeRequiredClass(testApp, tw);
}

BOOST_AUTO_TEST_CASE(enabled_css_theme_required_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WCssTheme>("default"));

  auto tw = createAllWidgets(testApp, true);
  testCssThemeRequiredClass(testApp, tw);
}

BOOST_AUTO_TEST_CASE(disabled_bootstrap2_theme_required_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap2Theme>());

  auto tw = createAllWidgets(testApp, false);
  testBootstrap2ThemeRequiredClass(testApp, tw);
}

BOOST_AUTO_TEST_CASE(enabled_bootstrap2_theme_required_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap2Theme>());

  auto tw = createAllWidgets(testApp, true);
  testBootstrap2ThemeRequiredClass(testApp, tw);
}

BOOST_AUTO_TEST_CASE(disabled_bootstrap3_theme_required_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap3Theme>());

  auto tw = createAllWidgets(testApp, false);
  testBootstrap3ThemeRequiredClass(testApp, tw);
}

BOOST_AUTO_TEST_CASE(enabled_bootstrap3_theme_required_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap3Theme>());

  auto tw = createAllWidgets(testApp, true);
  testBootstrap3ThemeRequiredClass(testApp, tw);
}

BOOST_AUTO_TEST_CASE(disabled_bootstrap5_theme_required_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap5Theme>());

  auto tw = createAllWidgets(testApp, false);
  testBootstrap5ThemeRequiredClass(testApp, tw);
}

BOOST_AUTO_TEST_CASE(enabled_bootstrap5_theme_required_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap5Theme>());

  auto tw = createAllWidgets(testApp, true);
  testBootstrap5ThemeRequiredClass(testApp, tw);
}

BOOST_AUTO_TEST_CASE(disabled_css_theme_style_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WCssTheme>("default"));

  // CREATE WIDGETS //
  auto tw = createAllWidgets(testApp, false);

  // TEST //
  //WDateEdit
  BOOST_TEST(!tw.dateEdit->hasStyleClass("Wt-dateedit"));

  //WDialog
  BOOST_TEST(!tw.dialog->hasStyleClass("Wt-dialog"));
  BOOST_TEST(!tw.dialog->hasStyleClass("Wt-outset"));

  auto dialogTemplate = dynamic_cast<WTemplate *>(tw.dialog->children()[0]);
  BOOST_TEST(!dialogTemplate->resolveWidget("layout")->hasStyleClass("movable"));

  BOOST_TEST(!tw.dialog->titleBar()->hasStyleClass("titlebar"));

  BOOST_TEST(!tw.dialog->contents()->hasStyleClass("body"));

  BOOST_TEST(!tw.dialog->footer()->hasStyleClass("footer"));

  //WMenu
  auto menuItems = tw.menu->items();
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("item"));
    BOOST_TEST(!(*i)->hasStyleClass("itemselected"));
  }

  //WMessageBox
  BOOST_TEST(!tw.messageBox->contents()->hasStyleClass("Wt-msgbox-body"));

  auto messageBoxContentsChilds = tw.messageBox->contents()->children();
  for (auto i = messageBoxContentsChilds.begin(); i != messageBoxContentsChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("Wt-msgbox-text"));
    BOOST_TEST(!(*i)->hasStyleClass("Wt-msgbox-icon"));
  }

  //WPanel
  BOOST_TEST(!tw.panel->hasStyleClass("Wt-panel"));
  BOOST_TEST(!tw.panel->hasStyleClass("Wt-outset"));

  BOOST_TEST(!tw.panel->titleBarWidget()->hasStyleClass("titlebar"));

  auto panelTitleBarChilds = tw.panel->titleBarWidget()->children();
  for (auto i = panelTitleBarChilds.begin(); i != panelTitleBarChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("Wt-collapse-button"));
  }

  BOOST_TEST(!tw.panel->centralWidget()->parent()->hasStyleClass("body"));

  //WPushButton
  BOOST_TEST(!tw.pushButton->hasStyleClass("Wt-btn"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("dropdown-toggle"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("with-label"));

  //WSplitButton
  BOOST_TEST(!tw.splitButton->hasStyleClass("btn-group"));
  BOOST_TEST(!tw.splitButton->dropDownButton()->hasStyleClass("dropdown-toggle"));

  //WTimeEdit
  BOOST_TEST(!tw.timeEdit->hasStyleClass("Wt-timeedit"));

  //WToolBar
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-toolbar"));
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-group"));
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-group-vertical"));

  auto toolbarChilds =  tw.toolbar->children()[0]->children();
  for (auto i = toolbarChilds.begin(); i != toolbarChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("btn-group"));
    BOOST_TEST(!(*i)->hasStyleClass("me-2"));
  }
}

BOOST_AUTO_TEST_CASE(disabled_bootstrap2_theme_style_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap2Theme>());

  // CREATE WIDGETS //
  auto tw = createAllWidgets(testApp, false);

  // TEST //
  std::stringstream output;
  std::string out;
  std::string expected;

  //WDateEdit
  BOOST_TEST(!tw.dateEdit->hasStyleClass("Wt-dateedit"));

  //WDialog
  BOOST_TEST(!tw.dialog->hasStyleClass("modal"));

  auto dialogTemplate = dynamic_cast<WTemplate *>(tw.dialog->children()[0]);
  BOOST_TEST(!dialogTemplate->resolveWidget("layout")->hasStyleClass("movable"));

  BOOST_TEST(!tw.dialog->titleBar()->hasStyleClass("modal-header"));

  BOOST_TEST(!tw.dialog->contents()->hasStyleClass("modal-body"));

  BOOST_TEST(!tw.dialog->footer()->hasStyleClass("modal-footer"));

  //WInPlaceEdit
  BOOST_TEST(!tw.inPlaceEdit->hasStyleClass("Wt-in-place-edit"));

  auto inPlaceEditChilds = tw.inPlaceEdit->children()[0]->children();
  for (auto i = inPlaceEditChilds.begin(); i != inPlaceEditChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("input-append"));
  }

  //WMenu
  BOOST_TEST(!tw.menu->hasStyleClass("nav"));

  auto menuItems = tw.menu->items();
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass(testApp.theme()->activeClass()));
  }

  //WMessageBox
  BOOST_TEST(!tw.messageBox->contents()->hasStyleClass("Wt-msgbox-body"));

  auto messageBoxContentsChilds = tw.messageBox->contents()->children();
  for (auto i = messageBoxContentsChilds.begin(); i != messageBoxContentsChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("Wt-msgbox-text"));
    BOOST_TEST(!(*i)->hasStyleClass("Wt-msgbox-icon"));
  }

  //WNavigationBar
  BOOST_TEST(!tw.navBar->hasStyleClass("navbar"));

  tw.navBar->renderTemplateText(output, tw.navBar->templateText());
  out = output.str();
  resetStream(output);
  strip(out);
  tw.navBar->renderTemplateText(output, WString::tr("Wt.WNavigationBar.template.default"));
  expected = output.str();
  resetStream(output);
  strip(expected);
  BOOST_TEST(out == expected);

  auto navBarExpendBtn = tw.navBar->resolveWidget("expand-button");
  BOOST_TEST(!navBarExpendBtn->hasStyleClass("btn-navbar"));

  auto navBarCollapseBtn = tw.navBar->resolveWidget("collapse-button");
  BOOST_TEST(!navBarCollapseBtn->hasStyleClass("btn-navbar"));

  auto navBarTitle = tw.navBar->resolveWidget("title-link");
  BOOST_TEST(!navBarTitle->hasStyleClass("brand"));

  auto navBarContents = tw.navBar->resolve<WContainerWidget *>("contents");
  BOOST_TEST(!navBarContents->hasStyleClass("nav-collapse"));

  auto navBarContentsChilds = navBarContents->children();
  for (auto i = navBarContentsChilds.begin(); i != navBarContentsChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("pull-left"));
    BOOST_TEST(!(*i)->hasStyleClass("pull-right"));
    BOOST_TEST(!(*i)->hasStyleClass("navbar-nav"));
    BOOST_TEST(!(*i)->hasStyleClass("navbar-search"));
    BOOST_TEST(!(*i)->hasStyleClass("navbar-form"));
  }

  //WPanel
  BOOST_TEST(!tw.panel->hasStyleClass("accordion-group"));

  BOOST_TEST(!tw.panel->titleBarWidget()->hasStyleClass("accordion-heading"));

  auto panelTitleBarChilds = tw.panel->titleBarWidget()->children();
  for (auto i = panelTitleBarChilds.begin(); i != panelTitleBarChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("Wt-collapse-button"));
    BOOST_TEST(!(*i)->hasStyleClass("accordion-toggle"));
  }

  BOOST_TEST(!tw.panel->centralWidget()->parent()->hasStyleClass("accordion-inner"));

  //WPushButton
  BOOST_TEST(!tw.pushButton->hasStyleClass("btn"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("dropdown-toggle"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("with-label"));

  //WSplitButton
  BOOST_TEST(!tw.splitButton->hasStyleClass("btn-group"));
  BOOST_TEST(!tw.splitButton->dropDownButton()->hasStyleClass("dropdown-toggle"));

  //WTimeEdit
  BOOST_TEST(!tw.timeEdit->hasStyleClass("Wt-timeedit"));

  //WToolBar
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-toolbar"));
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-group"));
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-group-vertical"));

  auto toolbarChilds =  tw.toolbar->children()[0]->children();
  for (auto i = toolbarChilds.begin(); i != toolbarChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("btn-group"));
    BOOST_TEST(!(*i)->hasStyleClass("me-2"));
  }
}

BOOST_AUTO_TEST_CASE(disabled_bootstrap3_theme_style_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap3Theme>());

  // CREATE WIDGETS //
  auto tw = createAllWidgets(testApp, false);

  // TEST //
  std::stringstream output;
  std::string out;
  std::string expected;

  //WDateEdit
  BOOST_TEST(!tw.dateEdit->hasStyleClass("Wt-dateedit"));
  BOOST_TEST(!tw.dateEdit->hasStyleClass("form-control"));

  //WDialog
  BOOST_TEST(!tw.dialog->hasStyleClass("Wt-dialog"));
  BOOST_TEST(!tw.dialog->hasStyleClass("modal-dialog"));

  auto dialogTemplate = dynamic_cast<WTemplate *>(tw.dialog->children()[0]);
  BOOST_TEST(!dialogTemplate->resolveWidget("layout")->hasStyleClass("movable"));
  BOOST_TEST(!dialogTemplate->resolveWidget("layout")->hasStyleClass("modal-content"));

  BOOST_TEST(!tw.dialog->titleBar()->hasStyleClass("modal-header"));

  BOOST_TEST(!tw.dialog->contents()->hasStyleClass("modal-body"));

  BOOST_TEST(!tw.dialog->footer()->hasStyleClass("modal-footer"));

  //WInPlaceEdit
  BOOST_TEST(!tw.inPlaceEdit->hasStyleClass("Wt-in-place-edit"));

  auto inPlaceEditChilds = tw.inPlaceEdit->children()[0]->children();
  for (auto i = inPlaceEditChilds.begin(); i != inPlaceEditChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("input-group"));
  }

  BOOST_TEST(!tw.inPlaceEdit->saveButton()->parent()->hasStyleClass("input-group-btn"));

  //WMenu
  BOOST_TEST(!tw.menu->hasStyleClass("nav"));

  auto menuItems = tw.menu->items();
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass(testApp.theme()->activeClass()));
  }

  //WMessageBox
  BOOST_TEST(!tw.messageBox->contents()->hasStyleClass("Wt-msgbox-body"));

  auto messageBoxContentsChilds = tw.messageBox->contents()->children();
  for (auto i = messageBoxContentsChilds.begin(); i != messageBoxContentsChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("Wt-msgbox-text"));
    BOOST_TEST(!(*i)->hasStyleClass("Wt-msgbox-icon"));
  }

  //WNavigationBar
  BOOST_TEST(!tw.navBar->hasStyleClass("navbar"));
  BOOST_TEST(!tw.navBar->hasStyleClass("navbar-default"));

  tw.navBar->renderTemplateText(output, tw.navBar->templateText());
  out = output.str();
  resetStream(output);
  strip(out);
  tw.navBar->renderTemplateText(output, WString::tr("Wt.WNavigationBar.template.default"));
  expected = output.str();
  resetStream(output);
  strip(expected);
  BOOST_TEST(out == expected);

  auto navBarExpendBtn = tw.navBar->resolveWidget("expand-button");
  BOOST_TEST(!navBarExpendBtn->hasStyleClass("navbar-toggle"));

  auto navBarCollapseBtn = tw.navBar->resolveWidget("collapse-button");
  BOOST_TEST(!navBarCollapseBtn->hasStyleClass("navbar-toggle"));

  auto navBarTitle = tw.navBar->resolveWidget("title-link");
  BOOST_TEST(!navBarTitle->hasStyleClass("navbar-brand"));

  auto navBarContents = tw.navBar->resolve<WContainerWidget *>("contents");
  BOOST_TEST(!navBarContents->hasStyleClass("navbar-collapse"));

  auto navBarContentsChilds = navBarContents->children();
  for (auto i = navBarContentsChilds.begin(); i != navBarContentsChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("navbar-left"));
    BOOST_TEST(!(*i)->hasStyleClass("navbar-right"));
    BOOST_TEST(!(*i)->hasStyleClass("navbar-nav"));
    BOOST_TEST(!(*i)->hasStyleClass("navbar-form"));
  }

  //WPanel
  BOOST_TEST(!tw.panel->hasStyleClass("panel"));
  BOOST_TEST(!tw.panel->hasStyleClass("panel-default"));

  BOOST_TEST(!tw.panel->titleBarWidget()->hasStyleClass("panel-heading"));

  auto panelTitleBarChilds = tw.panel->titleBarWidget()->children();
  for (auto i = panelTitleBarChilds.begin(); i != panelTitleBarChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("Wt-collapse-button"));
    BOOST_TEST(!(*i)->hasStyleClass("accordion-toggle"));
  }

  BOOST_TEST(!tw.panel->centralWidget()->parent()->hasStyleClass("panel-body"));

  //WPushButton
  BOOST_TEST(!tw.pushButton->hasStyleClass("btn"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("btn-default"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("dropdown-toggle"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("with-label"));

  //WSplitButton
  BOOST_TEST(!tw.splitButton->hasStyleClass("btn-group"));
  BOOST_TEST(!tw.splitButton->dropDownButton()->hasStyleClass("dropdown-toggle"));

  //WTimeEdit
  BOOST_TEST(!tw.timeEdit->hasStyleClass("Wt-timeedit"));
  BOOST_TEST(!tw.timeEdit->hasStyleClass("form-control"));

  //WToolBar
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-toolbar"));
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-group"));
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-group-vertical"));

  auto toolbarChilds =  tw.toolbar->children()[0]->children();
  for (auto i = toolbarChilds.begin(); i != toolbarChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("btn-group"));
    BOOST_TEST(!(*i)->hasStyleClass("me-2"));
  }
}

BOOST_AUTO_TEST_CASE(disabled_bootstrap5_theme_style_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap5Theme>());

  // CREATE WIDGETS //
  auto tw = createAllWidgets(testApp, false);

  // TEST //
  std::stringstream output;
  std::string out;
  std::string expected;

  //WDateEdit
  BOOST_TEST(!tw.dateEdit->hasStyleClass("Wt-dateedit"));

  //WDialog
  BOOST_TEST(!tw.dialog->hasStyleClass("Wt-dialog"));
  BOOST_TEST(!tw.dialog->hasStyleClass("modal-dialog"));

  auto dialogTemplate = dynamic_cast<WTemplate *>(tw.dialog->children()[0]);
  BOOST_TEST(!dialogTemplate->resolveWidget("layout")->hasStyleClass("movable"));
  BOOST_TEST(!dialogTemplate->resolveWidget("layout")->hasStyleClass("modal-content"));

  auto dialogTitleBar = tw.dialog->titleBar();
  BOOST_TEST(!dialogTitleBar->hasStyleClass("modal-header"));

  auto dialogTitleBarChilds = dialogTitleBar->children();
  for (auto i = dialogTitleBarChilds.begin(); i != dialogTitleBarChilds.end(); ++i) {
    auto titleBarTemplate = dynamic_cast<WTemplate *>(*i);
    if (titleBarTemplate) {
      titleBarTemplate->renderTemplateText(output, titleBarTemplate->templateText());
      out = output.str();
      resetStream(output);
      strip(out);
      titleBarTemplate->renderTemplateText(output, WString::tr("Wt.WDialog.titlebar.default"));
      expected = output.str();
      resetStream(output);
      strip(expected);
      BOOST_TEST(out == expected);
    }
  }

  BOOST_TEST(!tw.dialog->contents()->hasStyleClass("modal-body"));

  BOOST_TEST(!tw.dialog->footer()->hasStyleClass("modal-footer"));

  dialogTemplate->renderTemplateText(output, dialogTemplate->templateText());
  out = output.str();
  resetStream(output);
  strip(out);
  dialogTemplate->renderTemplateText(output, WString::tr("Wt.WDialog.template.default"));
  expected = output.str();
  resetStream(output);
  strip(expected);
  BOOST_TEST(out == expected);

  //WInPlaceEdit
  BOOST_TEST(!tw.inPlaceEdit->hasStyleClass("Wt-in-place-edit"));

  auto inPlaceEditBtnParent = tw.inPlaceEdit->saveButton()->parent();
  auto inPlaceEditChilds = tw.inPlaceEdit->children()[0]->children();
  for (auto i = inPlaceEditChilds.begin(); i != inPlaceEditChilds.end(); ++i) {
    BOOST_TEST(!(*i)->hasStyleClass("input-group"));
    BOOST_TEST(*i != inPlaceEditBtnParent);
  }

  BOOST_TEST(!tw.inPlaceEdit->saveButton()->hasStyleClass("btn-outline-secondary"));

  BOOST_TEST(!tw.inPlaceEdit->cancelButton()->hasStyleClass("btn-outline-secondary"));


  //WMenu
  BOOST_TEST(!tw.menu->hasStyleClass("nav"));

  auto menuItems = tw.menu->items();
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i) {
    BOOST_TEST(!(*i)->hasStyleClass(testApp.theme()->activeClass()));
    BOOST_TEST(!(*i)->hasStyleClass("nav-item"));

    auto anchor = (*i)->anchor();
    BOOST_TEST(!anchor->hasStyleClass(testApp.theme()->activeClass()));
    BOOST_TEST(!anchor->hasStyleClass("nav-link"));
  }

  //WMessageBox
  BOOST_TEST(!tw.messageBox->contents()->hasStyleClass("Wt-msgbox-body"));

  auto messageBoxContentsChilds = tw.messageBox->contents()->children();
  for (auto i = messageBoxContentsChilds.begin(); i != messageBoxContentsChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("Wt-msgbox-text"));
    BOOST_TEST(!(*i)->hasStyleClass("Wt-msgbox-icon"));
  }

  //WNavigationBar
  BOOST_TEST(!tw.navBar->hasStyleClass("navbar"));
  BOOST_TEST(!tw.navBar->hasStyleClass("navbar-expand-lg"));

  tw.navBar->renderTemplateText(output, tw.navBar->templateText());
  out = output.str();
  resetStream(output);
  strip(out);
  tw.navBar->renderTemplateText(output, WString::tr("Wt.WNavigationBar.template.default"));
  expected = output.str();
  resetStream(output);
  strip(expected);
  BOOST_TEST(out == expected);

  auto navBarExpendBtn = tw.navBar->resolveWidget("expand-button");
  BOOST_REQUIRE(navBarExpendBtn);
  BOOST_TEST(!navBarExpendBtn->hasStyleClass("navbar-toggler"));

  auto navBarCollapseBtn = tw.navBar->resolveWidget("collapse-button");
  BOOST_TEST(!navBarCollapseBtn->hasStyleClass("navbar-toggler"));

  auto navBarTitle = tw.navBar->resolveWidget("title-link");
  BOOST_TEST(!navBarTitle->hasStyleClass("navbar-brand"));

  auto navBarContents = tw.navBar->resolve<WContainerWidget *>("contents");
  BOOST_TEST(!navBarContents->hasStyleClass("navbar-collapse"));
  BOOST_TEST(!navBarContents->hasStyleClass("collapse"));

  auto navBarContentsChilds = navBarContents->children();
  for (auto i = navBarContentsChilds.begin(); i != navBarContentsChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("navbar-nav"));
    BOOST_TEST(!(*i)->hasStyleClass("d-flex"));
  }

  //WPanel
  BOOST_TEST(!tw.panel->hasStyleClass("Wt-panel"));
  BOOST_TEST(!tw.panel->hasStyleClass("card"));

  BOOST_TEST(!tw.panel->titleBarWidget()->hasStyleClass("card-header"));

  auto panelTitleBarChilds = tw.panel->titleBarWidget()->children();
  for (auto i = panelTitleBarChilds.begin(); i != panelTitleBarChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("Wt-collapse-button"));
  }

  BOOST_TEST(!tw.panel->centralWidget()->parent()->hasStyleClass("card-body"));

  //WPushButton
  BOOST_TEST(!tw.pushButton->hasStyleClass("btn"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("btn-secondary"));
  BOOST_TEST(!tw.pushButton->hasStyleClass("dropdown-toggle"));

  //WSplitButton
  BOOST_TEST(!tw.splitButton->hasStyleClass("btn-group"));
  BOOST_TEST(!tw.splitButton->dropDownButton()->hasStyleClass("dropdown-toggle"));

  //WTimeEdit
  BOOST_TEST(!tw.timeEdit->hasStyleClass("Wt-timeedit"));

  //WToolBar
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-toolbar"));
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-group"));
  BOOST_TEST(!tw.toolbar->hasStyleClass("btn-group-vertical"));

  auto toolbarChilds =  tw.toolbar->children()[0]->children();
  for (auto i = toolbarChilds.begin(); i != toolbarChilds.end(); ++i)
  {
    BOOST_TEST(!(*i)->hasStyleClass("btn-group"));
    BOOST_TEST(!(*i)->hasStyleClass("me-2"));
  }
}

BOOST_AUTO_TEST_CASE(enabled_css_theme_style_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WCssTheme>("default"));

  // CREATE WIDGETS //
  auto tw = createAllWidgets(testApp, true);

  // TEST //
  //WDateEdit
  BOOST_TEST(tw.dateEdit->hasStyleClass("Wt-dateedit"));

  //WDialog
  BOOST_TEST(tw.dialog->hasStyleClass("Wt-dialog"));
  BOOST_TEST(tw.dialog->hasStyleClass("Wt-outset"));

  auto dialogTemplate = dynamic_cast<WTemplate *>(tw.dialog->children()[0]);
  BOOST_TEST(dialogTemplate->resolveWidget("layout")->hasStyleClass("movable"));

  BOOST_TEST(tw.dialog->titleBar()->hasStyleClass("titlebar"));

  BOOST_TEST(tw.dialog->contents()->hasStyleClass("body"));

  BOOST_TEST(tw.dialog->footer()->hasStyleClass("footer"));

  //WMenu
  auto menuItems = tw.menu->items();
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("item") ||
                (*i)->hasStyleClass("itemselected")));
  }

  //WMessageBox
  BOOST_TEST(tw.messageBox->contents()->hasStyleClass("Wt-msgbox-body"));

  auto messageBoxContentsChilds = tw.messageBox->contents()->children();
  for (auto i = messageBoxContentsChilds.begin(); i != messageBoxContentsChilds.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("Wt-msgbox-text") ||
                (*i)->hasStyleClass("Wt-msgbox-icon")));
  }

  //WPanel
  BOOST_TEST(tw.panel->hasStyleClass("Wt-panel"));
  BOOST_TEST(tw.panel->hasStyleClass("Wt-outset"));

  BOOST_TEST(tw.panel->titleBarWidget()->hasStyleClass("titlebar"));

  auto panelTitleBarChilds = tw.panel->titleBarWidget()->children();

  bool panelHasCollapseButton = false;
  for (auto i = panelTitleBarChilds.begin(); i != panelTitleBarChilds.end(); ++i)
  {
    panelHasCollapseButton |= (*i)->hasStyleClass("Wt-collapse-button");
  }
  BOOST_TEST(panelHasCollapseButton);

  BOOST_TEST(tw.panel->centralWidget()->parent()->hasStyleClass("body"));

  //WPushButton
  BOOST_TEST(tw.pushButton->hasStyleClass("Wt-btn"));
  BOOST_TEST(tw.pushButton->hasStyleClass("dropdown-toggle"));
  BOOST_TEST(tw.pushButton->hasStyleClass("with-label"));

  //WSplitButton
  BOOST_TEST(tw.splitButton->hasStyleClass("btn-group"));
  BOOST_TEST(tw.splitButton->dropDownButton()->hasStyleClass("dropdown-toggle"));

  //WTimeEdit
  BOOST_TEST(tw.timeEdit->hasStyleClass("Wt-timeedit"));

  //WToolBar
  BOOST_TEST((tw.toolbar->hasStyleClass("btn-toolbar") ||
              tw.toolbar->hasStyleClass("btn-group")));
  BOOST_TEST(tw.toolbar->hasStyleClass("btn-group-vertical"));

  auto toolbarChilds =  tw.toolbar->children()[0]->children();
  for (auto i = toolbarChilds.begin(); i != toolbarChilds.end(); ++i)
  {
    BOOST_TEST((*i)->hasStyleClass("btn-group"));
    BOOST_TEST((*i)->hasStyleClass("me-2"));
  }
}

BOOST_AUTO_TEST_CASE(enabled_bootstrap2_theme_style_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap2Theme>());

  // CREATE WIDGETS //
  auto tw = createAllWidgets(testApp, true);

  // TEST //
  std::stringstream output;
  std::string out;
  std::string unstyled;

  //WDateEdit
  BOOST_TEST(tw.dateEdit->hasStyleClass("Wt-dateedit"));

  //WDialog
  BOOST_TEST(tw.dialog->hasStyleClass("modal"));

  auto dialogTemplate = dynamic_cast<WTemplate *>(tw.dialog->children()[0]);
  BOOST_TEST(dialogTemplate->resolveWidget("layout")->hasStyleClass("movable"));

  BOOST_TEST(tw.dialog->titleBar()->hasStyleClass("modal-header"));

  BOOST_TEST(tw.dialog->contents()->hasStyleClass("modal-body"));

  BOOST_TEST(tw.dialog->footer()->hasStyleClass("modal-footer"));

  //WInPlaceEdit
  BOOST_TEST(tw.inPlaceEdit->hasStyleClass("Wt-in-place-edit"));

  auto inPlaceEditChilds = tw.inPlaceEdit->children()[0]->children();
  bool inPlaceEditHasInputAppend = false;
  for (auto i = inPlaceEditChilds.begin(); i != inPlaceEditChilds.end(); ++i)
  {
    inPlaceEditHasInputAppend |= (*i)->hasStyleClass("input-append");
  }
  BOOST_TEST(inPlaceEditHasInputAppend);

  //WMenu
  BOOST_TEST(tw.menu->hasStyleClass("nav"));

  auto menuItems = tw.menu->items();
  bool menuHasActiveItemClass = false;
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    menuHasActiveItemClass |= (*i)->hasStyleClass(testApp.theme()->activeClass());
  }
  BOOST_TEST(menuHasActiveItemClass);

  //WMessageBox
  BOOST_TEST(tw.messageBox->contents()->hasStyleClass("Wt-msgbox-body"));

  auto messageBoxContentsChilds = tw.messageBox->contents()->children();
  for (auto i = messageBoxContentsChilds.begin(); i != messageBoxContentsChilds.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("Wt-msgbox-text") || (*i)->hasStyleClass("Wt-msgbox-icon")));
  }

  //WNavigationBar
  BOOST_TEST(tw.navBar->hasStyleClass("navbar"));

  tw.navBar->renderTemplateText(output, tw.navBar->templateText());
  out = output.str();
  resetStream(output);
  strip(out);
  tw.navBar->renderTemplateText(output, WString::tr("Wt.WNavigationBar.template.default"));
  unstyled = output.str();
  resetStream(output);
  strip(unstyled);
  BOOST_TEST(out != unstyled);

  auto navBarExpendBtn = tw.navBar->resolveWidget("expand-button");
  BOOST_TEST(navBarExpendBtn->hasStyleClass("btn-navbar"));

  auto navBarCollapseBtn = tw.navBar->resolveWidget("collapse-button");
  BOOST_TEST(navBarCollapseBtn->hasStyleClass("btn-navbar"));

  auto navBarTitle = tw.navBar->resolveWidget("title-link");
  BOOST_TEST(navBarTitle->hasStyleClass("brand"));

  auto navBarContents = tw.navBar->resolve<WContainerWidget *>("contents");
  BOOST_TEST(navBarContents->hasStyleClass("nav-collapse"));

  auto navBarContentsChilds = navBarContents->children();
  for (auto i = navBarContentsChilds.begin(); i != navBarContentsChilds.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("pull-left") ||
                (*i)->hasStyleClass("pull-right")));

    BOOST_TEST(((*i)->hasStyleClass("navbar-nav") ||
                (*i)->hasStyleClass("navbar-search") ||
                (*i)->hasStyleClass("navbar-form")));
  }

  //WPanel
  BOOST_TEST(tw.panel->hasStyleClass("accordion-group"));

  BOOST_TEST(tw.panel->titleBarWidget()->hasStyleClass("accordion-heading"));

  auto panelTitleBarChilds = tw.panel->titleBarWidget()->children();
  for (auto i = panelTitleBarChilds.begin(); i != panelTitleBarChilds.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("Wt-collapse-button") ||
                (*i)->hasStyleClass("accordion-toggle")));
  }

  BOOST_TEST(tw.panel->centralWidget()->parent()->hasStyleClass("accordion-inner"));

  //WPushButton
  BOOST_TEST(tw.pushButton->hasStyleClass("btn"));
  BOOST_TEST(tw.pushButton->hasStyleClass("dropdown-toggle"));
  BOOST_TEST(tw.pushButton->hasStyleClass("with-label"));

  //WSplitButton
  BOOST_TEST(tw.splitButton->hasStyleClass("btn-group"));
  BOOST_TEST(tw.splitButton->dropDownButton()->hasStyleClass("dropdown-toggle"));

  //WTimeEdit
  BOOST_TEST(tw.timeEdit->hasStyleClass("Wt-timeedit"));

  //WToolBar
  BOOST_TEST((tw.toolbar->hasStyleClass("btn-toolbar") ||
              tw.toolbar->hasStyleClass("btn-group")));

  BOOST_TEST(tw.toolbar->hasStyleClass("btn-group-vertical"));

  auto toolbarChilds =  tw.toolbar->children()[0]->children();
  for (auto i = toolbarChilds.begin(); i != toolbarChilds.end(); ++i)
  {
    BOOST_TEST((*i)->hasStyleClass("btn-group"));
    BOOST_TEST((*i)->hasStyleClass("me-2"));
  }
}

BOOST_AUTO_TEST_CASE(enabled_bootstrap3_theme_style_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap3Theme>());

  // CREATE WIDGETS //
  auto tw = createAllWidgets(testApp, true);

  // TEST //
  std::stringstream output;
  std::string out;
  std::string unstyled;

  //WDateEdit
  BOOST_TEST(tw.dateEdit->hasStyleClass("Wt-dateedit"));
  BOOST_TEST(tw.dateEdit->hasStyleClass("form-control"));

  //WDialog
  BOOST_TEST(tw.dialog->hasStyleClass("Wt-dialog"));
  BOOST_TEST(tw.dialog->hasStyleClass("modal-dialog"));

  auto dialogTemplate = dynamic_cast<WTemplate *>(tw.dialog->children()[0]);
  BOOST_TEST(dialogTemplate->resolveWidget("layout")->hasStyleClass("movable"));
  BOOST_TEST(dialogTemplate->resolveWidget("layout")->hasStyleClass("modal-content"));

  BOOST_TEST(tw.dialog->titleBar()->hasStyleClass("modal-header"));

  BOOST_TEST(tw.dialog->contents()->hasStyleClass("modal-body"));

  BOOST_TEST(tw.dialog->footer()->hasStyleClass("modal-footer"));

  //WInPlaceEdit
  BOOST_TEST(tw.inPlaceEdit->hasStyleClass("Wt-in-place-edit"));

  auto inPlaceEditChilds = tw.inPlaceEdit->children()[0]->children();
  bool inPlaceEditHasInputGroup = false;
  for (auto i = inPlaceEditChilds.begin(); i != inPlaceEditChilds.end(); ++i)
  {
    inPlaceEditHasInputGroup |= (*i)->hasStyleClass("input-group");
  }
  BOOST_TEST(inPlaceEditHasInputGroup);

  BOOST_TEST(tw.inPlaceEdit->saveButton()->parent()->hasStyleClass("input-group-btn"));

  //WMenu
  BOOST_TEST(tw.menu->hasStyleClass("nav"));

  auto menuItems = tw.menu->items();
  bool menuHasActiveItemClass = false;
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i)
  {
    menuHasActiveItemClass |= (*i)->hasStyleClass(testApp.theme()->activeClass());
  }
  BOOST_TEST(menuHasActiveItemClass);

  //WMessageBox
  BOOST_TEST(tw.messageBox->contents()->hasStyleClass("Wt-msgbox-body"));

  auto messageBoxContentsChilds = tw.messageBox->contents()->children();
  for (auto i = messageBoxContentsChilds.begin(); i != messageBoxContentsChilds.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("Wt-msgbox-text") ||
                (*i)->hasStyleClass("Wt-msgbox-icon")));
  }

  //WNavigationBar
  BOOST_TEST(tw.navBar->hasStyleClass("navbar"));
  BOOST_TEST(tw.navBar->hasStyleClass("navbar-default"));

  tw.navBar->renderTemplateText(output, tw.navBar->templateText());
  out = output.str();
  resetStream(output);
  strip(out);
  tw.navBar->renderTemplateText(output, WString::tr("Wt.WNavigationBar.template.default"));
  unstyled = output.str();
  resetStream(output);
  strip(unstyled);
  BOOST_TEST(out != unstyled);

  auto navBarExpendBtn = tw.navBar->resolveWidget("expand-button");
  BOOST_TEST(navBarExpendBtn->hasStyleClass("navbar-toggle"));

  auto navBarCollapseBtn = tw.navBar->resolveWidget("collapse-button");
  BOOST_TEST(navBarCollapseBtn->hasStyleClass("navbar-toggle"));

  auto navBarTitle = tw.navBar->resolveWidget("title-link");
  BOOST_TEST(navBarTitle->hasStyleClass("navbar-brand"));

  auto navBarContents = tw.navBar->resolve<WContainerWidget *>("contents");
  BOOST_TEST(navBarContents->hasStyleClass("navbar-collapse"));

  auto navBarContentsChilds = navBarContents->children();
  for (auto i = navBarContentsChilds.begin(); i != navBarContentsChilds.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("navbar-left") ||
                (*i)->hasStyleClass("navbar-right")));

    BOOST_TEST(((*i)->hasStyleClass("navbar-nav") ||
                (*i)->hasStyleClass("navbar-form")));
  }

  //WPanel
  BOOST_TEST(tw.panel->hasStyleClass("panel"));
  BOOST_TEST(tw.panel->hasStyleClass("panel-default"));

  BOOST_TEST(tw.panel->titleBarWidget()->hasStyleClass("panel-heading"));

  auto panelTitleBarChilds = tw.panel->titleBarWidget()->children();
  for (auto i = panelTitleBarChilds.begin(); i != panelTitleBarChilds.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("Wt-collapse-button") ||
                (*i)->hasStyleClass("accordion-toggle")));
  }

  BOOST_TEST(tw.panel->centralWidget()->parent()->hasStyleClass("panel-body"));

  //WPushButton
  BOOST_TEST(tw.pushButton->hasStyleClass("btn"));
  BOOST_TEST(tw.pushButton->hasStyleClass("btn-default"));
  BOOST_TEST(tw.pushButton->hasStyleClass("dropdown-toggle"));
  BOOST_TEST(tw.pushButton->hasStyleClass("with-label"));

  //WSplitButton
  BOOST_TEST(tw.splitButton->hasStyleClass("btn-group"));
  BOOST_TEST(tw.splitButton->dropDownButton()->hasStyleClass("dropdown-toggle"));

  //WTimeEdit
  BOOST_TEST(tw.timeEdit->hasStyleClass("Wt-timeedit"));
  BOOST_TEST(tw.timeEdit->hasStyleClass("form-control"));

  //WToolBar
  BOOST_TEST((tw.toolbar->hasStyleClass("btn-toolbar") ||
              tw.toolbar->hasStyleClass("btn-group")));

  BOOST_TEST(tw.toolbar->hasStyleClass("btn-group-vertical"));

  auto toolbarChilds =  tw.toolbar->children()[0]->children();
  for (auto i = toolbarChilds.begin(); i != toolbarChilds.end(); ++i)
  {
    BOOST_TEST((*i)->hasStyleClass("btn-group"));
    BOOST_TEST((*i)->hasStyleClass("me-2"));
  }
}

BOOST_AUTO_TEST_CASE(enabled_bootstrap5_theme_style_classes)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);
  testApp.setTheme(std::make_unique<WBootstrap5Theme>());

  // CREATE WIDGETS //
  auto tw = createAllWidgets(testApp, true);

  // TEST //
  std::stringstream output;
  std::string out;
  std::string unstyled;

  //WDateEdit
  BOOST_TEST(tw.dateEdit->hasStyleClass("Wt-dateedit"));

  //WDialog
  BOOST_TEST(tw.dialog->hasStyleClass("Wt-dialog"));

  auto dialogTemplate = dynamic_cast<WTemplate *>(tw.dialog->children()[0]);
  BOOST_TEST(dialogTemplate->resolveWidget("layout")->hasStyleClass("movable"));
  BOOST_TEST(dialogTemplate->resolveWidget("layout")->hasStyleClass("modal-content"));

  auto dialogTitleBar = tw.dialog->titleBar();
  BOOST_TEST(dialogTitleBar->hasStyleClass("modal-header"));

  auto dialogTitleBarChilds = dialogTitleBar->children();
  for (auto i = dialogTitleBarChilds.begin(); i != dialogTitleBarChilds.end(); ++i) {
    auto titleBarTemplate = dynamic_cast<WTemplate *>(*i);
    if (titleBarTemplate) {
      titleBarTemplate->renderTemplateText(output, titleBarTemplate->templateText());
      out = output.str();
      resetStream(output);
      strip(out);
      titleBarTemplate->renderTemplateText(output, WString::tr("Wt.WDialog.titlebar.default"));
      unstyled = output.str();
      resetStream(output);
      strip(unstyled);
      BOOST_TEST(out != unstyled);
    }
  }

  BOOST_TEST(tw.dialog->contents()->hasStyleClass("modal-body"));

  BOOST_TEST(tw.dialog->footer()->hasStyleClass("modal-footer"));

  dialogTemplate->renderTemplateText(output, dialogTemplate->templateText());
  out = output.str();
  resetStream(output);
  strip(out);
  dialogTemplate->renderTemplateText(output, WString::tr("Wt.WDialog.template.default"));
  unstyled = output.str();
  resetStream(output);
  strip(unstyled);
  BOOST_TEST(out != unstyled);

  //WInPlaceEdit
  BOOST_TEST(tw.inPlaceEdit->hasStyleClass("Wt-in-place-edit"));

  auto inPlaceEditBtnParent = tw.inPlaceEdit->saveButton()->parent();
  auto inPlaceEditChilds = tw.inPlaceEdit->children()[0]->children();
  bool inPlaceEditHasInputGroupWithBtn = false;
  for (auto i = inPlaceEditChilds.begin(); i != inPlaceEditChilds.end(); ++i) {
    inPlaceEditHasInputGroupWithBtn |= (*i)->hasStyleClass("input-group") &&
                                       (*i) == inPlaceEditBtnParent;
  }
  BOOST_TEST(inPlaceEditHasInputGroupWithBtn);

  BOOST_TEST(tw.inPlaceEdit->saveButton()->hasStyleClass("btn-outline-secondary"));

  BOOST_TEST(tw.inPlaceEdit->cancelButton()->hasStyleClass("btn-outline-secondary"));


  //WMenu
  BOOST_TEST(tw.menu->hasStyleClass("nav"));

  auto menuItems = tw.menu->items();
  bool menuHasActiveItemClass = false;
  for (auto i = menuItems.begin(); i != menuItems.end(); ++i) {
    BOOST_TEST((*i)->hasStyleClass("nav-item"));

    bool isActive = (*i)->hasStyleClass(testApp.theme()->activeClass());
    menuHasActiveItemClass |= isActive;

    auto anchor = (*i)->anchor();
    BOOST_TEST(anchor->hasStyleClass("nav-link"));

    // the anchor should be active when the item is active
    BOOST_TEST(isActive == anchor->hasStyleClass(testApp.theme()->activeClass()));
  }
  BOOST_TEST(menuHasActiveItemClass); // at least one item should be active

  //WMessageBox
  BOOST_TEST(tw.messageBox->contents()->hasStyleClass("Wt-msgbox-body"));

  auto messageBoxContentsChilds = tw.messageBox->contents()->children();
  for (auto i = messageBoxContentsChilds.begin(); i != messageBoxContentsChilds.end(); ++i)
  {
    BOOST_TEST(((*i)->hasStyleClass("Wt-msgbox-text") ||
                (*i)->hasStyleClass("Wt-msgbox-icon")));
  }

  //WNavigationBar
  BOOST_TEST(tw.navBar->hasStyleClass("navbar"));
  BOOST_TEST(tw.navBar->hasStyleClass("navbar-expand-lg"));

  tw.navBar->renderTemplateText(output, tw.navBar->templateText());
  out = output.str();
  resetStream(output);
  strip(out);
  tw.navBar->renderTemplateText(output, WString::tr("Wt.WNavigationBar.template.default"));
  unstyled = output.str();
  resetStream(output);
  strip(unstyled);
  BOOST_TEST(out != unstyled);

  auto navBarExpendBtn = tw.navBar->resolveWidget("expand-button");
  BOOST_TEST(navBarExpendBtn == nullptr);

  auto navBarCollapseBtn = tw.navBar->resolveWidget("collapse-button");
  BOOST_TEST(navBarCollapseBtn->hasStyleClass("navbar-toggler"));

  auto navBarTitle = tw.navBar->resolveWidget("title-link");
  BOOST_TEST(navBarTitle->hasStyleClass("navbar-brand"));

  auto navBarContents = tw.navBar->resolve<WContainerWidget *>("contents");
  BOOST_TEST(navBarContents->hasStyleClass("navbar-collapse"));
  BOOST_TEST(navBarContents->hasStyleClass("collapse"));

  auto navBarContentsChilds = navBarContents->children();
  bool navBarHasNavbarClass = false;
  bool navBarHasDFlexClass = false;
  for (auto i = navBarContentsChilds.begin(); i != navBarContentsChilds.end(); ++i)
  {
    navBarHasNavbarClass |= (*i)->hasStyleClass("navbar-nav");
    navBarHasDFlexClass |= (*i)->hasStyleClass("d-flex");
  }
  BOOST_TEST(navBarHasNavbarClass);
  BOOST_TEST(navBarHasDFlexClass);

  //WPanel
  BOOST_TEST(tw.panel->hasStyleClass("Wt-panel"));
  BOOST_TEST(tw.panel->hasStyleClass("card"));

  BOOST_TEST(tw.panel->titleBarWidget()->hasStyleClass("card-header"));

  auto panelTitleBarChilds = tw.panel->titleBarWidget()->children();
  bool panelHasCollapseButton = false;
  for (auto i = panelTitleBarChilds.begin(); i != panelTitleBarChilds.end(); ++i)
  {
    panelHasCollapseButton |= (*i)->hasStyleClass("Wt-collapse-button");
  }
  BOOST_TEST(panelHasCollapseButton);

  BOOST_TEST(tw.panel->centralWidget()->parent()->hasStyleClass("card-body"));

  //WPushButton
  BOOST_TEST(tw.pushButton->hasStyleClass("btn"));
  BOOST_TEST(tw.pushButton->hasStyleClass("btn-secondary"));
  BOOST_TEST(tw.pushButton->hasStyleClass("dropdown-toggle"));

  //WSplitButton
  BOOST_TEST(tw.splitButton->hasStyleClass("btn-group"));
  BOOST_TEST(tw.splitButton->dropDownButton()->hasStyleClass("dropdown-toggle"));

  //WTimeEdit
  BOOST_TEST(tw.timeEdit->hasStyleClass("Wt-timeedit"));

  //WToolBar
  BOOST_TEST((tw.toolbar->hasStyleClass("btn-toolbar") ||
              tw.toolbar->hasStyleClass("btn-group")));

  BOOST_TEST(tw.toolbar->hasStyleClass("btn-group-vertical"));

  auto toolbarChilds =  tw.toolbar->children()[0]->children();
  for (auto i = toolbarChilds.begin(); i != toolbarChilds.end(); ++i)
  {
    BOOST_TEST((*i)->hasStyleClass("btn-group"));
    BOOST_TEST((*i)->hasStyleClass("me-2"));
  }
}