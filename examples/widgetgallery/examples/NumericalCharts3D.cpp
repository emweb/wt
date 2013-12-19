#include "Tabs.h"
#include "DataModels.h"

#include "Wt/WAbstractTableModel"
#include "Wt/WCssDecorationStyle"
#include "Wt/WBorder"
#include "Wt/WContainerWidget"
#include "Wt/WTabWidget"
#include "Wt/Chart/WCartesian3DChart"
#include "Wt/Chart/WGridData"
#include "Wt/Chart/WEquidistantGridData"
#include "Wt/Chart/WScatterData"
#include "Wt/Chart/WStandardColorMap"

SAMPLE_BEGIN(NumChart3d)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

// create the chart and add a border to the widget
Wt::Chart::WCartesian3DChart *chart = new Wt::Chart::WCartesian3DChart(container);
chart->setType(Wt::Chart::ScatterPlot);
Wt::WCssDecorationStyle style;
style.setBorder(Wt::WBorder(Wt::WBorder::Solid, Wt::WBorder::Medium, Wt::black));
chart->setDecorationStyle(style);
chart->resize(600, 600);
// WPen pen;
// pen.setWidth(5);
// chart->setGridLinesPen(pen);
chart->setGridEnabled(Wt::Chart::XY_Plane, Wt::Chart::XAxis_3D, true);
chart->setGridEnabled(Wt::Chart::XY_Plane, Wt::Chart::YAxis_3D, true);
chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::XAxis_3D, true);
chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::ZAxis_3D, true);
chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::YAxis_3D, true);
chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::ZAxis_3D, true);


// make first dataset (WGridData)
WAbstractTableModel *model1 = new SombreroData(40, 40, -10, 10, -10, 10, container);
Wt::Chart::WGridData *dataset1 = new Wt::Chart::WGridData(model1);
dataset1->setColorMap(new Wt::Chart::WStandardColorMap(dataset1->minimum(ZAxis_3D),
						       dataset1->maximum(ZAxis_3D),
						       true));
GridDataSettings *datasettings1 = new GridDataSettings(chart, dataset1, container);

// make second dataset (WEquidistantGridData)
WAbstractTableModel *model2 = new PlaneData(40, 40, -10, 0.5f, -10, 0.5f, container);
Wt::Chart::WEquidistantGridData *dataset2 = new Wt::Chart::WEquidistantGridData
  (model2, -10, 0.5f, -10, 0.5f);
GridDataSettings *datasettings2 = new GridDataSettings(chart, dataset2, container);

// make third dataset (WScatterData)
WAbstractTableModel *model3 = new PointsData(100, container);
Wt::Chart::WScatterData *dataset3 = new Wt::Chart::WScatterData(model3);
ScatterDataSettings *datasettings3 = new ScatterDataSettings(dataset3, container);

// add the data to the chart
chart->addDataSeries(dataset1);
chart->addDataSeries(dataset2);
chart->addDataSeries(dataset3);

Wt::WTabWidget *configuration = new Wt::WTabWidget(container);
configuration->addTab(new ChartSettings(chart), "General Chart Settings",
		      Wt::WTabWidget::PreLoading);
configuration->addTab(datasettings1, "Dataset 1", Wt::WTabWidget::PreLoading);
configuration->addTab(datasettings2, "Dataset 2", Wt::WTabWidget::PreLoading);
configuration->addTab(datasettings3, "Dataset 3", Wt::WTabWidget::PreLoading);

SAMPLE_END(return container)
