/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "GraphicsWidgets.h"
#include "TopicTemplate.h"

#include <Wt/WAbstractItemModel.h>
#include <Wt/WImage.h>
#include <Wt/WMenu.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>

GraphicsWidgets::GraphicsWidgets()
    : TopicWidget()
{
  addText(tr("graphics-intro"),this);
}

void GraphicsWidgets::populateSubMenu(Wt::WMenu *menu)
{
  menu->setInternalBasePath("/graphics-charts");

  menu->addItem("2D painting", painting2d())->setPathComponent("");
  menu->addItem("Paintbrush", 
                deferCreate([this]{ return paintbrush(); }));
  menu->addItem("Category chart",
                deferCreate([this]{ return categoryChart(); }));
  menu->addItem("Scatter plot",
                deferCreate([this]{ return scatterPlot(); }));
  menu->addItem("Axis slider widget",
                deferCreate([this]{ return axisSliderWidget(); }));
  menu->addItem("Pie chart", 
                deferCreate([this]{ return pieChart(); }));
  menu->addItem("Leaflet maps",
                deferCreate([this]{ return leafletMap(); }));
  menu->addItem("Google maps",
                deferCreate([this]{ return googleMap(); }));
  menu->addItem("3D painting", 
                deferCreate([this]{ return painting3d(); }));
  menu->addItem("3D numerical chart",
                deferCreate([this]{ return numCharts3d(); }));
  menu->addItem("3D category chart",
                deferCreate([this]{ return catCharts3d(); }));
}


#include "examples/PaintingEvent.cpp"
#include "examples/PaintingShapes.cpp"
#include "examples/PaintingTransformations.cpp"
#include "examples/PaintingClipping.cpp"
#include "examples/PaintingStyle.cpp"
#include "examples/PaintingImages.cpp"
#include "examples/PaintingInteractive.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::painting2d()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-Painting2D");

  result->bindWidget("PaintingEvent", PaintingEvent());
  result->bindWidget("PaintingShapes", PaintingShapes());
  result->bindWidget("PaintingTransformations", PaintingTransformations());
  result->bindWidget("PaintingClipping", PaintingClipping());
  result->bindWidget("PaintingStyle", PaintingStyle());
  result->bindWidget("PaintingImages", PaintingImages());
  result->bindWidget("PaintingInteractive", PaintingInteractive());

  return std::move(result);
}


#include "examples/Paintbrush.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::paintbrush()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-Paintbrush");

  result->bindWidget("Paintbrush", Paintbrush());

  return std::move(result);
}


#include "examples/CategoryChart.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::categoryChart()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-CategoryChart");

  result->bindWidget("CategoryChart", CategoryChart());

  return std::move(result);
}


#include "examples/ScatterPlotData.cpp"
#include "examples/ScatterPlotCurve.cpp"
#include "examples/ScatterPlotInteractive.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::scatterPlot()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-ScatterPlot");

  result->bindWidget("ScatterPlotData", ScatterPlotData());
  result->bindWidget("ScatterPlotCurve", ScatterPlotCurve());
  result->bindWidget("ScatterPlotInteractive", ScatterPlotInteractive());

  return std::move(result);
}

#include "examples/AxisSliderWidget.cpp"
#include "examples/AxisSliderWidgetDifferentDataSeries.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::axisSliderWidget()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-AxisSliderWidget");

  result->bindWidget("AxisSliderWidget", AxisSliderWidget());
  result->bindWidget("AxisSliderWidgetDifferentDataSeries", AxisSliderWidgetDifferentDataSeries());

  return std::move(result);
}


#include "examples/PieChart.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::pieChart()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-PieChart");

  result->bindWidget("PieChart", PieChart());

  return std::move(result);
}

#include "examples/LeafletMap.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::leafletMap()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-LeafletMap");

  result->bindWidget("LeafletMap", LeafletMap());

  return std::move(result);
}


#include "examples/GoogleMap.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::googleMap()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-GoogleMap");

  result->bindWidget("GoogleMap", GoogleMap());

  // Show the XML-template as text
  result->bindString("GoogleMap-controls",
                     reindent(tr("graphics-GoogleMap-controls")),
                     TextFormat::Plain);
  return std::move(result);
}


#include "examples/Painting3D.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::painting3d()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-Painting3D");

  result->bindWidget("Painting3D", Painting3D());

  return std::move(result);
}

#include "examples/NumericalCharts3D.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::numCharts3d()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-NumCharts3D");

  result->bindWidget("NumericalCharts3D", NumChart3d());
 
  return std::move(result);
}

#include "examples/CategoryCharts3D.cpp"

std::unique_ptr<Wt::WWidget> GraphicsWidgets::catCharts3d()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("graphics-CatCharts3D");

  result->bindWidget("CategoryCharts3D", CatChart3d());

  return std::move(result);
}
