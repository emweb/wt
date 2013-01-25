/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "GraphicsWidgets.h"
#include "TopicTemplate.h"

#include <Wt/WAbstractItemModel>
#include <Wt/WImage>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>

GraphicsWidgets::GraphicsWidgets()
    : TopicWidget()
{
  addText(tr("graphics-intro"), this);
}

void GraphicsWidgets::populateSubMenu(Wt::WMenu *menu)
{
  menu->setInternalBasePath("/graphics-charts");

  menu->addItem("2D painting", painting2d())->setPathComponent("");
  menu->addItem("Paintbrush", 
		deferCreate(boost::bind
			    (&GraphicsWidgets::paintbrush, this)));
  menu->addItem("Category chart", 
		deferCreate(boost::bind
			    (&GraphicsWidgets::categoryChart, this)));
  menu->addItem("Scatter plot", 
		deferCreate(boost::bind
			    (&GraphicsWidgets::scatterPlot, this)));
  menu->addItem("Pie chart", 
		deferCreate(boost::bind
			    (&GraphicsWidgets::pieChart, this)));
  menu->addItem("Maps",
                deferCreate(boost::bind
			    (&GraphicsWidgets::googleMap, this)));
  menu->addItem("3D painting", 
		deferCreate(boost::bind
			    (&GraphicsWidgets::painting3d, this)));
}


#include "examples/PaintingEvent.cpp"
#include "examples/PaintingShapes.cpp"
#include "examples/PaintingTransformations.cpp"
#include "examples/PaintingClipping.cpp"
#include "examples/PaintingStyle.cpp"
#include "examples/PaintingImages.cpp"

Wt::WWidget *GraphicsWidgets::painting2d()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-Painting2D");

  result->bindWidget("PaintingEvent", PaintingEvent());
  result->bindWidget("PaintingShapes", PaintingShapes());
  result->bindWidget("PaintingTransformations", PaintingTransformations());
  result->bindWidget("PaintingClipping", PaintingClipping());
  result->bindWidget("PaintingStyle", PaintingStyle());
  result->bindWidget("PaintingImages", PaintingImages());

  return result;
}


#include "examples/Paintbrush.cpp"

Wt::WWidget *GraphicsWidgets::paintbrush()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-Paintbrush");

  result->bindWidget("Paintbrush", Paintbrush());

  return result;
}


#include "examples/CategoryChart.cpp"

Wt::WWidget *GraphicsWidgets::categoryChart()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-CategoryChart");

  result->bindWidget("CategoryChart", CategoryChart());

  return result;
}


#include "examples/ScatterPlotData.cpp"
#include "examples/ScatterPlotCurve.cpp"

Wt::WWidget *GraphicsWidgets::scatterPlot()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-ScatterPlot");

  result->bindWidget("ScatterPlotData", ScatterPlotData());
  result->bindWidget("ScatterPlotCurve", ScatterPlotCurve());

  return result;
}


#include "examples/PieChart.cpp"

Wt::WWidget *GraphicsWidgets::pieChart()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-PieChart");

  result->bindWidget("PieChart", PieChart());

  return result;
}


#include "examples/GoogleMap.cpp"

Wt::WWidget *GraphicsWidgets::googleMap()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-GoogleMap");

  result->bindWidget("GoogleMap", GoogleMap());

  // Show the XML-template as text
  result->bindString("GoogleMap-controls",
                     reindent(tr("graphics-GoogleMap-controls")),
		     Wt::PlainText);
  return result;
}


#include "examples/Painting3D.cpp"

Wt::WWidget *GraphicsWidgets::painting3d()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-Painting3D");

  result->bindWidget("Painting3D", Painting3D());

  return result;
}



/*
 * Reads a CSV file as an (editable) standard item model.
 */
Wt::WAbstractItemModel *readCsvFile(const std::string &fname,
				    Wt::WContainerWidget *parent)
{
  Wt::WStandardItemModel *model = new Wt::WStandardItemModel(0, 0, parent);
  std::ifstream f(fname.c_str());

  if (f) {
    readFromCsv(f, model);

    for (int row = 0; row < model->rowCount(); ++row)
      for (int col = 0; col < model->columnCount(); ++col) {
	model->item(row, col)->setFlags(Wt::ItemIsSelectable 
					| Wt::ItemIsEditable);
    }

    return model;
  } else {
    Wt::WString error(Wt::WString::tr("error-missing-data"));
    error.arg(fname, Wt::UTF8);
    new Wt::WText(error, parent);
    return 0;
  }
}


