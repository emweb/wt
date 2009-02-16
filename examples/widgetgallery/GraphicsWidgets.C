/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "GraphicsWidgets.h"
#include "PaintExample.h"
#include <Wt/WText>

GraphicsWidgets::GraphicsWidgets(EventDisplayer *ed)
  : ControlsWidget(ed, false)
{
  topic("WPaintedWidget", this);
  new WText(tr("graphics-intro"), this);
  new PaintExample(this);
}
