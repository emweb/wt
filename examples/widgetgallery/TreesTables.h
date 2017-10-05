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
  std::shared_ptr<Wt::WSortFilterProxyModel> filteredCocktails;
  std::shared_ptr<Wt::WSortFilterProxyModel> filteredSortedCocktails;

  std::unique_ptr<WWidget> tables();
  std::unique_ptr<WWidget> trees();
  std::unique_ptr<WWidget> treeTables();
  std::unique_ptr<WWidget> tableViews();
  std::unique_ptr<WWidget> treeViews();
  std::unique_ptr<WWidget> itemModels();
  std::unique_ptr<WWidget> proxyModels();

  Wt::WStringListModel *stringList_;

  void changeRegexp();
};

#endif
