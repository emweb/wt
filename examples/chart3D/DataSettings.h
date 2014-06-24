// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DATASETTINGS
#define DATASETTINGS

#include <Wt/WContainerWidget>

namespace Wt {
class WLineEdit;
class WComboBox;
class WCheckBox;
class WSlider;
class WTemplate;
class WText;
  namespace Chart {
class WCartesian3DChart;
class WAbstractDataSeries3D;
class WAbstractGridData;
class WGridData;
class WScatterData;
  }
}

using namespace Wt;
using namespace Wt::Chart;

class DataSettings : public WContainerWidget {
public:
  DataSettings();

  void bindBaseToTemplate(WTemplate *configtemplate);

  void bindBaseDataSet(WAbstractDataSeries3D *data);

private:
  WLineEdit *setName_;
  WLineEdit *pointsize_;
  WComboBox *pointSprite_;
  WComboBox *colormap_;
  WCheckBox *showColormap_;
  WComboBox *colormapSide_;

  // WText *description_;
  WCheckBox *hide_;

  WAbstractDataSeries3D *data_;
};

class NumGridDataSettings : public DataSettings {
public:
  NumGridDataSettings();

  void bindDataSet(WAbstractGridData *data);

private:
  WComboBox *typeSelection_;
  WCheckBox *enableMesh_;
  WLineEdit *penSize_;
  WComboBox *penColor_;

  WSlider *xClippingMin_;
  WSlider *xClippingMax_;
  WSlider *yClippingMin_;
  WSlider *yClippingMax_;
  WSlider *zClippingMin_;
  WSlider *zClippingMax_;

  WCheckBox *showClippingLines_;
  WComboBox *clippingLinesColor_;

  WCheckBox *showIsolines_;
  WComboBox *isolineColormap_;

  JSlot changeXClippingMin_;
  JSlot changeXClippingMax_;
  JSlot changeYClippingMin_;
  JSlot changeYClippingMax_;
  JSlot changeZClippingMin_;
  JSlot changeZClippingMax_;

  std::vector<Signals::connection> clippingConnections_;

  WAbstractGridData *gridData_;
};

class CatGridDataSettings : public DataSettings {
public:
  CatGridDataSettings();

  void bindDataSet(WAbstractGridData *data);

private:
  WLineEdit *barWidthX_;
  WLineEdit *barWidthY_;

  WAbstractGridData *gridData_;
};

class ScatterDataSettings : public DataSettings {
public:
  ScatterDataSettings();

  void bindDataSet(WScatterData *data);

private:
  WCheckBox *enableDroplines_;
  WLineEdit *penSize_;
  WComboBox *penColor_;

  WScatterData *scatterData_;
};


#endif
