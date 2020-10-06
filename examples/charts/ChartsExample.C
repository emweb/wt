/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <math.h>
#include <fstream>

#include "ChartsExample.h"
#include "ChartConfig.h"
#include "CsvUtil.h"

#include <Wt/WApplication.h>
#include <Wt/WDate.h>
#include <Wt/WEnvironment.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WText.h>

#include <Wt/WBorderLayout.h>
#include <Wt/WFitLayout.h>

#include <Wt/WStandardItem.h>
#include <Wt/WTableView.h>

#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WPieChart.h>

using namespace Wt;
using namespace Wt::Chart;

namespace {

  /*
   * A standard item which converts text edits to numbers
   */
  class NumericItem : public WStandardItem {
  public:
    virtual std::unique_ptr<WStandardItem> clone() const override {
      return std::make_unique<NumericItem>();
    }

    virtual void setData(const cpp17::any &data, ItemDataRole role = ItemDataRole::User) override {
      cpp17::any dt;

      if (role == ItemDataRole::Edit) {
        std::string s = asString(data).toUTF8();
	char *endptr;
	double d = strtod(s.c_str(), &endptr);
	if (*endptr == 0)
	  dt = cpp17::any(d);
	else
	  dt = data;
      }

      WStandardItem::setData(data, role);
    }
  };

  /*
   * Reads a CSV file as an (editable) standard item model.
   */
  std::shared_ptr<WAbstractItemModel> readCsvFile(const std::string &fname,
				  WContainerWidget *parent)
  {
    std::shared_ptr<WStandardItemModel> model
        = std::make_shared<WStandardItemModel>(0, 0);
    std::unique_ptr<NumericItem> prototype
        = std::make_unique<NumericItem>();
    model->setItemPrototype(std::move(prototype));
    std::ifstream f(fname.c_str());

    if (f) {
      readFromCsv(f, model.get());

      for (int row = 0; row < model->rowCount(); ++row)
          for (int col = 0; col < model->columnCount(); ++col) {
             model->item(row, col)->setFlags(ItemFlag::Selectable | ItemFlag::Editable);

	  /*
	    Example of tool tips (disabled here because they are not updated
	    when editing data)
 	   */

	  /*
	  WString toolTip = asString(model->headerData(col)) + ": "
	    + asString(model->item(row, col)->data(DisplayRole), "%.f");
	  model->item(row, col)->setToolTip(toolTip);
	   */
	}

      return model;
    } else {
      WString error(WString::tr("error-missing-data"));
      error.arg(fname, CharEncoding::UTF8);
      parent->addWidget(std::make_unique<WText>(error));
      return 0;
    }
  }
}

ChartsExample::ChartsExample()
  : WContainerWidget()
{
  this->addWidget(std::make_unique<WText>(WString::tr("introduction")));

  this->addWidget(std::make_unique<CategoryExample>());
  this->addWidget(std::make_unique<TimeSeriesExample>());
  this->addWidget(std::make_unique<ScatterPlotExample>());
  this->addWidget(std::make_unique<PieExample>());
}

CategoryExample::CategoryExample():
  WContainerWidget()
{
  this->addWidget(std::make_unique<WText>(WString::tr("category chart")));

  std::shared_ptr<WAbstractItemModel> model
    = readCsvFile(WApplication::appRoot() + "category.csv", this);

  if (!model)
    return;

  // Show a view that allows editing of the model.
  auto *w = this->addWidget(std::make_unique<WContainerWidget>());
  auto *table = w->addWidget(std::make_unique<WTableView>());

  table->setMargin(10, Side::Top | Side::Bottom);
  table->setMargin(WLength::Auto, Side::Left | Side::Right);

  table->setModel(model);
  table->setSortingEnabled(true);
  table->setColumnResizeEnabled(true);
  // table->setSelectionMode(SelectionMode::Extended);
  table->setAlternatingRowColors(true);
  table->setColumnAlignment(0, AlignmentFlag::Center);
  table->setHeaderAlignment(0, AlignmentFlag::Center);
  table->setRowHeight(22);

  // Editing does not really work without Ajax, it would require an
  // additional button somewhere to confirm the edited value.
  if (WApplication::instance()->environment().ajax()) {
    table->resize(600, 20 + 5*22);
    table->setEditTriggers(EditTrigger::SingleClicked);
  } else {
    table->resize(600, WLength::Auto);
    table->setEditTriggers(EditTrigger::None);
  }

  // We use a single delegate for all items which rounds values to
  // the closest integer value.
  std::shared_ptr<WItemDelegate> delegate
      = std::make_shared<WItemDelegate>();
  delegate->setTextFormat("%.f");
  table->setItemDelegate(delegate);

  table->setColumnWidth(0, 80);
  for (int i = 1; i < model->columnCount(); ++i)
    table->setColumnWidth(i, 120);

  /*
   * Create the category chart.
   */
  WCartesianChart *chart = this->addWidget(std::make_unique<WCartesianChart>());
  chart->setModel(model);        // set the model
  chart->setXSeriesColumn(0);    // set the column that holds the categories
  chart->setLegendEnabled(true); // enable the legend
  chart->setZoomEnabled(true);
  chart->setPanEnabled(true);

  // Automatically layout chart (space for axes, legend, ...)
  chart->setAutoLayoutEnabled(true);

  chart->setBackground(WColor(200,200,200));

  /*
   * Add all (but first) column as bar series
   */
  for (int i = 1; i < model->columnCount(); ++i) {
    std::unique_ptr<WDataSeries> s
        = std::make_unique<WDataSeries>(i, SeriesType::Bar);
    s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    chart->addSeries(std::move(s));
  }

  chart->resize(800, 400);

  chart->setMargin(10, Side::Top | Side::Bottom);
  chart->setMargin(WLength::Auto, Side::Left | Side::Right);

  /*
   * Provide a widget to manipulate chart properties
   */
  this->addWidget(std::make_unique<ChartConfig>(chart));
}

TimeSeriesExample::TimeSeriesExample():
  WContainerWidget()
{
  this->addWidget(std::make_unique<WText>(WString::tr("scatter plot")));

  std::shared_ptr<WAbstractItemModel> model
      = readCsvFile(WApplication::appRoot() + "timeseries.csv", this);

  if (!model)
    return;

  /*
   * Parses the first column as dates, to be able to use a date scale
   */
  for (int i = 0; i < model->rowCount(); ++i) {
    WString s = asString(model->data(i, 0));
    WDate d = WDate::fromString(s, "dd/MM/yy");
    model->setData(i, 0, d);
  }

  // Show a view that allows editing of the model.
  auto *w = this->addWidget(std::make_unique<WContainerWidget>());
  auto *table = w->addWidget(std::make_unique<WTableView>());

  table->setMargin(10, Side::Top | Side::Bottom);
  table->setMargin(WLength::Auto, Side::Left | Side::Right);

  table->setModel(model);
  table->setSortingEnabled(false); // Does not make much sense for time series
  table->setColumnResizeEnabled(true);
  table->setSelectionMode(SelectionMode::None);
  table->setAlternatingRowColors(true);
  table->setColumnAlignment(0, AlignmentFlag::Center);
  table->setHeaderAlignment(0, AlignmentFlag::Center);
  table->setRowHeight(22);

  // Editing does not really work without Ajax, it would require an
  // additional button somewhere to confirm the edited value.
  if (WApplication::instance()->environment().ajax()) {
    table->resize(800, 20 + 5*22);
    table->setEditTriggers(EditTrigger::SingleClicked);
  } else {
    table->resize(800, 20 + 5*22 + 25);
    table->setEditTriggers(EditTrigger::None);
  }

  std::shared_ptr<WItemDelegate> delegate
      = std::make_shared<WItemDelegate>();
  delegate->setTextFormat("%.1f");
  table->setItemDelegate(delegate);

  std::shared_ptr<WItemDelegate> delegateColumn
      = std::make_shared<WItemDelegate>();
  table->setItemDelegateForColumn(0, delegateColumn);

  table->setColumnWidth(0, 80);
  for (int i = 1; i < model->columnCount(); ++i)
    table->setColumnWidth(i, 90);

  /*
   * Create the scatter plot.
   */
  WCartesianChart *chart = this->addWidget(std::make_unique<WCartesianChart>());
  //chart->setPreferredMethod(WPaintedWidget::PngImage);
  //chart->setBackground(gray);
  chart->setModel(model);        // set the model
  chart->setXSeriesColumn(0);    // set the column that holds the X data
  chart->setLegendEnabled(true); // enable the legend
  chart->setZoomEnabled(true);
  chart->setPanEnabled(true);

  chart->setType(ChartType::Scatter);            // set type to ScatterPlot
  chart->axis(Axis::X).setScale(AxisScale::Date); // set scale of X axis to DateScale

  // Automatically layout chart (space for axes, legend, ...)
  chart->setAutoLayoutEnabled();

  chart->setBackground(WColor(200,200,200));
 /*
   * Add first two columns as line series
   */
  for (int i = 1; i < 3; ++i) {
    std::unique_ptr<WDataSeries> s
        = std::make_unique<WDataSeries>(i, SeriesType::Line);
    s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    chart->addSeries(std::move(s));
  }

  chart->resize(800, 400); // WPaintedWidget must be given explicit size

  chart->setMargin(10, Side::Top | Side::Bottom);            // add margin vertically
  chart->setMargin(WLength::Auto, Side::Left | Side::Right); // center horizontally

  this->addWidget(std::make_unique<ChartConfig>(chart));
}

ScatterPlotExample::ScatterPlotExample():
  WContainerWidget()
{
  this->addWidget(std::make_unique<WText>(WString::tr("scatter plot 2")));

  std::shared_ptr<WStandardItemModel> model
      = std::make_shared<WStandardItemModel>(40, 2);
  std::unique_ptr<NumericItem> prototype
      = std::make_unique<NumericItem>();
  model->setItemPrototype(std::move(prototype));
  model->setHeaderData(0, WString("X"));
  model->setHeaderData(1, WString("Y = sin(X)"));

  for (unsigned i = 0; i < 40; ++i) {
    double x = (static_cast<double>(i) - 20) / 4;

    model->setData(i, 0, x);
    model->setData(i, 1, sin(x));
  }
 
  /*
   * Create the scatter plot.
   */
  WCartesianChart *chart = this->addWidget(std::make_unique<WCartesianChart>());
  chart->setModel(model);        // set the model
  chart->setXSeriesColumn(0);    // set the column that holds the X data
  chart->setLegendEnabled(true); // enable the legend
  chart->setZoomEnabled(true);
  chart->setPanEnabled(true);
  chart->setCrosshairEnabled(true);

  chart->setBackground(WColor(200,200,200));

  chart->setType(ChartType::Scatter);   // set type to ScatterPlot

  // Typically, for mathematical functions, you want the axes to cross
  // at the 0 mark:
  chart->axis(Axis::X).setLocation(AxisValue::Zero);
  chart->axis(Axis::Y).setLocation(AxisValue::Zero);

  // Automatically layout chart (space for axes, legend, ...)
  chart->setAutoLayoutEnabled();

  // Add the curves
  std::unique_ptr<WDataSeries> s
      = std::make_unique<WDataSeries>(1, SeriesType::Curve);
  s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
  chart->addSeries(std::move(s));

  chart->resize(800, 300); // WPaintedWidget must be given explicit size

  chart->setMargin(10, Side::Top | Side::Bottom);            // add margin vertically
  chart->setMargin(WLength::Auto, Side::Left | Side::Right); // center horizontally

  ChartConfig *config = this->addWidget(std::make_unique<ChartConfig>(chart));
  config->setValueFill(FillRangeType::ZeroValue);
}

PieExample::PieExample():
  WContainerWidget()
{
  this->addWidget(std::make_unique<WText>(WString::tr("pie chart")));

  std::shared_ptr<WStandardItemModel> model
      = std::make_shared<WStandardItemModel>();
  std::unique_ptr<NumericItem> prototype
      = std::make_unique<NumericItem>();
  model->setItemPrototype(std::move(prototype));
  
  //headers
  model->insertColumns(model->columnCount(), 2);
  model->setHeaderData(0, WString("Item"));
  model->setHeaderData(1, WString("Sales"));

  //data
  model->insertRows(model->rowCount(), 6);
  int row = 0;
  model->setData(row, 0, WString("Blueberry"));
  model->setData(row, 1, 120);
  // model->setData(row, 1, WString("Blueberry"), ToolTipRole);
  row++;
  model->setData(row, 0, WString("Cherry"));
  model->setData(row, 1, 30);
  row++;
  model->setData(row, 0, WString("Apple"));
  model->setData(row, 1, 260);
  row++;
  model->setData(row, 0, WString("Boston Cream"));
  model->setData(row, 1, 160);
  row++;
  model->setData(row, 0, WString("Other"));
  model->setData(row, 1, 40);
  row++;
  model->setData(row, 0, WString("Vanilla Cream"));
  model->setData(row, 1, 120);
  row++;

  //set all items to be editable and selectable
  for (int row = 0; row < model->rowCount(); ++row)
    for (int col = 0; col < model->columnCount(); ++col)
      model->item(row, col)->setFlags(ItemFlag::Selectable | ItemFlag::Editable);

  WContainerWidget *w = this->addWidget(std::make_unique<WContainerWidget>());
  WTableView* table = w->addWidget(std::make_unique<WTableView>());

  table->setMargin(10, Side::Top | Side::Bottom);
  table->setMargin(WLength::Auto, Side::Left | Side::Right);
  table->setSortingEnabled(true);
  table->setModel(model);
  table->setColumnWidth(1, 100);
  table->setRowHeight(22);

  if (WApplication::instance()->environment().ajax()) {
    table->resize(150 + 100 + 14, 20 + 6 * 22);
    table->setEditTriggers(EditTrigger::SingleClicked);
  } else {
    table->resize(150 + 100 + 14, WLength::Auto);
    table->setEditTriggers(EditTrigger::None);
  }

  /*
   * Create the pie chart.
   */
  WPieChart *chart = this->addWidget(std::make_unique<WPieChart>());
  chart->setModel(model);       // set the model
  chart->setLabelsColumn(0);    // set the column that holds the labels
  chart->setDataColumn(1);      // set the column that holds the data

  // configure location and type of labels
  chart->setDisplayLabels(LabelOption::Outside | LabelOption::TextLabel | LabelOption::TextPercentage);

  // enable a 3D and shadow effect
  chart->setPerspectiveEnabled(true, 0.2);
  chart->setShadowEnabled(true);

  // explode the first item
  chart->setExplode(0, 0.3);

  chart->resize(800, 300); // WPaintedWidget must be given an explicit size

  chart->setMargin(10, Side::Top | Side::Bottom);            // add margin vertically
  chart->setMargin(WLength::Auto, Side::Left | Side::Right); // center horizontally
}

