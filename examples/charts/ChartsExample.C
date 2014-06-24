/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <math.h>
#include <fstream>

#include "ChartsExample.h"
#include "ChartConfig.h"
#include "CsvUtil.h"

#include <Wt/WApplication>
#include <Wt/WDate>
#include <Wt/WEnvironment>
#include <Wt/WItemDelegate>
#include <Wt/WStandardItemModel>
#include <Wt/WText>

#include <Wt/WBorderLayout>
#include <Wt/WFitLayout>

#include <Wt/WStandardItem>
#include <Wt/WTableView>

#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WPieChart>

using namespace Wt;
using namespace Wt::Chart;

namespace {

  /*
   * A standard item which converts text edits to numbers
   */
  class NumericItem : public WStandardItem {
  public:
    virtual NumericItem *clone() const {
      return new NumericItem();
    }

    virtual void setData(const boost::any &data, int role = UserRole) {
      boost::any dt;

      if (role == EditRole) {
	std::string s = Wt::asString(data).toUTF8();
	char *endptr;
	double d = strtod(s.c_str(), &endptr);
	if (*endptr == 0)
	  dt = boost::any(d);
	else
	  dt = data;
      }

      WStandardItem::setData(data, role);
    }
  };

  /*
   * Reads a CSV file as an (editable) standard item model.
   */
  WAbstractItemModel *readCsvFile(const std::string &fname,
				  WContainerWidget *parent)
  {
    WStandardItemModel *model = new WStandardItemModel(0, 0, parent);
    model->setItemPrototype(new NumericItem());
    std::ifstream f(fname.c_str());

    if (f) {
      readFromCsv(f, model);

      for (int row = 0; row < model->rowCount(); ++row)
	for (int col = 0; col < model->columnCount(); ++col) {
	  model->item(row, col)->setFlags(ItemIsSelectable | ItemIsEditable);

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
      error.arg(fname, UTF8);
      new WText(error, parent);
      return 0;
    }
  }
}

ChartsExample::ChartsExample(WContainerWidget *root)
  : WContainerWidget(root)
{
  new WText(WString::tr("introduction"), this);

  new CategoryExample(this);
  new TimeSeriesExample(this);
  new ScatterPlotExample(this);
  new PieExample(this);
}

CategoryExample::CategoryExample(Wt::WContainerWidget *parent):
  WContainerWidget(parent)
{
  new WText(WString::tr("category chart"), this);

  WAbstractItemModel *model
    = readCsvFile(WApplication::appRoot() + "category.csv", this);

  if (!model)
    return;

  // Show a view that allows editing of the model.
  WContainerWidget *w = new WContainerWidget(this);
  WTableView *table = new WTableView(w);

  table->setMargin(10, Top | Bottom);
  table->setMargin(WLength::Auto, Left | Right);

  table->setModel(model);
  table->setSortingEnabled(true);
  table->setColumnResizeEnabled(true);
  // table->setSelectionMode(ExtendedSelection);
  table->setAlternatingRowColors(true);
  table->setColumnAlignment(0, AlignCenter);
  table->setHeaderAlignment(0, AlignCenter);
  table->setRowHeight(22);

  // Editing does not really work without Ajax, it would require an
  // additional button somewhere to confirm the edited value.
  if (WApplication::instance()->environment().ajax()) {
    table->resize(600, 20 + 5*22);
    table->setEditTriggers(WAbstractItemView::SingleClicked);
  } else {
    table->resize(600, WLength::Auto);
    table->setEditTriggers(WAbstractItemView::NoEditTrigger);
  }

  // We use a single delegate for all items which rounds values to
  // the closest integer value.
  WItemDelegate *delegate = new WItemDelegate(this);
  delegate->setTextFormat("%.f");
  table->setItemDelegate(delegate);

  table->setColumnWidth(0, 80);
  for (int i = 1; i < model->columnCount(); ++i)
    table->setColumnWidth(i, 120);

  /*
   * Create the category chart.
   */
  WCartesianChart *chart = new WCartesianChart(this);
  // chart->setPreferredMethod(WPaintedWidget::PngImage);
  chart->setModel(model);        // set the model
  chart->setXSeriesColumn(0);    // set the column that holds the categories
  chart->setLegendEnabled(true); // enable the legend

  // Automatically layout chart (space for axes, legend, ...)
  chart->setAutoLayoutEnabled(true);

  /*
   * Add all (but first) column as bar series
   */
  for (int i = 1; i < model->columnCount(); ++i) {
    WDataSeries s(i, BarSeries);
    s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    chart->addSeries(s);
  }

  chart->resize(800, 400);

  chart->setMargin(10, Top | Bottom);
  chart->setMargin(WLength::Auto, Left | Right);

  /*
   * Provide a widget to manipulate chart properties
   */
  new ChartConfig(chart, this);
}

TimeSeriesExample::TimeSeriesExample(Wt::WContainerWidget *parent):
  WContainerWidget(parent)
{
  new WText(WString::tr("scatter plot"), this);

  WAbstractItemModel *model = readCsvFile(
    WApplication::appRoot() + "timeseries.csv", this);

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
  WContainerWidget *w = new WContainerWidget(this);
  WTableView *table = new WTableView(w);

  table->setMargin(10, Top | Bottom);
  table->setMargin(WLength::Auto, Left | Right);

  table->setModel(model);
  table->setSortingEnabled(false); // Does not make much sense for time series
  table->setColumnResizeEnabled(true);
  table->setSelectionMode(NoSelection);
  table->setAlternatingRowColors(true);
  table->setColumnAlignment(0, AlignCenter);
  table->setHeaderAlignment(0, AlignCenter);
  table->setRowHeight(22);

  // Editing does not really work without Ajax, it would require an
  // additional button somewhere to confirm the edited value.
  if (WApplication::instance()->environment().ajax()) {
    table->resize(800, 20 + 5*22);
    table->setEditTriggers(WAbstractItemView::SingleClicked);
  } else {
    table->resize(800, 20 + 5*22 + 25);
    table->setEditTriggers(WAbstractItemView::NoEditTrigger);
  }

  WItemDelegate *delegate = new WItemDelegate(this);
  delegate->setTextFormat("%.1f");
  table->setItemDelegate(delegate);
  table->setItemDelegateForColumn(0, new WItemDelegate(this));

  table->setColumnWidth(0, 80);
  for (int i = 1; i < model->columnCount(); ++i)
    table->setColumnWidth(i, 90);

  /*
   * Create the scatter plot.
   */
  WCartesianChart *chart = new WCartesianChart(this);
  //chart->setPreferredMethod(WPaintedWidget::PngImage);
  //chart->setBackground(gray);
  chart->setModel(model);        // set the model
  chart->setXSeriesColumn(0);    // set the column that holds the X data
  chart->setLegendEnabled(true); // enable the legend

  chart->setType(ScatterPlot);            // set type to ScatterPlot
  chart->axis(XAxis).setScale(DateScale); // set scale of X axis to DateScale

  // Automatically layout chart (space for axes, legend, ...)
  chart->setAutoLayoutEnabled();

  /*
   * Add first two columns as line series
   */
  for (int i = 1; i < 3; ++i) {
    WDataSeries s(i, LineSeries);
    s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    chart->addSeries(s);
  }

  chart->resize(800, 400); // WPaintedWidget must be given explicit size

  chart->setMargin(10, Top | Bottom);            // add margin vertically
  chart->setMargin(WLength::Auto, Left | Right); // center horizontally

  new ChartConfig(chart, this);
}

ScatterPlotExample::ScatterPlotExample(WContainerWidget *parent):
  WContainerWidget(parent)
{
  new WText(WString::tr("scatter plot 2"), this);

  WStandardItemModel *model = new WStandardItemModel(40, 2, this);
  model->setItemPrototype(new NumericItem());
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
  WCartesianChart *chart = new WCartesianChart(this);
  chart->setModel(model);        // set the model
  chart->setXSeriesColumn(0);    // set the column that holds the X data
  chart->setLegendEnabled(true); // enable the legend

  chart->setType(ScatterPlot);   // set type to ScatterPlot

  // Typically, for mathematical functions, you want the axes to cross
  // at the 0 mark:
  chart->axis(XAxis).setLocation(ZeroValue);
  chart->axis(YAxis).setLocation(ZeroValue);

  // Automatically layout chart (space for axes, legend, ...)
  chart->setAutoLayoutEnabled();

  // Add the curves
  WDataSeries s(1, CurveSeries);
  s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
  chart->addSeries(s);

  chart->resize(800, 300); // WPaintedWidget must be given explicit size

  chart->setMargin(10, Top | Bottom);            // add margin vertically
  chart->setMargin(WLength::Auto, Left | Right); // center horizontally

  ChartConfig *config = new ChartConfig(chart, this);
  config->setValueFill(ZeroValueFill);
}

PieExample::PieExample(WContainerWidget *parent):
  WContainerWidget(parent)
{
  new WText(WString::tr("pie chart"), this);

  WStandardItemModel *model = new WStandardItemModel(this);
  model->setItemPrototype(new NumericItem());
  
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
      model->item(row, col)->setFlags(ItemIsSelectable | ItemIsEditable);

  WContainerWidget *w = new WContainerWidget(this);
  WTableView* table = new WTableView(w);

  table->setMargin(10, Top | Bottom);
  table->setMargin(WLength::Auto, Left | Right);
  table->setSortingEnabled(true);
  table->setModel(model);
  table->setColumnWidth(1, 100);
  table->setRowHeight(22);

  if (WApplication::instance()->environment().ajax()) {
    table->resize(150 + 100 + 14, 20 + 6 * 22);
    table->setEditTriggers(WAbstractItemView::SingleClicked);
  } else {
    table->resize(150 + 100 + 14, WLength::Auto);
    table->setEditTriggers(WAbstractItemView::NoEditTrigger);    
  }

  /*
   * Create the pie chart.
   */
  WPieChart *chart = new WPieChart(this);
  chart->setModel(model);       // set the model
  chart->setLabelsColumn(0);    // set the column that holds the labels
  chart->setDataColumn(1);      // set the column that holds the data

  // configure location and type of labels
  chart->setDisplayLabels(Outside | TextLabel | TextPercentage);

  // enable a 3D and shadow effect
  chart->setPerspectiveEnabled(true, 0.2);
  chart->setShadowEnabled(true);

  // explode the first item
  chart->setExplode(0, 0.3);

  chart->resize(800, 300); // WPaintedWidget must be given an explicit size

  chart->setMargin(10, Top | Bottom);            // add margin vertically
  chart->setMargin(WLength::Auto, Left | Right); // center horizontally
}

