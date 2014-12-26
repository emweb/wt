#include "Tabs.h"
#include "Wt/WDoubleValidator"
#include "Wt/WIntValidator"
#include "Wt/Chart/WAbstractChartImplementation"
#include "Wt/WPushButton"
#include "Wt/WSelectionBox"

#include <limits>
#include <functional>

ChartSettings::ChartSettings(WCartesian3DChart *chart,
			     WContainerWidget * parent)
  : WContainerWidget(parent)
{
  WTemplate* template_ = new WTemplate(Wt::WString::tr("chartconfig-template"), this);

  WCheckBox *autoRangeX_ = new WCheckBox(this);
  template_->bindWidget("xAuto", autoRangeX_);
  autoRangeX_->setCheckState(Wt::Checked);
  chart->initLayout();
  WLineEdit *xMin_ = new WLineEdit
    (Wt::asString(chart->axis(XAxis_3D).minimum()), this);
  template_->bindWidget("xAxisMin", xMin_);
  xMin_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  xMin_->setEnabled(false);
  WLineEdit *xMax_ = new WLineEdit
    (Wt::asString(chart->axis(XAxis_3D).maximum()), this);
  template_->bindWidget("xAxisMax", xMax_);
  xMax_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  xMax_->setEnabled(false);

  WCheckBox *autoRangeY_ = new WCheckBox(this);
  template_->bindWidget("yAuto", autoRangeY_);
  autoRangeY_->setCheckState(Wt::Checked);
  WLineEdit *yMin_ = new WLineEdit
    (Wt::asString(chart->axis(YAxis_3D).minimum()), this);
  template_->bindWidget("yAxisMin", yMin_);
  yMin_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  yMin_->setEnabled(false);
  WLineEdit *yMax_ = new WLineEdit
    (Wt::asString(chart->axis(YAxis_3D).maximum()), this);
  template_->bindWidget("yAxisMax", yMax_);
  yMax_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  yMax_->setEnabled(false);

  WCheckBox *autoRangeZ_ = new WCheckBox(this);
  template_->bindWidget("zAuto", autoRangeZ_);
  autoRangeZ_->setCheckState(Wt::Checked);
  WLineEdit *zMin_ = new WLineEdit
    (Wt::asString(chart->axis(ZAxis_3D).minimum()), this);
  template_->bindWidget("zAxisMin", zMin_);
  zMin_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  zMin_->setEnabled(false);
  WLineEdit *zMax_ = new WLineEdit
    (Wt::asString(chart->axis(ZAxis_3D).maximum()), this);
  template_->bindWidget("zAxisMax", zMax_);
  zMax_->setValidator(new Wt::WDoubleValidator(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
  zMax_->setEnabled(false);

  WLineEdit *title = new WLineEdit(this);
  template_->bindWidget("chartTitle", title);
  WCheckBox *enableLegend = new WCheckBox(this);
  template_->bindWidget("chartLegend", enableLegend);
  WComboBox *legendSide = new WComboBox(this);
  legendSide->addItem("Left");
  legendSide->addItem("Right");
  legendSide->addItem("Top");
  legendSide->addItem("Bottom");
  template_->bindWidget("legendside", legendSide);
  switch (chart->legendSide()) {
  case Left:
    legendSide->setCurrentIndex(0); break;
  case Right:
    legendSide->setCurrentIndex(1); break;
  case Top:
    legendSide->setCurrentIndex(2); break;
  case Bottom:
    legendSide->setCurrentIndex(3); break;
  default:
    break;
  }
  WComboBox *legendAlignment = new WComboBox(this);
  legendAlignment->addItem("Left");
  legendAlignment->addItem("Center");
  legendAlignment->addItem("Right");
  legendAlignment->addItem("Top");
  legendAlignment->addItem("Middle");
  legendAlignment->addItem("Bottom");
  template_->bindWidget("legendalignment", legendAlignment);
  switch (chart->legendAlignment()) {
  case AlignLeft:
    legendAlignment->setCurrentIndex(0); break;
  case AlignCenter:
    legendAlignment->setCurrentIndex(1); break;
  case AlignRight:
    legendAlignment->setCurrentIndex(2); break;
  case AlignTop:
    legendAlignment->setCurrentIndex(3); break;
  case AlignMiddle:
    legendAlignment->setCurrentIndex(4); break;
  case AlignBottom:
    legendAlignment->setCurrentIndex(5); break;
  default:
    break;
  }
  WCheckBox *enableGridLines = new WCheckBox(this);
  template_->bindWidget("gridlines", enableGridLines);
  WCheckBox *enableIntersectionLines = new WCheckBox(this);
  template_->bindWidget("intersectionlines", enableIntersectionLines);
  WComboBox *intersectionLineColor = new WComboBox(this);
  intersectionLineColor->addItem("red");
  intersectionLineColor->addItem("green");
  intersectionLineColor->addItem("blue");
  intersectionLineColor->addItem("black");
  intersectionLineColor->addItem("cyan");
  intersectionLineColor->addItem("magenta");
  intersectionLineColor->addItem("yellow");
  intersectionLineColor->setCurrentIndex(intersectionLineColor->count() - 1);
  template_->bindWidget("intersectionlinecolor", intersectionLineColor);
  WLineEdit *widgetWidth = new WLineEdit(Wt::asString(chart->width().value()), this);
  widgetWidth->setValidator(new Wt::WIntValidator(1, 2000));
  WLineEdit *widgetHeight = new WLineEdit(Wt::asString(chart->height().value()), this);
  widgetHeight->setValidator(new Wt::WIntValidator(1, 2000));
  template_->bindWidget("width", widgetWidth);
  template_->bindWidget("height", widgetHeight);

  WLineEdit *xAxisTitle = new WLineEdit(chart->axis(XAxis_3D).title(), this);
  WLineEdit *yAxisTitle = new WLineEdit(chart->axis(YAxis_3D).title(), this);
  WLineEdit *zAxisTitle = new WLineEdit(chart->axis(ZAxis_3D).title(), this);
  template_->bindWidget("xTitle", xAxisTitle);
  template_->bindWidget("yTitle", yAxisTitle);
  template_->bindWidget("zTitle", zAxisTitle);
    

  // hook it up
  autoRangeX_->checked().connect(std::bind([=] () {
	chart->axis(XAxis_3D).setAutoLimits(Chart::MinimumValue 
					     | Chart::MaximumValue);
	xMin_->setEnabled(false); xMax_->setEnabled(false);
      }));
  autoRangeX_->unChecked().connect(std::bind([=] () {
	xMin_->setEnabled(true); xMax_->setEnabled(true);
	chart->axis(XAxis_3D).setRange(Wt::asNumber(xMin_->text()),
					Wt::asNumber(xMax_->text()));
      }));
  autoRangeY_->checked().connect(std::bind([=] () {
	chart->axis(YAxis_3D).setAutoLimits(Chart::MinimumValue 
					     | Chart::MaximumValue);
	yMin_->setEnabled(false); yMax_->setEnabled(false);
      }));
  autoRangeY_->unChecked().connect(std::bind([=] () {
	yMin_->setEnabled(true); yMax_->setEnabled(true);
	chart->axis(YAxis_3D).setRange(Wt::asNumber(yMin_->text()),
					  Wt::asNumber(yMax_->text()));
      }));
  autoRangeZ_->checked().connect(std::bind([=] () {
	chart->axis(ZAxis_3D).setAutoLimits(Chart::MinimumValue 
					     | Chart::MaximumValue);
	zMin_->setEnabled(false); zMax_->setEnabled(false);
      }));
  autoRangeZ_->unChecked().connect(std::bind([=] () {
	zMin_->setEnabled(true); zMax_->setEnabled(true);
	chart->axis(ZAxis_3D).setRange(Wt::asNumber(zMin_->text()),
					Wt::asNumber(zMax_->text()));
      }));

  xMin_->changed().connect(std::bind([=] () {
	chart->axis(XAxis_3D).setRange(Wt::asNumber(xMin_->text()),
					Wt::asNumber(xMax_->text()));
      }));
  xMax_->changed().connect(std::bind([=] () {
	chart->axis(XAxis_3D).setRange(Wt::asNumber(xMin_->text()),
					Wt::asNumber(xMax_->text()));
      }));
  yMin_->changed().connect(std::bind([=] () {
	chart->axis(YAxis_3D).setRange(Wt::asNumber(yMin_->text()),
					  Wt::asNumber(yMax_->text()));
      }));
  yMax_->changed().connect(std::bind([=] () {
	chart->axis(YAxis_3D).setRange(Wt::asNumber(yMin_->text()),
					  Wt::asNumber(yMax_->text()));
      }));
  zMin_->changed().connect(std::bind([=] () {
	chart->axis(ZAxis_3D).setRange(Wt::asNumber(zMin_->text()),
					  Wt::asNumber(zMax_->text()));
      }));
  zMax_->changed().connect(std::bind([=] () {
	chart->axis(ZAxis_3D).setRange(Wt::asNumber(zMin_->text()),
					  Wt::asNumber(zMax_->text()));
      }));

  enableGridLines->checked().connect(std::bind([=]() {
	chart->setGridEnabled(Wt::Chart::XY_Plane, Wt::Chart::XAxis_3D, true);
	chart->setGridEnabled(Wt::Chart::XY_Plane, Wt::Chart::YAxis_3D, true);
	chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::XAxis_3D, true);
	chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::ZAxis_3D, true);
	chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::YAxis_3D, true);
	chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::ZAxis_3D, true);
      }));
  enableGridLines->unChecked().connect(std::bind([=]() {
	chart->setGridEnabled(Wt::Chart::XY_Plane, Wt::Chart::XAxis_3D, false);
	chart->setGridEnabled(Wt::Chart::XY_Plane, Wt::Chart::YAxis_3D, false);
	chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::XAxis_3D, false);
	chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::ZAxis_3D, false);
	chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::YAxis_3D, false);
	chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::ZAxis_3D, false);
      }));

  auto changeIntersectionLinesColor = [=]() {
	switch (intersectionLineColor->currentIndex()) {
	case 0:
	  chart->setIntersectionLinesColor(WColor(255,0,0)); break;
	case 1:
	  chart->setIntersectionLinesColor(WColor(0,255,0)); break;
	case 2:
	  chart->setIntersectionLinesColor(WColor(0,0,255)); break;
	case 3:
	  chart->setIntersectionLinesColor(WColor(0,0,0)); break;
	case 4:
	  chart->setIntersectionLinesColor(WColor(0,255,255)); break;
	case 5:
	  chart->setIntersectionLinesColor(WColor(255,0,255)); break;
	case 6:
	  chart->setIntersectionLinesColor(WColor(255,255,0)); break;
	}
      };

  enableIntersectionLines->checked().connect(std::bind([=]() {
	chart->setIntersectionLinesEnabled(true);
	changeIntersectionLinesColor();
      }));
  enableIntersectionLines->unChecked().connect(std::bind([=]() {
	chart->setIntersectionLinesEnabled(false);
      }));

  intersectionLineColor->changed().connect(std::bind(changeIntersectionLinesColor));

  enableLegend->checked().connect(std::bind([=]() {
	chart->setLegendEnabled(true);
      }));
  enableLegend->unChecked().connect(std::bind([=]() {
	chart->setLegendEnabled(false);
      }));

  legendSide->changed().connect(std::bind([=]() {
	switch (legendSide->currentIndex()) {
	case 0:
	  chart->setLegendLocation(Left, chart->legendAlignment()); break;
	case 1:
	  chart->setLegendLocation(Right, chart->legendAlignment()); break;
	case 2:
	  chart->setLegendLocation(Top, chart->legendAlignment()); break;
	case 3:
	  chart->setLegendLocation(Bottom, chart->legendAlignment()); break;
	}
      }));
  legendAlignment->changed().connect(std::bind([=]() {
	switch (legendAlignment->currentIndex()) {
	case 0:
	  chart->setLegendLocation(chart->legendSide(), AlignLeft); break;
	case 1:
	  chart->setLegendLocation(chart->legendSide(), AlignCenter); break;
	case 2:
	  chart->setLegendLocation(chart->legendSide(), AlignRight); break;
	case 3:
	  chart->setLegendLocation(chart->legendSide(), AlignTop); break;
	case 4:
	  chart->setLegendLocation(chart->legendSide(), AlignMiddle); break;
	case 5:
	  chart->setLegendLocation(chart->legendSide(), AlignBottom); break;
	}
      }));

  title->changed().connect(std::bind([=] () {
	chart->setTitle(Wt::asString(title->text()));
      }));

  widgetWidth->changed().connect(std::bind([=] () {
	chart->resize(Wt::asNumber(widgetWidth->text()),
		       Wt::asNumber(widgetHeight->text()));
      }));
  widgetHeight->changed().connect(std::bind([=] () {
	chart->resize(Wt::asNumber(widgetWidth->text()),
		       Wt::asNumber(widgetHeight->text()));
      }));

  xAxisTitle->changed().connect(std::bind([=]() {
	chart->axis(XAxis_3D).setTitle(xAxisTitle->text());
      }));
  yAxisTitle->changed().connect(std::bind([=]() {
	chart->axis(YAxis_3D).setTitle(yAxisTitle->text());
      }));
  zAxisTitle->changed().connect(std::bind([=]() {
	chart->axis(ZAxis_3D).setTitle(zAxisTitle->text());
      }));
  
}


/*
 * Data selection Widget definition
 */
DataSelection::DataSelection(WCartesian3DChart *chart)
{
  WTemplate* template_ = new WTemplate(Wt::WString::tr("dataselection-template"), this);

  notShown = new WSelectionBox(this);
  template_->bindWidget("notshownselection", notShown);
  shown = new WSelectionBox(this);
  template_->bindWidget("shownselection", shown);

  WPushButton *addSelected = new WPushButton("add dataset", this);
  template_->bindWidget("addbutton", addSelected);
  WPushButton *removeSelected = new WPushButton("remove dataset", this);
  template_->bindWidget("removebutton", removeSelected);

  // define functionality
  notShown->sactivated().connect(std::bind([=] (WString selection) {
	//WString selection;
  	shown->clearSelection();
  	for (unsigned i = 0; i < dataCollection_.size(); i++) {
  	  if (dataCollection_[i].first == selection)
  	    selectionChange_.emit(dataCollection_[i].second);
  	}
      }, std::placeholders::_1));
  shown->sactivated().connect(std::bind([=] (WString selection) {
	notShown->clearSelection();
	for (unsigned i = 0; i < dataCollection_.size(); i++) {
  	  if (dataCollection_[i].first == selection)
  	    selectionChange_.emit(dataCollection_[i].second);
  	}
      }, std::placeholders::_1));
  addSelected->clicked().connect(std::bind([=] () {
	int index = notShown->currentIndex();
	if (index == -1)
	  return;
  
	const WString currentText = notShown->currentText();
	for (unsigned i = 0; i < dataCollection_.size(); i++) {
	  if (dataCollection_[i].first == currentText){
	      chart->addDataSeries(dataCollection_[i].second);
	      selectionChange_.emit(dataCollection_[i].second);
	  }
	}
	shown->addItem(currentText);
	shown->setCurrentIndex(shown->count()-1);
	notShown->removeItem(index);
	notShown->clearSelection();
	
      }));
  removeSelected->clicked().connect(std::bind([=] () {
	int index = shown->currentIndex();
	if (index == -1)
	  return;
  
	const WString currentText = shown->currentText();
	for (unsigned i = 0; i < dataCollection_.size(); i++) {
	  if (dataCollection_[i].first == currentText)
	    chart->removeDataSeries(dataCollection_[i].second);
	}
	notShown->addItem(currentText);
	notShown->setCurrentIndex(shown->count()-1);
	shown->removeItem(index);
	shown->clearSelection();
      }));
}

void DataSelection::addDataToCollection(WString name,
					WAbstractDataSeries3D* data)
{
  DataSelectionItem item(name, data);
  dataCollection_.push_back(item);
  notShown->addItem(name);  
}


DataConfig::DataConfig(WCartesian3DChart* chart)
{
  WTemplate *template_ = new WTemplate(Wt::WString::tr("totaldataconfig-template"), this);

  dataselection_ = new DataSelection(chart);
  template_->bindWidget("dataselection", dataselection_);

  WStackedWidget *sw = new WStackedWidget(this);
  sw->addWidget(new WText("Select data to configure it"));
  numgriddatasettings_ = new NumGridDataSettings();
  catgriddatasettings_ = new CatGridDataSettings();
  scatterdatasettings_ = new ScatterDataSettings();
  sw->addWidget(numgriddatasettings_);
  sw->addWidget(catgriddatasettings_);
  sw->addWidget(scatterdatasettings_);
  template_->bindWidget("dataconfig", sw);

  dataselection_->selectionChanged().connect(std::bind([=] (WAbstractDataSeries3D* data) {
	if (dynamic_cast<WAbstractGridData*>(data)) {
	  WAbstractGridData *gridData = dynamic_cast<WAbstractGridData*>(data);
	  if (gridData->type() == BarSeries3D) {
	    catgriddatasettings_->bindDataSet(gridData);
	    sw->setCurrentIndex(2);
	  } else {
	    numgriddatasettings_->bindDataSet(gridData);
	    sw->setCurrentIndex(1);
	  }
	} else if (dynamic_cast<WScatterData*>(data)) {
	  scatterdatasettings_->bindDataSet(dynamic_cast<WScatterData*>(data));
	  sw->setCurrentIndex(3);
	}
      }, std::placeholders::_1));
}

void DataConfig::addDataToCollection(WString name, WAbstractDataSeries3D *data)
{
  dataselection_->addDataToCollection(name, data);
}
