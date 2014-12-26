#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WCssDecorationStyle>
#include <Wt/WBorder>
#include <Wt/WImage>
#include <Wt/Chart/WCartesian3DChart>
#include <Wt/Chart/WGridData>

#include "../treeview-dragdrop/CsvUtil.h"

#include "DataModels.h"

SAMPLE_BEGIN(CatChart3d)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

// create the chart
Wt::Chart::WCartesian3DChart *chart
    = new Wt::Chart::WCartesian3DChart(container);
chart->setType(Wt::Chart::CategoryChart);

// disable server-side rendering fallback; our VPSes don't have that
chart->setRenderOptions(Wt::WGLWidget::ClientSideRendering | Wt::WGLWidget::AntiAliasing);

Wt::WCssDecorationStyle style;
style.setBorder(Wt::WBorder(Wt::WBorder::Solid, Wt::WBorder::Medium,
			    Wt::black));
chart->setDecorationStyle(style);

chart->resize(800, 600);
chart->setTitle("Fish consumption in western Europe");
chart->axis(Wt::Chart::ZAxis_3D).setTitle("Consumption (pcs/year)");
chart->setLegendStyle(Wt::WFont(), Wt::WPen(),
		      Wt::WBrush(Wt::WColor(Wt::lightGray)));
chart->setLegendEnabled(true);
chart->setGridEnabled(Wt::Chart::XZ_Plane, Wt::Chart::ZAxis_3D, true);
chart->setGridEnabled(Wt::Chart::YZ_Plane, Wt::Chart::ZAxis_3D, true);

// load data
Wt::WStandardItemModel *model = 
    csvToModel(Wt::WApplication::appRoot() + "fish_consumption.csv",
	       container, false);

// highlight Belgian codfish consumption
for (int i=0; i < model->rowCount(); i++) {
    for (int j=0; j < model->columnCount(); j++) {
        if (Wt::asString(model->data(0, j)) == Wt::WString("codfish") &&
	    Wt::asString(model->data(i, 0)) == Wt::WString("Belgium"))
	    model->setData(i, j,
			   Wt::WColor(Wt::cyan), Wt::MarkerBrushColorRole);
    }
}

Wt::Chart::WGridData *isotopes = new Wt::Chart::WGridData(model);
isotopes->setTitle("made-up data");
isotopes->setType(Wt::Chart::BarSeries3D);

// add the dataseries to the chart
chart->addDataSeries(isotopes);

chart->setAlternativeContent
    (new Wt::WImage(Wt::WLink("pics/categoricalChartScreenshot.png")));

SAMPLE_END(return container)
