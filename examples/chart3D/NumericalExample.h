// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NUMERICALEXAMPLE_H
#define NUMERICALEXAMPLE_H

#include <Wt/WAbstractTableModel.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/Chart/WCartesian3DChart.h>
#include <Wt/Chart/WGridData.h>
#include <Wt/Chart/WEquidistantGridData.h>
#include <Wt/WText.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTableView.h>
#include "Tabs.h"

class SombreroData;
class PlaneData;
class PointsData;
class Parabola;

class NumericalExample: public Wt::WContainerWidget
{
public:
  NumericalExample();
  ~NumericalExample();

private:
  Wt::Chart::WCartesian3DChart *chart_;
  Wt::WTabWidget *configuration_;

  std::shared_ptr<SombreroData> sombrModel_;
  std::shared_ptr<PlaneData>    xPlaneModel_;
  std::shared_ptr<PlaneData>    yPlaneModel_;
  std::shared_ptr<PlaneData>    xPlaneModelSize_;
  std::shared_ptr<PlaneData>    yPlaneModelColor_;
  std::shared_ptr<PointsData>   spiralModel_;
  std::shared_ptr<Parabola>     para1Model_;
  std::shared_ptr<Parabola>     para2Model_;

  std::vector<Wt::Chart::WAbstractDataSeries3D *> series_;
};


#endif
