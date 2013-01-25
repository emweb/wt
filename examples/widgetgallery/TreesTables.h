// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TREE_TABLES_H_
#define TREE_TABLES_H_

#include "TopicWidget.h"

class TreesTables : public TopicWidget
{
public:
  TreesTables();

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WLineEdit *regexpFilter;
  Wt::WSortFilterProxyModel *filteredCocktails;
  Wt::WSortFilterProxyModel *filteredSortedCocktails;

  Wt::WWidget *tables();
  Wt::WWidget *trees();
  Wt::WWidget *treeTables();
  Wt::WWidget *tableViews();
  Wt::WWidget *treeViews();
  Wt::WWidget *itemModels();
  Wt::WWidget *proxyModels();

  Wt::WStringListModel *stringList_;

  void changeRegexp();
};

#endif
