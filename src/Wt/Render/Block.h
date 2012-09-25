// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef RENDER_BLOCK_H_
#define RENDER_BLOCK_H_

#include <Wt/WFont>
#include <Wt/WGlobal>
#include <Wt/WWebWidget>

#include "DomElement.h"
#include "LayoutBox.h"
#include "rapidxml/rapidxml.hpp"

namespace Wt {
  namespace Render {

class WTextRenderer;
class Block;
class Line;

typedef std::vector<Block *> BlockList;

struct PageState {
  PageState() 
  {}

  double y;
  double minX, maxX;
  BlockList floats;
  int page;
};

struct Range {
  Range(double start, double end) 
    : start(start), end(end)
  {}

  double start, end;
};

class Block
{
public:
  Block(rapidxml::xml_node<> *node, Block *parent);
  virtual ~Block();

  void determineDisplay();
  bool normalizeWhitespace(bool haveWhitespace,
			   rapidxml::xml_document<> &doc);

  bool isFloat() const { return float_ != 0; }
  bool isInline() const { return inline_; }
  DomElementType type() const { return type_; }
  bool isText() const;
  std::string text() const;
  bool inlineChildren() const;
  AlignmentFlag horizontalAlignment() const;
  AlignmentFlag verticalAlignment() const;
  Side floatSide() const { return float_; }

  double layoutBlock(PageState &ps,
		     bool canIncreaseWidth,
		     const WTextRenderer& renderer,
		     double collapseMarginTop,
		     double collapseMarginBottom,
		     double cellHeight = -1);

  void render(WTextRenderer& renderer, int page);

  static void clearFloats(PageState &ps);
  static void clearFloats(PageState &ps,
			  double minWidth);

  std::vector<InlineBox> inlineLayout; // for inline elements, one per line
  std::vector<BlockBox>  blockLayout;  // otherwise, one per page

  static void adjustAvailableWidth(double y, int page,
				   const BlockList& floats,
				   Range &rangeX);

  static bool isWhitespace(char c);

private:
  struct CssLength {
    double length;
    bool defined;
  };

  enum PercentageRule {
    PercentageOfFontSize,
    PercentageOfParentSize,
    IgnorePercentage
  };

private:
  rapidxml::xml_node<> *node_;
  Block *parent_;
  DomElementType type_;
  Side float_;
  bool inline_;
  BlockList children_;
  double currentWidth_;
  double contentsHeight_;
  mutable std::map<std::string, std::string> css_;

  std::string attributeValue(const char *attribute) const;
  int attributeValue(const char *attribute, int defaultValue) const;

  std::string cssProperty(Property property) const;
  std::string inheritedCssProperty(Property property) const;
  double cssWidth(double fontScale) const;
  double cssHeight(double fontScale) const;
  CssLength cssLength(Property top, Side side, double fontScale) const;
  double cssMargin(Side side, double fontScale) const;
  double cssPadding(Side side, double fontScale) const;
  double cssBorderWidth(Side side, double fontScale) const;
  WColor cssBorderColor(Side side) const;
  WColor cssColor() const;
  AlignmentFlag cssTextAlign() const;
  double cssBoxMargin(Side side, double fontScale) const;
  double cssLineHeight(double fontLineHeight, double fontScale) const;
  double cssFontSize(double fontScale = 1) const;
  WFont::Style cssFontStyle() const;
  int cssFontWeight() const;
  WFont cssFont(double fontScale) const;
  std::string cssTextDecoration() const;
  double cssDecodeLength(const std::string& length, double fontScale,
			 double defaultValue,
			 PercentageRule percentage = PercentageOfFontSize,
			 double parentSize = 0)
    const;
  static bool isPercentageLength(const std::string& length);

  double currentParentWidth() const;

  bool isInside(DomElementType type) const;

  void pageBreak(PageState& ps);
  double layoutInline(Line& line, BlockList& floats,
		      double minX, double maxX, bool canIncreaseWidth,
		      const WTextRenderer& renderer);
  void layoutTable(PageState &ps,
		   bool canIncreaseWidth,
		   const WTextRenderer& renderer,
		   double cssSetWidth);
  double layoutFloat(double y, int page, BlockList& floats,
		     double lineX, double lineHeight,
		     double minX, double maxX,
		     bool canIncreaseWidth,
		     const WTextRenderer& renderer);
  void tableDoLayout(double x, PageState &ps, int cellSpacing,
		     const std::vector<double>& widths,
		     bool protectRows, Block *repeatHead,
		     const WTextRenderer& renderer);
  void tableRowDoLayout(double x, PageState &ps,
			int cellSpacing,
			const std::vector<double>& widths,
			const WTextRenderer& renderer,
			double rowHeight);
  void tableComputeColumnWidths(std::vector<double>& minima,
				std::vector<double>& maxima,
				const WTextRenderer& renderer,
				Block *table);
  int cellComputeColumnWidths(int col, bool maximum,
			      std::vector<double>& values,
			      const WTextRenderer& renderer,
			      Block *table);

  LayoutBox toBorderBox(const LayoutBox& bb, double fontScale) const;
  double maxLayoutY(int page) const;
  double minLayoutY(int page) const;
  double maxChildrenLayoutY(int page) const;
  double minChildrenLayoutY(int page) const;
  double childrenLayoutHeight(int page) const;
  void reLayout(const BlockBox& from, const BlockBox& to);

  void renderText(const std::string& text, WTextRenderer& renderer, int page);
  void renderBorders(const LayoutBox& bb, WTextRenderer& renderer,
		     WFlags<Side> verticals);

  WString generateItem() const;

  static void advance(PageState &ps, double height,
		      const WTextRenderer& renderer);
  static double diff(double y, int page, double startY, int startPage,
		     const WTextRenderer& renderer);

  static double positionFloat(double x,
			      PageState &ps,
			      double lineHeight, double width,
			      bool canIncreaseWidth,
			      const WTextRenderer& renderer,
			      Side floatSide);

  static void unsupportedAttributeValue(const char *attribute,
					const std::string& value);
  static void unsupportedCssValue(Property property,
				  const std::string& value);

  static bool isAggregate(const std::string& cssProperty);

  bool isTableCell() const
    { return type_ == DomElement_TD || type_ == DomElement_TH; }

  friend class Line;
};

  }
}

#endif // RENDER_BLOCK_H_
