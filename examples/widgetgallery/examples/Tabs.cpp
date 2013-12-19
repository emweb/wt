#include "Tabs.h"
#include "DataModels.h"
#include "Wt/WDoubleValidator"
#include "Wt/WIntValidator"
#include "Wt/Chart/WCartesian3DChart"
#include "Wt/Chart/WEquidistantGridData"
#include "Wt/Chart/WGridData"
#include "Wt/Chart/WScatterData"

#include <limits>
#include <functional>

ChartSettings::ChartSettings(WCartesian3DChart *chart,
			     WContainerWidget * parent)
  : WContainerWidget(parent)
{
  WTemplate *template_ = new WTemplate(Wt::WString::tr("chartconfig-template"), this);

  chart->initLayout();

  // x-axis related controls
  WCheckBox *autoRangeX = new WCheckBox(this);
  template_->bindWidget("xAuto", autoRangeX);
  autoRangeX->setCheckState(Wt::Checked);
  WLineEdit *xMin = new WLineEdit(Wt::asString(chart->axis(XAxis_3D).minimum()), this);
  template_->bindWidget("xAxisMin", xMin);
  xMin->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  xMin->setEnabled(false);
  WLineEdit *xMax = new WLineEdit(Wt::asString(chart->axis(XAxis_3D).maximum()), this);
  template_->bindWidget("xAxisMax", xMax);
  xMax->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  xMax->setEnabled(false);

  // y-axis related controls
  WCheckBox *autoRangeY = new WCheckBox(this);
  template_->bindWidget("yAuto", autoRangeY);
  autoRangeY->setCheckState(Wt::Checked);
  WLineEdit *yMin = new WLineEdit(Wt::asString(chart->axis(YAxis_3D).minimum()), this);
  template_->bindWidget("yAxisMin", yMin);
  yMin->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  yMin->setEnabled(false);
  WLineEdit *yMax = new WLineEdit(Wt::asString(chart->axis(YAxis_3D).maximum()), this);
  template_->bindWidget("yAxisMax", yMax);
  yMax->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  yMax->setEnabled(false);

  // z-axis related controls
  WCheckBox *autoRangeZ = new WCheckBox(this);
  template_->bindWidget("zAuto", autoRangeZ);
  autoRangeZ->setCheckState(Wt::Checked);
  WLineEdit *zMin = new WLineEdit(Wt::asString(chart->axis(ZAxis_3D).minimum()), this);
  template_->bindWidget("zAxisMin", zMin);
  zMin->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  zMin->setEnabled(false);
  WLineEdit *zMax = new WLineEdit(Wt::asString(chart->axis(ZAxis_3D).maximum()), this);
  template_->bindWidget("zAxisMax", zMax);
  zMax->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  zMax->setEnabled(false);

  // hook it up
  autoRangeX->checked().connect(std::bind([=] () {
	switch (autoRangeX->checkState()) {
	case Unchecked:
	  xMin->setEnabled(true);
	  xMax->setEnabled(true);
	  break;
	case Checked:
	  xMin->setText(Wt::asString(chart->axis(XAxis_3D).minimum()));
	  xMin->setEnabled(false);
	  xMax->setText(Wt::asString(chart->axis(XAxis_3D).maximum()));
	  xMax->setEnabled(false);
	  
	  chart->axis(XAxis_3D).setAutoLimits(Chart::MinimumValue 
					       | Chart::MaximumValue);
	  break;
	default:
	  break;
	}
      }));
  autoRangeX->unChecked().connect(std::bind([=] () {
	switch (autoRangeX->checkState()) {
	case Unchecked:
	  xMin->setEnabled(true);
	  xMax->setEnabled(true);
	  break;
	case Checked:
	  xMin->setText(Wt::asString(chart->axis(XAxis_3D).minimum()));
	  xMin->setEnabled(false);
	  xMax->setText(Wt::asString(chart->axis(XAxis_3D).maximum()));
	  xMax->setEnabled(false);
	  
	  chart->axis(XAxis_3D).setAutoLimits(Chart::MinimumValue 
					       | Chart::MaximumValue);
	  break;
	default:
	  break;
	}
      }));
  autoRangeY->checked().connect(std::bind([=] () {
	switch (autoRangeY->checkState()) {
	case Unchecked:
	  yMin->setEnabled(true);
	  yMax->setEnabled(true);
	  break;
	case Checked:
	  yMin->setText(Wt::asString(chart->axis(YAxis_3D).minimum())); yMin->setEnabled(false);
	  yMax->setText(Wt::asString(chart->axis(YAxis_3D).maximum())); yMax->setEnabled(false);

	  chart->axis(YAxis_3D).setAutoLimits(Chart::MinimumValue 
					       | Chart::MaximumValue);
	  break;
	default:
	  break;
	}
      }));
  autoRangeY->unChecked().connect(std::bind([=] () {
	switch (autoRangeY->checkState()) {
	case Unchecked:
	  yMin->setEnabled(true);
	  yMax->setEnabled(true);
	  break;
	case Checked:
	  yMin->setText(Wt::asString(chart->axis(YAxis_3D).minimum())); yMin->setEnabled(false);
	  yMax->setText(Wt::asString(chart->axis(YAxis_3D).maximum())); yMax->setEnabled(false);

	  chart->axis(YAxis_3D).setAutoLimits(Chart::MinimumValue 
					       | Chart::MaximumValue);
	  break;
	default:
	  break;
	}
      }));
  autoRangeZ->checked().connect(std::bind([=] () {
	switch (autoRangeZ->checkState()) {
	case Unchecked:
	  zMin->setEnabled(true);
	  zMax->setEnabled(true);
	  break;
	case Checked:
	  zMin->setText(Wt::asString(chart->axis(ZAxis_3D).minimum())); zMin->setEnabled(false);
	  zMax->setText(Wt::asString(chart->axis(ZAxis_3D).maximum())); zMax->setEnabled(false);

	  chart->axis(ZAxis_3D).setAutoLimits(Chart::MinimumValue 
					       | Chart::MaximumValue);
	  break;
	default:
	  break;
	}
      }));
  autoRangeZ->unChecked().connect(std::bind([=] () {
	switch (autoRangeZ->checkState()) {
	case Unchecked:
	  zMin->setEnabled(true);
	  zMax->setEnabled(true);
	  break;
	case Checked:
	  zMin->setText(Wt::asString(chart->axis(ZAxis_3D).minimum())); zMin->setEnabled(false);
	  zMax->setText(Wt::asString(chart->axis(ZAxis_3D).maximum())); zMax->setEnabled(false);

	  chart->axis(ZAxis_3D).setAutoLimits(Chart::MinimumValue 
					       | Chart::MaximumValue);
	  break;
	default:
	  break;
	}
      }));

  xMin->changed().connect(std::bind([=] () {
	chart->axis(XAxis_3D).setRange(Wt::asNumber(xMin->text()),
				       Wt::asNumber(xMax->text()));
      }));
  xMax->changed().connect(std::bind([=] () {
	chart->axis(XAxis_3D).setRange(Wt::asNumber(xMin->text()),
				       Wt::asNumber(xMax->text()));
      }));
  yMin->changed().connect(std::bind([=] () {
	chart->axis(YAxis_3D).setRange(Wt::asNumber(yMin->text()),
				       Wt::asNumber(yMax->text()));
      }));
  yMax->changed().connect(std::bind([=] () {
	chart->axis(YAxis_3D).setRange(Wt::asNumber(yMin->text()),
					  Wt::asNumber(yMax->text()));
      }));
  zMin->changed().connect(std::bind([=] () {
	chart->axis(ZAxis_3D).setRange(Wt::asNumber(zMin->text()),
					  Wt::asNumber(zMax->text()));
      }));
  zMax->changed().connect(std::bind([=] () {
	chart->axis(ZAxis_3D).setRange(Wt::asNumber(zMin->text()),
					  Wt::asNumber(zMax->text()));
      }));
}


GridDataSettings::GridDataSettings(WCartesian3DChart *chart,
				   WAbstractGridData *data,
				   WContainerWidget * parent)
  : WContainerWidget(parent),
    data_(data),
    chart_(chart),
    xStart_(-10.0), xEnd_(10.0),
    yStart_(-10.0), yEnd_(10.0),
    nbXPts_(20), nbYPts_(20)
{
  template_ = 
    new Wt::WTemplate(Wt::WString::tr("dataconfig-template"), this);
  template_->addFunction("id", &WTemplate::Functions::id);
  template_->addFunction("tr", &WTemplate::Functions::tr);
  template_->addFunction("block", &WTemplate::Functions::block);

  typeSelection_ = new WComboBox(this);
  typeSelection_->addItem("Points");
  typeSelection_->addItem("Surface");
  template_->bindWidget("typeSelection", typeSelection_);

  xMin_ = new WLineEdit(this);
  template_->bindWidget("x-axisMin", xMin_);
  xMin_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));  

  xMax_ = new WLineEdit(this);
  template_->bindWidget("x-axisMax", xMax_);
  xMax_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));

  yMin_ = new WLineEdit(this);
  template_->bindWidget("y-axisMin", yMin_);
  yMin_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));

  yMax_ = new WLineEdit(this);
  template_->bindWidget("y-axisMax", yMax_);
  yMax_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));

  nbPtsX_ = new WLineEdit(this);
  nbPtsX_->setValidator(new Wt::WIntValidator(0, 50));
  template_->bindWidget("nbXPoints", nbPtsX_);

  nbPtsY_ = new WLineEdit(this);
  nbPtsY_->setValidator(new Wt::WIntValidator(0, 50));
  template_->bindWidget("nbYPoints", nbPtsY_);

  WLineEdit *ptSize_ = new WLineEdit(this);
  ptSize_->setValidator(new Wt::WDoubleValidator(0.0, std::numeric_limits<double>::max()));
  template_->bindWidget("pointSize", ptSize_);
  ptSize_->setText(Wt::asString(data_->pointSize()));

  enableMesh_ = new WCheckBox(this);
  template_->bindWidget("meshEnabled", enableMesh_);

  hidden_ = new WCheckBox(this);
  hidden_->setCheckState(Wt::Unchecked);
  template_->bindWidget("hiddenEnabled", hidden_);

  if (dynamic_cast<EquidistantGrid*>(data->model()) != 0) {
    EquidistantGrid* model = dynamic_cast<EquidistantGrid*>(data->model());
    xMin_->setText(Wt::asString(model->xMin()));
    xMax_->setText(Wt::asString(model->xMax()));
    yMin_->setText(Wt::asString(model->yMin()));
    yMax_->setText(Wt::asString(model->yMax()));
    nbPtsX_->setText(Wt::asString(model->nbXPts()));
    nbPtsY_->setText(Wt::asString(model->nbYPts()));
  }
  

  xMin_->changed().connect(this, &GridDataSettings::updateData);
  xMax_->changed().connect(this, &GridDataSettings::updateData);
  yMin_->changed().connect(this, &GridDataSettings::updateData);
  yMax_->changed().connect(this, &GridDataSettings::updateData);
  nbPtsX_->changed().connect(this, &GridDataSettings::updateData);
  nbPtsY_->changed().connect(this, &GridDataSettings::updateData);
  enableMesh_->checked().connect(this, &GridDataSettings::enableMesh);
  enableMesh_->unChecked().connect(this, &GridDataSettings::enableMesh);
  hidden_->checked().connect(this, &GridDataSettings::hideData);
  hidden_->unChecked().connect(this, &GridDataSettings::hideData);
  typeSelection_->sactivated().connect(this, &GridDataSettings::changeChartType);
  ptSize_->changed().connect(std::bind([=] () {
	data->setPointSize(Wt::asNumber(ptSize_->text()));
      }));
}

GridDataSettings::~GridDataSettings()
{
}

void GridDataSettings::updateData()
{
  if (nbPtsX_->validate() != Wt::WValidator::Valid ||
      nbPtsY_->validate() != Wt::WValidator::Valid)
    return;

  WAbstractItemModel *model = data_->model();
  SombreroData *sombreroModel = dynamic_cast<SombreroData*>(model);
  if (sombreroModel != 0) {
    sombreroModel->update(Wt::asNumber(xMin_->text()),
			  Wt::asNumber(xMax_->text()),
			  Wt::asNumber(yMin_->text()),
			  Wt::asNumber(yMax_->text()),
			  (int)Wt::asNumber(nbPtsX_->text()),
			  (int)Wt::asNumber(nbPtsY_->text()));
  } else if (dynamic_cast<PlaneData*>(model) != 0) {
    PlaneData *planeModel = dynamic_cast<PlaneData*>(model);
    double xDelta = (Wt::asNumber(xMax_->text()) - Wt::asNumber(xMin_->text()))/
      Wt::asNumber(nbPtsX_->text());
    double yDelta = (Wt::asNumber(yMax_->text()) - Wt::asNumber(yMin_->text()))/
      Wt::asNumber(nbPtsY_->text());
    planeModel->update(Wt::asNumber(xMin_->text()),
		       xDelta,
		       Wt::asNumber(yMin_->text()),
		       yDelta,
		       (int)Wt::asNumber(nbPtsX_->text()),
		       (int)Wt::asNumber(nbPtsY_->text()));
    dynamic_cast<WEquidistantGridData*>(data_)
      ->setXAbscis(Wt::asNumber(xMin_->text()), xDelta);
    dynamic_cast<WEquidistantGridData*>(data_)
      ->setYAbscis(Wt::asNumber(yMin_->text()), yDelta);
  }

  
}

void GridDataSettings::enableMesh()
{
  if (data_->type() != SurfaceSeries3D) {
    enableMesh_->setCheckState(Wt::Unchecked);
    return;
  }

  data_->setSurfaceMeshEnabled(enableMesh_->checkState() == Wt::Checked);
}

void GridDataSettings::changeChartType(WString selection)
{
  if ( selection == WString("Points") ) {
    data_->setType(Wt::Chart::PointSeries3D);
  } else if ( selection == WString("Surface") ) {
    data_->setType(Wt::Chart::SurfaceSeries3D);
  }
}

void GridDataSettings::hideData()
{
  if (hidden_->checkState() == Wt::Checked) {
    data_->setHidden(true);
  } else if (hidden_->checkState() == Wt::Unchecked) {
    data_->setHidden(false);
  }
}


ScatterDataSettings::ScatterDataSettings(WScatterData *data,
					 WContainerWidget *parent)
  : WContainerWidget(parent)
{
  template_ = new WTemplate(Wt::WString::tr("scatterdata-config"), this);

  WLineEdit *ptSize_ = new WLineEdit(this);
  ptSize_->setValidator(new Wt::WDoubleValidator(0.0, std::numeric_limits<double>::max()));
  template_->bindWidget("pointSize", ptSize_);
  ptSize_->setText(Wt::asString(data->pointSize()));

  enableDroplines_ = new WCheckBox(this);
  template_->bindWidget("enableDroplines", enableDroplines_);
  hide_ = new WCheckBox(this);
  template_->bindWidget("hide", hide_);

  ptSize_->changed().connect(std::bind([=] () {
	data->setPointSize(Wt::asNumber(ptSize_->text()));
      }));
  enableDroplines_->checked().connect(std::bind([=] () {
      data->setDroplinesEnabled(true);
      }));
  enableDroplines_->unChecked().connect(std::bind([=] () {
	data->setDroplinesEnabled(false);
      }));
  hide_->checked().connect(std::bind([=] (){
	data->setHidden(true);
      }));
  hide_->unChecked().connect(std::bind([=] (){
	data->setHidden(false);
      }));
}

CategoryDataSettings::CategoryDataSettings(WAbstractGridData *data,
					   WContainerWidget *parent)
  : WContainerWidget(parent),
    data_(data)
{
  template_ = new WTemplate(Wt::WString::tr("categorydata-config"), this);

  barWidthX_ = new WLineEdit(this);
  template_->bindWidget("barWidthX", barWidthX_);
  barWidthX_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  barWidthX_->setText(Wt::asString(data_->barWidthX()));
  barWidthY_ = new WLineEdit(this);
  template_->bindWidget("barWidthY", barWidthY_);
  barWidthY_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  barWidthY_->setText(Wt::asString(data_->barWidthY()));
  hideData_ = new WCheckBox(this);
  template_->bindWidget("hide", hideData_);

  barWidthX_->changed().connect(this, &CategoryDataSettings::adjustBarWidth);
  barWidthY_->changed().connect(this, &CategoryDataSettings::adjustBarWidth);
  hideData_->checked().connect(std::bind([=] (){
	data->setHidden(true);
      }));
  hideData_->unChecked().connect(std::bind([=] (){
	data->setHidden(false);
      }));
}

void CategoryDataSettings::adjustBarWidth()
{
  data_->setBarWidth((float)Wt::asNumber(barWidthX_->text()),
		     (float)Wt::asNumber(barWidthY_->text()));
}

