#include <Wt/WApplication.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WBorder.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/Chart/WCartesian3DChart.h>
#include <Wt/Chart/WGridData.h>
#include <Wt/Chart/WEquidistantGridData.h>
#include <Wt/Chart/WScatterData.h>
#include <Wt/Chart/WStandardColorMap.h>

#include "DataModels.h"

SAMPLE_BEGIN(NumChart3d)

auto container = cpp14::make_unique<WContainerWidget>();

// create the chart and add a border to the widget
Chart::WCartesian3DChart *chart = container->addNew<Chart::WCartesian3DChart>();
chart->setType(Chart::ChartType::Scatter);

// disable server-side rendering fallback; our VPSes don't have that
chart->setRenderOptions(GLRenderOption::ClientSide | GLRenderOption::AntiAliasing);

WCssDecorationStyle style;
style.setBorder(WBorder(BorderStyle::Solid, BorderWidth::Medium,
                            WColor(StandardColor::Black)));
chart->setDecorationStyle(style);

chart->resize(900, 700);
chart->setGridEnabled(Chart::Plane::XY, Chart::Axis::X3D, true);
chart->setGridEnabled(Chart::Plane::XY, Chart::Axis::Y3D, true);
chart->setGridEnabled(Chart::Plane::XZ, Chart::Axis::X3D, true);
chart->setGridEnabled(Chart::Plane::XZ, Chart::Axis::Z3D, true);
chart->setGridEnabled(Chart::Plane::YZ, Chart::Axis::Y3D, true);
chart->setGridEnabled(Chart::Plane::YZ, Chart::Axis::Z3D, true);

chart->axis(Chart::Axis::X3D).setTitle("X");
chart->axis(Chart::Axis::Y3D).setTitle("Y");
chart->axis(Chart::Axis::Z3D).setTitle("Z");

chart->setIntersectionLinesEnabled(true);
chart->setIntersectionLinesColor(WColor(0, 255, 255));

// make first dataset (WGridData)
auto model1 = std::make_shared<SombreroData>(40, 40);
auto dataset1 = cpp14::make_unique<Chart::WGridData>(model1);
dataset1->setType(Chart::Series3DType::Surface);
dataset1->setSurfaceMeshEnabled(true);
auto colormap =
    std::make_shared<Chart::WStandardColorMap>(dataset1->minimum(Chart::Axis::Z3D),
                                     dataset1->maximum(Chart::Axis::Z3D),
				     true);
dataset1->setColorMap(colormap);

// make second dataset (WEquidistantGridData)
auto model2 = std::make_shared<PlaneData>(40, 40);
for (int i=0; i < model2->rowCount(); i++) { // set a few size-roles
    model2->setData(i, 0, 5, ItemDataRole::MarkerScaleFactor);
    model2->setData(i, model2->columnCount()-1, 5, ItemDataRole::MarkerScaleFactor);
}

for (int i=0; i < model2->columnCount(); i++) {
    model2->setData(0, i, 5, ItemDataRole::MarkerScaleFactor);
    model2->setData(model2->rowCount()-1, i, 5, ItemDataRole::MarkerScaleFactor);
}

for (int i=0; i < model2->rowCount(); i++) { // set a few color-roles
    model2->setData(i, 5, WColor(0,255,0), ItemDataRole::MarkerBrushColor);
    model2->setData(i, 6, WColor(0,0,255), ItemDataRole::MarkerBrushColor);
    model2->setData(i, 7, WColor(0,255,0), ItemDataRole::MarkerBrushColor);
    model2->setData(i, 8, WColor(0,0,255), ItemDataRole::MarkerBrushColor);
}

auto dataset2 =
    cpp14::make_unique<Chart::WEquidistantGridData>(model2, -10, 0.5f, -10, 0.5f);

// make third dataset (WScatterData)
auto model3 = std::make_shared<SpiralData>(100);
auto dataset3 = cpp14::make_unique<Chart::WScatterData>(model3);
dataset3->setPointSize(5);

// make fourth dataset (WEquidistantGridData, intersecting with dataset1)
auto model4 = std::make_shared<HorizontalPlaneData>(20, 20);
auto dataset4 =
  cpp14::make_unique<Chart::WEquidistantGridData>(model4, -10, 1.0f, -10, 1.0f);
dataset4->setType(Chart::Series3DType::Surface);
dataset4->setSurfaceMeshEnabled(true);

// add the data to the chart
chart->addDataSeries(std::move(dataset1));
chart->addDataSeries(std::move(dataset2));
chart->addDataSeries(std::move(dataset3));
chart->addDataSeries(std::move(dataset4));

chart->setAlternativeContent
    (cpp14::make_unique<WImage>(WLink("pics/numericalChartScreenshot.png")));

SAMPLE_END(return std::move(container))
