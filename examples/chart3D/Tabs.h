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
#include <Wt/Chart/WAbstractDataSeries3D>
#include <Wt/Chart/WGridData>
#include <Wt/Chart/WScatterData>
#include <Wt/WTemplate>
#include <Wt/WStackedWidget>

#include "DataSettings.h"

using namespace Wt;
using namespace Wt::Chart;

class ChartSettings : public WContainerWidget
{
public:
  ChartSettings(WCartesian3DChart *chart,
		WContainerWidget *parent = 0);
};


class DataSelection : public WContainerWidget
{
public:
  DataSelection(WCartesian3DChart *chart);
  
  void addDataToCollection(WString name, WAbstractDataSeries3D* data);
  Signal<WAbstractDataSeries3D*>& selectionChanged() { return selectionChange_; }

private:
  typedef std::pair<WString, WAbstractDataSeries3D*> DataSelectionItem;
  std::vector<DataSelectionItem> dataCollection_;
  Signal<WAbstractDataSeries3D*> selectionChange_;

  WSelectionBox *notShown;
  WSelectionBox *shown;
};

// This class provides all kinds of data-configuration and always shows only one
class DataConfig : public WContainerWidget {
public:
  DataConfig(WCartesian3DChart* chart);

  void addDataToCollection(WString name, WAbstractDataSeries3D *data);

private:
  DataSelection* dataselection_;

  NumGridDataSettings *numgriddatasettings_;
  CatGridDataSettings *catgriddatasettings_;
  ScatterDataSettings *scatterdatasettings_;
};

#endif
