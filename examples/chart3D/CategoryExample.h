// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CATEGORYEXAMPLE_H
#define CATEGORYEXAMPLE_H

#include <Wt/WContainerWidget.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WText.h>
#include <Wt/WCheckBox.h>
#include <Wt/WTabWidget.h>
#include <Wt/Chart/WCartesian3DChart.h>
#include <Wt/Chart/WGridData.h>

#include "Tabs.h"

class CategoryExample: public Wt::WContainerWidget
{
public:
  CategoryExample();
  ~CategoryExample();
  
private:
  void setUpData();

  Wt::WText *title_;
  Wt::Chart::WCartesian3DChart *chart_;

  std::shared_ptr<Wt::WAbstractItemModel> isotopeModel_;
  std::shared_ptr<Wt::WAbstractItemModel> planeModel_;
  std::shared_ptr<Wt::WAbstractItemModel> randomModel_;
  std::shared_ptr<Wt::WAbstractItemModel> yPlaneModel0_;
  std::shared_ptr<Wt::WAbstractItemModel> yPlaneModel_;
  std::shared_ptr<Wt::WAbstractItemModel> xPlaneModel_;
  std::shared_ptr<Wt::WAbstractItemModel> xPlaneModelColor_;

  std::vector<Wt::Chart::WAbstractDataSeries3D*> series_;
};

#endif
