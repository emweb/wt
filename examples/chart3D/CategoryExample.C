#include "CategoryExample.h"
#include "Models.h"

#include "CsvUtil.h"
#include "Wt/WApplication.h"
#include "Wt/Chart/WStandardColorMap.h"
#include "Wt/Chart/WGridData.h"
#include "Wt/Chart/WEquidistantGridData.h"
#include "Wt/WFont.h"
#include "Wt/WString.h"

#include <fstream>
#include <string>
#include <iostream>

namespace {
  std::unique_ptr<Wt::WStandardItemModel> readCsvFile(const std::string &fname)
  {
    std::unique_ptr<Wt::WStandardItemModel> model(std::make_unique<Wt::WStandardItemModel>(0, 0));
    std::ifstream f(fname.c_str());

    if (!f) {
      throw std::string("No .csv resource");
    }
    readFromCsv(f, model.get(), -1, false);
    return model;
  }
}

CategoryExample::CategoryExample()
  : WContainerWidget()
{
  setContentAlignment(Wt::AlignmentFlag::Center);
  chart_ = this->addWidget(std::make_unique<Wt::Chart::WCartesian3DChart>());
  // Disabling server side rendering for Wt website.
  chart_->setRenderOptions(Wt::GLRenderOption::ClientSide | Wt::GLRenderOption::AntiAliasing);
  chart_->setType(Wt::Chart::ChartType::Category);

//  WCssDecorationStyle style;
//  style.setBorder(WBorder(BorderStyle::Solid, BorderWidth::Medium, WColor("black")));
//  chart_->setLegendStyle(style);

  chart_->resize(600, 600);
  
  // **** Dataset
  isotopeModel_ = readCsvFile(Wt::WApplication::appRoot() + "isotope_decay.csv");
  // add some color-roles
  for (int i=1; i<isotopeModel_->rowCount(); i++) {
    for (int j=1; j<isotopeModel_->columnCount(); j++) {
      double value = Wt::asNumber(isotopeModel_->data(i, j));
      if (value < 3)
        isotopeModel_->setData(i, j, Wt::WColor(Wt::StandardColor::Blue), Wt::ItemDataRole::MarkerBrushColor);
    }
  }
  auto isotopes_ = new Wt::Chart::WGridData(isotopeModel_);
  series_.push_back(isotopes_);
  isotopes_->setType(Wt::Chart::Series3DType::Bar);
  
  // **** Dataset
  planeModel_ = readCsvFile(Wt::WApplication::appRoot() + "hor_plane.csv");
  auto horPlane_ = new Wt::Chart::WGridData(planeModel_);
  series_.push_back(horPlane_);
  horPlane_->setType(Wt::Chart::Series3DType::Bar);

  // **** Dataset
  randomModel_ = readCsvFile(Wt::WApplication::appRoot() + "hor_plane.csv");
  for (int i=1; i<randomModel_->rowCount(); i++) {
    for (int j=1; j<randomModel_->columnCount(); j++) {
      randomModel_->setData(i, j, Wt::asNumber(randomModel_->data(i, j)) + (rand() % 100)/100.0);
    }
  }
  auto randomPlane_ = new Wt::Chart::WGridData(randomModel_);
  series_.push_back(randomPlane_);
  randomPlane_->setType(Wt::Chart::Series3DType::Bar);

  // **** Dataset
  yPlaneModel_ = std::make_shared<PlaneData>(20, 20, 0, 1, 0, 1, true, 100, 100);
  auto yPlaneFunc_ = new Wt::Chart::WEquidistantGridData(yPlaneModel_,0,1,0,1);
  series_.push_back(yPlaneFunc_);
  yPlaneFunc_->setType(Wt::Chart::Series3DType::Bar);

  // **** Dataset
  xPlaneModel_ = std::make_shared<PlaneData>(20, 20, 0, 1, 0, 1, false, 100, 100);
  auto xPlaneFunc_ = new Wt::Chart::WEquidistantGridData(xPlaneModel_,0,1,0,1);
  series_.push_back(xPlaneFunc_);
  xPlaneFunc_->setType(Wt::Chart::Series3DType::Bar);

  // **** Dataset
  xPlaneModelColor_ = std::make_shared<PlaneData>(20, 20, 0, 1, 0, 1, false, 5, 100);
  auto xPlaneFuncColor_ = new Wt::Chart::WEquidistantGridData(xPlaneModelColor_,0,1,0,1);
  series_.push_back(xPlaneFuncColor_);
  xPlaneFuncColor_->setType(Wt::Chart::Series3DType::Bar);
  
  // Data configuration widget
  std::unique_ptr<DataConfig> dataconfig(std::make_unique<DataConfig>(chart_));
  dataconfig->addDataToCollection("Isotope data (10x10) with ColorRoles", isotopes_);
  dataconfig->addDataToCollection("Horizontal Plane (10x10)", horPlane_);
  dataconfig->addDataToCollection("Random Data (10x10)", randomPlane_);
  // dataconfig->addDataToCollection("tilted plane y (20x20)", yPlaneFunc_);
  // dataconfig->addDataToCollection("tilted plane x (20x20)", xPlaneFunc_);
  // dataconfig->addDataToCollection("tilted plane x (20x20) with ColorRoles", xPlaneFuncColor_);
  
  Wt::WTabWidget *configuration_ = this->addWidget(std::make_unique<Wt::WTabWidget>());
  configuration_->addTab(std::make_unique<ChartSettings>(chart_), "General Chart Settings", Wt::ContentLoading::Eager);
  configuration_->addTab(std::move(dataconfig), "Data selection and configuration", Wt::ContentLoading::Eager);
}

CategoryExample::~CategoryExample()
{
  std::vector<Wt::Chart::WAbstractDataSeries3D*> onChart = chart_->dataSeries();
  for (unsigned i=0; i < onChart.size(); i++)
    chart_->removeDataSeries(onChart[i]).release();
  for (auto series : series_)
    delete series;
}
