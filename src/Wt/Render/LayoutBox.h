// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef RENDER_LAYOUT_BOX_H_
#define RENDER_LAYOUT_BOX_H_

namespace Wt {
  namespace Render {

struct LayoutBox {
  LayoutBox();

  bool isNull() const { return page == -1; }

  int page;
  double x, y;
  double width;
  double height;
};

struct InlineBox : public LayoutBox
{
  InlineBox();

  int utf8Pos, utf8Count;

  // minimum width of whitespace contained in this linebox.
  double whitespaceWidth;
  int whitespaceCount;

  double baseline /*, lineTop, lineHeight */;
};

struct BlockBox : public LayoutBox {
};

  }
}

#endif // RENDER_LAYOUT_BOX_H_
