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
  menu->addItem("3D numerical chart",
		deferCreate(boost::bind
			    (&GraphicsWidgets::numCharts3d, this)));
  menu->addItem("3D category chart",
		deferCreate(boost::bind
			    (&GraphicsWidgets::catCharts3d, this)));
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

#include "examples/NumericalCharts3D.cpp"

Wt::WWidget *GraphicsWidgets::numCharts3d()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-NumCharts3D");

  result->bindWidget("NumericalCharts3D", NumChart3d());
 
  return result;
}

#include "examples/CategoryCharts3D.cpp"

Wt::WWidget *GraphicsWidgets::catCharts3d()
{
  Wt::WTemplate *result = new TopicTemplate("graphics-CatCharts3D");

  result->bindWidget("CategoryCharts3D", CatChart3d());

  return result;
}
