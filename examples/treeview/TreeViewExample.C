/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "TreeViewExample.h"

#include <iostream>
#include <boost/lexical_cast.hpp>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WPanel>
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

TreeViewExample::TreeViewExample(WStandardItemModel *model,
				 const WString& titleText)
  : model_(model)
{
  belgium_ = model_->item(0, 0)->child(0, 0);

  new WText(titleText, this);

  /*
   * Now create the view
   */
  WPanel *panel = new WPanel(this);
  panel->resize(600, 300);
  panel->setCentralWidget(treeView_ = new WTreeView());
  
  if (!WApplication::instance()->environment().ajax())
    treeView_->resize(WLength::Auto, 290);

  treeView_->setAlternatingRowColors(true);
  treeView_->setRowHeight(25);
  treeView_->setModel(model_);

  treeView_->setColumnWidth(1, WLength(100));
  treeView_->setColumnAlignment(1, AlignCenter);
  treeView_->setColumnWidth(3, WLength(100));
  treeView_->setColumnAlignment(3, AlignCenter);

  treeView_->setRowHeaderCount(1);
  treeView_->setColumnWidth(0, 300);

  /*
   * Expand the first (and single) top level node
   */
  treeView_->setExpanded(model->index(0, 0), true);
  treeView_->setExpanded(model->index(0, 0, model->index(0, 0)), true);

  /*
   * Setup some buttons to manipulate the view and the model.
   */
  WContainerWidget *wc = new WContainerWidget(this);
  WPushButton *b;
  
  b = new WPushButton("Toggle row height", wc);
  b->clicked().connect(this, &TreeViewExample::toggleRowHeight);
  b->setToolTip("Toggles row height between 31px and 25px");
  
  b = new WPushButton("Toggle stripes", wc);
  b->clicked().connect(this, &TreeViewExample::toggleStripes);
  b->setToolTip("Toggle alternating row colors");
  
  b = new WPushButton("Toggle root", wc);
  b->clicked().connect(this, &TreeViewExample::toggleRoot);
  b->setToolTip("Toggles root item between all and the first continent.");

  b = new WPushButton("Add rows", wc);
  b->clicked().connect(this, &TreeViewExample::addRows);
  b->setToolTip("Adds some cities to Belgium");
}

WStandardItemModel *TreeViewExample::createModel(bool useInternalPath,
						 WObject *parent)
{
  /*
   * Setup a model.
   *
   * We use the standard item model, which is a general model
   * suitable for hierarchical (tree-like) data, but stores all data
   * in memory.
   */
  WStandardItemModel *result = new WStandardItemModel(0, 4, parent);

  /*
   * Headers ...
   */
  result->setHeaderData(0, Horizontal, std::string("Places"));
  result->setHeaderData(1, Horizontal, std::string("Weather"));
  result->setHeaderData(2, Horizontal, std::string("Drink"));
  result->setHeaderData(3, Horizontal, std::string("Visited"));
  
  /*
   * ... and data
   */
  WStandardItem *continent, *country;
  
  result->appendRow(continent = continentItem("Europe"));
  
  continent->appendRow(country = countryItem("Belgium", "be"));
  country->appendRow(cityItems("Brussels", Rain, "Beer", useInternalPath,
			       true));
  country->appendRow(cityItems("Leuven", Rain, "Beer", useInternalPath, true));
  
  continent->appendRow(country = countryItem("France", "fr"));
  country->appendRow(cityItems("Paris", Cloud, "Wine", useInternalPath, true));
  country->appendRow(cityItems("Bordeaux", SunCloud, "Bordeaux wine",
			       useInternalPath, false));
  
  continent->appendRow(country = countryItem("Spain", "sp"));
  country->appendRow(cityItems("Barcelona", Sun, "Cava", useInternalPath,
			       true));
  country->appendRow(cityItems("Madrid", Sun, "San Miguel", useInternalPath,
			       false));
  
  result->appendRow(continent = continentItem("Africa"));
  
  continent->appendRow(country = countryItem("Morocco (المغرب)", "ma"));
  country->appendRow(cityItems("Casablanca", Sun, "Tea", useInternalPath,
			       false));

  return result;
}

WStandardItem *TreeViewExample::continentItem(const std::string& continent)
{
  WStandardItem *result = new WStandardItem(continent);
  result->setColumnCount(4);
  return result;
}

WStandardItem *TreeViewExample::countryItem(const std::string& country,
					    const std::string& code)
{
  WStandardItem *result = new WStandardItem(WString::fromUTF8(country));
  result->setIcon("icons/flag_" + code + ".png");
  
  return result;
}

std::vector<WStandardItem *>
TreeViewExample::cityItems(const std::string& city,
			   WeatherIcon weather,
			   const std::string& drink,
			   bool useInternalPath, bool visited)
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
  if (useInternalPath)
    item->setLink(WLink(WLink::InternalPath, "/drinks/" + drink));
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
  if (treeView_->rowHeight() == WLength(31))
    treeView_->setRowHeight(25);
  else
    treeView_->setRowHeight(31);
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
  static int COUNT = 10;

  for (int i = 0; i < COUNT; ++i) {
    std::string cityName = "City "
      + boost::lexical_cast<std::string>(belgium_->rowCount() + 1);
    
    bool useInternalPath = false;
    belgium_->appendRow(cityItems(cityName, Storm, "Juice", useInternalPath,
				  false));
  }

  treeView_->scrollTo(belgium_->child(belgium_->rowCount() -COUNT )->index(),
		      WAbstractItemView::PositionAtTop);
}
