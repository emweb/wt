/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "MvcWidgets.h"
#include "TreeViewExample.h"
#include "DeferredWidget.h"

#include <Wt/WBreak>
#include <Wt/WComboBox>
#include <Wt/WPanel>
#include <Wt/WPushButton>
#include <Wt/WSelectionBox>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WStringListModel>
#include <Wt/WText>
#include <Wt/WTreeView>
#include <Wt/WTable>
#include <Wt/Ext/ComboBox>
#include <iostream>
using namespace Wt;

MvcWidgets::MvcWidgets(EventDisplayer *ed):
  ControlsWidget(ed, true)
{
  new WText(tr("mvc-intro"), this);

  stringList_ = new WStringListModel(this);
  std::vector<WString> strings;
  strings.push_back("Alfa");
  strings.push_back("Bravo");
  strings.push_back("Charly");
  strings.push_back("Delta");
  strings.push_back("Echo");
  strings.push_back("Foxtrot");
  strings.push_back("Golf");
  strings.push_back("Hotel");
  strings.push_back("Indiana Jones");
  stringList_->setStringList(strings);

}

void MvcWidgets::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("The Models", models());
  menu->addItem("Proxy models", proxyModels());
  menu->addItem("Combobox Views",
		deferCreate(boost::bind(&MvcWidgets::viewsCombo, this)));
  menu->addItem("WTableView",
		deferCreate(boost::bind(&MvcWidgets::viewsTable, this)));
  menu->addItem("WTreeView",
		deferCreate(boost::bind(&MvcWidgets::viewsTree, this)));
  menu->addItem("Chart Views", viewsChart());
}

#ifndef WT_TARGET_JAVA
void MvcWidgets::comboBoxAdd()
{
  if (extComboBox_->currentIndex() == -1) {
    std::vector<WString> sl = stringList_->stringList();
    sl.push_back(extComboBox_->currentText());
    stringList_->setStringList(sl);
  }
}
#endif

WWidget *MvcWidgets::models()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WAbstractItemModel", "WAbstractListModel", "WStandardItemModel",
	"WStringListModel", result);
  new WText(tr("mvc-models"), result);
  return result;
}

WWidget *MvcWidgets::proxyModels()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WAbstractProxyModel", "WSortFilterProxyModel", result);
  new WText(tr("mvc-proxymodels"), result);

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

  new WText("<b>Filter regular expression: </b>", result);
  regexpFilter = new WLineEdit(result);
  regexpFilter->setText("Gi.*");
  regexpFilter->enterPressed().
    connect(this, &MvcWidgets::changeRegexp);
  WPushButton *filter = new WPushButton("Apply", result);
  filter->clicked().
    connect(this, &MvcWidgets::changeRegexp);
  
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

  for (int i = 0; i < headers.size(); ++i) {
    layout->columnAt(i)->setWidth(WLength(25, WLength::Percentage));
    layout->elementAt(0, i)->setPadding(4);
    layout->elementAt(0, i)->setContentAlignment(AlignCenter);

    new WText(headers[i], layout->elementAt(0, i));
    new WBreak(layout->elementAt(0, i));

    WSelectionBox *view = new WSelectionBox(layout->elementAt(0, i));
    view->setModel(models[i]);
    view->setVerticalSize(cocktails->rowCount());
    view->resize(WLength(90, WLength::Percentage), WLength::Auto);
  }

  return result;
}

WWidget *MvcWidgets::viewsCombo()
{
  WContainerWidget *result = new WContainerWidget();

  // WComboBox
  topic("WComboBox", "WSelectionBox", "Ext::ComboBox", result);
  new WText(tr("mvc-stringlistviews"), result);
  new WText("<h3>WComboBox</h3>", result);
  (new WComboBox(result))->setModel(stringList_);

  // WSelectionBox
  new WText("<h3>WSelectionBox</h3>", result);
  (new WSelectionBox(result))->setModel(stringList_);

#ifndef WT_TARGET_JAVA
  // Ext::ComboBox
  new WText("<h3>Ext::ComboBox</h3>", result);
  extComboBox_ = new Ext::ComboBox(result);
  extComboBox_->setModel(stringList_);
  extComboBox_->setEditable(true);
  WPushButton *pb = new WPushButton("Press here to add the edited value "
				    "to the model", result);
  pb->clicked().connect(this, &MvcWidgets::comboBoxAdd);
#endif
  
  return result;
}

WWidget *MvcWidgets::viewsTable()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTableView", result);
  new WText(tr("mvc-WTableView"), result);
  return result;
}

WWidget *MvcWidgets::viewsTree()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTreeView", result);

  WStandardItemModel *model = TreeViewExample::createModel(false, this);

  TreeViewExample *tv1 = new TreeViewExample(model, tr("mvc-WTreeView"));
  result->addWidget(tv1);

  TreeViewExample *tv2 = new TreeViewExample(model,
					     tr("mvc-WTreeView-column1Fixed"));
  result->addWidget(tv2);

  tv2->treeView()->setColumn1Fixed(true);
  tv2->treeView()->setColumnWidth(0, 300);

  return result;
}

WWidget *MvcWidgets::viewsChart()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Chart::WCartesianChart", "Chart::WPieChart", result);
  new WText(tr("mvc-Chart"), result);
  return result;
}
