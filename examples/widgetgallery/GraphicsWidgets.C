/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "GraphicsWidgets.h"
#include "PaintExample.h"
#include "PaintBrush.h"

#include <Wt/WText>
#include <Wt/WGlobal>
#include <Wt/WCssDecorationStyle>
#include <Wt/WBorder>
#include <Wt/WBreak>
#include <Wt/WPushButton>

GraphicsWidgets::GraphicsWidgets(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  colorMapper_ = new WSignalMapper<WColor >();

  topic("WPaintedWidget", this);
  new WText(tr("graphics-intro"), this);
}

GraphicsWidgets::~GraphicsWidgets()
{
  delete colorMapper_;
}

void GraphicsWidgets::populateSubMenu(WMenu *menu)
{
  menu->addItem("Emweb logo example", emwebLogo());
  menu->addItem("Paintbrush example", paintbrush());
}

WWidget* GraphicsWidgets::emwebLogo()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WPaintedWidget", result);
  
  new PaintExample(result, false);
  
  return result;
}

WWidget* GraphicsWidgets::paintbrush()
{
  WContainerWidget *result = new WContainerWidget();
  
  topic("WPaintedWidget", result);

  new WText(tr("graphics-paintbrush"), result);

  WTable* layout = new WTable(result);

  PaintBrush* canvas = new PaintBrush(710, 400, layout->elementAt(0,0));
  canvas->decorationStyle().setBorder(WBorder(WBorder::Solid));

  new WText("Color chooser:", layout->elementAt(0,1));
  WTable* colorTable = new WTable(layout->elementAt(0,1));
  addColor(colorTable, 0, 0, WColor(black));
  addColor(colorTable, 0, 1, WColor(red));
  addColor(colorTable, 1, 0, WColor(green));
  addColor(colorTable, 1, 1, WColor(blue));
  new WBreak(layout->elementAt(0,1));
  WPushButton* clearButton = new WPushButton("Clear", layout->elementAt(0,1));
  clearButton->clicked().connect(SLOT(canvas, PaintBrush::clear));
  layout->elementAt(0,1)->setPadding(3);

  colorMapper_->mapped().connect(SLOT(canvas, PaintBrush::setColor));
  
  return result;
}


void GraphicsWidgets::addColor(WTable* table, 
			       int row, 
			       int column, 
			       const WColor& color)
{
  table->elementAt(row, column)
    ->decorationStyle().setBackgroundColor(color);
  table->elementAt(row, column)
    ->resize(15, 15);

  colorMapper_->mapConnect(table->elementAt(row, column)->clicked(), color);
}
