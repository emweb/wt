/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPaintDevice.h"

#include "Wt/WPainterPath.h"
#include "Wt/WRectF.h"
#include "Wt/WPointF.h"

#include "Wt/WPainter.h"

namespace Wt {

WTextItem::WTextItem(const WString& text, double width, double nextWidth)
  : text_(text),
    width_(width),
    nextWidth_(nextWidth)
{ }

WPaintDevice::~WPaintDevice()
{ }

}
