// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TABS_H
#define TABS_H

#include <Wt/WContainerWidget.h>
#include <Wt/WTextArea.h>
#include <Wt/WLabel.h>
#include <Wt/WString.h>
#include <Wt/WLineEdit.h>
#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/Chart/WCartesian3DChart.h>
#include <Wt/Chart/WChart3DImplementation.h>
#include <Wt/Chart/WAbstractDataSeries3D.h>
#include <Wt/Chart/WGridData.h>
#include <Wt/Chart/WScatterData.h>
#include <Wt/WTemplate.h>
#include <Wt/WStackedWidget.h>

#include "DataSettings.h"

class ChartSettings : public Wt::WContainerWidget
{
public:
  ChartSettings(Wt::Chart::WCartesian3DChart *chart);
};


class DataSelection : public Wt::WContainerWidget
{
public:
  DataSelection(Wt::Chart::WCartesian3DChart *chart);
  
  void addDataToCollection(Wt::WString name, Wt::Chart::WAbstractDataSeries3D *data);
  Wt::Signal<Wt::Chart::WAbstractDataSeries3D*>& selectionChanged() { return selectionChange_; }

private:
  typedef std::pair<Wt::WString, Wt::Chart::WAbstractDataSeries3D *> DataSelectionItem;
  std::vector<DataSelectionItem> dataCollection_;
  Wt::Signal<Wt::Chart::WAbstractDataSeries3D*> selectionChange_;

  Wt::WSelectionBox *notShown;
  Wt::WSelectionBox *shown;
};

// This class provides all kinds of data-configuration and always shows only one
class DataConfig : public Wt::WContainerWidget {
public:
  DataConfig(Wt::Chart::WCartesian3DChart* chart);

  void addDataToCollection(Wt::WString name, Wt::Chart::WAbstractDataSeries3D *data);

private:
  DataSelection* dataselection_;

  NumGridDataSettings *numgriddatasettings_;
  CatGridDataSettings *catgriddatasettings_;
  ScatterDataSettings *scatterdatasettings_;
};

#endif
