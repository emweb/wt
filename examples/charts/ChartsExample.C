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
#include <Wt/WStandardItemModel>
#include <Wt/WText>

#include <Wt/WBorderLayout>
#include <Wt/WFitLayout>

#include <Wt/Ext/Calendar>
#include <Wt/Ext/Container>
#include <Wt/Ext/DateField>
#include <Wt/Ext/LineEdit>
#include <Wt/Ext/NumberField>
#include <Wt/Ext/Panel>
#include <Wt/Ext/TableView>

#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WPieChart>

using namespace Wt;
using namespace Wt::Chart;
namespace {
  WAbstractItemModel *readCsvFile(const char *fname,
				  WContainerWidget *parent)
  {
    WStandardItemModel *model = new WStandardItemModel(0, 0, parent);
    std::ifstream f(fname);
    
    if (f) {
      readFromCsv(f, model);
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

  WAbstractItemModel *model = readCsvFile("category.csv", this);

  if (!model)
    return;

  /*
   * If we have JavaScript, show an Ext table view that allows editing
   * of the model.
   */
  if (wApp->environment().javaScript()) {
    WContainerWidget *w = new WContainerWidget(this);
    Ext::TableView *table = new Ext::TableView(w);
    table->setMargin(10, Top | Bottom);
    table->setMargin(WLength(), Left | Right);
    table->resize(500, 175);
    table->setModel(model);
    table->setAutoExpandColumn(0);

    table->setEditor(0, new Ext::LineEdit());

    for (int i = 1; i < model->columnCount(); ++i) {
      Ext::NumberField *nf = new Ext::NumberField();
      table->setEditor(i, nf);
    }
  }

  /*
   * Create the category chart.
   */
  WCartesianChart *chart = new WCartesianChart(this);
  chart->setModel(model);        // set the model
  chart->setXSeriesColumn(0);    // set the column that holds the categories
  chart->setLegendEnabled(true); // enable the legend

  // Provide space for the X and Y axis and title. 
  chart->setPlotAreaPadding(100, Left);
  chart->setPlotAreaPadding(50, Top | Bottom);

  //chart->axis(YAxis).setBreak(70, 110);

  /*
   * Add all (but first) column as bar series
   */
  for (int i = 1; i < model->columnCount(); ++i) {
    WDataSeries s(i, BarSeries);
    chart->addSeries(s);
  }

  chart->resize(800, 400); // WPaintedWidget must be given explicit size

  chart->setMargin(10, Top | Bottom);        // add margin vertically
  chart->setMargin(WLength(), Left | Right); // center horizontally

  new ChartConfig(chart, this);
}

TimeSeriesExample::TimeSeriesExample(Wt::WContainerWidget *parent):
  WContainerWidget(parent)
{
  new WText(WString::tr("scatter plot"), this);

  WAbstractItemModel *model = readCsvFile("timeseries.csv", this);

  if (!model)
    return;

  /*
   * Parse the first column as dates
   */
  for (int i = 0; i < model->rowCount(); ++i) {
    WString s = asString(model->data(i, 0));
    WDate d = WDate::fromString(s, "dd/MM/yy");
    model->setData(i, 0, boost::any(d));
  }

  /*
   * Create the scatter plot.
   */
  WCartesianChart *chart = new WCartesianChart(this);
  chart->setModel(model);        // set the model
  chart->setXSeriesColumn(0);    // set the column that holds the X data
  chart->setLegendEnabled(true); // enable the legend

  chart->setType(ScatterPlot);            // set type to ScatterPlot
  chart->axis(XAxis).setScale(DateScale); // set scale of X axis to DateScale

  // Provide space for the X and Y axis and title. 
  chart->setPlotAreaPadding(100, Left);
  chart->setPlotAreaPadding(50, Top | Bottom);

  /*
   * Add first two columns as line series
   */
  for (int i = 1; i < 3; ++i) {
    WDataSeries s(i, LineSeries);
    chart->addSeries(s);
  }

  chart->resize(800, 400); // WPaintedWidget must be given explicit size

  chart->setMargin(10, Top | Bottom);        // add margin vertically
  chart->setMargin(WLength(), Left | Right); // center horizontally

  new ChartConfig(chart, this);
}

ScatterPlotExample::ScatterPlotExample(WContainerWidget *parent):
  WContainerWidget(parent)
{
  new WText(WString::tr("scatter plot 2"), this);

  WStandardItemModel *model = new WStandardItemModel(100, 2, this);
  model->setHeaderData(0, boost::any(WString("X")));
  model->setHeaderData(1, boost::any(WString("Y = sin(X)")));

  for (unsigned i = 0; i < 40; ++i) {
    double x = (static_cast<double>(i) - 20) / 4;

    model->setData(i, 0, boost::any(x));
    model->setData(i, 1, boost::any(sin(x)));
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

  // Provide space for the X and Y axis and title. 
  chart->setPlotAreaPadding(100, Left);
  chart->setPlotAreaPadding(50, Top | Bottom);

  // Add the two curves
  chart->addSeries(WDataSeries(1, CurveSeries));

  chart->resize(800, 300); // WPaintedWidget must be given explicit size

  chart->setMargin(10, Top | Bottom);        // add margin vertically
  chart->setMargin(WLength(), Left | Right); // center horizontally

  ChartConfig *config = new ChartConfig(chart, this);
  config->setValueFill(ZeroValueFill);
}

PieExample::PieExample(WContainerWidget *parent):
  WContainerWidget(parent)
{
  new WText(WString::tr("pie chart"), this);

  WAbstractItemModel *model = readCsvFile("pie.csv", this);

  if (!model)
    return;

  /*
   * If we have JavaScript, show an Ext table view that allows editing
   * of the model.
   */
  if (wApp->environment().javaScript()) {
    WContainerWidget *w = new WContainerWidget(this);
    Ext::TableView *table = new Ext::TableView(w);
    table->setMargin(10, Top | Bottom);
    table->setMargin(WLength(), Left | Right);
    table->resize(300, 175);
    table->setModel(model);
    table->setAutoExpandColumn(0);

    table->setEditor(0, new Ext::LineEdit());

    for (int i = 1; i < model->columnCount(); ++i)
      table->setEditor(i, new Ext::NumberField());
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

  // enable a 3D effect
  chart->setPerspectiveEnabled(true, 0.2);

  // explode the first item
  chart->setExplode(0, 0.3);

  chart->resize(800, 300); // WPaintedWidget must be given explicit size

  chart->setMargin(10, Top | Bottom);        // add margin vertically
  chart->setMargin(WLength(), Left | Right); // center horizontally
}

