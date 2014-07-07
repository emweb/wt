#include "NumericalExample.h"
#include "Models.h"

#include "Wt/Chart/WStandardColorMap"
#include "Wt/WColor"
#include "Wt/WPushButton"
#include "Wt/WBorder"
#include "Wt/WCssDecorationStyle"
#include "Wt/WLineEdit"
#include "Wt/WText"
#include "Wt/WMatrix4x4"

#include <stdlib.h>
#include <sstream>

NumericalExample::NumericalExample(WContainerWidget *parent)
  : WContainerWidget(parent)
{
  setContentAlignment(AlignCenter);

  chart_ = new WCartesian3DChart(this);
  // Disabling server side rendering for Wt website.
  chart_->setRenderOptions(WGLWidget::ClientSideRendering | WGLWidget::AntiAliasing);
  chart_->setLegendStyle(WFont(), WPen(), WBrush(WColor(lightGray)));

  Wt::WCssDecorationStyle style;
  style.setBorder(Wt::WBorder(Wt::WBorder::Solid, Wt::WBorder::Medium, Wt::black));
  chart_->setDecorationStyle(style);


  // first dataset
  sombrModel_ = new SombreroData(40, 40, -10, 10, -10, 10);
  WGridData *sombreroFunc_ = new WGridData(sombrModel_);
  series_.push_back(sombreroFunc_);

  // second dataset
  yPlaneModel_ = new PlaneData(21, 21, -10, 1, -10, 1, true, 100, 100);
  WEquidistantGridData *yPlaneFunc_ = new WEquidistantGridData(yPlaneModel_,
							       -10, 1, -10, 1);
  series_.push_back(yPlaneFunc_);

  // third dataset
  xPlaneModel_ = new PlaneData(21, 21, -10, 1, -10, 1, false, 100, 100);
  WEquidistantGridData *xPlaneFunc_ = new WEquidistantGridData(xPlaneModel_,
							       -10, 1, -10, 1);
  series_.push_back(xPlaneFunc_);

  // dataset
  yPlaneModelColor_ = new PlaneData(21, 21, -10, 1, -10, 1, true, 1, 100);
  WEquidistantGridData *yPlaneFuncColor_ = new WEquidistantGridData(yPlaneModelColor_,
							       -10, 1, -10, 1);
  series_.push_back(yPlaneFuncColor_);

  // dataset
  xPlaneModelSize_ = new PlaneData(21, 21, -10, 1, -10, 1, false, 100, 1);
  WEquidistantGridData *xPlaneFuncSize_ = new WEquidistantGridData(xPlaneModelSize_,
							       -10, 1, -10, 1);
  series_.push_back(xPlaneFuncSize_);

  // dataset
  spiralModel_ = new PointsData(100);
  WScatterData *spiral_ = new WScatterData(spiralModel_);
  series_.push_back(spiral_);

  // dataset
  para1Model_ = new Parabola(-20, 1, -20, 1, 0.01, 0, false, 0);
  WEquidistantGridData *parabola1 = new WEquidistantGridData(para1Model_,
							     -20, 1, -20, 1);
  series_.push_back(parabola1);

  // dataset
  para2Model_ = new Parabola(-10, 0.5, -10, 0.5, 0.1, -4, true, 0);
  WEquidistantGridData *parabola2 = new WEquidistantGridData(para2Model_,
							     -10, 0.5, -10, 0.5);
  series_.push_back(parabola2);

  chart_->resize(600,600);

  // Data configuration widget
  DataConfig *dataconfig = new DataConfig(chart_);
  dataconfig->addDataToCollection("Sombrero data", sombreroFunc_);
  dataconfig->addDataToCollection("Plane tilted along x", xPlaneFunc_);
  dataconfig->addDataToCollection("Plane tilted along y", yPlaneFunc_);
  dataconfig->addDataToCollection("Plane tilted along x (with sizeRoles)", xPlaneFuncSize_);
  dataconfig->addDataToCollection("Plane tilted along y (with colorRoles)", yPlaneFuncColor_);
  dataconfig->addDataToCollection("spiral", spiral_);
  dataconfig->addDataToCollection("parabola", parabola1);
  dataconfig->addDataToCollection("parabola (with colorRoles)", parabola2);

  configuration_ = new WTabWidget(this);
  configuration_->addTab(new ChartSettings(chart_), "General Chart Settings", Wt::WTabWidget::PreLoading);
  configuration_->addTab(dataconfig, "Data selection and configuration", Wt::WTabWidget::PreLoading);

  // WPushButton *button = new WPushButton("show camera-matrix", this);
  // WText *box = new WText("Nothing yet", this);
  // button->clicked().connect(std::bind([=]() {
  // 	const WMatrix4x4& mat = chart_->cameraMatrix();
  // 	std::stringstream matrep;
  // 	for (int i=0; i<4; i++){
  // 	  matrep << "[";
  // 	  for (int j=0; j<4; j++) {
  // 	    matrep << (Wt::asString(mat(i,j), "%.2f")).toUTF8();
  // 	    matrep << (j!=3 ? "," : "");
  // 	  }
  // 	  matrep << "]" << std::endl;
  // 	}
  // 	box->setText(matrep.str());
  //     }));
  WPushButton *button1 = new WPushButton("perspective view", this);
  WPushButton *button2 = new WPushButton("top view", this);
  WPushButton *button3 = new WPushButton("side view", this);
  WMatrix4x4 worldTransform;
  worldTransform.lookAt(
		   0.5, 0.5, 5, // camera position
		   0.5, 0.5, 0.5,      // looking at
		   0, 1, 0);        // 'up' vector
  button1->clicked().connect(std::bind([=]() {
	WMatrix4x4 cameraMat = worldTransform;
	cameraMat.translate(0.5, 0.5, 0.5);
	cameraMat.rotate(45.0, 0.0, 1.0, 0.0);
	cameraMat.rotate(20.0, 1.0, 0.0, 1.0);
	cameraMat.scale(2);
	cameraMat.translate(-0.5, -0.5, -0.5);
	chart_->setCameraMatrix(cameraMat);
      }));
  button2->clicked().connect(std::bind([=]() {
	WMatrix4x4 cameraMat = worldTransform;
	cameraMat.translate(0.5, 0.5, 0.5);
	cameraMat.rotate(90.0, 1.0, 0.0, 0.0);
	cameraMat.scale(2);
	cameraMat.translate(-0.5, -0.5, -0.5);
	chart_->setCameraMatrix(cameraMat);
      }));
  button3->clicked().connect(std::bind([=]() {
	WMatrix4x4 cameraMat = worldTransform;
	cameraMat.translate(0.5, 0.5, 0.5);
	cameraMat.rotate(90.0, 0.0, 1.0, 0.0);
	cameraMat.scale(2);
	cameraMat.translate(-0.5, -0.5, -0.5);
	chart_->setCameraMatrix(cameraMat);
      }));
}


NumericalExample::~NumericalExample()
{
  delete sombrModel_;
  delete xPlaneModel_;
  delete yPlaneModel_;
  delete xPlaneModelSize_;
  delete yPlaneModelColor_;
  delete spiralModel_;
  delete para1Model_;
  delete para2Model_;

  std::vector<WAbstractDataSeries3D*> onChart = chart_->dataSeries();
  for (unsigned i=0; i < onChart.size(); i++)
    chart_->removeDataSeries(onChart[i]);
  for (unsigned i=0; i < series_.size(); i++)
    delete series_[i];
}
