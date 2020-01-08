/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WMenu.h>

#include "TreesTables.h"
#include "TopicTemplate.h"
#include "DeferredWidget.h"

#include "../treeview-dragdrop/CsvUtil.h"

#include <iostream>

TreesTables::TreesTables():
  TopicWidget()
{
  addText(tr("mvc-intro"), this);
}

void TreesTables::populateSubMenu(WMenu *menu)
{
  menu->setInternalBasePath("/trees-tables");

  menu->addItem("Tables", tables())->setPathComponent("");
  menu->addItem("Trees",
                deferCreate([this]{ return trees(); }));
  menu->addItem("Tree Tables",
                deferCreate([this]{ return treeTables(); }));
  menu->addItem("MVC Table Views",
                deferCreate([this]{ return tableViews(); }));
  menu->addItem("MVC Tree Views",
                deferCreate([this]{ return treeViews(); }));
  menu->addItem("MVC Item models",
                deferCreate([this]{ return itemModels(); }));
  // menu->addItem("Proxy item models", proxyModels());
}

#include "examples/PlainTable.cpp"
#include "examples/StyledTable.cpp"

std::unique_ptr<WWidget> TreesTables::tables()
{
  auto result = cpp14::make_unique<TopicTemplate>("treestables-Tables");
 
  result->bindWidget("PlainTable", PlainTable());
  result->bindWidget("StyledTable", StyledTable());
 
  return std::move(result);
}

#include "examples/Tree.cpp"

std::unique_ptr<WWidget> TreesTables::trees()
{
  auto result = cpp14::make_unique<TopicTemplate>("treestables-Trees");

  result->bindWidget("Tree", Tree());

  return std::move(result);
}

#include "examples/TreeTable.cpp"

std::unique_ptr<WWidget> TreesTables::treeTables()
{
  auto result = cpp14::make_unique<TopicTemplate>("treestables-TreeTables");

  result->bindWidget("TreeTable", TreeTable());

  return std::move(result);
}

#include "examples/VirtualModel.cpp"
#include "examples/SmallTableView.cpp"
#include "examples/LargeTableView.cpp"
#include "examples/ComboDelegateTable.cpp"

std::unique_ptr<WWidget> TreesTables::tableViews()
{
  auto result = cpp14::make_unique<TopicTemplate>("treestables-TableViews");

  result->bindWidget("SmallTableView", SmallTableView());
  result->bindWidget("LargeTableView", LargeTableView());
  result->bindWidget("ComboDelegateTable", ComboDelegateTable());

  return std::move(result);
}

#ifndef GIT_REPOSITORY
#ifndef WT_TARGET_JAVA
#define GIT_REPOSITORY "../.."
#else
#define GIT_REPOSITORY "/home/koen/git/jwt"
#endif
#endif

#ifndef WT_WIN32
#include "examples/GitModel.cpp"
#include "examples/TreeView.cpp"
#endif

std::unique_ptr<WWidget> TreesTables::treeViews()
{
  auto result = cpp14::make_unique<TopicTemplate>("treestables-TreeViews");
#ifndef WT_WIN32
  result->bindWidget("TreeView", TreeView());
#else
  result->bindString("TreeView", "Example not available on windows");
#endif
  return std::move(result);
}

std::unique_ptr<WWidget> TreesTables::itemModels()
{
  auto result = cpp14::make_unique<TopicTemplate>("treestables-ItemModels");

  result->bindWidget("LargeTableView", LargeTableView());
#ifndef WT_WIN32
  result->bindWidget("TreeView", TreeView());
#else
  result->bindString("TreeView", "Example not available on windows");
#endif

  return std::move(result);
}

#if 0
void TreesTables::changeRegexp()
{
  WString regexp = regexpFilter->text();

  bool valid;
#ifndef WT_TARGET_JAVA
  valid = WRegExp(regexp).isValid();
#else
  try {
    WRegExp r(regexp.toUTF8());
    valid = true;
  } catch (std::exception& e) {
    valid = false;
  }
#endif

  if (valid) {
    filteredCocktails->setFilterRegExp(regexp);
    filteredSortedCocktails->setFilterRegExp(regexp);
    regexpFilter->removeStyleClass("Wt-invalid");
  } else {
    regexpFilter->addStyleClass("Wt-invalid");
  }
}

std::unique_ptr<WWidget> TreesTables::proxyModels()
{
  auto result = cpp14::make_unique<WContainerWidget>();

  topic("WAbstractProxyModel", "WSortFilterProxyModel", result); // TODO
  result->addWidget(addText(tr("mvc-proxymodels")));

  auto cocktails = std::make_shared<WStandardItemModel>();
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("The Last WordLime Rickey"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Gin pahit"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Alexander"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Montgomery"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Gin Sour"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Hanky-Panky"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Gimlet"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Chocolate Soldier"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Joker"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Mickey Slim"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Long Island Iced Tea"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Old Etonian"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Lorraine"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Bijou"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Bronx"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Gin and tonic"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Pall Mall"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Gin Fizz"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("French 75"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Martini"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Negroni"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("20th Century"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("My Fair Lady"));
  cocktails->appendRow(cpp14::make_unique<WStandardItem>("Gibson"));

  result->addWidget(addText("<b>Filter regular expression: </b>"));
  regexpFilter = result->addWidget(cpp14::make_unique<WLineEdit>());
  regexpFilter->setText("Gi.*");
  regexpFilter->enterPressed().
    connect(this, &TreesTables::changeRegexp);
  WPushButton *filter = result->addWidget(cpp14::make_unique<WPushButton>("Apply"));
  filter->clicked().
    connect(this, &TreesTables::changeRegexp);
  
  std::vector<std::shared_ptr<WAbstractItemModel>> models;
  std::vector<WString> headers;

  headers.push_back(WString("<b>Source:</b>"));
  models.push_back(cocktails);

  headers.push_back(WString("<b>Sorted proxy:</b>"));
  std::shared_ptr<WSortFilterProxyModel> sortedCocktails =
      std::make_shared<WSortFilterProxyModel>();
  sortedCocktails->setSourceModel(cocktails);
  sortedCocktails->setDynamicSortFilter(true);
  sortedCocktails->sort(0);
  models.push_back(sortedCocktails);

  headers.push_back(WString("<b>Filtered proxy:</b>"));
  filteredCocktails = std::make_shared<WSortFilterProxyModel>();
  filteredCocktails->setSourceModel(cocktails);
  filteredCocktails->setDynamicSortFilter(true);
  filteredCocktails->setFilterKeyColumn(0);
  filteredCocktails->setFilterRole(ItemDataRole::Display);
  filteredCocktails->setFilterRegExp(regexpFilter->text());
  models.push_back(filteredCocktails);

  headers.push_back(WString("<b>Sorted and filtered proxy:</b>"));
  filteredSortedCocktails = std::make_shared<WSortFilterProxyModel>();
  filteredSortedCocktails->setSourceModel(cocktails);
  filteredSortedCocktails->setDynamicSortFilter(true);
  filteredSortedCocktails->setFilterKeyColumn(0);
  filteredSortedCocktails->setFilterRole(Wt::DisplayRole);
  filteredSortedCocktails->setFilterRegExp(regexpFilter->text());
  filteredSortedCocktails->sort(0);
  models.push_back(filteredSortedCocktails);

  WTable *layout = result->addWidget(cpp14::make_unique<WTable>());

  for (unsigned i = 0; i < headers.size(); ++i) {
    layout->columnAt(i)->setWidth(WLength(25, LengthUnit::Percentage));
    layout->elementAt(0, i)->setPadding(4);
    layout->elementAt(0, i)->setContentAlignment(AligmentFlag::Center);

    layout->elementAt(0, i)->addWidget(addText(headers[i]));
    latout->elementAt(0, i)->addWidget(cpp14::make_unique<WBreak>());

    WSelectionBox *view =
        layout->elementAt(0, i)->addWidget(cpp14::make_unique<WSelectionBox>());
    view->setModel(models[i]);
    view->setVerticalSize(cocktails->rowCount());
    view->resize(WLength(90, LengthUnit::Percentage), WLength::Auto);
  }

  return result;
}

#endif
