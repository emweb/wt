/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "TreeViewExample.h"

#include <iostream>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WPanel.h>
#include <Wt/WPushButton.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WText.h>
#include <Wt/WTreeView.h>

static const char *weatherIcons[] = {
  "sun01.png",
  "cloudy01.png",
  "w_cloud.png",
  "rain.png",
  "storm.png",
  "snow.png"
};

TreeViewExample::TreeViewExample(std::shared_ptr<WStandardItemModel> model,
                                 const WString& titleText)
  : model_(model)
{
  belgium_ = model_->item(0, 0)->child(0, 0);

  this->addWidget(std::make_unique<WText>(titleText));

  /*
   * Now create the view
   */
  WPanel *panel = this->addWidget(std::make_unique<WPanel>());
  panel->resize(600, 300);

  auto treeView = std::make_unique<WTreeView>();
  treeView_ = treeView.get();
  panel->setCentralWidget(std::move(treeView));
  
  if (!WApplication::instance()->environment().ajax())
    treeView_->resize(WLength::Auto, 290);

  treeView_->setAlternatingRowColors(true);
  treeView_->setRowHeight(25);
  treeView_->setModel(model_);

  treeView_->setColumnWidth(1, WLength(100));
  treeView_->setColumnAlignment(1, AlignmentFlag::Center);
  treeView_->setColumnWidth(3, WLength(100));
  treeView_->setColumnAlignment(3, AlignmentFlag::Center);

  treeView_->setRowHeaderCount(1);
  treeView_->setColumnWidth(0, 300);

  /*
   * Expand the first (and single) top level node
   */
  treeView_->setExpanded(model->index(0, 0), true);
  treeView_->setExpanded(model->index(0, 0, model->index(0, 0)), true);

  treeView_->setSortingEnabled(false);
  treeView_->setSortingEnabled(0, true);
  treeView_->setSortingEnabled(2, true);

  /*
   * Setup some buttons to manipulate the view and the model.
   */
  WContainerWidget *wc =
      this->addWidget(std::make_unique<WContainerWidget>());
  WPushButton *b;
  
  b = wc->addWidget(std::make_unique<WPushButton>("Toggle row height"));
  b->clicked().connect(this, &TreeViewExample::toggleRowHeight);
  b->setToolTip("Toggles row height between 31px and 25px");
  
  b = wc->addWidget(std::make_unique<WPushButton>("Toggle stripes"));
  b->clicked().connect(this, &TreeViewExample::toggleStripes);
  b->setToolTip("Toggle alternating row colors");
  
  b = wc->addWidget(std::make_unique<WPushButton>("Toggle root"));
  b->clicked().connect(this, &TreeViewExample::toggleRoot);
  b->setToolTip("Toggles root item between all and the first continent.");

  b = wc->addWidget(std::make_unique<WPushButton>("Add rows"));
  b->clicked().connect(this, &TreeViewExample::addRows);
  b->setToolTip("Adds some cities to Belgium");
}

std::shared_ptr<WStandardItemModel> TreeViewExample::createModel(bool useInternalPath)
{
  /*
   * Setup a model.
   *
   * We use the standard item model, which is a general model
   * suitable for hierarchical (tree-like) data, but stores all data
   * in memory.
   */
  auto result = std::make_shared<WStandardItemModel>(0, 4);

  /*
   * Headers ...
   */
  result->setHeaderData(0, Orientation::Horizontal, std::string("Places"));
  result->setHeaderData(1, Orientation::Horizontal, std::string("Weather"));
  result->setHeaderData(2, Orientation::Horizontal, std::string("Drink"));
  result->setHeaderData(3, Orientation::Horizontal, std::string("Visited"));
  
  /*
   * ... and data
   */
  std::unique_ptr<WStandardItem> continent, country;
  WStandardItem *continentPtr, *countryPtr;

  continent = continentItem("Europe");
  continentPtr = continent.get();
  result->appendRow(std::move(continent));

  country = countryItem("Belgium", "be");
  countryPtr = country.get();
  continentPtr->appendRow(std::move(country));
  countryPtr->appendRow(cityItems("Brussels", Rain, "Beer",
                                  useInternalPath, true));
  countryPtr->appendRow(cityItems("Leuven", Rain, "Beer",
                                  useInternalPath, true));
  
  country = countryItem("France", "fr");
  countryPtr = country.get();
  continentPtr->appendRow(std::move(country));
  countryPtr->appendRow(cityItems("Paris", Cloud, "Wine",
                                  useInternalPath, true));
  countryPtr->appendRow(cityItems("Bordeaux", SunCloud, "Bordeaux wine",
                                  useInternalPath, false));
  
  country = countryItem("Spain", "sp");
  countryPtr = country.get();
  continentPtr->appendRow(std::move(country));
  countryPtr->appendRow(cityItems("Barcelona", Sun, "Cava",
                                  useInternalPath, true));
  countryPtr->appendRow(cityItems("Madrid", Sun, "San Miguel",
                                  useInternalPath, false));
  
  continent = continentItem("Africa");
  continentPtr = continent.get();
  result->appendRow(std::move(continent));
  
  country = countryItem("Morocco (المغرب)", "ma");
  countryPtr = country.get();
  continentPtr->appendRow(std::move(country));
  countryPtr->appendRow(cityItems("Casablanca", Sun, "Tea",
                                  useInternalPath, false));

  return result;
}

std::unique_ptr<WStandardItem> TreeViewExample::continentItem(const std::string& continent)
{
  std::unique_ptr<WStandardItem> result
      = std::make_unique<WStandardItem>(continent);
  result->setColumnCount(4);
  return result;
}

std::unique_ptr<WStandardItem> TreeViewExample::countryItem(const std::string& country,
					    const std::string& code)
{
  std::unique_ptr<WStandardItem> result
      = std::make_unique<WStandardItem>(WString(country));
  result->setIcon("icons/flag_" + code + ".png");
  return result;
}

std::vector<std::unique_ptr<WStandardItem>>
TreeViewExample::cityItems(const std::string& city,
			   WeatherIcon weather,
			   const std::string& drink,
			   bool useInternalPath, bool visited)
{
  std::vector<std::unique_ptr<WStandardItem>> result;
  std::unique_ptr<WStandardItem> item;
  
  // column 0: country
  item = std::make_unique<WStandardItem>(WString(city));
  result.push_back(std::move(item));
  
  // column 1: weather
  item = std::make_unique<WStandardItem>();
  item->setIcon(std::string("icons/") + weatherIcons[weather]);
  result.push_back(std::move(item));
  
  // column 2: drink
  item = std::make_unique<WStandardItem>(drink);
  if (useInternalPath)
    item->setLink(WLink(LinkType::InternalPath, "/drinks/" + drink));
  result.push_back(std::move(item));
  
  // column 3: visited
  item = std::make_unique<WStandardItem>();
  item->setCheckable(true);
  item->setChecked(visited);
  result.push_back(std::move(item));
  
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
      + asString(belgium_->rowCount() + 1).toUTF8();
    
    bool useInternalPath = false;
    belgium_->appendRow(cityItems(cityName, Storm, "Juice",
                                  useInternalPath, false));
  }

  treeView_->scrollTo(belgium_->child(belgium_->rowCount() -COUNT )->index(),
                      ScrollHint::PositionAtTop);
}
