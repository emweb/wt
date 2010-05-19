// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXT_KITCHEN_APPLICATION_H_
#define EXT_KITCHEN_APPLICATION_H_

#include <Wt/WApplication>

#include <iostream>

namespace Wt {
  class WStandardItemModel;
  class WTable;
  class WTreeNode;
  class WTableView;
  class WWidget;

  namespace Ext {
    class ComboBox;
    class MessageBox;
    class TableView;
    class TabWidget;
    class TextEdit;
  }
}

class ExtKitchenApplication : public Wt::WApplication
{
public:
  ExtKitchenApplication(const Wt::WEnvironment& env);

private:
  typedef void (ExtKitchenApplication::*ShowExample)();

  Wt::WWidget                       *currentExample_;
  Wt::WContainerWidget              *exampleContainer_;

  Wt::WWidget *createExampleTree();
  Wt::WTreeNode *createExampleNode(const Wt::WString& label,
				   Wt::WTreeNode *parentNode,
				   ShowExample f);
  void setExample(Wt::WWidget *exampleWidget);

  void menuAndToolBarExample();
  void formWidgetsExample();
  void tableViewExample();
  void dialogExample();
  void tabWidgetExample();

  void createDialog();
  void createDialog2();
  void createDialog3();
  void createDialog4();
  void createDialog5();
  void createDialog6();
  void createDialog7();

  /*
   * Delete the dialog
   */
  Wt::Ext::MessageBox *mbox_;
  void testDelete();

  /*
   * Modify the combo box
   */
  void formModify();
  Wt::Ext::ComboBox *cb;
  Wt::Ext::TextEdit *html_;

  /*
   * Modify the WTableView model
   */
  void addRow();
  void removeRow();
  void resetModel();
  void addChildren();
  Wt::WStandardItemModel *model_;
  Wt::Ext::TableView *table1_, *table2_;
  Wt::WTableView *tableView_;

  /*
   * Modify the tabwidget
   */
  Wt::Ext::TabWidget *tb;
  void hideTab();
  void showTab();
  void modifyTabWidget();
};

#endif // EXT_KITCHEN_APPLICATION_H_
