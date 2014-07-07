#include "CategoryExample.h"
#include "Models.h"

#include "CsvUtil.h"
#include "Wt/WApplication"
#include "Wt/Chart/WStandardColorMap"
#include "Wt/Chart/WGridData"
#include "Wt/Chart/WEquidistantGridData"
#include "Wt/WFont"
#include "Wt/WString"

#include <fstream>
#include <string>
#include <iostream>

namespace {
  Wt::WStandardItemModel* readCsvFile(const std::string &fname)
  {
    WStandardItemModel *model = new WStandardItemModel(0, 0);
    std::ifstream f(fname.c_str());

    if (!f) {
      throw std::string("No .csv resource");
    }
    readFromCsv(f, model, -1, false);
    return model;
  }
}

CategoryExample::CategoryExample(WContainerWidget *parent)
  : WContainerWidget(parent)
{
  setContentAlignment(AlignCenter);
  chart_ = new WCartesian3DChart(this);
  // Disabling server side rendering for Wt website.
  chart_->setRenderOptions(WGLWidget::ClientSideRendering | WGLWidget::AntiAliasing);
  chart_->setType(CategoryChart);

  Wt::WCssDecorationStyle style;
  style.setBorder(Wt::WBorder(Wt::WBorder::Solid, Wt::WBorder::Medium, Wt::black));
  chart_->setDecorationStyle(style);

  chart_->resize(600, 600);
  
  // **** Dataset
  isotopeModel_ = readCsvFile(WApplication::appRoot() + "isotope_decay.csv");
  // add some color-roles
  for (int i=1; i<isotopeModel_->rowCount(); i++) {
    for (int j=1; j<isotopeModel_->columnCount(); j++) {
      double value = Wt::asNumber(isotopeModel_->data(i, j));
      if (value < 3)
  	isotopeModel_->setData(i, j, WColor(blue), MarkerBrushColorRole);
    }
  }
  WGridData *isotopes = new WGridData(isotopeModel_);
  series_.push_back(isotopes);
  isotopes->setType(BarSeries3D);
  
  // **** Dataset
  planeModel_ = readCsvFile(WApplication::appRoot() + "hor_plane.csv");
  WGridData *horPlane = new WGridData(planeModel_);
  series_.push_back(horPlane);
  horPlane->setType(BarSeries3D);

  // **** Dataset
  randomModel_ = readCsvFile(WApplication::appRoot() + "hor_plane.csv");
  for (int i=1; i<randomModel_->rowCount(); i++) {
    for (int j=1; j<randomModel_->columnCount(); j++) {
      randomModel_->setData(i, j,Wt::asNumber(randomModel_->data(i, j)) + (rand() % 100)/100.0);
    }
  }
  WGridData *randomPlane = new WGridData(randomModel_);
  series_.push_back(randomPlane);
  randomPlane->setType(BarSeries3D);

  // **** Dataset
  yPlaneModel_ = new PlaneData(20, 20, 0, 1, 0, 1, true, 100, 100);
  WEquidistantGridData *yPlaneFunc = new WEquidistantGridData(yPlaneModel_,
							      0, 1, 0, 1);
  series_.push_back(yPlaneFunc);
  yPlaneFunc->setType(BarSeries3D);

  // **** Dataset
  xPlaneModel_ = new PlaneData(20, 20, 0, 1, 0, 1, false, 100, 100);
  WEquidistantGridData *xPlaneFunc = new WEquidistantGridData(xPlaneModel_,
							      0, 1, 0, 1);
  series_.push_back(xPlaneFunc);
  xPlaneFunc->setType(BarSeries3D);

  // **** Dataset
  xPlaneModelColor_ = new PlaneData(20, 20, 0, 1, 0, 1, false, 5, 100);
  WEquidistantGridData *xPlaneFuncColor = new WEquidistantGridData(xPlaneModelColor_, 0, 1, 0, 1);
  series_.push_back(xPlaneFuncColor);
  xPlaneFuncColor->setType(BarSeries3D);
  

  
  // Data configuration widget
  DataConfig *dataconfig = new DataConfig(chart_);
  dataconfig->addDataToCollection("Isotope data (10x10) with ColorRoles", isotopes);
  dataconfig->addDataToCollection("Horizontal Plane (10x10)", horPlane);  
  dataconfig->addDataToCollection("Random Data (10x10)", randomPlane);  
  // dataconfig->addDataToCollection("tilted plane y (20x20)", yPlaneFunc);
  // dataconfig->addDataToCollection("tilted plane x (20x20)", xPlaneFunc);
  // dataconfig->addDataToCollection("tilted plane x (20x20) with ColorRoles", xPlaneFuncColor);
  
  

  WTabWidget *configuration_ = new WTabWidget(this);
  configuration_->addTab(new ChartSettings(chart_), "General Chart Settings", Wt::WTabWidget::PreLoading);
  configuration_->addTab(dataconfig, "Data selection and configuration", Wt::WTabWidget::PreLoading);
}

CategoryExample::~CategoryExample()
{
  delete xPlaneModelColor_;
  delete xPlaneModel_;
  delete yPlaneModel_;
  delete randomModel_;
  delete planeModel_;
  delete isotopeModel_;

  std::vector<WAbstractDataSeries3D*> onChart = chart_->dataSeries();
  for (unsigned i=0; i < onChart.size(); i++)
    chart_->removeDataSeries(onChart[i]);
  for (unsigned i=0; i < series_.size(); i++)
    delete series_[i];
}
