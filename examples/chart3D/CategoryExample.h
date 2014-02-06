// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CATEGORYEXAMPLE_H
#define CATEGORYEXAMPLE_H

#include <Wt/WContainerWidget>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WText>
#include <Wt/WCheckBox>
#include <Wt/WTabWidget>
#include <Wt/Chart/WCartesian3DChart>
#include <Wt/Chart/WGridData>

#include "Tabs.h"

using namespace Wt;
using namespace Wt::Chart;

class CategoryExample: public WContainerWidget
{
public:
  CategoryExample(WContainerWidget *parent = 0);
  ~CategoryExample();
  
private:
  void setUpData();

  WText *title_;
  WCartesian3DChart *chart_;

  WAbstractItemModel *isotopeModel_;
  WAbstractItemModel *planeModel_;
  WAbstractItemModel *randomModel_;
  WAbstractItemModel *yPlaneModel0_;
  WAbstractItemModel *yPlaneModel_;
  WAbstractItemModel *xPlaneModel_;
  WAbstractItemModel *xPlaneModelColor_;

  std::vector<WAbstractDataSeries3D*> series_;
};

#endif
