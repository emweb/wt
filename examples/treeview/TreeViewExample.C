/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "TreeViewExample.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WText>
#include <Wt/WTreeView>

using namespace Wt;

static const char *weatherIcons[] = {
  "sun01.png",
  "cloudy01.png",
  "w_cloud.png",
  "rain.png",
  "storm.png",
  "snow.png"
};

TreeViewExample::TreeViewExample(bool useInternalPath,
				 WContainerWidget *parent)
  : WContainerWidget(parent),
    useInternalPath_(useInternalPath)
				 
{
  new WText(WString::tr("treeview-introduction"), this);
  
  /*
   * Setup a model.
   *
   * We use the standard item model, which is a general model
   * suitable for hierarchical (tree-like) data, but stores all data
   * in memory.
   */
  model_ = new WStandardItemModel(0, 4, this);

  /*
   * Headers ...
   */
  model_->setHeaderData(0, Horizontal, std::string("Places"));
  model_->setHeaderData(1, Horizontal, std::string("Weather"));
  model_->setHeaderData(2, Horizontal, std::string("Drink"));
  model_->setHeaderData(3, Horizontal, std::string("Visited"));
  
  /*
   * ... and data
   */
  WStandardItem *continent, *country;
  
  model_->appendRow(continent = continentItem("Europe"));
  
  continent->appendRow(country = countryItem("Belgium", "be"));
  country->appendRow(cityItems("Brussels", Rain, "Beer", true));
  country->appendRow(cityItems("Leuven", Rain, "Beer", true));
  
  belgium_ = country;
  
  continent->appendRow(country = countryItem("France", "fr"));
  country->appendRow(cityItems("Paris", Cloud, "Wine", true));
  country->appendRow(cityItems("Bordeaux", SunCloud, "Bordeaux wine", false));
  
  continent->appendRow(country = countryItem("Spain", "sp"));
  country->appendRow(cityItems("Barcelona", Sun, "Cava", true));
  country->appendRow(cityItems("Madrid", Sun, "San Miguel", false));
  
  model_->appendRow(continent = continentItem("Africa"));
  
  continent->appendRow(country = countryItem("Morocco (المغرب)", "ma"));
  country->appendRow(cityItems("Casablanca", Sun, "Tea", false));
  
  /*
   * Now create the view
   */
  treeView_ = new WTreeView(this);
  //treeView_->setColumn1Fixed(true);
  treeView_->setAlternatingRowColors(!treeView_->alternatingRowColors());
  treeView_->setRowHeight(30);
  //treeView_->setHeaderHeight(40, true);
  treeView_->setModel(model_);
  //treeView_->setSortingEnabled(false);
  //treeView_->setColumnResizeEnabled(false);
  treeView_->setSelectionMode(NoSelection);
  treeView_->resize(600, 300);

  treeView_->setColumnWidth(1, WLength(100));
  treeView_->setColumnAlignment(1, AlignCenter);
  treeView_->setColumnWidth(3, WLength(100));
  treeView_->setColumnAlignment(3, AlignCenter);

  /*
   * Expand the first (and single) top level node
   */
  treeView_->setExpanded(model_->index(0, 0), true);
  
  /*
   * Setup some buttons to manipulate the view and the model.
   */
  WContainerWidget *wc = new WContainerWidget(this);
  WPushButton *b;
  
  b = new WPushButton("Toggle row height", wc);
  b->clicked.connect(SLOT(this, TreeViewExample::toggleRowHeight));
  b->setToolTip("Toggles row height between 30px and 25px");
  
  b = new WPushButton("Toggle stripes", wc);
  b->clicked.connect(SLOT(this, TreeViewExample::toggleStripes));
  b->setToolTip("Toggle alternating row colors");
  
  b = new WPushButton("Toggle root", wc);
  b->clicked.connect(SLOT(this, TreeViewExample::toggleRoot));
  b->setToolTip("Toggles root item between all and the first continent.");
  
  b = new WPushButton("Add rows", wc);
  b->clicked.connect(SLOT(this, TreeViewExample::addRows));
  b->setToolTip("Adds some cities to Belgium");
  
}

WStandardItem *TreeViewExample::continentItem(const std::string& continent)
{
  return new WStandardItem(continent);
}

WStandardItem *TreeViewExample::countryItem(const std::string& country,
			   const std::string& code)
{
  WStandardItem *result = new WStandardItem(country);
  result->setIcon("icons/flag_" + code + ".png");
  
  return result;
}

std::vector<WStandardItem *> TreeViewExample::cityItems(const std::string& city,
				       WeatherIcon weather,
				       const std::string& drink,
				       bool visited)
{
  std::vector<WStandardItem *> result;
  WStandardItem *item;
  
  // column 0: country
  item = new WStandardItem(WString::fromUTF8(city));
  result.push_back(item);
  
  // column 1: weather
  item = new WStandardItem();
  item->setIcon(std::string("icons/") + weatherIcons[weather]);
  result.push_back(item);
  
  // column 2: drink
  item = new WStandardItem(drink);
  if (useInternalPath_) {
    item->setInternalPath("/drinks/" + drink);
  }
  result.push_back(item);
  
  // column 3: visited
  item = new WStandardItem();
  item->setCheckable(true);
  item->setChecked(visited);
  result.push_back(item);
  
  return result;
}

void TreeViewExample::toggleRowHeight()
{
  if (treeView_->rowHeight() == WLength(30))
    treeView_->setRowHeight(25);
  else
    treeView_->setRowHeight(30);
}

void TreeViewExample::toggleStripes()
{
  treeView_->setAlternatingRowColors(!treeView_->alternatingRowColors());
}

void TreeViewExample::toggleRoot()
{
  if (treeView_->rootIndex() == WModelIndex())
    treeView_->setRootIndex(model_->index(0, 0));
  else
    treeView_->setRootIndex(WModelIndex());
}

void TreeViewExample::addRows()
{
  for (int i = 0; i < 5; ++i) {
    std::string cityName = "City "
      + boost::lexical_cast<std::string>(belgium_->rowCount() + 1);
    
    belgium_->appendRow(cityItems(cityName, Storm, "Juice", false));
  }
}
