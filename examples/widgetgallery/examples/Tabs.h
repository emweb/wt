// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TABS_H
#define TABS_H

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>
#include <Wt/WLabel>
#include <Wt/WString>
#include <Wt/WLineEdit>
#include <Wt/WCheckBox>
#include <Wt/WComboBox>
#include <Wt/Chart/WCartesian3DChart>
#include <Wt/Chart/WChart3DImplementation>
#include <Wt/Chart/WAbstractGridData>
#include <Wt/Chart/WScatterData>
#include "Wt/WTemplate"

using namespace Wt;
using namespace Wt::Chart;

class ChartExample;

class ChartSettings : public WContainerWidget
{
public:
  ChartSettings(WCartesian3DChart *chart,
		WContainerWidget *parent = 0);
};

class GridDataSettings : public WContainerWidget
{
public:
  GridDataSettings(WCartesian3DChart *chart,
		   WAbstractGridData *data,
		   WContainerWidget *parent = 0);
  ~GridDataSettings();

private:
  void updateData();
  void enableMesh();
  void hideData();
  void changeChartType(WString selection);

  WAbstractGridData *data_;
  WCartesian3DChart *chart_;

  WTemplate *template_;
  WLabel *title_;
  WComboBox *typeSelection_;
  WLineEdit *xMin_, *xMax_;
  WLineEdit *yMin_, *yMax_;
  WLineEdit *nbPtsX_, *nbPtsY_;
  WCheckBox *enableColorBands_;
  WLabel *enableMeshLabel_;
  WCheckBox *enableMesh_;
  WCheckBox *hidden_;

  double xStart_, xEnd_;
  double yStart_, yEnd_;
  int nbXPts_, nbYPts_;

  friend class ChartExample;
};

class ScatterDataSettings : public WContainerWidget
{
public:
  ScatterDataSettings(WScatterData *data,
		      WContainerWidget *parent = 0);

private:
  WTemplate *template_;

  WCheckBox *enableDroplines_;
  WCheckBox *hide_;
};

class CategoryDataSettings : public WContainerWidget
{
public:
  CategoryDataSettings(WAbstractGridData *data,
		       WContainerWidget *parent = 0);

private:
  void adjustBarWidth();

  WAbstractGridData *data_;

  WTemplate *template_;
  WLineEdit *barWidthX_;
  WLineEdit *barWidthY_;
  WCheckBox *hideData_;
};

#endif
