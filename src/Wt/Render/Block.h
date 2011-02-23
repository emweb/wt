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

struct PleaseWiden {
  PleaseWiden(double w)
    : width(w)
  { }

  double width;
};

class WTextRenderer;
class Block;
class Line;

typedef std::vector<Block *> BlockList;

class Block
{
public:
  Block(rapidxml::xml_node<> *node, Block *parent);
  virtual ~Block();

  void determineDisplay();
  bool normalizeWhitespace(bool haveWhitespace, rapidxml::memory_pool<>& pool);

  bool isFloat() const { return float_ != None; }
  bool isInline() const { return inline_; }
  DomElementType type() const { return type_; }
  bool isText() const;
  std::string text() const;
  bool inlineChildren() const;
  AlignmentFlag horizontalAlignment() const;
  AlignmentFlag verticalAlignment() const;
  Side floatSide() const { return float_; }

  void layoutBlock(double& y, int& page, BlockList& floats,
		   double minX, double maxX, bool canIncreaseWidth,
		   const WTextRenderer& renderer,
		   double collapseMarginTop,
		   double& collapseMarginBottom,
		   double cellHeight = -1);

  void render(WTextRenderer& renderer, int page);

  static void clearFloats(BlockList& floats, int page);
  static void clearFloats(double& y, int& page, BlockList& floats,
			  double minX, double maxX, double minWidth);

  std::vector<InlineBox> inlineLayout; // for inline elements, one per line
  std::vector<BlockBox>  blockLayout;  // otherwise, one per page

  static void adjustAvailableWidth(double y, int page,
				   double& minX, double& maxX,
				   const BlockList& floats);

  static bool isWhitespace(char c);

private:
  rapidxml::xml_node<> *node_;
  Block *parent_;
  DomElementType type_;
  bool inline_;
  Side float_;
  BlockList children_;
  double contentsHeight_;

  std::string attributeValue(const char *attribute) const;
  int attributeValue(const char *attribute, int defaultValue) const;

  std::string cssProperty(Property property) const;
  std::string cssProperty(Property property, const char *aggregate,
			  int aggregateIndex = -1) const;
  std::string inheritedCssProperty(Property property) const;
  double cssWidth(double fontScale) const;
  double cssHeight(double fontScale) const;
  double cssLength(Property top, const char *aggregate, Side side,
		   double fontScale, bool& defined) const;
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
			 double defaultValue) const;

  bool isInside(DomElementType type) const;

  void layoutInline(Line& line, BlockList& floats,
		    double minX, double maxX, bool canIncreaseWidth,
		    const WTextRenderer& renderer);
  void layoutTable(double& y, int& page, BlockList& floats,
		   double& minX, double& maxX, bool canIncreaseWidth,
		   const WTextRenderer& renderer);
  void layoutFloat(double y, int page, BlockList& floats,
		   double lineX, double lineHeight,
		   double minX, double maxX,
		   bool canIncreaseWidth,
		   const WTextRenderer& renderer);

  void tableDoLayout(double x, double& y, int& page, int cellSpacing,
		     const std::vector<double>& widths,
		     const WTextRenderer& renderer);
  void tableRowDoLayout(double x, double& y, int& page,
			int cellSpacing,
			const std::vector<double>& widths,
			const WTextRenderer& renderer,
			double rowHeight);
  void tableComputeColumnWidths(std::vector<double>& minima,
				std::vector<double>& maxima,
				const WTextRenderer& renderer);
  int cellComputeColumnWidths(int col, bool maximum,
			      std::vector<double>& values,
			      const WTextRenderer& renderer);

  LayoutBox toBorderBox(const LayoutBox& bb, double fontScale) const;

  void renderText(const std::string& text, WTextRenderer& renderer, int page);
  void renderBorders(const LayoutBox& bb, WTextRenderer& renderer,
		     WFlags<Side> verticals);

  WString generateItem() const;

  double layoutHeight() const;

  static void advance(double& y, int& page, double height,
		      const WTextRenderer& renderer);
  static double diff(double y, int page, double startY, int startPage,
		     const WTextRenderer& renderer);

  static void positionFloat(double& x, double& y, int& page,
			    double lineHeight, double width,
			    const BlockList& floats,
			    double minX, double maxX, bool canIncreaseWidth,
			    const WTextRenderer& renderer,
			    Side floatSide);

  static void unsupportedAttributeValue(const char *attribute,
					const std::string& value);
  static void unsupportedCssValue(Property property,
				  const std::string& value);
  static void unsupportedElement(const std::string& tag);

  friend class Line;
};

  }
}

#endif // RENDER_BLOCK_H_
