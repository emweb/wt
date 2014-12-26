/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>

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

void TreesTables::populateSubMenu(Wt::WMenu *menu)
{
  menu->setInternalBasePath("/trees-tables");

  menu->addItem("Tables", tables())->setPathComponent("");
  menu->addItem("Trees",
		deferCreate(boost::bind
			    (&TreesTables::trees, this)));
  menu->addItem("Tree Tables",
		deferCreate(boost::bind
			    (&TreesTables::treeTables, this)));
  menu->addItem("MVC Table Views",
		deferCreate(boost::bind
			    (&TreesTables::tableViews, this)));
  menu->addItem("MVC Tree Views",
		deferCreate(boost::bind
			    (&TreesTables::treeViews, this)));
  menu->addItem("MVC Item models",
		deferCreate(boost::bind
			    (&TreesTables::itemModels, this)));
  // menu->addItem("Proxy item models", proxyModels());
}

#include "examples/PlainTable.cpp"
#include "examples/StyledTable.cpp"

Wt::WWidget *TreesTables::tables()
{
  Wt::WTemplate *result = new TopicTemplate("treestables-Tables");
 
  result->bindWidget("PlainTable", PlainTable());
  result->bindWidget("StyledTable", StyledTable());
 
  return result;
}

#include "examples/Tree.cpp"

Wt::WWidget *TreesTables::trees()
{
  Wt::WTemplate *result = new TopicTemplate("treestables-Trees");

  result->bindWidget("Tree", Tree());

  return result;
}

#include "examples/TreeTable.cpp"

Wt::WWidget *TreesTables::treeTables()
{
  Wt::WTemplate *result = new TopicTemplate("treestables-TreeTables");

  result->bindWidget("TreeTable", TreeTable());

  return result;
}

#include "examples/VirtualModel.cpp"
#include "examples/SmallTableView.cpp"
#include "examples/LargeTableView.cpp"
#include "examples/ComboDelegateTable.cpp"

Wt::WWidget *TreesTables::tableViews()
{
  Wt::WTemplate *result = new TopicTemplate("treestables-TableViews");

  result->bindWidget("SmallTableView", SmallTableView());
  result->bindWidget("LargeTableView", LargeTableView());
  result->bindWidget("ComboDelegateTable", ComboDelegateTable());

  return result;  
}

#ifndef GIT_REPOSITORY
#ifndef WT_TARGET_JAVA
#define GIT_REPOSITORY "../../.git"
#else
#define GIT_REPOSITORY "/home/koen/git/jwt"
#endif
#endif

#ifndef WT_WIN32
#include "examples/GitModel.cpp"
#include "examples/TreeView.cpp"
#endif

Wt::WWidget *TreesTables::treeViews()
{
  Wt::WTemplate *result = new TopicTemplate("treestables-TreeViews");
#ifndef WT_WIN32
  result->bindWidget("TreeView", TreeView());
#else
  result->bindString("TreeView", "Example not available on windows");
#endif
  return result;  
}

Wt::WWidget *TreesTables::itemModels()
{
  Wt::WTemplate *result = new TopicTemplate("treestables-ItemModels");

  result->bindWidget("LargeTableView", LargeTableView());
#ifndef WT_WIN32
  result->bindWidget("TreeView", TreeView());
#else
  result->bindString("TreeView", "Example not available on windows");
#endif

  return result;
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

WWidget *TreesTables::proxyModels()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WAbstractProxyModel", "WSortFilterProxyModel", result);
  addText(tr("mvc-proxymodels"), result);

  WStandardItemModel* cocktails = new WStandardItemModel(result);
  cocktails->appendRow(new WStandardItem("The Last WordLime Rickey"));
  cocktails->appendRow(new WStandardItem("Gin pahit"));
  cocktails->appendRow(new WStandardItem("Alexander"));
  cocktails->appendRow(new WStandardItem("Montgomery"));
  cocktails->appendRow(new WStandardItem("Gin Sour"));
  cocktails->appendRow(new WStandardItem("Hanky-Panky"));
  cocktails->appendRow(new WStandardItem("Gimlet"));
  cocktails->appendRow(new WStandardItem("Chocolate Soldier"));
  cocktails->appendRow(new WStandardItem("Joker"));
  cocktails->appendRow(new WStandardItem("Mickey Slim"));
  cocktails->appendRow(new WStandardItem("Long Island Iced Tea"));
  cocktails->appendRow(new WStandardItem("Old Etonian"));
  cocktails->appendRow(new WStandardItem("Lorraine"));
  cocktails->appendRow(new WStandardItem("Bijou"));
  cocktails->appendRow(new WStandardItem("Bronx"));
  cocktails->appendRow(new WStandardItem("Gin and tonic"));
  cocktails->appendRow(new WStandardItem("Pall Mall"));
  cocktails->appendRow(new WStandardItem("Gin Fizz"));
  cocktails->appendRow(new WStandardItem("French 75"));
  cocktails->appendRow(new WStandardItem("Martini"));
  cocktails->appendRow(new WStandardItem("Negroni"));
  cocktails->appendRow(new WStandardItem("20th Century"));
  cocktails->appendRow(new WStandardItem("My Fair Lady"));
  cocktails->appendRow(new WStandardItem("Gibson"));

  addText("<b>Filter regular expression: </b>", result);
  regexpFilter = new WLineEdit(result);
  regexpFilter->setText("Gi.*");
  regexpFilter->enterPressed().
    connect(this, &TreesTables::changeRegexp);
  WPushButton *filter = new WPushButton("Apply", result);
  filter->clicked().
    connect(this, &TreesTables::changeRegexp);
  
  std::vector<WAbstractItemModel*> models;
  std::vector<WString> headers;

  headers.push_back(WString("<b>Source:</b>"));
  models.push_back(cocktails);

  headers.push_back(WString("<b>Sorted proxy:</b>"));
  WSortFilterProxyModel *sortedCocktails = new WSortFilterProxyModel(this);
  sortedCocktails->setSourceModel(cocktails);
  sortedCocktails->setDynamicSortFilter(true);
  sortedCocktails->sort(0);
  models.push_back(sortedCocktails);

  headers.push_back(WString("<b>Filtered proxy:</b>"));
  filteredCocktails = new WSortFilterProxyModel(this);
  filteredCocktails->setSourceModel(cocktails);
  filteredCocktails->setDynamicSortFilter(true);
  filteredCocktails->setFilterKeyColumn(0);
  filteredCocktails->setFilterRole(Wt::DisplayRole);
  filteredCocktails->setFilterRegExp(regexpFilter->text());
  models.push_back(filteredCocktails);

  headers.push_back(WString("<b>Sorted and filtered proxy:</b>"));
  filteredSortedCocktails = new WSortFilterProxyModel(this);
  filteredSortedCocktails->setSourceModel(cocktails);
  filteredSortedCocktails->setDynamicSortFilter(true);
  filteredSortedCocktails->setFilterKeyColumn(0);
  filteredSortedCocktails->setFilterRole(Wt::DisplayRole);
  filteredSortedCocktails->setFilterRegExp(regexpFilter->text());
  filteredSortedCocktails->sort(0);
  models.push_back(filteredSortedCocktails);

  WTable *layout = new WTable(result);

  for (unsigned i = 0; i < headers.size(); ++i) {
    layout->columnAt(i)->setWidth(WLength(25, WLength::Percentage));
    layout->elementAt(0, i)->setPadding(4);
    layout->elementAt(0, i)->setContentAlignment(AlignCenter);

    addText(headers[i], layout->elementAt(0, i));
    new WBreak(layout->elementAt(0, i));

    WSelectionBox *view = new WSelectionBox(layout->elementAt(0, i));
    view->setModel(models[i]);
    view->setVerticalSize(cocktails->rowCount());
    view->resize(WLength(90, WLength::Percentage), WLength::Auto);
  }

  return result;
}

#endif
