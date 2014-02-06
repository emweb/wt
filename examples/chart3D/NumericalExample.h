// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NUMERICALEXAMPLE_H
#define NUMERICALEXAMPLE_H

#include <Wt/WAbstractTableModel>
#include <Wt/WContainerWidget>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/Chart/WCartesian3DChart>
#include <Wt/Chart/WGridData>
#include <Wt/Chart/WEquidistantGridData>
#include <Wt/WText>
#include <Wt/WTabWidget>
#include <Wt/WTableView>
#include "Tabs.h"

using namespace Wt;
using namespace Wt::Chart;

class SombreroData;
class PlaneData;
class PointsData;
class Parabola;

class NumericalExample: public WContainerWidget
{
public:
  NumericalExample(WContainerWidget *parent);
  ~NumericalExample();

private:
  WCartesian3DChart *chart_;
  WTabWidget *configuration_;

  SombreroData *sombrModel_;
  PlaneData *xPlaneModel_;
  PlaneData *yPlaneModel_;
  PlaneData *xPlaneModelSize_;
  PlaneData *yPlaneModelColor_;
  PointsData *spiralModel_;
  Parabola *para1Model_;
  Parabola *para2Model_;

  std::vector<WAbstractDataSeries3D*> series_;
};


#endif
