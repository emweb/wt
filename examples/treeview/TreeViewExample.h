// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef TREEVIEWEXAMPLE_H_
#define TREEVIEWEXAMPLE_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WString.h>
#include <string>
#include <vector>

using namespace Wt;

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

class TreeViewExample : public WContainerWidget
{
public:
  TreeViewExample(std::shared_ptr<WStandardItemModel> model,
                  const WString& titleText);

  WTreeView *treeView() const { return treeView_; }

  static std::shared_ptr<WStandardItemModel> createModel(bool useInternalPath);

private:
  WStandardItem      *belgium_;
  std::shared_ptr<WStandardItemModel> model_;
  WTreeView          *treeView_;

  static std::unique_ptr<WStandardItem> continentItem(const std::string& continent);
  static std::unique_ptr<WStandardItem> countryItem(const std::string& country,
					const std::string& code);
  static std::vector<std::unique_ptr<WStandardItem> > cityItems(const std::string& city,
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
