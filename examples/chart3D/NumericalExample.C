#include "NumericalExample.h"
#include "Models.h"

#include "Wt/Chart/WStandardColorMap.h"
#include "Wt/WColor.h"
#include "Wt/WPushButton.h"
#include "Wt/WBorder.h"
#include "Wt/WCssDecorationStyle.h"
#include "Wt/WLineEdit.h"
#include "Wt/WText.h"
#include "Wt/WMatrix4x4.h"

#include <stdlib.h>
#include <sstream>

NumericalExample::NumericalExample()
  : WContainerWidget()
{
  setContentAlignment(Wt::AlignmentFlag::Center);

  chart_ = this->addWidget(std::make_unique<Wt::Chart::WCartesian3DChart>());
  // Disabling server side rendering for Wt website.
  chart_->setRenderOptions(Wt::GLRenderOption::ClientSide | Wt::GLRenderOption::AntiAliasing);
  chart_->setLegendStyle(Wt::WFont(), Wt::WPen(), Wt::WBrush(Wt::WColor("lightGray")));

//  WCssDecorationStyle style;
//  style.setBorder(WBorder(BorderStyle::Solid, BorderWidth::Medium, WColor("black")));
//  chart_->setLegendStyle(style);


  // first dataset
    sombrModel_ = std::make_shared<SombreroData>(40, 40, -10, 10, -10, 10);
    auto sombreroFunc_ = new Wt::Chart::WGridData(sombrModel_);
    series_.push_back(sombreroFunc_);

    // second dataset
    yPlaneModel_ = std::make_shared<PlaneData>(21, 21, -10, 1, -10, 1, true, 100, 100);
    auto yPlaneFunc_ = new Wt::Chart::WEquidistantGridData(yPlaneModel_, -10, 1, -10, 1);
    series_.push_back(yPlaneFunc_);

    // third dataset
    xPlaneModel_ = std::make_shared<PlaneData>(21, 21, -10, 1, -10, 1, false, 100, 100);
    auto xPlaneFunc_ = new Wt::Chart::WEquidistantGridData(xPlaneModel_, -10, 1, -10, 1);
    series_.push_back(xPlaneFunc_);

    // dataset
    yPlaneModelColor_ = std::make_shared<PlaneData>(21, 21, -10, 1, -10, 1, true, 1, 100);
    auto yPlaneFuncColor_ = new Wt::Chart::WEquidistantGridData(yPlaneModelColor_, -10, 1, -10, 1);
    series_.push_back(yPlaneFuncColor_);

    // dataset
    xPlaneModelSize_ = std::make_shared<PlaneData>(21, 21, -10, 1, -10, 1, false, 100, 1);
    auto xPlaneFuncSize_ = new Wt::Chart::WEquidistantGridData(xPlaneModelSize_, -10, 1, -10, 1);
    series_.push_back(xPlaneFuncSize_);

    // dataset
    spiralModel_ = std::make_shared<PointsData>(100);
    auto spiral_ = new Wt::Chart::WScatterData(spiralModel_);
    series_.push_back(spiral_);

    // dataset
    para1Model_ = std::make_shared<Parabola>(-20, 1, -20, 1, 0.01, 0, false, 0);
    auto parabola1_ = new Wt::Chart::WEquidistantGridData(para1Model_, -20, 1, -20, 1);
    series_.push_back(parabola1_);

    // dataset
    para2Model_ = std::make_shared<Parabola>(-10, 0.5, -10, 0.5, 0.1, -4, true, 0);
    auto parabola2_ = new Wt::Chart::WEquidistantGridData(para2Model_, -10, 0.5, -10, 0.5);
    series_.push_back(parabola2_);
  chart_->resize(600,600);

  // Data configuration widget
  auto dataconfig = std::make_unique<DataConfig>(chart_);
  dataconfig->addDataToCollection("Sombrero data", sombreroFunc_);
  dataconfig->addDataToCollection("Plane tilted along x", xPlaneFunc_);
  dataconfig->addDataToCollection("Plane tilted along y", yPlaneFunc_);
  dataconfig->addDataToCollection("Plane tilted along x (with sizeRoles)", xPlaneFuncSize_);
  dataconfig->addDataToCollection("Plane tilted along y (with colorRoles)", yPlaneFuncColor_);
  dataconfig->addDataToCollection("spiral", spiral_);
  dataconfig->addDataToCollection("parabola", parabola1_);
  dataconfig->addDataToCollection("parabola (with colorRoles)", parabola2_);

  configuration_ = this->addWidget(std::make_unique<Wt::WTabWidget>());
  configuration_->addTab(std::make_unique<ChartSettings>(chart_), "General Chart Settings", Wt::ContentLoading::Eager);
  configuration_->addTab(std::move(dataconfig), "Data selection and configuration", Wt::ContentLoading::Eager);

//   Wt::WPushButton *button = this->addWidget(std::make_unique<Wt::WPushButton>("show camera-matrix"));
//   Wt::WText *box = this->addWidget(std::make_unique<Wt::WText>("Nothing yet"));
//   button->clicked().connect([=]() {
//   	const Wt::WMatrix4x4& mat = chart_->cameraMatrix();
//   	std::stringstream matrep;
//   	for (int i=0; i<4; i++){
//   	  matrep << "[";
//   	  for (int j=0; j<4; j++) {
//   	    matrep << (Wt::asString(mat(i,j), "%.2f")).toUTF8();
//   	    matrep << (j!=3 ? "," : "");
//   	  }
//   	  matrep << "]" << std::endl;
//   	}
//   	box->setText(matrep.str());
//       });
  Wt::WPushButton *button1 = this->addWidget(std::make_unique<Wt::WPushButton>("perspective view"));
  Wt::WPushButton *button2 = this->addWidget(std::make_unique<Wt::WPushButton>("top view"));
  Wt::WPushButton *button3 = this->addWidget(std::make_unique<Wt::WPushButton>("side view"));
  Wt::WMatrix4x4 worldTransform;
  worldTransform.lookAt(
		   0.5, 0.5, 5, // camera position
		   0.5, 0.5, 0.5,      // looking at
		   0, 1, 0);        // 'up' vector
  button1->clicked().connect([=]() {
        Wt::WMatrix4x4 cameraMat = worldTransform;
	cameraMat.translate(0.5, 0.5, 0.5);
	cameraMat.rotate(45.0, 0.0, 1.0, 0.0);
	cameraMat.rotate(20.0, 1.0, 0.0, 1.0);
	cameraMat.scale(2);
	cameraMat.translate(-0.5, -0.5, -0.5);
	chart_->setCameraMatrix(cameraMat);
      });
  button2->clicked().connect([=]() {
        Wt::WMatrix4x4 cameraMat = worldTransform;
	cameraMat.translate(0.5, 0.5, 0.5);
	cameraMat.rotate(90.0, 1.0, 0.0, 0.0);
	cameraMat.scale(2);
	cameraMat.translate(-0.5, -0.5, -0.5);
	chart_->setCameraMatrix(cameraMat);
      });
  button3->clicked().connect([=]() {
        Wt::WMatrix4x4 cameraMat = worldTransform;
	cameraMat.translate(0.5, 0.5, 0.5);
	cameraMat.rotate(90.0, 0.0, 1.0, 0.0);
	cameraMat.scale(2);
	cameraMat.translate(-0.5, -0.5, -0.5);
	chart_->setCameraMatrix(cameraMat);
      });
}


NumericalExample::~NumericalExample()
{
  std::vector<Wt::Chart::WAbstractDataSeries3D*> onChart = chart_->dataSeries();
  for (auto series : onChart)
    chart_->removeDataSeries(series).release();
  for (auto series : series_)
    delete series;
}
