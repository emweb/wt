// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef TREEVIEWEXAMPLE_H_
#define TREEVIEWEXAMPLE_H_

#include <Wt/WContainerWidget>
#include <Wt/WString>
#include <string>
#include <vector>

enum WeatherIcon {
  Sun,
  SunCloud,
  Cloud,
  Rain,
  Storm,
  Snow
};

namespace Wt {
  class WStandardItem;
  class WStandardItemModel;
  class WTreeView;
  class WText;
}

class TreeViewExample : public Wt::WContainerWidget
{
public:
  TreeViewExample(Wt::WStandardItemModel *model,
		  const Wt::WString& titleText);

  Wt::WTreeView *treeView() const { return treeView_; }

  static Wt::WStandardItemModel *createModel(bool useInternalPath,
					     WObject *parent);

private:
  Wt::WStandardItem      *belgium_;
  Wt::WStandardItemModel *model_;
  Wt::WTreeView          *treeView_;

  static Wt::WStandardItem *continentItem(const std::string& continent);
  static Wt::WStandardItem *countryItem(const std::string& country,
					const std::string& code);
  static std::vector<Wt::WStandardItem *> cityItems(const std::string& city,
						    WeatherIcon weather,
						    const std::string& drink,
						    bool useInternalPath,
						    bool visited);
  void toggleRowHeight();
  void toggleStripes();
  void toggleRoot();
  void addRows();
};

#endif
