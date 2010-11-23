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
  topic("WPaintedWidget", this);
  new WText(tr("graphics-intro"), this);
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

void addColor(PaintBrush *const canvas,
	      WTableCell *cell, 
	      const WColor& color)
{
  cell->decorationStyle().setBackgroundColor(color);
  cell->resize(15, 15);

  const WColor *const javaColor = &color;

  cell->clicked().connect(boost::bind(&PaintBrush::setColor, 
				      canvas, 
				      *javaColor));
}

WWidget* GraphicsWidgets::paintbrush()
{
  WContainerWidget *result = new WContainerWidget();
  
  topic("WPaintedWidget", result);

  new WText(tr("graphics-paintbrush"), result);

  WTable* layout = new WTable(result);

  PaintBrush *const canvas = new PaintBrush(710, 400, layout->elementAt(0,0));
  canvas->decorationStyle().setBorder(WBorder(WBorder::Solid));

  new WText("Color chooser:", layout->elementAt(0,1));
  WTable* colorTable = new WTable(layout->elementAt(0,1));
  addColor(canvas, colorTable->elementAt(0, 0), WColor(black));
  addColor(canvas, colorTable->elementAt(0, 1), WColor(red));
  addColor(canvas, colorTable->elementAt(1, 0), WColor(green));
  addColor(canvas, colorTable->elementAt(1, 1), WColor(blue));
  new WBreak(layout->elementAt(0,1));
  WPushButton* clearButton = new WPushButton("Clear", layout->elementAt(0,1));
  clearButton->clicked().connect(canvas, &PaintBrush::clear);
  layout->elementAt(0,1)->setPadding(3);

  return result;
}
