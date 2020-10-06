#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WBorder.h>
#include <Wt/WImage.h>
#include <Wt/Chart/WCartesian3DChart.h>
#include <Wt/Chart/WGridData.h>

#include "../treeview-dragdrop/CsvUtil.h"

#include "DataModels.h"

SAMPLE_BEGIN(CatChart3d)

auto container = std::make_unique<WContainerWidget>();

// create the chart
auto chart
    = container->addNew<Chart::WCartesian3DChart>();
chart->setType(Chart::ChartType::Category);

// disable server-side rendering fallback; our VPSes don't have that
chart->setRenderOptions(GLRenderOption::ClientSide| GLRenderOption::AntiAliasing);

WCssDecorationStyle style;
style.setBorder(WBorder(BorderStyle::Solid, BorderWidth::Medium,
			    WColor(StandardColor::Black)));
chart->setDecorationStyle(style);

chart->resize(800, 600);
chart->setTitle("Fish consumption in western Europe");
chart->axis(Chart::Axis::Z3D).setTitle("Consumption (pcs/year)");
chart->setLegendStyle(WFont(), WPen(),
		      WBrush(WColor(StandardColor::LightGray)));
chart->setLegendEnabled(true);
chart->setGridEnabled(Chart::Plane::XZ, Chart::Axis::Z3D, true);
chart->setGridEnabled(Chart::Plane::YZ, Chart::Axis::Z3D, true);

// load data
auto model =
    csvToModel(WApplication::appRoot() + "fish_consumption.csv", false);

// highlight Belgian codfish consumption
for (int i=0; i < model->rowCount(); i++) {
    for (int j=0; j < model->columnCount(); j++) {
        if (asString(model->data(0, j)) == WString("codfish") &&
            asString(model->data(i, 0)) == WString("Belgium"))
	    model->setData(i, j,
			   WColor(StandardColor::Cyan), ItemDataRole::MarkerBrushColor);
    }
}

auto isotopes = std::make_unique<Chart::WGridData>(model);
isotopes->setTitle("made-up data");
isotopes->setType(Chart::Series3DType::Bar);

// add the dataseries to the chart
chart->addDataSeries(std::move(isotopes));

chart->setAlternativeContent
    (std::make_unique<WImage>(WLink("pics/categoricalChartScreenshot.png")));

SAMPLE_END(return std::move(container))
