// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
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

  std::unique_ptr<Wt::WWidget> tables();
  std::unique_ptr<Wt::WWidget> trees();
  std::unique_ptr<Wt::WWidget> treeTables();
  std::unique_ptr<Wt::WWidget> tableViews();
  std::unique_ptr<Wt::WWidget> treeViews();
  std::unique_ptr<Wt::WWidget> itemModels();
  std::unique_ptr<Wt::WWidget> proxyModels();

  Wt::WStringListModel *stringList_;

  void changeRegexp();
};

#endif
