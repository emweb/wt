/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPaintDevice"

namespace Wt {

WTextItem::WTextItem(const WString& text, double width)
  : text_(text),
    width_(width)
{ }

WPaintDevice::~WPaintDevice()
{ }

}
