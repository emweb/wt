/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ChartConfig.h"
#include "PanelList.h"

#include <iostream>

#include <Wt/WAbstractItemModel.h>
#include <Wt/WApplication.h>
#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WDoubleValidator.h>
#include <Wt/WDate.h>
#include <Wt/WEnvironment.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLocale.h>
#include <Wt/WPanel.h>
#include <Wt/WPushButton.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>
#include <Wt/WPainterPath.h>

#include <Wt/Chart/WCartesianChart.h>

using namespace Wt;
using namespace Wt::Chart;

namespace {
  void addHeader(WTable *t, const char *value) {
    t->elementAt(0, t->columnCount())->addWidget(cpp14::make_unique<WText>(value));
  }

  void addEntry(std::shared_ptr<WAbstractItemModel> model, const char *value) {
    model->insertRows(model->rowCount(), 1);
    model->setData(model->rowCount()-1, 0, cpp17::any(std::string(value)));
  }

  bool getDouble(WLineEdit *edit, double& value) {
    try {
      value = WLocale::currentLocale().toDouble(edit->text());
      return true;
    } catch (...) {
      return false;
    }
  }

  int seriesIndexOf(WCartesianChart* chart, int modelColumn) {
    for (unsigned i = 0; i < chart->series().size(); ++i)
      if (chart->series()[i]->modelColumn() == modelColumn)
	return i;
    
    return -1;
  }
}

ChartConfig::ChartConfig(WCartesianChart *chart)
  : WContainerWidget(),
    chart_(chart),
    fill_(FillRangeType::MinimumValue)
{
  chart_->setLegendStyle(chart_->legendFont(), WPen(WColor("black")),
			 WBrush(WColor(0xFF, 0xFA, 0xE5)));

  PanelList *list = this->addWidget(cpp14::make_unique<PanelList>());

  std::shared_ptr<WIntValidator> sizeValidator
      = std::make_shared<WIntValidator>(200,2000);
  sizeValidator->setMandatory(true);

  std::shared_ptr<WDoubleValidator> anyNumberValidator
      = std::make_shared<WDoubleValidator>();
  anyNumberValidator->setMandatory(true);

  std::shared_ptr<WDoubleValidator> angleValidator
      = std::make_shared<WDoubleValidator>(-90,90);
  angleValidator->setMandatory(true);

  // ---- Chart properties ----

  std::shared_ptr<WStandardItemModel> orientation
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(orientation, "Vertical");
  addEntry(orientation, "Horizontal");

  std::shared_ptr<WStandardItemModel> legendLocation
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(legendLocation, "Outside");
  addEntry(legendLocation, "Inside");

  std::shared_ptr<WStandardItemModel> legendSide
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(legendSide, "Top");
  addEntry(legendSide, "Right");
  addEntry(legendSide, "Bottom");
  addEntry(legendSide, "Left");

  std::shared_ptr<WStandardItemModel> legendAlignment
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(legendAlignment, "AlignLeft");
  addEntry(legendAlignment, "AlignCenter");
  addEntry(legendAlignment, "AlignRight");
  addEntry(legendAlignment, "AlignTop");
  addEntry(legendAlignment, "AlignMiddle");
  addEntry(legendAlignment, "AlignBottom");

  std::unique_ptr<WTable> chartConfig
      = cpp14::make_unique<WTable>();
  chartConfig->setMargin(WLength::Auto, Side::Left | Side::Right);

  int row = 0;
  chartConfig->elementAt(row, 0)->addWidget(cpp14::make_unique<WText>("Title:"));
  titleEdit_ = chartConfig->elementAt(row,1)->addWidget(cpp14::make_unique<WLineEdit>());
  connectSignals(titleEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(cpp14::make_unique<WText>("Width:"));
  chartWidthEdit_ = chartConfig->elementAt(row,1)->addWidget(cpp14::make_unique<WLineEdit>());
  chartWidthEdit_
    ->setText(WLocale::currentLocale().toString(chart_->width().value()));
  chartWidthEdit_->setValidator(sizeValidator);
  chartWidthEdit_->setMaxLength(4);
  connectSignals(chartWidthEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(cpp14::make_unique<WText>("Height:"));
  chartHeightEdit_ = chartConfig->elementAt(row,1)->addWidget(cpp14::make_unique<WLineEdit>());
  chartHeightEdit_
    ->setText(WLocale::currentLocale().toString(chart_->height().value()));
  chartHeightEdit_->setValidator(sizeValidator);
  chartHeightEdit_->setMaxLength(4);
  connectSignals(chartHeightEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(cpp14::make_unique<WText>("Orientation:"));
  chartOrientationEdit_ = chartConfig->elementAt(row,1)->addWidget(cpp14::make_unique<WComboBox>());
  chartOrientationEdit_->setModel(orientation);
  chartOrientationEdit_->setCurrentIndex(0);
  connectSignals(chartOrientationEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(cpp14::make_unique<WText>("Legend location:"));
  legendLocationEdit_ = chartConfig->elementAt(row,1)->addWidget(cpp14::make_unique<WComboBox>());
  legendLocationEdit_->setModel(legendLocation);
  legendLocationEdit_->setCurrentIndex(0);
  connectSignals(legendLocationEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(cpp14::make_unique<WText>("Legend side:"));
  legendSideEdit_ = chartConfig->elementAt(row,1)->addWidget(cpp14::make_unique<WComboBox>());
  legendSideEdit_->setModel(legendSide);
  legendSideEdit_->setCurrentIndex(1);
  connectSignals(legendSideEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(cpp14::make_unique<WText>("Legend alignment:"));
  legendAlignmentEdit_ = chartConfig->elementAt(row,1)->addWidget(cpp14::make_unique<WComboBox>());
  legendAlignmentEdit_->setModel(legendAlignment);
  legendAlignmentEdit_->setCurrentIndex(4);
  connectSignals(legendAlignmentEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(cpp14::make_unique<WText>("Border:"));
  borderEdit_ = chartConfig->elementAt(row,1)->addWidget(cpp14::make_unique<WCheckBox>());
  borderEdit_->setChecked(false);
  connectSignals(borderEdit_);
  ++row;

  for (int i = 0; i < chartConfig->rowCount(); ++i) {
    chartConfig->elementAt(i, 0)->setStyleClass("tdhead");
    chartConfig->elementAt(i, 1)->setStyleClass("tddata");
  }

  WPanel *p = list->addWidget("Chart properties", std::move(chartConfig));
  p->setMargin(WLength::Auto, Side::Left | Side::Right);
  p->resize(1080, WLength::Auto);
  p->setMargin(20, Side::Top | Side::Bottom);

  // ---- Series properties ----

  std::shared_ptr<WStandardItemModel> types
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(types, "Points");
  addEntry(types, "Line");
  addEntry(types, "Curve");
  addEntry(types, "Bar");
  addEntry(types, "Line Area");
  addEntry(types, "Curve Area");
  addEntry(types, "Stacked Bar");
  addEntry(types, "Stacked Line Area");
  addEntry(types, "Stacked Curve Area");

  std::shared_ptr<WStandardItemModel> markers
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(markers, "None");
  addEntry(markers, "Square");
  addEntry(markers, "Circle");
  addEntry(markers, "Cross");
  addEntry(markers, "X cross");
  addEntry(markers, "Triangle");
  addEntry(markers, "Pipe");
  addEntry(markers, "Star");
  addEntry(markers, "Inverted triangle");
  addEntry(markers, "Asterisk");
  addEntry(markers, "Diamond");

  std::shared_ptr<WStandardItemModel> axes
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(axes, "1st Y axis");
  addEntry(axes, "2nd Y axis");

  std::shared_ptr<WStandardItemModel> labels
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(labels, "None");
  addEntry(labels, "X");
  addEntry(labels, "Y");
  addEntry(labels, "X: Y");

  std::unique_ptr<WTable> seriesConfig
      = cpp14::make_unique<WTable>();
  WTable *seriesConfigPtr = seriesConfig.get();
  seriesConfig->setMargin(WLength::Auto, Side::Left | Side::Right);
  ::addHeader(seriesConfigPtr, "Name");
  ::addHeader(seriesConfigPtr, "Enabled");
  ::addHeader(seriesConfigPtr, "Type");
  ::addHeader(seriesConfigPtr, "Marker");
  ::addHeader(seriesConfigPtr, "Y axis");
  ::addHeader(seriesConfigPtr, "Legend");
  ::addHeader(seriesConfigPtr, "Shadow");
  ::addHeader(seriesConfigPtr, "Value labels");

  seriesConfig->rowAt(0)->setStyleClass("trhead");

  for (int j = 1; j < chart->model()->columnCount(); ++j) {
    SeriesControl sc;

    seriesConfig->elementAt(j,0)->addWidget(cpp14::make_unique<WText>(chart->model()->headerData(j)));

    sc.enabledEdit = seriesConfig->elementAt(j,1)->addWidget(cpp14::make_unique<WCheckBox>());
    connectSignals(sc.enabledEdit);

    sc.typeEdit = seriesConfig->elementAt(j,2)->addWidget(cpp14::make_unique<WComboBox>());
    sc.typeEdit->setModel(types);
	sc.typeEdit->setCurrentIndex(0);
    connectSignals(sc.typeEdit);

    sc.markerEdit = seriesConfig->elementAt(j,3)->addWidget(cpp14::make_unique<WComboBox>());
    sc.markerEdit->setModel(markers);
    sc.markerEdit->setCurrentIndex(0);
    connectSignals(sc.markerEdit);

    sc.axisEdit = seriesConfig->elementAt(j,4)->addWidget(cpp14::make_unique<WComboBox>());
    sc.axisEdit->setModel(axes);
	sc.axisEdit->setCurrentIndex(0);
    connectSignals(sc.axisEdit);

    sc.legendEdit = seriesConfig->elementAt(j, 5)->addWidget(cpp14::make_unique<WCheckBox>());
    connectSignals(sc.legendEdit);

    sc.shadowEdit = seriesConfig->elementAt(j,6)->addWidget(cpp14::make_unique<WCheckBox>());
    connectSignals(sc.shadowEdit);

    sc.labelsEdit = seriesConfig->elementAt(j,7)->addWidget(cpp14::make_unique<WComboBox>());
    sc.labelsEdit->setModel(labels);
	sc.labelsEdit->setCurrentIndex(0);
    connectSignals(sc.labelsEdit);

    int si = seriesIndexOf(chart, j);

    if (si != -1) {
      sc.enabledEdit->setChecked();
      const WDataSeries& s = chart_->series(j);
      switch (s.type()) {
      case SeriesType::Point:
	sc.typeEdit->setCurrentIndex(0); break;
      case SeriesType::Line:
    sc.typeEdit->setCurrentIndex(s.fillRange() != FillRangeType::None ?
				     (s.isStacked() ? 7 : 4) : 1); break;
      case SeriesType::Curve:
    sc.typeEdit->setCurrentIndex(s.fillRange() != FillRangeType::None ?
				     (s.isStacked() ? 8 : 5) : 2); break;
      case SeriesType::Bar:
	sc.typeEdit->setCurrentIndex(s.isStacked() ? 6 : 3);
      }

      sc.markerEdit->setCurrentIndex((int)s.marker());
      sc.legendEdit->setChecked(s.isLegendEnabled());
      sc.shadowEdit->setChecked(s.shadow() != WShadow());
    }

    seriesControls_.push_back(sc);

    seriesConfig->rowAt(j)->setStyleClass("trdata");
  }

  p = list->addWidget("Series properties", std::move(seriesConfig));
  p->expand();
  p->setMargin(WLength::Auto, Side::Left | Side::Right);
  p->resize(1080, WLength::Auto);
  p->setMargin(20, Side::Top | Side::Bottom);

  // ---- Axis properties ----

  std::shared_ptr<WStandardItemModel> yScales
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(yScales, "Linear scale");
  addEntry(yScales, "Log scale");

  std::shared_ptr<WStandardItemModel> xScales
      = std::make_shared<WStandardItemModel>(0,1);
  addEntry(xScales, "Categories");
  addEntry(xScales, "Linear scale");
  addEntry(xScales, "Log scale");
  addEntry(xScales, "Date scale");

  std::unique_ptr<WTable> axisConfig
      = cpp14::make_unique<WTable>();
  axisConfig->setMargin(WLength::Auto, Side::Left | Side::Right);
  WTable *axisConfigPtr = axisConfig.get();

  ::addHeader(axisConfigPtr, "Axis");
  ::addHeader(axisConfigPtr, "Visible");
  ::addHeader(axisConfigPtr, "Scale");
  ::addHeader(axisConfigPtr, "Automatic");
  ::addHeader(axisConfigPtr, "Minimum");
  ::addHeader(axisConfigPtr, "Maximum");
  ::addHeader(axisConfigPtr, "Gridlines");
  ::addHeader(axisConfigPtr, "Label angle");
  ::addHeader(axisConfigPtr, "Title");
  ::addHeader(axisConfigPtr, "Title orientation");
  ::addHeader(axisConfigPtr, "Tick direction");
  ::addHeader(axisConfigPtr, "Location");

  axisConfig->rowAt(0)->setStyleClass("trhead");

  for (int i = 0; i < 3; ++i) {
    const char *axisName[] = { "X axis", "1st Y axis", "2nd Y axis" };
    int j = i + 1;

    const WAxis& axis = chart_->axis(static_cast<Axis>(i));
    AxisControl sc;

    axisConfig->elementAt(j,0)->addWidget(cpp14::make_unique<WText>(WString(axisName[i], CharEncoding::UTF8)));

    sc.visibleEdit = axisConfig->elementAt(j,1)->addWidget(cpp14::make_unique<WCheckBox>());
    sc.visibleEdit->setChecked(axis.isVisible());
    connectSignals(sc.visibleEdit);

    sc.scaleEdit = axisConfig->elementAt(j,2)->addWidget(cpp14::make_unique<WComboBox>());
    if (axis.scale() == AxisScale::Discrete)
      sc.scaleEdit->addItem("Discrete scale");
    else {
      if (axis.id() == Axis::X) {
        sc.scaleEdit->setModel(xScales);

        sc.scaleEdit->setCurrentIndex(static_cast<int>(axis.scale()));
      } else {
        sc.scaleEdit->setModel(yScales);
        sc.scaleEdit->setCurrentIndex(static_cast<int>(axis.scale()) - 1);
      }
    }
    connectSignals(sc.scaleEdit);

    bool autoValues = axis.autoLimits() == (AxisValue::Minimum | AxisValue::Maximum);

    sc.minimumEdit = axisConfig->elementAt(j,4)->addWidget(cpp14::make_unique<WLineEdit>());
    sc.minimumEdit->setText(WLocale::currentLocale()
                            .toString(axis.minimum()));
    sc.minimumEdit->setValidator(anyNumberValidator);
    sc.minimumEdit->setEnabled(!autoValues);
    connectSignals(sc.minimumEdit);

    sc.maximumEdit = axisConfig->elementAt(j,5)->addWidget(cpp14::make_unique<WLineEdit>());
    sc.maximumEdit->setText(WLocale::currentLocale()
                            .toString(axis.maximum()));
    sc.maximumEdit->setValidator(anyNumberValidator);
    sc.maximumEdit->setEnabled(!autoValues);
    connectSignals(sc.maximumEdit);

    sc.autoEdit = axisConfig->elementAt(j,3)->addWidget(cpp14::make_unique<WCheckBox>());
    sc.autoEdit->setChecked(autoValues);
    connectSignals(sc.autoEdit);
    sc.autoEdit->checked().connect(sc.maximumEdit, &WLineEdit::disable);
    sc.autoEdit->unChecked().connect(sc.maximumEdit, &WLineEdit::enable);
    sc.autoEdit->checked().connect(sc.minimumEdit, &WLineEdit::disable);
    sc.autoEdit->unChecked().connect(sc.minimumEdit, &WLineEdit::enable);

    sc.gridLinesEdit = axisConfig->elementAt(j,6)->addWidget(cpp14::make_unique<WCheckBox>());
    connectSignals(sc.gridLinesEdit);

    sc.labelAngleEdit = axisConfig->elementAt(j,7)->addWidget(cpp14::make_unique<WLineEdit>());
    sc.labelAngleEdit->setText("0");
    sc.labelAngleEdit->setValidator(angleValidator);
    connectSignals(sc.labelAngleEdit);

    sc.titleEdit = axisConfig->elementAt(j,8)->addWidget(cpp14::make_unique<WLineEdit>());
    sc.titleEdit->setText("");
    connectSignals(sc.titleEdit);

    sc.titleOrientationEdit = axisConfig->elementAt(j,9)->addWidget(cpp14::make_unique<WComboBox>());
    sc.titleOrientationEdit->addItem("Horizontal");
    sc.titleOrientationEdit->addItem("Vertical");
	sc.titleOrientationEdit->setCurrentIndex(0);
    connectSignals(sc.titleOrientationEdit);

    sc.tickDirectionEdit = axisConfig->elementAt(j,10)->addWidget(cpp14::make_unique<WComboBox>());
    sc.tickDirectionEdit->addItem("Outwards");
    sc.tickDirectionEdit->addItem("Inwards");
	sc.tickDirectionEdit->setCurrentIndex(0);
    connectSignals(sc.tickDirectionEdit);

    sc.locationEdit = axisConfig->elementAt(j,11)->addWidget(cpp14::make_unique<WComboBox>());
    sc.locationEdit->addItem("Minimum value");
    sc.locationEdit->addItem("Maximum value");
    sc.locationEdit->addItem("Zero value");
    sc.locationEdit->addItem("Both sides");
	sc.locationEdit->setCurrentIndex(0);
    if (axis.location() == AxisValue::Zero) {
      sc.locationEdit->setCurrentIndex(2);
    }
    connectSignals(sc.locationEdit);

    axisConfig->rowAt(j)->setStyleClass("trdata");

    axisControls_.push_back(sc);
  }

  p = list->addWidget("Axis properties", std::move(axisConfig));
  p->setMargin(WLength::Auto, Side::Left | Side::Right);
  p->resize(1080, WLength::Auto);
  p->setMargin(20, Side::Top | Side::Bottom);

  /*
   * If we do not have JavaScript, then add a button to reflect changes to
   * the chart.
   */
  if (!WApplication::instance()->environment().javaScript()) {
    auto *b = this->addWidget(cpp14::make_unique<WPushButton>());
    b->setText("Update chart");
    b->setInline(false); // so we can add margin to center horizontally
    b->setMargin(WLength::Auto, Side::Left | Side::Right);
    b->clicked().connect(this, &ChartConfig::update);
  }
}

void ChartConfig::setValueFill(FillRangeType fill)
{
  fill_ = fill;
}

void ChartConfig::update()
{
  bool haveLegend = false;
  std::vector<std::unique_ptr<WDataSeries>> series;

  for (int i = 1; i < chart_->model()->columnCount(); ++i) {
    SeriesControl& sc = seriesControls_[i-1];

    if (sc.enabledEdit->isChecked()) {
      std::unique_ptr<WDataSeries> s
          = cpp14::make_unique<WDataSeries>(i);

      switch (sc.typeEdit->currentIndex()) {
      case 0:
    s->setType(SeriesType::Point);
    if (sc.markerEdit->currentIndex() == 0)
      sc.markerEdit->setCurrentIndex(1);
    break;
      case 1:
    s->setType(SeriesType::Line);
    break;
      case 2:
    s->setType(SeriesType::Curve);
    break;
      case 3:
    s->setType(SeriesType::Bar);
    break;
      case 4:
    s->setType(SeriesType::Line);
    s->setFillRange(fill_);
    break;
      case 5:
    s->setType(SeriesType::Curve);
    s->setFillRange(fill_);
    break;
      case 6:
    s->setType(SeriesType::Bar);
    s->setStacked(true);
    break;
      case 7:
    s->setType(SeriesType::Line);
    s->setFillRange(fill_);
    s->setStacked(true);
    break;
      case 8:
    s->setType(SeriesType::Curve);
    s->setFillRange(fill_);
    s->setStacked(true);
      }

      //set WPainterPath to draw a pipe
      if(sc.markerEdit->currentIndex() == static_cast<int>(MarkerType::Custom)){ //was customMarker before
        WPainterPath pp = WPainterPath();
        pp.moveTo(0, -6);
        pp.lineTo(0, 6);
        s->setCustomMarker(pp);
      }

      s->setMarker(static_cast<MarkerType>(sc.markerEdit->currentIndex()));

      if (sc.axisEdit->currentIndex() == 1) {
        s->bindToAxis(Axis::Y2);
      }

      if (sc.legendEdit->isChecked()) {
        s->setLegendEnabled(true);
        haveLegend = true;
      } else
        s->setLegendEnabled(false);

      if (sc.shadowEdit->isChecked()) {
        s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
      } else
        s->setShadow(WShadow());

      switch (sc.labelsEdit->currentIndex()) {
      case 1:
    s->setLabelsEnabled(Axis::X);
    break;
      case 2:
    s->setLabelsEnabled(Axis::Y);
    break;
      case 3:
    s->setLabelsEnabled(Axis::X);
    s->setLabelsEnabled(Axis::Y);
    break;
      }

      series.push_back(std::move(s));
    }
  }

  chart_->setSeries(std::move(series));

  for (int i = 0; i < 3; ++i) {
    AxisControl& sc = axisControls_[i];
    WAxis& axis = chart_->axis(static_cast<Axis>(i));

    axis.setVisible(sc.visibleEdit->isChecked());

    if (sc.scaleEdit->count() != 1) {
      int k = sc.scaleEdit->currentIndex();
      if (axis.id() != Axis::X)
        k += 1;
      else {
        if (k == 0)
            chart_->setType(ChartType::Category);
        else
            chart_->setType(ChartType::Scatter);
      }

      switch (k) {
      case 1:
    axis.setScale(AxisScale::Linear); break;
      case 2:
    axis.setScale(AxisScale::Log); break;
      case 3:
    axis.setScale(AxisScale::Date); break;
      }
    }

    if (sc.autoEdit->isChecked())
      axis.setAutoLimits(AxisValue::Minimum | AxisValue::Maximum);
    else {
      if (!(axis.autoLimits() & (AxisValue::Minimum | AxisValue::Maximum)).empty()) {
	sc.minimumEdit->setText(WLocale::currentLocale()
				.toString(axis.minimum()));
	sc.maximumEdit->setText(WLocale::currentLocale()
				.toString(axis.maximum()));
      }
      if (validate(sc.minimumEdit) && validate(sc.maximumEdit)) {
          double min, max;
          getDouble(sc.minimumEdit, min);
          getDouble(sc.maximumEdit, max);

          if (axis.scale() == AxisScale::Log)
              if (min <= 0)
                  min = 0.0001;

          if (axis.scale() == AxisScale::Date){
              //the number of julian days until year 1986
              WDate dMin = WDate(1900,1,1);
              double gregDaysMin = (double)dMin.toJulianDay();
              //the number of julian days until year 1988
              WDate dMax = WDate(3000,1,1);
              double gregDaysMax = (double)dMax.toJulianDay();

              bool greg_year_validation =
                      (min > gregDaysMin &&
                       min < gregDaysMax &&
                       max > gregDaysMin &&
                       max < gregDaysMax);

              if(!greg_year_validation){
                  min = gregDaysMin;
                  max = gregDaysMax;
              }
          }

          axis.setRange(min, max);
      }

    }

    if (validate(sc.labelAngleEdit)) {
      double angle;
      getDouble(sc.labelAngleEdit, angle);
      axis.setLabelAngle(angle);
    }

    axis.setGridLinesEnabled(sc.gridLinesEdit->isChecked());

    axis.setTitle(sc.titleEdit->text());

    axis.setTitleOrientation(sc.titleOrientationEdit->currentIndex() == 0 ? Orientation::Horizontal : Orientation::Vertical);

    axis.setTickDirection(sc.tickDirectionEdit->currentIndex() == 0 ? TickDirection::Outwards : TickDirection::Inwards);

    switch (sc.locationEdit->currentIndex()) {
      case 0:
    axis.setLocation(AxisValue::Minimum);
	break;
      case 1:
    axis.setLocation(AxisValue::Maximum);
	break;
      case 2:
    axis.setLocation(AxisValue::Zero);
	break;
      case 3:
    axis.setLocation(AxisValue::Both);
	break;
    }
  }

  chart_->setTitle(titleEdit_->text());

  if (validate(chartWidthEdit_) && validate(chartHeightEdit_)) {
    double width, height;
    getDouble(chartWidthEdit_, width);
    getDouble(chartHeightEdit_, height);
    chart_->resize(width, height);
  }

  switch (chartOrientationEdit_->currentIndex()) {
  case 0:
    chart_->setOrientation(Orientation::Vertical); break;
  case 1:
    chart_->setOrientation(Orientation::Horizontal); break;
  }

  chart_->setLegendEnabled(haveLegend);

  if (haveLegend) {
    LegendLocation location = LegendLocation::Outside;
    Side side = Side::Right;
    AlignmentFlag alignment = AlignmentFlag::Middle;
    switch (legendLocationEdit_->currentIndex()) {
    case 0: location = LegendLocation::Outside; break;
    case 1: location = LegendLocation::Inside; break;
    }

    switch (legendSideEdit_->currentIndex()) {
    case 0: side = Side::Top; break;
    case 1: side = Side::Right; break;
    case 2: side = Side::Bottom; break;
    case 3: side = Side::Left; break;
    }

    if (side == Side::Left || side == Side::Right) {
      if (legendAlignmentEdit_->currentIndex() < 3)
	legendAlignmentEdit_->setCurrentIndex(4);
    } else {
      if (legendAlignmentEdit_->currentIndex() >= 3)
	legendAlignmentEdit_->setCurrentIndex(2);
    }

    switch (legendAlignmentEdit_->currentIndex()) {
    case 0: alignment = AlignmentFlag::Left; break;
    case 1: alignment = AlignmentFlag::Center; break;
    case 2: alignment = AlignmentFlag::Right; break;
    case 3: alignment = AlignmentFlag::Top; break;
    case 4: alignment = AlignmentFlag::Middle; break;
    case 5: alignment = AlignmentFlag::Bottom; break;
    }

    chart_->setLegendLocation(location, side, alignment);

    chart_->setLegendColumns((side == Side::Top || side == Side::Bottom ) ? 2 : 1,
			     WLength(100));
  }

  if (borderEdit_->isChecked()) {
    chart_->setBorderPen(WPen());
  } else {
    chart_->setBorderPen(PenStyle::None);
  }
}

bool ChartConfig::validate(WFormWidget *w)
{
  bool valid = w->validate() == ValidationState::Valid;

  if (!WApplication::instance()->environment().javaScript()) {
    w->setStyleClass(valid ? "" : "Wt-invalid");
    w->setToolTip(valid ? "" : "Invalid value");
  }

  return valid;
}

void ChartConfig::connectSignals(WFormWidget *w)
{
  w->changed().connect(this, &ChartConfig::update);
  if (dynamic_cast<WLineEdit *>(w))
    w->enterPressed().connect(this, &ChartConfig::update);
}
