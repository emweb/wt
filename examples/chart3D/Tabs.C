#include "Tabs.h"
#include "Wt/WDoubleValidator.h"
#include "Wt/WIntValidator.h"
#include "Wt/Chart/WAbstractChartImplementation.h"
#include "Wt/WPushButton.h"
#include "Wt/WSelectionBox.h"

#include <limits>
#include <functional>


ChartSettings::ChartSettings(Wt::Chart::WCartesian3DChart *chart)
  : WContainerWidget()
{
  Wt::WTemplate* template_ =
      this->addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("chartconfig-template")));

  auto autoRangeX = std::make_unique<Wt::WCheckBox>();
  auto autoRangeX_ = template_->bindWidget("xAuto", std::move(autoRangeX));
  autoRangeX_->setCheckState(Wt::CheckState::Checked);
  chart->initLayout();

  auto xMin = std::make_unique<Wt::WLineEdit>(Wt::asString(chart->axis(Wt::Chart::Axis::X3D).minimum()));
  auto xMin_ = template_->bindWidget("xAxisMin", std::move(xMin));
  xMin_->setValidator(std::make_shared<Wt::WDoubleValidator>(
                        -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max()));
  xMin_->setEnabled(false);

  auto xMax = std::make_unique<Wt::WLineEdit>(Wt::asString(chart->axis(Wt::Chart::Axis::X3D).maximum()));
  auto xMax_ = template_->bindWidget("xAxisMax", std::move(xMax));
  xMax_->setValidator(std::make_shared<Wt::WDoubleValidator>(
                        -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max()));
  xMax_->setEnabled(false);

  auto autoRangeY = std::make_unique<Wt::WCheckBox>();
  auto autoRangeY_ = template_->bindWidget("yAuto", std::move(autoRangeY));
  autoRangeY_->setCheckState(Wt::CheckState::Checked);

  auto yMin = std::make_unique<Wt::WLineEdit>(Wt::asString(chart->axis(Wt::Chart::Axis::Y3D).minimum()));
  auto yMin_ = template_->bindWidget("yAxisMin", std::move(yMin));
  yMin_->setValidator(std::make_shared<Wt::WDoubleValidator>(
                        -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max()));
  yMin_->setEnabled(false);

  auto yMax = std::make_unique<Wt::WLineEdit>(Wt::asString(chart->axis(Wt::Chart::Axis::Y3D).maximum()));
  auto yMax_ = template_->bindWidget("yAxisMax", std::move(yMax));
  yMax_->setValidator(std::make_shared<Wt::WDoubleValidator>(
                        -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max()));
  yMax_->setEnabled(false);

  auto autoRangeZ = std::make_unique<Wt::WCheckBox>();
  auto autoRangeZ_ = template_->bindWidget("zAuto", std::move(autoRangeZ));
  autoRangeZ_->setCheckState(Wt::CheckState::Checked);

  auto zMin = std::make_unique<Wt::WLineEdit>(Wt::asString(chart->axis(Wt::Chart::Axis::Z3D).minimum()));
  auto zMin_ = template_->bindWidget("zAxisMin", std::move(zMin));
  zMin_->setValidator(std::make_shared<Wt::WDoubleValidator>(
                        -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max()));
  zMin_->setEnabled(false);

  auto zMax = std::make_unique<Wt::WLineEdit>(Wt::asString(chart->axis(Wt::Chart::Axis::Z3D).maximum()));
  auto zMax_ = template_->bindWidget("zAxisMax", std::move(zMax));
  zMax_->setValidator(std::make_shared<Wt::WDoubleValidator>(
                        -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max()));
  zMax_->setEnabled(false);

  auto title = std::make_unique<Wt::WLineEdit>();
  auto title_ = template_->bindWidget("chartTitle", std::move(title));

  auto enableLegend = std::make_unique<Wt::WCheckBox>();
  auto enableLegend_ = template_->bindWidget("chartLegend", std::move(enableLegend));

  auto legendSide = std::make_unique<Wt::WComboBox>();
  auto legendSide_ = template_->bindWidget("legendside", std::move(legendSide));
  legendSide_->addItem("Left");
  legendSide_->addItem("Right");
  legendSide_->addItem("Top");
  legendSide_->addItem("Bottom");

  switch (chart->legendSide()) {
  case Wt::Side::Left:
    legendSide_->setCurrentIndex(0); break;
  case Wt::Side::Right:
    legendSide_->setCurrentIndex(1); break;
  case Wt::Side::Top:
    legendSide_->setCurrentIndex(2); break;
  case Wt::Side::Bottom:
    legendSide_->setCurrentIndex(3); break;
  default:
    break;
  }

  auto legendAlignment = std::make_unique<Wt::WComboBox>();
  auto legendAlignment_ = template_->bindWidget("legendalignment", std::move(legendAlignment));
  legendAlignment_->addItem("Left");
  legendAlignment_->addItem("Center");
  legendAlignment_->addItem("Right");
  legendAlignment_->addItem("Top");
  legendAlignment_->addItem("Middle");
  legendAlignment_->addItem("Bottom");

  switch (chart->legendAlignment()) {
  case Wt::AlignmentFlag::Left:
    legendAlignment_->setCurrentIndex(0); break;
  case Wt::AlignmentFlag::Center:
    legendAlignment_->setCurrentIndex(1); break;
  case Wt::AlignmentFlag::Right:
    legendAlignment_->setCurrentIndex(2); break;
  case Wt::AlignmentFlag::Top:
    legendAlignment_->setCurrentIndex(3); break;
  case Wt::AlignmentFlag::Middle:
    legendAlignment_->setCurrentIndex(4); break;
  case Wt::AlignmentFlag::Bottom:
    legendAlignment_->setCurrentIndex(5); break;
  default:
    break;
  }

  auto enableGridLines = std::make_unique<Wt::WCheckBox>();
  auto enableGridLines_ =
      template_->bindWidget("gridlines", std::move(enableGridLines));

  auto enableIntersectionLines = std::make_unique<Wt::WCheckBox>();
  auto enableIntersectionLines_ =
      template_->bindWidget("intersectionlines", std::move(enableIntersectionLines));

  auto intersectionLineColor = std::make_unique<Wt::WComboBox>();
  auto intersectionLineColor_ =
      template_->bindWidget("intersectionlinecolor", std::move(intersectionLineColor));
  intersectionLineColor_->addItem("red");
  intersectionLineColor_->addItem("green");
  intersectionLineColor_->addItem("blue");
  intersectionLineColor_->addItem("black");
  intersectionLineColor_->addItem("cyan");
  intersectionLineColor_->addItem("magenta");
  intersectionLineColor_->addItem("yellow");
  intersectionLineColor_->setCurrentIndex(intersectionLineColor_->count() - 1);

  auto widgetWidth = std::make_unique<Wt::WLineEdit>(Wt::asString(chart->width().value()));
  auto widgetWidth_ = template_->bindWidget("width", std::move(widgetWidth));
  widgetWidth_->setValidator(std::make_shared<Wt::WIntValidator>(1, 2000));
  auto widgetHeight = std::make_unique<Wt::WLineEdit>(Wt::asString(chart->height().value()));
  auto widgetHeight_ = template_->bindWidget("height", std::move(widgetHeight));
  widgetHeight_->setValidator(std::make_shared<Wt::WIntValidator>(1, 2000));

  auto xAxisTitle_ = std::make_unique<Wt::WLineEdit>(chart->axis(Wt::Chart::Axis::X3D).title());
  auto xAxisTitle = template_->bindWidget("xTitle", std::move(xAxisTitle_));
  auto yAxisTitle_ = std::make_unique<Wt::WLineEdit>(chart->axis(Wt::Chart::Axis::Y3D).title());
  auto yAxisTitle = template_->bindWidget("yTitle", std::move(yAxisTitle_));
  auto zAxisTitle_ = std::make_unique<Wt::WLineEdit>(chart->axis(Wt::Chart::Axis::Z3D).title());
  auto zAxisTitle = template_->bindWidget("zTitle", std::move(zAxisTitle_));

  // hook it up
  autoRangeX_->checked().connect([=] () {
        chart->axis(Wt::Chart::Axis::X3D).setAutoLimits(Wt::Chart::AxisValue::Minimum
                                                       | Wt::Chart::AxisValue::Maximum);
        xMin_->setEnabled(false); xMax_->setEnabled(false);
      });
  autoRangeX_->unChecked().connect([=] () {
        xMin_->setEnabled(true); xMax_->setEnabled(true);
        chart->axis(Wt::Chart::Axis::X3D).setRange(asNumber(xMin_->text()),
                                        asNumber(xMax_->text()));
      });
  autoRangeY_->checked().connect([=] () {
        chart->axis(Wt::Chart::Axis::Y3D).setAutoLimits(Wt::Chart::AxisValue::Minimum
                                             | Wt::Chart::AxisValue::Maximum);
        yMin_->setEnabled(false); yMax_->setEnabled(false);
      });
  autoRangeY_->unChecked().connect([=] () {
        yMin_->setEnabled(true); yMax_->setEnabled(true);
        chart->axis(Wt::Chart::Axis::Y3D).setRange(asNumber(yMin_->text()),
                                          asNumber(yMax_->text()));
      });
  autoRangeZ_->checked().connect([=] () {
        chart->axis(Wt::Chart::Axis::Z3D).setAutoLimits(Wt::Chart::AxisValue::Minimum
                                             | Wt::Chart::AxisValue::Maximum);
        zMin_->setEnabled(false); zMax_->setEnabled(false);
      });
  autoRangeZ_->unChecked().connect([=] () {
        zMin_->setEnabled(true); zMax_->setEnabled(true);
        chart->axis(Wt::Chart::Axis::Z3D).setRange(asNumber(zMin_->text()),
                                        asNumber(zMax_->text()));
      });

  xMin_->changed().connect([=] () {
        chart->axis(Wt::Chart::Axis::X3D).setRange(asNumber(xMin_->text()),
                                        asNumber(xMax_->text()));
      });
  xMax_->changed().connect([=] () {
        chart->axis(Wt::Chart::Axis::X3D).setRange(asNumber(xMin_->text()),
                                        asNumber(xMax_->text()));
      });
  yMin_->changed().connect([=] () {
        chart->axis(Wt::Chart::Axis::Y3D).setRange(asNumber(yMin_->text()),
                                          asNumber(yMax_->text()));
      });
  yMax_->changed().connect([=] () {
        chart->axis(Wt::Chart::Axis::Y3D).setRange(asNumber(yMin_->text()),
                                          asNumber(yMax_->text()));
      });
  zMin_->changed().connect([=] () {
        chart->axis(Wt::Chart::Axis::Z3D).setRange(asNumber(zMin_->text()),
                                          asNumber(zMax_->text()));
      });
  zMax_->changed().connect([=] () {
        chart->axis(Wt::Chart::Axis::Z3D).setRange(asNumber(zMin_->text()),
                                          asNumber(zMax_->text()));
      });

  enableGridLines_->checked().connect([=]() {
        chart->setGridEnabled(Wt::Chart::Plane::XY, Wt::Chart::Axis::X3D, true);
        chart->setGridEnabled(Wt::Chart::Plane::XY, Wt::Chart::Axis::Y3D, true);
        chart->setGridEnabled(Wt::Chart::Plane::XZ, Wt::Chart::Axis::X3D, true);
        chart->setGridEnabled(Wt::Chart::Plane::XZ, Wt::Chart::Axis::Z3D, true);
        chart->setGridEnabled(Wt::Chart::Plane::YZ, Wt::Chart::Axis::Y3D, true);
        chart->setGridEnabled(Wt::Chart::Plane::YZ, Wt::Chart::Axis::Z3D, true);
      });
  enableGridLines_->unChecked().connect([=]() {
        chart->setGridEnabled(Wt::Chart::Plane::XY, Wt::Chart::Axis::X3D, false);
        chart->setGridEnabled(Wt::Chart::Plane::XY, Wt::Chart::Axis::Y3D, false);
        chart->setGridEnabled(Wt::Chart::Plane::XZ, Wt::Chart::Axis::X3D, false);
        chart->setGridEnabled(Wt::Chart::Plane::XZ, Wt::Chart::Axis::Z3D, false);
        chart->setGridEnabled(Wt::Chart::Plane::YZ, Wt::Chart::Axis::Y3D, false);
        chart->setGridEnabled(Wt::Chart::Plane::YZ, Wt::Chart::Axis::Z3D, false);
      });

  const auto changeIntersectionLinesColor = [=]() {
        switch (intersectionLineColor_->currentIndex()) {
	case 0:
          chart->setIntersectionLinesColor(Wt::WColor(255,0,0)); break;
	case 1:
          chart->setIntersectionLinesColor(Wt::WColor(0,255,0)); break;
	case 2:
          chart->setIntersectionLinesColor(Wt::WColor(0,0,255)); break;
	case 3:
          chart->setIntersectionLinesColor(Wt::WColor(0,0,0)); break;
	case 4:
          chart->setIntersectionLinesColor(Wt::WColor(0,255,255)); break;
	case 5:
          chart->setIntersectionLinesColor(Wt::WColor(255,0,255)); break;
	case 6:
          chart->setIntersectionLinesColor(Wt::WColor(255,255,0)); break;
	}
      };

  enableIntersectionLines_->checked().connect([=]() {
	chart->setIntersectionLinesEnabled(true);
	changeIntersectionLinesColor();
      });
  enableIntersectionLines_->unChecked().connect([=]() {
	chart->setIntersectionLinesEnabled(false);
      });

  intersectionLineColor_->changed().connect(changeIntersectionLinesColor);

  enableLegend_->checked().connect([=]() {
	chart->setLegendEnabled(true);
      });
  enableLegend_->unChecked().connect([=]() {
	chart->setLegendEnabled(false);
      });

  legendSide_->changed().connect([=]() {
        switch (legendSide_->currentIndex()) {
	case 0:
          chart->setLegendLocation(Wt::Side::Left, chart->legendAlignment()); break;
	case 1:
          chart->setLegendLocation(Wt::Side::Right, chart->legendAlignment()); break;
	case 2:
          chart->setLegendLocation(Wt::Side::Top, chart->legendAlignment()); break;
	case 3:
          chart->setLegendLocation(Wt::Side::Bottom, chart->legendAlignment()); break;
	}
      });
  legendAlignment_->changed().connect([=]() {
        switch (legendAlignment_->currentIndex()) {
	case 0:
          chart->setLegendLocation(chart->legendSide(), Wt::AlignmentFlag::Left); break;
	case 1:
          chart->setLegendLocation(chart->legendSide(), Wt::AlignmentFlag::Center); break;
	case 2:
          chart->setLegendLocation(chart->legendSide(), Wt::AlignmentFlag::Right); break;
	case 3:
          chart->setLegendLocation(chart->legendSide(), Wt::AlignmentFlag::Top); break;
	case 4:
          chart->setLegendLocation(chart->legendSide(), Wt::AlignmentFlag::Middle); break;
	case 5:
          chart->setLegendLocation(chart->legendSide(), Wt::AlignmentFlag::Bottom); break;
	}
      });

  title_->changed().connect([=] () {
        chart->setTitle(Wt::asString(title_->text()));
      });

  widgetWidth_->changed().connect([=] () {
        chart->resize(Wt::asNumber(widgetWidth_->text()),
                       Wt::asNumber(widgetHeight_->text()));
      });
  widgetHeight_->changed().connect([=] () {
        chart->resize(Wt::asNumber(widgetWidth_->text()),
                       Wt::asNumber(widgetHeight_->text()));
      });

  xAxisTitle->changed().connect([=]() {
        chart->axis(Wt::Chart::Axis::X3D).setTitle(xAxisTitle->text());
      });
  yAxisTitle->changed().connect([=]() {
        chart->axis(Wt::Chart::Axis::Y3D).setTitle(yAxisTitle->text());
      });
  zAxisTitle->changed().connect([=]() {
        chart->axis(Wt::Chart::Axis::Z3D).setTitle(zAxisTitle->text());
      });
}


/*
 * Data selection Widget definition
 */
DataSelection::DataSelection(Wt::Chart::WCartesian3DChart *chart)
{
  Wt::WTemplate* template_ =
      this->addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("dataselection-template")));

  auto notShown_ = std::make_unique<Wt::WSelectionBox>();
  notShown = template_->bindWidget("notshownselection", std::move(notShown_));

  auto shown_ = std::make_unique<Wt::WSelectionBox>();
  shown = template_->bindWidget("shownselection", std::move(shown_));

  auto addSelected_ = std::make_unique<Wt::WPushButton>("add dataset");
  auto addSelected = template_->bindWidget("addbutton", std::move(addSelected_));

  auto removeSelected_ = std::make_unique<Wt::WPushButton>("remove dataset");
  auto removeSelected = template_->bindWidget("removebutton", std::move(removeSelected_));

  // define functionality
  notShown->sactivated().connect([=] (Wt::WString selection) {
	//WString selection;
  	shown->clearSelection();
        for (auto& item : dataCollection_) {
          if (item.first == selection)
            selectionChange_.emit(item.second);
  	}
      });
  shown->sactivated().connect([=] (Wt::WString selection) {
	notShown->clearSelection();
	for (auto& item : dataCollection_) {
	  if (item.first == selection)
	    selectionChange_.emit(item.second);
  	}
      });
  addSelected->clicked().connect([=] () {
	int index = notShown->currentIndex();
	if (index == -1)
	  return;
  
        const Wt::WString currentText = notShown->currentText();
	for (auto& item : dataCollection_) {
	  if (item.first == currentText){
              std::unique_ptr<Wt::Chart::WAbstractDataSeries3D> series(item.second);
	      chart->addDataSeries(std::move(series));
	      selectionChange_.emit(item.second);
	  }
	}
	shown->addItem(currentText);
	shown->setCurrentIndex(shown->count()-1);
	notShown->removeItem(index);
	notShown->clearSelection();
	
      });
  removeSelected->clicked().connect([=] () {
	int index = shown->currentIndex();
	if (index == -1)
	  return;
  
        const Wt::WString currentText = shown->currentText();
	for (auto& item : dataCollection_) {
	  if (item.first == currentText)
	    chart->removeDataSeries(item.second).release();
	}
	notShown->addItem(currentText);
	notShown->setCurrentIndex(shown->count()-1);
	shown->removeItem(index);
	shown->clearSelection();
      });
}

void DataSelection::addDataToCollection(Wt::WString name,
                                        Wt::Chart::WAbstractDataSeries3D *data)
{
  DataSelectionItem item(name, data);
  dataCollection_.push_back(item);
  notShown->addItem(name);  
}


DataConfig::DataConfig(Wt::Chart::WCartesian3DChart* chart)
{
  Wt::WTemplate *template_ = this->addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("totaldataconfig-template")));

  auto dataSelection = std::make_unique<DataSelection>(chart);
  dataselection_ = template_->bindWidget("dataselection", std::move(dataSelection));

  auto sw_(std::make_unique<Wt::WStackedWidget>());
  sw_->addWidget(std::make_unique<Wt::WText>("Select data to configure it"));
  auto numgriddatasettings(std::make_unique<NumGridDataSettings>());
  numgriddatasettings_ = numgriddatasettings.get();
  auto catgriddatasettings(std::make_unique<CatGridDataSettings>());
  catgriddatasettings_ = catgriddatasettings.get();
  auto scatterdatasettings(std::make_unique<ScatterDataSettings>());
  scatterdatasettings_ = scatterdatasettings.get();
  sw_->addWidget(std::move(numgriddatasettings));
  sw_->addWidget(std::move(catgriddatasettings));
  sw_->addWidget(std::move(scatterdatasettings));
  auto sw = template_->bindWidget("dataconfig", std::move(sw_));

  dataselection_->selectionChanged().connect([=] (Wt::Chart::WAbstractDataSeries3D* data) {
        if (dynamic_cast<Wt::Chart::WAbstractGridData*>(data)) {
          Wt::Chart::WAbstractGridData *gridData = dynamic_cast<Wt::Chart::WAbstractGridData*>(data);
          if (gridData->type() == Wt::Chart::Series3DType::Bar) {
	    catgriddatasettings_->bindDataSet(gridData);
	    sw->setCurrentIndex(2);
	  } else {
	    numgriddatasettings_->bindDataSet(gridData);
	    sw->setCurrentIndex(1);
	  }
        } else if (dynamic_cast<Wt::Chart::WScatterData*>(data)) {
          scatterdatasettings_->bindDataSet(dynamic_cast<Wt::Chart::WScatterData*>(data));
	  sw->setCurrentIndex(3);
	}
      });
}

void DataConfig::addDataToCollection(Wt::WString name, Wt::Chart::WAbstractDataSeries3D *data)
{
  dataselection_->addDataToCollection(name, data);
}
