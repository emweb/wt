#include <Wt/WApplication>
#include <Wt/WCssDecorationStyle>
#include <Wt/WBorder>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/Chart/WCartesian3DChart>
#include <Wt/Chart/WGridData>
#include <Wt/Chart/WEquidistantGridData>
#include <Wt/Chart/WScatterData>
#include <Wt/Chart/WStandardColorMap>

#include "DataModels.h"

SAMPLE_BEGIN(NumChart3d)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

// create the chart and add a border to the widget
Wt::Chart::WCartesian3DChart *chart
    = new Wt::Chart::WCartesian3DChart(container);
chart->setType(Wt::Chart::ScatterPlot);

// disable server-side rendering fallback; our VPSes don't have that
chart->setRenderOptions(Wt::WGLWidget::ClientSideRendering | Wt::WGLWidget::AntiAliasing);

Wt::WCssDecorationStyle style;
style.setBorder(Wt::WBorder(Wt::WBorder::Solid, Wt::WBorder::Medium,
			    Wt::black));
chart->setDecorationStyle(style);

chart->resize(600, 600);
chart->setGridEnabled(Wt::Chart::XY_Plane, Wt::Chart::XAxis_3D, true);
chart->setGridEnabled(Wt::Chart::XY_Plane, Wt::Chart::YAxis_3D, true);
chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::XAxis_3D, true);
chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::ZAxis_3D, true);
chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::YAxis_3D, true);
chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::ZAxis_3D, true);

chart->axis(Wt::Chart::XAxis_3D).setTitle("X");
chart->axis(Wt::Chart::YAxis_3D).setTitle("Y");
chart->axis(Wt::Chart::ZAxis_3D).setTitle("Z");

chart->setIntersectionLinesEnabled(true);
chart->setIntersectionLinesColor(Wt::WColor(0, 255, 255));

// make first dataset (WGridData)
Wt::WStandardItemModel *model1 = new SombreroData(40, 40, container);
Wt::Chart::WGridData *dataset1 = new Wt::Chart::WGridData(model1);
dataset1->setType(Wt::Chart::SurfaceSeries3D);
dataset1->setSurfaceMeshEnabled(true);
Wt::Chart::WStandardColorMap *colormap =
    new Wt::Chart::WStandardColorMap(dataset1->minimum(Wt::Chart::ZAxis_3D),
				     dataset1->maximum(Wt::Chart::ZAxis_3D),
				     true);
dataset1->setColorMap(colormap);

// make second dataset (WEquidistantGridData)
Wt::WStandardItemModel *model2 = new PlaneData(40, 40, container);
for (int i=0; i < model2->rowCount(); i++) { // set a few size-roles
    model2->setData(i, 0, 5, Wt::MarkerScaleFactorRole);
    model2->setData(i, model2->columnCount()-1, 5, Wt::MarkerScaleFactorRole);
}

for (int i=0; i < model2->columnCount(); i++) {
    model2->setData(0, i, 5, Wt::MarkerScaleFactorRole);
    model2->setData(model2->rowCount()-1, i, 5, Wt::MarkerScaleFactorRole);
}

for (int i=0; i < model2->rowCount(); i++) { // set a few color-roles
    model2->setData(i, 5, Wt::WColor(0,255,0), Wt::MarkerBrushColorRole);
    model2->setData(i, 6, Wt::WColor(0,0,255), Wt::MarkerBrushColorRole);
    model2->setData(i, 7, Wt::WColor(0,255,0), Wt::MarkerBrushColorRole);
    model2->setData(i, 8, Wt::WColor(0,0,255), Wt::MarkerBrushColorRole);
}

Wt::Chart::WEquidistantGridData *dataset2 =
    new Wt::Chart::WEquidistantGridData(model2, -10, 0.5f, -10, 0.5f);

// make third dataset (WScatterData)
Wt::WStandardItemModel *model3 = new SpiralData(100, container);
Wt::Chart::WScatterData *dataset3 = new Wt::Chart::WScatterData(model3);
dataset3->setPointSize(5);

// make fourth dataset (WEquidistantGridData, intersecting with dataset1)
Wt::WStandardItemModel *model4 = new HorizontalPlaneData(20, 20, container);
Wt::Chart::WEquidistantGridData *dataset4 =
  new Wt::Chart::WEquidistantGridData(model4, -10, 1.0f, -10, 1.0f);
dataset4->setType(Wt::Chart::SurfaceSeries3D);
dataset4->setSurfaceMeshEnabled(true);

// add the data to the chart
chart->addDataSeries(dataset1);
chart->addDataSeries(dataset2);
chart->addDataSeries(dataset3);
chart->addDataSeries(dataset4);

chart->setAlternativeContent
    (new Wt::WImage(Wt::WLink("pics/numericalChartScreenshot.png")));

SAMPLE_END(return container)
