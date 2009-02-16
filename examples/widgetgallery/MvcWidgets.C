/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "MvcWidgets.h"
#include "TreeViewExample.h"
#include "DeferredWidget.h"

#include <Wt/WComboBox>
#include <Wt/WPushButton>
#include <Wt/WSelectionBox>
#include <Wt/WStringListModel>
#include <Wt/WText>
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
  menu->addItem("Combobox Views",
		deferCreate(boost::bind(&MvcWidgets::viewsCombo, this)));
  menu->addItem("WTreeView", viewsTree());
  menu->addItem("Chart Views", viewsChart());
  menu->addItem("Ext::TableView",
		deferCreate(boost::bind(&MvcWidgets::viewsExtTable, this)));
}

void MvcWidgets::comboBoxAdd()
{
  if (extComboBox_->currentIndex() == -1) {
    std::vector<WString> sl = stringList_->stringList();
    sl.push_back(extComboBox_->currentText());
    //std::cout << "combobox text: " << extComboBox_->currentText() << std::endl;
    //sl.push_back("Blabla");
    stringList_->setStringList(sl);
    //stringList_->insertRows(0, 1);
    //stringList_->setData(0, 0, WString("Blabla"));
  }
}

WWidget *MvcWidgets::models()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WAbstractItemModel", "WAbstractListModel", "WStandardItemModel",
	"WStringListModel", result);
  new WText(tr("mvc-models"), result);
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

  // Ext::ComboBox
  new WText("<h3>Ext::ComboBox</h3>", result);
  extComboBox_ = new Ext::ComboBox(result);
  extComboBox_->setModel(stringList_);
  extComboBox_->setEditable(true);
  WPushButton *pb = new WPushButton("Press here to add the edited value "
				    "to the model", result);
  pb->clicked.connect(SLOT(this, MvcWidgets::comboBoxAdd));
  
  return result;
}

WWidget *MvcWidgets::viewsExtTable()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::TableView", result);
  new WText(tr("mvc-ExtTable"), result);
  return result;
}

WWidget *MvcWidgets::viewsTree()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTreeView", result);
  new WText(tr("mvc-WTreeView"), result);
  new TreeViewExample(false, result);
  return result;
}

WWidget *MvcWidgets::viewsChart()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Chart::WCartesianChart", "Chart::WPieChart", result);
  new WText(tr("mvc-Chart"), result);
  return result;
}
