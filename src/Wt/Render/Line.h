// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef RENDER_LINE_H_
#define RENDER_LINE_H_

#include <Wt/WGlobal.h>
#include <vector>

namespace Wt {
  namespace Render {

class WTextRenderer;

class Block;
typedef std::vector<Block *> BlockList;

class Line
{
public:
  static const double LEFT_MARGIN_X;

  Line(double x, double y, int page);

  double x() const { return x_; }
  double y() const { return y_; }
  int page() const { return page_; }
  double height() const { return height_; }
  double bottom() const { return y_ + height_; }

  void setX(double x) { x_ = x; }
  void setLineBreak(bool lineBreak) { lineBreak_ = lineBreak; }
  void newLine(double y, double minX, int page);

  void reflow(Block *lineFloat);
  void moveToNextPage(BlockList& floats, double minX, double maxX,
		      const WTextRenderer& renderer);
  void adjustHeight(double height, double baseline,
		    double minimumLineHeight);
  void finish(AlignmentFlag textAlign,
	      BlockList& floats, double minX, double maxX,
	      const WTextRenderer& renderer);

  void addBlock(Block *b);

private:
  int page_;
  double x_, y_, height_, baseline_;
  BlockList blocks_;
  bool lineBreak_;
};

  }
}

#endif // RENDER_LINE_H_
