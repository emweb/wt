/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "LayoutBox.h"

namespace Wt {
  namespace Render {

LayoutBox::LayoutBox()
  : page(-1),
    x(0),
    y(0),
    width(0),
    height(0)
{ }

InlineBox::InlineBox()
  : utf8Pos(0),
    utf8Count(0),
    whitespaceWidth(0),
    whitespaceCount(0),
    baseline(0)
{ }

  }
}
