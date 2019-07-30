// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef RENDER_BLOCK_H_
#define RENDER_BLOCK_H_

#include <Wt/WFont.h>
#include <Wt/WGlobal.h>
#include <Wt/WWebWidget.h>
#include <Wt/WPainter.h>

#include "web/DomElement.h"
#include "LayoutBox.h"
#include "3rdparty/rapidxml/rapidxml.hpp"
#include "Wt/Render/Specificity.h"

namespace Wt {
  namespace Render {

class WTextRenderer;
class Block;
class Line;
class StyleSheet;

typedef std::vector<Block *> BlockList;

struct PageState
{
  PageState()
    : y(0), minX(0), maxX(0), page(0)
  { }

  double y;
  double minX, maxX;
  BlockList floats;
  int page;
};

struct CellState : public PageState
{
  int lastRow;
  double x;
  Block *cell;
};

struct Range
{
  Range(double start, double end) 
    : start(start), end(end)
  { }

  double start, end;
};

enum class FloatSide {
  None,
  Left,
  Right
};

// exported for test.exe
class WT_API Block
{
public:
  Block(Wt::rapidxml::xml_node<> *node, Block *parent);
  virtual ~Block();

  const Block* parent() const { return parent_; }
  BlockList children() const { return children_; }
  void determineDisplay();
  bool normalizeWhitespace(bool haveWhitespace,
			   Wt::rapidxml::xml_document<> &doc);

  bool isFloat() const { return float_ != FloatSide::None; }
  bool isInline() const { return inline_; }
  DomElementType type() const { return type_; }
  bool isText() const;
  std::string text() const;
  bool inlineChildren() const;
  AlignmentFlag horizontalAlignment() const;
  AlignmentFlag verticalAlignment() const;
  FloatSide floatSide() const { return float_; }

  bool isTableCell() const
    { return type_ == DomElementType::TD || type_ == DomElementType::TH; }

  bool isTable() const
    { return type_ == DomElementType::TABLE; }

  Block *table() const;
  bool tableCollapseBorders() const;

  double layoutBlock(PageState &ps,
		     bool canIncreaseWidth,
		     const WTextRenderer& renderer,
		     double collapseMarginTop,
		     double collapseMarginBottom,
		     double cellHeight = -1);

  void collectStyles(WStringStream& ss);
  void setStyleSheet(StyleSheet* styleSheet);
  void actualRender(WTextRenderer& renderer, WPainter& painter, LayoutBox& lb);

  void render(WTextRenderer& renderer, WPainter& painter, int page);

  static void clearFloats(PageState &ps);
  static void clearFloats(PageState &ps,
			  double minWidth);

  std::vector<InlineBox> inlineLayout; // for inline elements, one per line
  std::vector<BlockBox>  blockLayout;  // otherwise, one per page

  static void adjustAvailableWidth(double y, int page,
				   const BlockList& floats,
				   Range &rangeX);

  static bool isWhitespace(char c);

  std::string id() const;
  const std::vector<std::string>& classes() const { return classes_; }
  std::string cssProperty(Property property) const;
  std::string attributeValue(const char *attribute) const;

private:
  struct CssLength {
    double length;
    bool defined;
  };

  enum class PercentageRule {
    PercentageOfFontSize,
    PercentageOfParentSize,
    IgnorePercentage
  };

  struct PropertyValue
  {
    PropertyValue() { }
    PropertyValue(const std::string& value, const Specificity& s)
      : value_(value), s_(s) { }

    std::string value_;
    Specificity s_;
  };

  enum Corner { TopLeft, TopRight, BottomLeft, BottomRight };

  enum class WidthType {
    AsSetWidth,
    MinimumWidth,
    MaximumWidth
  };

  struct BorderElement {
    const Block *block;
    Side side;

    BorderElement() : block(nullptr), side(Side::Left) { }
    BorderElement(const Block *aBlock, Side aSide)
      : block(aBlock), side(aSide) { }
  };

  Wt::rapidxml::xml_node<> *node_;
  Block *parent_;
  BlockList offsetChildren_;
  Block *offsetParent_;
  DomElementType type_;
  std::vector<std::string> classes_;
  FloatSide float_;
  bool inline_;
  BlockList children_;
  const LayoutBox *currentTheadBlock_;
  double currentWidth_;
  double contentsHeight_;
  mutable std::map<std::string, PropertyValue> css_;
  mutable WFont font_;
  StyleSheet* styleSheet_;
  mutable std::set<Property> noPropertyCache_;

  /* For table */
  int tableRowCount_, tableColCount_;

  /* For table cell */
  int cellRow_, cellCol_;

  int attributeValue(const char *attribute, int defaultValue) const;

  void updateAggregateProperty(const std::string& property,
                               const std::string& aggregate,
                               const Specificity& spec,
                               const std::string& value) const;
  void fillinStyle(const std::string& style,
                   const Specificity &specificity) const;
  bool isPositionedAbsolutely() const;
  std::string inheritedCssProperty(Property property) const;
  double cssWidth(double fontScale) const;
  double cssHeight(double fontScale) const;
  CssLength cssLength(Property top, Side side, double fontScale) const;
  double cssMargin(Side side, double fontScale) const;
  double cssPadding(Side side, double fontScale) const;
  double cssBorderSpacing(double fontScale) const;

  double cssBorderWidth(Side side, double fontScale) const;
  double collapsedBorderWidth(Side side, double fontScale) const;
  double rawCssBorderWidth(Side side, double fontScale,
			   bool indicateHidden = false) const;
  WColor cssBorderColor(Side side) const;
  WColor collapsedBorderColor(Side side) const;
  WColor rawCssBorderColor(Side side) const;

  WColor cssColor() const;
  AlignmentFlag cssTextAlign() const;
  double cssBoxMargin(Side side, double fontScale) const;
  double cssLineHeight(double fontLineHeight, double fontScale) const;
  double cssFontSize(double fontScale = 1) const;
  std::string cssPosition() const;
  FontStyle cssFontStyle() const;
  int cssFontWeight() const;
  WFont cssFont(double fontScale) const;
  std::string cssTextDecoration() const;
  double cssDecodeLength(const std::string& length, double fontScale,
			 double defaultValue,
			 PercentageRule percentage =
			   PercentageRule::PercentageOfFontSize,
			 double parentSize = 0)
    const;
  static bool isPercentageLength(const std::string& length);

  double currentParentWidth() const;

  bool isInside(DomElementType type) const;

  void pageBreak(PageState& ps);
  void inlinePageBreak(const std::string& pageBreak,
		       Line& line, BlockList& floats,
		       double minX, double maxX,
		       const WTextRenderer& renderer);
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
  void layoutAbsolute(const WTextRenderer& renderer);

  void tableDoLayout(double x, PageState &ps, double cellSpacing,
		     const std::vector<double>& widths,
		     std::vector<CellState>& rowSpanBackLog,
		     bool protectRows, Block *repeatHead,
		     const WTextRenderer& renderer);
  void tableRowDoLayout(double x, PageState &ps,
			double cellSpacing,
			const std::vector<double>& widths,
			std::vector<CellState>& rowSpanBackLog,
			const WTextRenderer& renderer,
			double rowHeight);
  void tableCellDoLayout(double x, const PageState &ps,
			 double cellSpacing, PageState& rowEnd,
			 const std::vector<double>& widths,
			 const WTextRenderer& renderer,
			 double rowHeight);
  double tableCellX(const std::vector<double>& widths,
		    double cellSpacing) const;
  double tableCellWidth(const std::vector<double>& widths,
			double cellSpacing) const;
  void tableComputeColumnWidths(std::vector<double>& minima,
				std::vector<double>& maxima,
				std::vector<double>& asSet,
				const WTextRenderer& renderer,
				Block *table);

  BorderElement collapseCellBorders(Side side) const;
  int numberTableCells(int row, std::vector<int>& rowSpan);
  Block *findTableCell(int row, int col) const;
  Block *siblingTableCell(Side side) const;

  void cellComputeColumnWidths(WidthType type,
			       std::vector<double>& values,
			       const WTextRenderer& renderer,
			       Block *table);

  void setOffsetParent();
  Block *findOffsetParent();
  LayoutBox layoutTotal();
  LayoutBox firstInlineLayoutBox();

  LayoutBox toBorderBox(const LayoutBox& bb, double fontScale) const;
  double maxLayoutY(int page) const;
  double minLayoutY(int page) const;
  double maxChildrenLayoutY(int page) const;
  double minChildrenLayoutY(int page) const;
  double childrenLayoutHeight(int page) const;
  void reLayout(const LayoutBox& from, const LayoutBox& to);

  void renderText(const std::string& text, WTextRenderer& renderer,
                   WPainter& painter, int page);
  void renderBorders(const LayoutBox& bb, WTextRenderer& renderer, WPainter &painter,
                     WFlags<Side> verticals);

  WString generateItem() const;

  int firstLayoutPage() const;

  int lastLayoutPage() const;

  static void advance(PageState &ps, double height,
		      const WTextRenderer& renderer);
  static double diff(double y, int page, double startY, int startPage,
		     const WTextRenderer& renderer);

  static double positionFloat(double x,
			      PageState &ps,
			      double lineHeight, double width,
			      bool canIncreaseWidth,
			      const WTextRenderer& renderer,
			      FloatSide floatSide);

  static void unsupportedAttributeValue(const char *attribute,
					const std::string& value);
  static void unsupportedCssValue(Property property,
				  const std::string& value);

  static bool isAggregate(const std::string& cssProperty);

  static double maxBorderWidth(Block *b1, Side s1,
			       Block *b2, Side s2,
			       Block *b3, Side s3,
			       Block *b4, Side s4,
			       double fontScale);

  friend class Line;
};

  }
}

#endif // RENDER_BLOCK_H_
