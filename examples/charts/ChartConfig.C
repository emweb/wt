/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ChartConfig.h"
#include "PanelList.h"

#include <iostream>
#include <boost/date_time/gregorian/greg_year.hpp>

#include <Wt/WAbstractItemModel>
#include <Wt/WApplication>
#include <Wt/WCheckBox>
#include <Wt/WComboBox>
#include <Wt/WDoubleValidator>
#include <Wt/WDate>
#include <Wt/WEnvironment>
#include <Wt/WIntValidator>
#include <Wt/WLineEdit>
#include <Wt/WLocale>
#include <Wt/WPanel>
#include <Wt/WPushButton>
#include <Wt/WStandardItemModel>
#include <Wt/WTable>
#include <Wt/WText>
#include <Wt/WPainterPath>

#include <Wt/Chart/WCartesianChart>

using namespace Wt;
using namespace Wt::Chart;

namespace {
  void addHeader(WTable *t, const char *value) {
    t->elementAt(0, t->columnCount())->addWidget(new WText(value));
  }

  void addEntry(WAbstractItemModel *model, const char *value) {
    model->insertRows(model->rowCount(), 1);
    model->setData(model->rowCount()-1, 0, boost::any(std::string(value)));
  }

  void addEntry(WAbstractItemModel *model, const WString &value) {
    model->insertRows(model->rowCount(), 1);
    model->setData(model->rowCount()-1, 0, boost::any(value));
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

  WString axisName(Axis axis, int yAxis)
  {
    if (axis == XAxis)
      return Wt::utf8("X Axis");
    else {
      return Wt::utf8("Y axis {1}").arg(yAxis + 1);
    }
  }
}

ChartConfig::ChartConfig(WCartesianChart *chart, WContainerWidget *parent)
  : WContainerWidget(parent),
    chart_(chart),
    fill_(MinimumValueFill)
{
  chart_->setLegendStyle(chart_->legendFont(), WPen(black),
			 WBrush(WColor(0xFF, 0xFA, 0xE5)));

  PanelList *list = new PanelList(this);

  WIntValidator *sizeValidator = new WIntValidator(200, 2000, this);
  sizeValidator->setMandatory(true);

  anyNumberValidator_ = new WDoubleValidator(this);
  anyNumberValidator_->setMandatory(true);

  angleValidator_ = new WDoubleValidator(-90, 90, this);
  angleValidator_->setMandatory(true);

  // ---- Chart properties ----

  WStandardItemModel *orientation = new WStandardItemModel(0, 1, this);
  addEntry(orientation, "Vertical");
  addEntry(orientation, "Horizontal");

  WStandardItemModel *legendLocation = new WStandardItemModel(0, 1, this);
  addEntry(legendLocation, "Outside");
  addEntry(legendLocation, "Inside");

  WStandardItemModel *legendSide = new WStandardItemModel(0, 1, this);
  addEntry(legendSide, "Top");
  addEntry(legendSide, "Right");
  addEntry(legendSide, "Bottom");
  addEntry(legendSide, "Left");

  WStandardItemModel *legendAlignment = new WStandardItemModel(0, 1, this);
  addEntry(legendAlignment, "AlignLeft");
  addEntry(legendAlignment, "AlignCenter");
  addEntry(legendAlignment, "AlignRight");
  addEntry(legendAlignment, "AlignTop");
  addEntry(legendAlignment, "AlignMiddle");
  addEntry(legendAlignment, "AlignBottom");

  WTable *chartConfig = new WTable();
  chartConfig->setMargin(WLength::Auto, Left | Right);

  int row = 0;
  chartConfig->elementAt(row, 0)->addWidget(new WText("Title:"));
  titleEdit_ = new WLineEdit(chartConfig->elementAt(row, 1));
  connectSignals(titleEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(new WText("Width:"));
  chartWidthEdit_ = new WLineEdit(chartConfig->elementAt(row, 1));
  chartWidthEdit_
    ->setText(WLocale::currentLocale().toString(chart_->width().value()));
  chartWidthEdit_->setValidator(sizeValidator);
  chartWidthEdit_->setMaxLength(4);
  connectSignals(chartWidthEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(new WText("Height:"));
  chartHeightEdit_ = new WLineEdit(chartConfig->elementAt(row, 1));
  chartHeightEdit_
    ->setText(WLocale::currentLocale().toString(chart_->height().value()));
  chartHeightEdit_->setValidator(sizeValidator);
  chartHeightEdit_->setMaxLength(4);
  connectSignals(chartHeightEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(new WText("Orientation:"));
  chartOrientationEdit_ = new WComboBox(chartConfig->elementAt(row, 1));
  chartOrientationEdit_->setModel(orientation);
  chartOrientationEdit_->setCurrentIndex(0);
  connectSignals(chartOrientationEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(new WText("Legend location:"));
  legendLocationEdit_ = new WComboBox(chartConfig->elementAt(row, 1));
  legendLocationEdit_->setModel(legendLocation);
  legendLocationEdit_->setCurrentIndex(0);
  connectSignals(legendLocationEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(new WText("Legend side:"));
  legendSideEdit_ = new WComboBox(chartConfig->elementAt(row, 1));
  legendSideEdit_->setModel(legendSide);
  legendSideEdit_->setCurrentIndex(1);
  connectSignals(legendSideEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(new WText("Legend alignment:"));
  legendAlignmentEdit_ = new WComboBox(chartConfig->elementAt(row, 1));
  legendAlignmentEdit_->setModel(legendAlignment);
  legendAlignmentEdit_->setCurrentIndex(4);
  connectSignals(legendAlignmentEdit_);
  ++row;

  chartConfig->elementAt(row, 0)->addWidget(new WText("Border:"));
  borderEdit_ = new WCheckBox(chartConfig->elementAt(row, 1));
  borderEdit_->setChecked(false);
  connectSignals(borderEdit_);
  ++row;

  for (int i = 0; i < chartConfig->rowCount(); ++i) {
    chartConfig->elementAt(i, 0)->setStyleClass("tdhead");
    chartConfig->elementAt(i, 1)->setStyleClass("tddata");
  }

  WPanel *p = list->addWidget("Chart properties", chartConfig);
  p->setMargin(WLength::Auto, Left | Right);
  p->resize(1160, WLength::Auto);
  p->setMargin(20, Top | Bottom);

  // ---- Series properties ----

  WStandardItemModel *types = new WStandardItemModel(0, 1, this);
  addEntry(types, "Points");
  addEntry(types, "Line");
  addEntry(types, "Curve");
  addEntry(types, "Bar");
  addEntry(types, "Line Area");
  addEntry(types, "Curve Area");
  addEntry(types, "Stacked Bar");
  addEntry(types, "Stacked Line Area");
  addEntry(types, "Stacked Curve Area");

  WStandardItemModel *markers = new WStandardItemModel(0, 1, this);
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

  yAxesModel_ = new WStandardItemModel(0, 1, this);
  addEntry(yAxesModel_, axisName(YAxis, 0));
  addEntry(yAxesModel_, axisName(YAxis, 1));

  WStandardItemModel *labels = new WStandardItemModel(0, 1, this);
  addEntry(labels, "None");
  addEntry(labels, "X");
  addEntry(labels, "Y");
  addEntry(labels, "X: Y");

  WTable *seriesConfig = new WTable();
  seriesConfig->setMargin(WLength::Auto, Left | Right);

  ::addHeader(seriesConfig, "Name");
  ::addHeader(seriesConfig, "Enabled");
  ::addHeader(seriesConfig, "Type");
  ::addHeader(seriesConfig, "Marker");
  ::addHeader(seriesConfig, "Y axis");
  ::addHeader(seriesConfig, "Legend");
  ::addHeader(seriesConfig, "Shadow");
  ::addHeader(seriesConfig, "Value labels");

  seriesConfig->rowAt(0)->setStyleClass("trhead");

  for (int j = 1; j < chart->model()->columnCount(); ++j) {
    SeriesControl sc;

    new WText(chart->model()->headerData(j),
	      seriesConfig->elementAt(j, 0));

    sc.enabledEdit = new WCheckBox(seriesConfig->elementAt(j, 1));
    connectSignals(sc.enabledEdit);

    sc.typeEdit = new WComboBox(seriesConfig->elementAt(j, 2));
    sc.typeEdit->setModel(types);
	sc.typeEdit->setCurrentIndex(0);
    connectSignals(sc.typeEdit);

    sc.markerEdit = new WComboBox(seriesConfig->elementAt(j, 3));
    sc.markerEdit->setModel(markers);
    sc.markerEdit->setCurrentIndex(0);
    connectSignals(sc.markerEdit);

    sc.axisEdit = new WComboBox(seriesConfig->elementAt(j, 4));
    sc.axisEdit->setModel(yAxesModel_);
    sc.axisEdit->setCurrentIndex(0);
    connectSignals(sc.axisEdit);

    sc.legendEdit = new WCheckBox(seriesConfig->elementAt(j, 5));
    connectSignals(sc.legendEdit);

    sc.shadowEdit = new WCheckBox(seriesConfig->elementAt(j, 6));
    connectSignals(sc.shadowEdit);

    sc.labelsEdit = new WComboBox(seriesConfig->elementAt(j, 7));
    sc.labelsEdit->setModel(labels);
	sc.labelsEdit->setCurrentIndex(0);
    connectSignals(sc.labelsEdit);

    int si = seriesIndexOf(chart, j);

    if (si != -1) {
      sc.enabledEdit->setChecked();
      const WDataSeries& s = chart_->series(j);
      switch (s.type()) {
      case PointSeries:
	sc.typeEdit->setCurrentIndex(0); break;
      case LineSeries:
	sc.typeEdit->setCurrentIndex(s.fillRange() != NoFill ?
				     (s.isStacked() ? 7 : 4) : 1); break;
      case CurveSeries:
	sc.typeEdit->setCurrentIndex(s.fillRange() != NoFill ?
				     (s.isStacked() ? 8 : 5) : 2); break;
      case BarSeries:
	sc.typeEdit->setCurrentIndex(s.isStacked() ? 6 : 3);
      }

      sc.markerEdit->setCurrentIndex((int)s.marker());
      sc.legendEdit->setChecked(s.isLegendEnabled());
      sc.shadowEdit->setChecked(s.shadow() != WShadow());
    }

    seriesControls_.push_back(sc);

    seriesConfig->rowAt(j)->setStyleClass("trdata");
  }

  p = list->addWidget("Series properties", seriesConfig);
  p->expand();
  p->setMargin(WLength::Auto, Left | Right);
  p->resize(1160, WLength::Auto);
  p->setMargin(20, Top | Bottom);

  // ---- Axis properties ----

  yScales_ = new WStandardItemModel(0, 1, this);
  addEntry(yScales_, "Linear scale");
  addEntry(yScales_, "Log scale");

  xScales_ = new WStandardItemModel(0, 1, this);
  addEntry(xScales_, "Categories");
  addEntry(xScales_, "Linear scale");
  addEntry(xScales_, "Log scale");
  addEntry(xScales_, "Date scale");

  WContainerWidget *axisConfig = new WContainerWidget();
  axisConfig_ = new WTable(axisConfig);
  axisConfig_->setMargin(WLength::Auto, Left | Right);

  ::addHeader(axisConfig_, "Axis");
  ::addHeader(axisConfig_, "Visible");
  ::addHeader(axisConfig_, "Scale");
  ::addHeader(axisConfig_, "Automatic");
  ::addHeader(axisConfig_, "Minimum");
  ::addHeader(axisConfig_, "Maximum");
  ::addHeader(axisConfig_, "Gridlines");
  ::addHeader(axisConfig_, "Label angle");
  ::addHeader(axisConfig_, "Title");
  ::addHeader(axisConfig_, "Title orientation");
  ::addHeader(axisConfig_, "Tick direction");
  ::addHeader(axisConfig_, "Location");

  axisConfig_->rowAt(0)->setStyleClass("trhead");

  addAxis(XAxis, 0);
  addAxis(YAxis, 0);
  addAxis(YAxis, 1);

  WPushButton *addAxisBtn =
      new WPushButton(Wt::utf8("Add Y axis"), axisConfig);
  addAxisBtn->clicked().connect(this, &ChartConfig::addYAxis);
  WPushButton *clearAxesBtn =
      new WPushButton(Wt::utf8("Clear Y axes"), axisConfig);
  clearAxesBtn->clicked().connect(this, &ChartConfig::clearYAxes);

  p = list->addWidget("Axis properties", axisConfig);
  p->setMargin(WLength::Auto, Left | Right);
  p->resize(1160, WLength::Auto);
  p->setMargin(20, Top | Bottom);

  /*
   * If we do not have JavaScript, then add a button to reflect changes to
   * the chart.
   */
  if (!WApplication::instance()->environment().javaScript()) {
    WPushButton *b = new WPushButton(this);
    b->setText("Update chart");
    b->setInline(false); // so we can add margin to center horizontally
    b->setMargin(WLength::Auto, Left | Right);
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
  std::vector<WDataSeries *> series;

  for (int i = 1; i < chart_->model()->columnCount(); ++i) {
    SeriesControl& sc = seriesControls_[i-1];

    if (sc.enabledEdit->isChecked()) {
      WDataSeries *s = new WDataSeries(i);

      switch (sc.typeEdit->currentIndex()) {
      case 0:
	s->setType(PointSeries);
	if (sc.markerEdit->currentIndex() == 0)
	  sc.markerEdit->setCurrentIndex(1);
	break;
      case 1:
	s->setType(LineSeries);
	break;
      case 2:
	s->setType(CurveSeries);
	break;
      case 3:
	s->setType(BarSeries);
	break;
      case 4:
	s->setType(LineSeries);
	s->setFillRange(fill_);
	break;
      case 5:
	s->setType(CurveSeries);
	s->setFillRange(fill_);
	break;
      case 6:
	s->setType(BarSeries);
	s->setStacked(true);
	break;
      case 7:
	s->setType(LineSeries);
	s->setFillRange(fill_);
	s->setStacked(true);
	break;
      case 8:
	s->setType(CurveSeries);
	s->setFillRange(fill_);
	s->setStacked(true);
      }

      //set WPainterPath to draw a pipe
      if(sc.markerEdit->currentIndex() == CustomMarker){
	WPainterPath pp = WPainterPath();
	pp.moveTo(0, -6);
	pp.lineTo(0, 6);
	s->setCustomMarker(pp);
      }

      s->setMarker(static_cast<MarkerType>(sc.markerEdit->currentIndex()));

      s->bindToYAxis(sc.axisEdit->currentIndex());

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
	s->setLabelsEnabled(XAxis);
	break;
      case 2:
	s->setLabelsEnabled(YAxis);
	break;
      case 3:
	s->setLabelsEnabled(XAxis);
	s->setLabelsEnabled(YAxis);
	break;
      }

      series.push_back(s);
    }
  }

  chart_->setSeries(series);

  for (std::size_t i = 0; i < axisControls_.size(); ++i) {
    AxisControl& sc = axisControls_[i];
    WAxis& axis = i == 0 ? chart_->axis(XAxis) : chart_->yAxis(i - 1);

    axis.setVisible(sc.visibleEdit->isChecked());

    if (sc.scaleEdit->count() != 1) {
      int k = sc.scaleEdit->currentIndex();
      if (axis.id() != XAxis)
	k += 1;
      else {
	if (k == 0)
	  chart_->setType(CategoryChart);
	else
	  chart_->setType(ScatterPlot);
      }

      switch (k) {
      case 1:
	axis.setScale(LinearScale); break;
      case 2:
	axis.setScale(LogScale); break;
      case 3:
	axis.setScale(DateScale); break;
      }
    }

    if (sc.autoEdit->isChecked())
      axis.setAutoLimits(MinimumValue | MaximumValue);
    else {
      if (axis.autoLimits() & (MinimumValue | MaximumValue)) {
	sc.minimumEdit->setText(WLocale::currentLocale()
				.toString(axis.minimum()));
	sc.maximumEdit->setText(WLocale::currentLocale()
				.toString(axis.maximum()));
      }
      if (validate(sc.minimumEdit) && validate(sc.maximumEdit)) {
          double min, max;
          getDouble(sc.minimumEdit, min);
          getDouble(sc.maximumEdit, max);

          if (axis.scale() == LogScale)
              if (min <= 0)
                  min = 0.0001;

          if (axis.scale() == DateScale){
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

    axis.setTitleOrientation(sc.titleOrientationEdit->currentIndex() == 0 ? Horizontal : Vertical);

    axis.setTickDirection(sc.tickDirectionEdit->currentIndex() == 0 ? Outwards : Inwards);

    switch (sc.locationEdit->currentIndex()) {
      case 0:
	axis.setLocation(MinimumValue);
	break;
      case 1:
	axis.setLocation(MaximumValue);
	break;
      case 2:
	axis.setLocation(ZeroValue);
	break;
      case 3:
	axis.setLocation(BothSides);
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
    chart_->setOrientation(Vertical); break;
  case 1:
    chart_->setOrientation(Horizontal); break;
  }

  chart_->setLegendEnabled(haveLegend);

  if (haveLegend) {
    LegendLocation location = LegendOutside;
    Side side = Right;
    AlignmentFlag alignment = AlignMiddle;
    switch (legendLocationEdit_->currentIndex()) {
    case 0: location = LegendOutside; break;
    case 1: location = LegendInside; break;
    }

    switch (legendSideEdit_->currentIndex()) {
    case 0: side = Top; break;
    case 1: side = Right; break;
    case 2: side = Bottom; break;
    case 3: side = Left; break;
    }

    if (side == Left || side == Right) {
      if (legendAlignmentEdit_->currentIndex() < 3)
	legendAlignmentEdit_->setCurrentIndex(4);
    } else {
      if (legendAlignmentEdit_->currentIndex() >= 3)
	legendAlignmentEdit_->setCurrentIndex(2);
    }

    switch (legendAlignmentEdit_->currentIndex()) {
    case 0: alignment = AlignLeft; break;
    case 1: alignment = AlignCenter; break;
    case 2: alignment = AlignRight; break;
    case 3: alignment = AlignTop; break;
    case 4: alignment = AlignMiddle; break;
    case 5: alignment = AlignBottom; break;
    }

    chart_->setLegendLocation(location, side, alignment);

    chart_->setLegendColumns((side == Top || side == Bottom ) ? 2 : 1,
			     WLength(100));
  }

  if (borderEdit_->isChecked()) {
    chart_->setBorderPen(WPen());
  } else {
    chart_->setBorderPen(NoPen);
  }
}

bool ChartConfig::validate(WFormWidget *w)
{
  bool valid = w->validate() == WValidator::Valid;

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

void ChartConfig::addYAxis()
{
  int yAxis = chart_->addYAxis(new WAxis());
  addAxis(YAxis, yAxis);
  addEntry(yAxesModel_, axisName(YAxis, yAxis));
  if (yAxis == 0)
    update();
}

void ChartConfig::addAxis(Axis ax, int yAxis)
{
  int j = ax == XAxis ? 1 : yAxis + 2;

  const WAxis& axis = ax == XAxis ? chart_->axis(XAxis) : chart_->yAxis(yAxis);
  AxisControl sc;

  new WText(axisName(axis.id(), axis.yAxisId()), axisConfig_->elementAt(j, 0));

  sc.visibleEdit = new WCheckBox(axisConfig_->elementAt(j, 1));
  sc.visibleEdit->setChecked(axis.isVisible());
  connectSignals(sc.visibleEdit);

  sc.scaleEdit = new WComboBox(axisConfig_->elementAt(j, 2));
  if (axis.scale() == CategoryScale)
    sc.scaleEdit->addItem("Category scale");
  else {
    if (axis.id() == XAxis) {
      sc.scaleEdit->setModel(xScales_);
      sc.scaleEdit->setCurrentIndex(axis.scale());
    } else {
      sc.scaleEdit->setModel(yScales_);
      sc.scaleEdit->setCurrentIndex(axis.scale() - 1);
    }
  }
  connectSignals(sc.scaleEdit);

  bool autoValues = axis.autoLimits() == (MinimumValue | MaximumValue);

  sc.minimumEdit = new WLineEdit(axisConfig_->elementAt(j, 4));
  sc.minimumEdit->setText(WLocale::currentLocale()
                          .toString(axis.minimum()));
  sc.minimumEdit->setValidator(anyNumberValidator_);
  sc.minimumEdit->setEnabled(!autoValues);
  connectSignals(sc.minimumEdit);

  sc.maximumEdit = new WLineEdit(axisConfig_->elementAt(j, 5));
  sc.maximumEdit->setText(WLocale::currentLocale()
                          .toString(axis.maximum()));
  sc.maximumEdit->setValidator(anyNumberValidator_);
  sc.maximumEdit->setEnabled(!autoValues);
  connectSignals(sc.maximumEdit);

  sc.autoEdit = new WCheckBox(axisConfig_->elementAt(j, 3));
  sc.autoEdit->setChecked(autoValues);
  connectSignals(sc.autoEdit);
  sc.autoEdit->checked().connect(sc.maximumEdit, &WLineEdit::disable);
  sc.autoEdit->unChecked().connect(sc.maximumEdit, &WLineEdit::enable);
  sc.autoEdit->checked().connect(sc.minimumEdit, &WLineEdit::disable);
  sc.autoEdit->unChecked().connect(sc.minimumEdit, &WLineEdit::enable);

  sc.gridLinesEdit = new WCheckBox(axisConfig_->elementAt(j, 6));
  connectSignals(sc.gridLinesEdit);

  sc.labelAngleEdit = new WLineEdit(axisConfig_->elementAt(j, 7));
  sc.labelAngleEdit->setText("0");
  sc.labelAngleEdit->setValidator(angleValidator_);
  connectSignals(sc.labelAngleEdit);

  sc.titleEdit = new WLineEdit(axisConfig_->elementAt(j, 8));
  sc.titleEdit->setText("");
  connectSignals(sc.titleEdit);

  sc.titleOrientationEdit = new WComboBox(axisConfig_->elementAt(j, 9));
  sc.titleOrientationEdit->addItem("Horizontal");
  sc.titleOrientationEdit->addItem("Vertical");
      sc.titleOrientationEdit->setCurrentIndex(0);
  connectSignals(sc.titleOrientationEdit);

  sc.tickDirectionEdit = new WComboBox(axisConfig_->elementAt(j, 10));
  sc.tickDirectionEdit->addItem("Outwards");
  sc.tickDirectionEdit->addItem("Inwards");
      sc.tickDirectionEdit->setCurrentIndex(0);
  connectSignals(sc.tickDirectionEdit);

  sc.locationEdit = new WComboBox(axisConfig_->elementAt(j, 11));
  sc.locationEdit->addItem("Minimum value");
  sc.locationEdit->addItem("Maximum value");
  sc.locationEdit->addItem("Zero value");
  sc.locationEdit->addItem("Both sides");
  sc.locationEdit->setCurrentIndex(0);
  if (axis.location() == MaximumValue) {
    sc.locationEdit->setCurrentIndex(1);
  } else if (axis.location() == ZeroValue) {
    sc.locationEdit->setCurrentIndex(2);
  }
  connectSignals(sc.locationEdit);

  if (ax != XAxis) {
    WPushButton *removeAxisButton =
        new WPushButton(Wt::utf8("x"), axisConfig_->elementAt(j, 12));
    removeAxisButton->clicked().connect(boost::bind(&ChartConfig::removeYAxis, this, &axis));
  }

  axisConfig_->rowAt(j)->setStyleClass("trdata");

  axisControls_.push_back(sc);
}

void ChartConfig::removeYAxis(const WAxis *axis)
{
  int yAxis = axis->yAxisId();
  for (std::size_t i = 0; i < chart_->series().size(); ++i) {
    if (chart_->series()[i]->yAxis() == yAxis)
      chart_->series()[i]->bindToYAxis(-1);
  }
  chart_->removeYAxis(yAxis);
  axisConfig_->deleteRow(yAxis + 2);
  yAxesModel_->removeRow(yAxis);
  axisControls_.erase(axisControls_.begin() + yAxis + 1);
  update();
}

void ChartConfig::clearYAxes()
{
  if (chart_->yAxisCount() == 0)
    return;

  for (std::size_t i = 0; i < chart_->series().size(); ++i) {
    chart_->series()[i]->bindToYAxis(-1);
  }
  chart_->clearYAxes();
  while (axisConfig_->rowCount() > 2)
    axisConfig_->deleteRow(2);
  yAxesModel_->clear();
  axisControls_.resize(1);
}
