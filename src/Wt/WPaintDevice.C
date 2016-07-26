/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPaintDevice"

#include "Wt/WPainterPath"
#include "Wt/WRectF"
#include "Wt/WPointF"

#include "Wt/WPainter"

namespace Wt {

WTextItem::WTextItem(const WString& text, double width, double nextWidth)
  : text_(text),
    width_(width),
    nextWidth_(nextWidth)
{ }

WPaintDevice::~WPaintDevice()
{ }

}
