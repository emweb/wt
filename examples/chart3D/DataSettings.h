// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DATASETTINGS
#define DATASETTINGS

#include <Wt/WContainerWidget.h>

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

class DataSettings : public Wt::WContainerWidget {
public:
  DataSettings();

  void bindBaseToTemplate(Wt::WTemplate *configtemplate);

  void bindBaseDataSet(Wt::Chart::WAbstractDataSeries3D *data);

private:
  Wt::WLineEdit *setName_;
  Wt::WLineEdit *pointsize_;
  Wt::WComboBox *pointSprite_;
  Wt::WComboBox *colormap_;
  Wt::WCheckBox *showColormap_;
  Wt::WComboBox *colormapSide_;

  // WText *description_;
  Wt::WCheckBox *hide_;

  Wt::Chart::WAbstractDataSeries3D *data_;
};

class NumGridDataSettings : public DataSettings {
public:
  NumGridDataSettings();

  void bindDataSet(Wt::Chart::WAbstractGridData *data);

private:
  Wt::WComboBox *typeSelection_;
  Wt::WCheckBox *enableMesh_;
  Wt::WLineEdit *penSize_;
  Wt::WComboBox *penColor_;

  Wt::WSlider *xClippingMin_;
  Wt::WSlider *xClippingMax_;
  Wt::WSlider *yClippingMin_;
  Wt::WSlider *yClippingMax_;
  Wt::WSlider *zClippingMin_;
  Wt::WSlider *zClippingMax_;

  Wt::WCheckBox *showClippingLines_;
  Wt::WComboBox *clippingLinesColor_;

  Wt::WCheckBox *showIsolines_;
  Wt::WComboBox *isolineColormap_;

  Wt::JSlot changeXClippingMin_;
  Wt::JSlot changeXClippingMax_;
  Wt::JSlot changeYClippingMin_;
  Wt::JSlot changeYClippingMax_;
  Wt::JSlot changeZClippingMin_;
  Wt::JSlot changeZClippingMax_;

  std::vector<Wt::Signals::connection> clippingConnections_;

  Wt::Chart::WAbstractGridData *gridData_;
};

class CatGridDataSettings : public DataSettings {
public:
  CatGridDataSettings();

  void bindDataSet(Wt::Chart::WAbstractGridData *data);

private:
  Wt::WLineEdit *barWidthX_;
  Wt::WLineEdit *barWidthY_;

  Wt::Chart::WAbstractGridData *gridData_;
};

class ScatterDataSettings : public DataSettings {
public:
  ScatterDataSettings();

  void bindDataSet(Wt::Chart::WScatterData *data);

private:
  Wt::WCheckBox *enableDroplines_;
  Wt::WLineEdit *penSize_;
  Wt::WComboBox *penColor_;

  Wt::Chart::WScatterData *scatterData_;
};


#endif
