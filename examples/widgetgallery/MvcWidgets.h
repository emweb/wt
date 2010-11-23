// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MVCWIDGETS_H_
#define MVCWIDGETS_H_

#include "ControlsWidget.h"

#include <Wt/WSortFilterProxyModel>
#include <Wt/WLineEdit>

namespace Wt {
  class WStringListModel;
  namespace Ext {
    class ComboBox;
  }
}

class MvcWidgets : public ControlsWidget
{
public:
  MvcWidgets(EventDisplayer *ed);

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WLineEdit *regexpFilter;
  Wt::WSortFilterProxyModel *filteredCocktails;
  Wt::WSortFilterProxyModel *filteredSortedCocktails;

  Wt::WWidget *models();
  Wt::WWidget *proxyModels();
  Wt::WWidget *viewsCombo();
  Wt::WWidget *viewsTable();
  Wt::WWidget *viewsTree();
  Wt::WWidget *viewsChart();


  Wt::WStringListModel *stringList_;
#ifndef WT_TARGET_JAVA
  Wt::Ext::ComboBox *extComboBox_;
  void comboBoxAdd();
#endif

  void changeRegexp() {
    filteredCocktails->setFilterRegExp(regexpFilter->text());
    filteredSortedCocktails->setFilterRegExp(regexpFilter->text());
  }
};

#endif
