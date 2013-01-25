#include "CsvUtil.h"

#include <fstream>

#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WDataSeries>
#include <Wt/WAbstractItemView>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WTableView>

SAMPLE_BEGIN(CategoryChart)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

//WStandardItemModel *model = new WStandardItemModel(0, 0, parent);
//std::ifstream f(Wt::WApplication::appRoot() + "category.csv");

//if (!f)
//  return container();

//readFromCsv(f, model);

//// Show a view that allows editing of the model.
//Wt::WContainerWidget *w = new Wt::WContainerWidget(container);
//Wt::WTableView *table = new Wt::WTableView(w);

//table->setMargin(10, Top | Bottom);
//table->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);

//table->setModel(model);
//table->setSortingEnabled(true);
//table->setColumnResizeEnabled(true);
//// table->setSelectionMode(ExtendedSelection);
//table->setAlternatingRowColors(true);
//table->setColumnAlignment(0, Wt::AlignCenter);
//table->setHeaderAlignment(0, Wt::AlignCenter);
//table->setRowHeight(22);

//// Editing does not really work without Ajax, it would require an
//// additional button somewhere to confirm the edited value.
//if (WApplication::instance()->environment().ajax()) {
//  table->resize(600, 20 + 5*22);
//  table->setEditTriggers(Wt::WAbstractItemView::SingleClicked);
//} else {
//  table->resize(600, WLength::Auto);
//  table->setEditTriggers(Wt::WAbstractItemView::NoEditTrigger);
//}

//// We use a single delegate for all items which rounds values to
//// the closest integer value.
////WItemDelegate *delegate = new WItemDelegate(this);
////delegate->setTextFormat("%.f");
////table->setItemDelegate(delegate);

//table->setColumnWidth(0, 80);
//for (int i = 1; i < model->columnCount(); ++i)
//  table->setColumnWidth(i, 120);

///*
// * Create the category chart.
// */
//Wt::Chart::WCartesianChart *chart = new Wt::WCartesianChart(container);
//// chart->setPreferredMethod(WPaintedWidget::PngImage);
//chart->setModel(model);        // set the model
//chart->setXSeriesColumn(0);    // set the column that holds the categories
//chart->setLegendEnabled(true); // enable the legend

//// Provide space for the X and Y axis and title.
//chart->setPlotAreaPadding(80, Wt::Left);
//chart->setPlotAreaPadding(40, Wt::Top | Wt::Bottom);

///*
// * Add all (but first) column as bar series
// */
//for (int i = 1; i < model->columnCount(); ++i) {
//  Wt::Chart::WDataSeries s(i, Wt::Chart::BarSeries);
//  s.setShadow(Wt::WShadow(3, 3, Wt::WColor(0, 0, 0, 127), 3));
//  chart->addSeries(s);
//}

//chart->resize(800, 400);

//chart->setMargin(10, Wt::Top | Wt::Bottom);
//chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);

SAMPLE_END(return container)
