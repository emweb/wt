#include "Tabs.h"

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WStandardItemModel"
#include "Wt/WTabWidget"
#include "Wt/WCssDecorationStyle"
#include "Wt/WBorder"
#include "Wt/Chart/WCartesian3DChart"
#include "Wt/Chart/WGridData"

#include "../treeview-dragdrop/CsvUtil.h"
#include <fstream>

SAMPLE_BEGIN(CatChart3d)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

// create the chart and add a border to the widget
Wt::Chart::WCartesian3DChart *chart = new Wt::Chart::WCartesian3DChart(container);
chart->setType(Wt::Chart::CategoryChart);
Wt::WCssDecorationStyle style;
style.setBorder(Wt::WBorder(Wt::WBorder::Solid, Wt::WBorder::Medium, Wt::black));
chart->setDecorationStyle(style);
chart->resize(600, 600);

// load first set of data (horizontal plane)
Wt::WStandardItemModel *model1 = 
  csvToModel(Wt::WApplication::appRoot() + "hor_plane.csv", container, false);
Wt::Chart::WGridData *horPlane = new Wt::Chart::WGridData(model1);
horPlane->setType(Wt::Chart::BarSeries3D);

// load second set of data (stability of isotopes)
Wt::WStandardItemModel *model2 = 
  csvToModel(Wt::WApplication::appRoot() + "isotope_decay.csv", container, false);
Wt::Chart::WGridData *isotopes = new Wt::Chart::WGridData(model2);
isotopes->setType(Wt::Chart::BarSeries3D);

// add the dataseries to the chart

chart->addDataSeries(isotopes);
chart->addDataSeries(horPlane);
// Set up the WTabWidget for configuring the charts
CategoryDataSettings *settings1 = new CategoryDataSettings(horPlane, container);
CategoryDataSettings *settings2 = new CategoryDataSettings(isotopes, container);
Wt::WTabWidget *configuration = new Wt::WTabWidget(container);
configuration->addTab(settings1, "horizontal plane", Wt::WTabWidget::PreLoading);
configuration->addTab(settings2, "isotope data", Wt::WTabWidget::PreLoading);

SAMPLE_END(return container)
