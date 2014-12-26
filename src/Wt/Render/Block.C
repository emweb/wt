/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// #define DEBUG_LAYOUT

#include "Wt/WFontMetrics"
#include "Wt/WLogger"
#include "Wt/WPaintDevice"
#include "Wt/WPainter"
#include "Wt/Render/WTextRenderer"

#include "Block.h"
#include "CssParser.h"
#include "Line.h"
#include "WebUtils.h"
#include "RenderUtils.h"
#include "DomElement.h"

#include <boost/algorithm/string.hpp>

using namespace Wt::rapidxml;

namespace {
  const double MARGINX = -1;
  const double EPSILON = 1e-4;
  bool isEpsilonMore(double x, double limit) {
    return x - EPSILON > limit;
  }

  bool isEpsilonLess(double x, double limit) {
    return x + EPSILON < limit;
  }
}

namespace Wt {

LOGGER("Render.Block");

  namespace Render {

int sideToIndex(Wt::Side side)
{
  switch (side) {
  case Wt::Top: return 0;
  case Wt::Right: return 1;
  case Wt::Bottom: return 2;
  case Wt::Left: return 3;
  default:
    throw WException("Unexpected side: " + 
		     boost::lexical_cast<std::string>(side));
  }
}

double sum(const std::vector<double>& v)
{
  double result = 0;
  for (unsigned i = 0; i < v.size(); ++i)
    result += v[i];
  return result;
}

Block::Block(xml_node<> *node, Block *parent)
  : node_(node),
    parent_(parent),
    type_(DomElement_UNKNOWN),
#ifndef WT_TARGET_JAVA
    float_(None),
#endif
    inline_(false),
    currentTheadBlock_(0),
    currentWidth_(0),
    contentsHeight_(0),
    styleSheet_(0)
{
  if (node) {
    if (Render::Utils::isXMLElement(node)) {
      type_ = DomElement::parseTagName(node->name());
      if (type_ == DomElement_UNKNOWN) {
	LOG_ERROR("unsupported element: " << node->name());
	type_ = DomElement_DIV;
      }

      std::string s = attributeValue("class");
      boost::split(classes_, s, boost::is_any_of(" "));
    }

    Render::Utils::fetchBlockChildren(node, this, children_);
  }
}

Block::~Block()
{
  for (unsigned i = 0; i < children_.size(); ++i)
    delete children_[i];
}

std::string Block::id() const
{
  return attributeValue("id");
}

bool Block::isWhitespace(char c)
{
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

bool Block::isText() const
{
  return (node_ && children_.empty() && type_ == DomElement_UNKNOWN)
    || type_ == DomElement_LI;
}

std::string Block::text() const
{
  if (type_ == DomElement_LI)
    return generateItem().toUTF8();
  else
    return Render::Utils::nodeValueToString(node_);
}

void Block::collectStyles(WStringStream& ss)
{
  for (unsigned int i = 0; i < children_.size(); ++i) {
    if (children_[i]->type_ == DomElement_STYLE) {
      ss << Render::Utils::nodeValueToString(children_[i]->node_);
      children_.erase(children_.begin() + i);
      --i;
    } else
      children_[i]->collectStyles(ss);
  }
}

void Block::setStyleSheet(StyleSheet* styleSheet)
{
  styleSheet_ = styleSheet;
  css_.clear();
  for (unsigned int i = 0; i < children_.size(); ++i)
    children_[i]->setStyleSheet(styleSheet);
}

void Block::determineDisplay()
{
  std::string fl = cssProperty(PropertyStyleFloat);

  if (!fl.empty()) {
    if (fl == "left")
      float_ = Left;
    else if (fl == "right")
      float_ = Right;
    else {
      unsupportedCssValue(PropertyStyleFloat, fl);
    }
  } else if (type_ == DomElement_IMG || isTable()) {
    std::string align = attributeValue("align");
    if (!align.empty()) {
      if (align == "left")
	float_ = Left;
      else if (align == "right")
	float_ = Right;
      else
	unsupportedAttributeValue("align", align);
    }
  }

  /*
     Invariant:
      - children in a block element are either all block elements
        or all inline elements
      - children in an inline element are all inline elements

     A text element is inline unless:
      - it contains one block element
      - it is contained in a block element whose children are block elements
   */

  bool allChildrenInline = true;
  for (unsigned i = 0; i < children_.size(); ++i) {
    Block *b = children_[i];
    b->determineDisplay();
    if (!b->isFloat() && !b->isInline())
      allChildrenInline = false;
  }

  if (!allChildrenInline) {
    int firstToGroup = -1;
    for (unsigned i = 0; i <= children_.size(); ++i) {
      Block *b = i < children_.size() ? children_[i] : 0;

      if (!b || !b->isFloat()) {
	if (b && b->inline_ && firstToGroup == -1)
	  firstToGroup = i;

	if ((!b || !b->inline_)
	    && firstToGroup != -1 && (int)i > firstToGroup - 1) {
	  Block *anonymous = new Block(0, this);
	  children_.insert(children_.begin() + i, anonymous);

	  anonymous->inline_ = false;
	  for (unsigned j = firstToGroup; j < i; ++j) {
	    anonymous->children_.push_back(children_[firstToGroup]);
	    children_.erase(children_.begin() + firstToGroup);
	  }

	  i -= i - firstToGroup;

	  firstToGroup = -1;
	}
      }
    }
  }

  switch (type_) {
  case DomElement_UNKNOWN:
    if (allChildrenInline)
      inline_ = true;
    break;
  default:
    if (!isFloat()) {
      std::string display = cssProperty(PropertyStyleDisplay);
      if (!display.empty())
	if (display == "inline")
	  inline_ = true;
	else if (display == "block")
	  inline_ = false;
	else {
	  LOG_ERROR("display '" << display << "' is not supported.");
	  inline_ = false;
	}
      else
	inline_ = DomElement::isDefaultInline(type_);

      if (inline_ && !allChildrenInline)
	LOG_ERROR("inline element " << DomElement::tagName(type_) <<
		  " cannot contain block elements");
    } else
      inline_ = false;
  }

  if (type_ == DomElement_TABLE) {
    std::vector<int> rowSpan;

    int row = numberTableCells(0, rowSpan);
    int maxRowSpan = 0;
    for (unsigned i = 0; i < rowSpan.size(); ++i)
      maxRowSpan = std::max(maxRowSpan, rowSpan[i]);

    tableRowCount_ = row + maxRowSpan;
    tableColCount_ = rowSpan.size();
  }
}

/*
 * Normalizes whitespace inbetween nodes.
 */
bool Block::normalizeWhitespace(bool haveWhitespace, 
				Wt::rapidxml::xml_document<> &doc)
{
  bool whitespaceIn = haveWhitespace;

  if (!isInline())
    haveWhitespace = true;

  if (type_ == DomElement_UNKNOWN && isText()) {
    haveWhitespace = Render::Utils::normalizeWhitespace(this, 
							node_, 
							haveWhitespace, 
							doc);
  } else {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *b = children_[i];
      haveWhitespace = b->normalizeWhitespace(haveWhitespace, doc);
    }
  }

  if (!isInline())
    return whitespaceIn;
  else
    return haveWhitespace;
}

int Block::numberTableCells(int row, std::vector<int>& rowSpan)
{
  if (   type_ == DomElement_TABLE
      || type_ == DomElement_TBODY
      || type_ == DomElement_THEAD
      || type_ == DomElement_TFOOT) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      row = c->numberTableCells(row, rowSpan);
    }
  } else if (type_ == DomElement_TR) {
    int col = 0;

    cellRow_ = row;

    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      if (c->isTableCell()) {
	/* skip overspanned cells by previous rows */
	while (col < (int)rowSpan.size() && rowSpan[col] > 0)
	  ++col;

	c->cellCol_ = col;
	c->cellRow_ = row;

	int rs = c->attributeValue("rowspan", 1);
	int cs = c->attributeValue("colspan", 1);

	while ((int)rowSpan.size() <= col + cs - 1)
	  rowSpan.push_back(1);

	for (int k = 0; k < cs; ++k)
	  rowSpan[col + k] = rs;

	col += cs;
      }
    }

    for (unsigned i = 0; i < rowSpan.size(); ++i)
      if (rowSpan[i] > 0)
	rowSpan[i] = rowSpan[i] - 1;

    ++row;
  }

  return row;
}

bool Block::inlineChildren() const
{
  if (inline_)
    return true;

  for (unsigned i = 0; i < children_.size(); ++i)
    if (!children_[i]->isFloat() && children_[i]->inline_)
      return true;

  return false;
}
 
AlignmentFlag Block::horizontalAlignment() const
{
  /* Based on CSS margins */
  std::string marginLeft = cssProperty(PropertyStyleMarginLeft);
  std::string marginRight = cssProperty(PropertyStyleMarginRight);

  if (marginLeft == "auto") {
    if (marginRight == "auto")
      return AlignCenter;
    else
      return AlignRight;
  } else {
    if (marginRight == "auto")
      return AlignLeft;
    else
      return AlignJustify;
  }
}

AlignmentFlag Block::verticalAlignment() const
{
  std::string va = cssProperty(PropertyStyleVerticalAlign);
  if (va.empty())
    va = attributeValue("valign");

  if (va.empty() || va == "middle")
    return AlignMiddle;
  else if (va == "bottom")
    return AlignBottom;
  else
    return AlignTop;
}

Block::CssLength Block::cssLength(Property top, Side side, 
				  double fontScale) const
{
  Block::CssLength result;

  if (!node_) {
    result.defined = false;
    result.length = 0;
    return result;
  }

  int index = sideToIndex(side);
  Property property = (Property)(top + index);

  std::string value = cssProperty(property);

  if (!value.empty()) {
    WLength l(value.c_str());
    result.defined = true;
    result.length = l.toPixels(cssFontSize(fontScale));
    return result;
  } else {
    result.defined = false;
    result.length = 0;
    return result;
  }
}

double Block::cssPadding(Side side, double fontScale) const
{
  Block::CssLength result = cssLength(PropertyStylePaddingTop, side, fontScale);

  if (!result.defined) {
    if (isTableCell())
      return 1;
    else if ((type_ == DomElement_UL || type_ == DomElement_OL) && side == Left)
      return 40;
  }

  return result.length;
}

double Block::cssMargin(Side side, double fontScale) const
{
  CssLength result;
  result.length = 0;

  if (type_ == DomElement_TD)
    return 0;

  try {
    result = cssLength(PropertyStyleMarginTop, side, fontScale);
  } catch (std::exception& e) {
    /* catches 'auto' margin length */
  }

  if (!result.defined) {
    if (side == Top || side == Bottom) {
      if (   type_ == DomElement_H4
	  || type_ == DomElement_P
	  /* || type_ == DomElement_BLOCKQUOTE */
	  || type_ == DomElement_FIELDSET
	  || type_ == DomElement_FORM
	  /* || type_ == DomElement_DL */
	  /* || type_ == DomElement_DIR */
	  /* || type_ == DomElement_MENU */)
	return 1.12 * cssFontSize(fontScale);
      else if (type_ == DomElement_UL
	       || type_ == DomElement_OL) {
	if (!(isInside(DomElement_UL) || isInside(DomElement_OL)))
	  return 1.12 * cssFontSize(fontScale);
	else
	  return 0;
      } else if (type_ == DomElement_H1)
	return 0.67 * cssFontSize(fontScale);
      else if (type_ == DomElement_H2)
	return 0.75 * cssFontSize(fontScale);
      else if (type_ == DomElement_H3)
	return 0.83 * cssFontSize(fontScale);
      else if (type_ == DomElement_H5)
	return 1.5 * cssFontSize(fontScale);
      else if (type_ == DomElement_H6)
	return 1.67 * cssFontSize(fontScale);
      else if (type_ == DomElement_HR)
	return 0.5 * cssFontSize(fontScale);
    }
  }

  return result.length;
}

bool Block::isInside(DomElementType type) const
{
  if (parent_) {
    if (parent_->type_ == type)
      return true;
    else
      return parent_->isInside(type);
  } else
    return false;
}

double Block::cssBorderSpacing(double fontScale) const
{
  if (tableCollapseBorders())
    return 0;

  std::string spacingStr = cssProperty(PropertyStyleBorderSpacing);

  if (!spacingStr.empty()) {
    WLength l(spacingStr.c_str());
    return l.toPixels(cssFontSize(fontScale));
  } else
    return attributeValue("cellspacing", 2);
}

Block *Block::table() const
{
  Block *result = parent_;
  while (result && !result->isTable())
    result = result->parent_;
  return result;
}

bool Block::tableCollapseBorders() const
{
  return cssProperty(PropertyStyleBorderCollapse) == "collapse";
}

double Block::cssBorderWidth(Side side, double fontScale) const
{
  if (isTableCell()) {
    Block *t = table();
    if (t && t->tableCollapseBorders())
      return collapsedBorderWidth(side, fontScale);
    else
      return rawCssBorderWidth(side, fontScale);
  } else if (isTable() && tableCollapseBorders())
    return collapsedBorderWidth(side, fontScale);
  else
    return rawCssBorderWidth(side, fontScale);
}

Block *Block::findTableCell(int row, int col) const
{
  if (   type_ == DomElement_TABLE
      || type_ == DomElement_TBODY
      || type_ == DomElement_THEAD
      || type_ == DomElement_TFOOT) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      Block *result = c->findTableCell(row, col);
      if (result)
	return result;
    }

    return 0;
  } else if (type_ == DomElement_TR) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      if (c->isTableCell()) {
	int rs = c->attributeValue("rowspan", 1);
	int cs = c->attributeValue("colspan", 1);

	if (row >= c->cellRow_ && row < c->cellRow_ + rs &&
	    col >= c->cellCol_ && col < c->cellCol_ + cs)
	  return c;
      }
    }

    return 0;
  }

  return 0;
}

Block *Block::siblingTableCell(Side side) const
{
  Block *t = table();

  switch (side) {
  case Left:
    if (cellCol_ == 0)
      return 0;
    else
      return t->findTableCell(cellRow_, cellCol_ - 1);
  case Right: {
    int nextCol = cellCol_ + attributeValue("colspan", 1);
    if (nextCol >= t->tableColCount_)
      return 0;
    else
      return t->findTableCell(cellRow_, nextCol);
  }
  case Top:
    if (cellRow_ == 0)
      return 0;
    else
      return t->findTableCell(cellRow_ - 1, cellCol_);
  case Bottom: {
    int nextRow = cellRow_ + attributeValue("rowspan", 1);
    if (nextRow >= t->tableRowCount_)
      return 0;
    else
      return t->findTableCell(nextRow, cellCol_);
  }
  default:
    break;
  }

  return 0;
}

Block::BorderElement Block::collapseCellBorders(Side side) const
{
  std::vector<BorderElement> elements;
  elements.reserve(2);

  Block *s = siblingTableCell(side);
  Block *t = table();

  switch (side) {
  case Left:
    if (s) {
      elements.push_back(BorderElement(s, Right));
      elements.push_back(BorderElement(this, Left));
    } else {
      elements.push_back(BorderElement(this, Left));
      elements.push_back(BorderElement(t, Left));
    }

    break;
  case Top:
    if (s) {
      elements.push_back(BorderElement(s, Bottom));
      elements.push_back(BorderElement(this, Top));
    } else {
      elements.push_back(BorderElement(this, Top));
      elements.push_back(BorderElement(t, Top));
    }

    break;
  case Right:
    elements.push_back(BorderElement(this, Right));
    if (s)
      elements.push_back(BorderElement(s, Left));
    else
      elements.push_back(BorderElement(t, Right));
    
    break;
  case Bottom:
    elements.push_back(BorderElement(this, Bottom));
    if (s)
      elements.push_back(BorderElement(s, Top));
    else
      elements.push_back(BorderElement(t, Bottom));
  default:
    break;
  }

  double borderWidth = 0;
  BorderElement result;

  for (unsigned i = 0; i < elements.size(); ++i) {
    double c = elements[i].block->rawCssBorderWidth(elements[i].side, 1, true);
    if (c == -1)
      return elements[i];

    if (c > borderWidth) {
      result = elements[i];
      borderWidth = std::max(borderWidth, c);
    }
  }

  if (!result.block) {
    /*
      Disabled because I can't understand when a browser does this or not,
      and tinyMCE annoyingly alwasy puts this border attribute.
     
    bool tableDefault = t->attributeValue("border", 0) > 0;
    if (tableDefault)
      return BorderElement(t, side);
    else

     */
    return elements[0];
  } else
    return result;
}

double Block::collapsedBorderWidth(Side side, double fontScale) const
{
  /* 17.6.2.1 border conflict resolution */
  assert (isTable() || isTableCell());

  if (isTable()) {
    // we should examine all bordering cells and take the widest to
    // accurately reflect the table width/height, but let's not bother (yet)

    return 0;
  }

  BorderElement be = collapseCellBorders(side);
  return be.block->rawCssBorderWidth(be.side, fontScale);
}

double Block::rawCssBorderWidth(Side side, double fontScale,
				bool indicateHidden) const
{
  if (!node_)
    return 0;

  int index = sideToIndex(side);
  Property property = (Property)(PropertyStyleBorderTop + index);

  std::string borderStr = cssProperty(property);
  std::string borderWidthStr;

  if (!borderStr.empty()) {
    std::vector<std::string> values;
    boost::split(values, borderStr, boost::is_any_of(" "));

    if (values.size() > 1 && values[1] == "hidden") {
      if (indicateHidden)
	return -1;
      else
	return 0;
    }

    borderWidthStr = values[0];
  }

  if (borderWidthStr.empty()) {
    property = (Property)(PropertyStyleBorderWidthTop + index);
    borderWidthStr = cssProperty(property);
  }

  double result = 0;

  if (!borderWidthStr.empty()) {
    WLength l(borderWidthStr.c_str());
    result = l.toPixels(cssFontSize(fontScale));
  }

  if (result == 0) {
    if (isTable()) {
      result = attributeValue("border", 0) ? 1 : 0;
    } else if (isTableCell()) {
      /*
       * If the table has a 'border' itself, then we have a default
       * border of 1px (note Firefox and Chrome disagree on the color
       * of that border)
       */
      Block *t = table();
      if (t && !t->tableCollapseBorders())
	result = t->attributeValue("border", 0) ? 1 : 0;
    } else if (type_ == DomElement_HR)
      result = 1;
  }

  return result;
}

WColor Block::cssBorderColor(Side side) const
{
  if (isTableCell()) {
    Block *t = table();
    if (t && t->tableCollapseBorders())
      return collapsedBorderColor(side);
    else
      return rawCssBorderColor(side);
  } else if (isTable() && tableCollapseBorders())
    return collapsedBorderColor(side);
  else
    return rawCssBorderColor(side);
}

WColor Block::collapsedBorderColor(Side side) const
{
  /* 17.6.2.1 border conflict resolution */
  assert (isTable() || isTableCell());

  if (isTable()) {
    /* do not actually render the border around the table */
    return WColor();
  }

  BorderElement be = collapseCellBorders(side);
  return be.block->rawCssBorderColor(be.side);
}

WColor Block::rawCssBorderColor(Side side) const
{
  int index = sideToIndex(side);
  Property property = (Property)(PropertyStyleBorderTop + index);

  std::string borderStr = cssProperty(property);
  std::string borderColorStr;

  if (!borderStr.empty()) {
    std::vector<std::string> values;
    boost::split(values, borderStr, boost::is_any_of(" "));

    if (values.size() > 2)
      borderColorStr = values[2];
  }

  if (borderColorStr.empty()) {
    property = (Property)(PropertyStyleBorderColorTop + index);
    borderColorStr = cssProperty(property);
  }

  if (!borderColorStr.empty())
    return WColor(WString::fromUTF8(borderColorStr));

  return black;
}

WColor Block::cssColor() const
{
  std::string color = inheritedCssProperty(PropertyStyleColor);

  if (!color.empty())
    return WColor(WString::fromUTF8(color));
  else
    return black;
}

double Block::cssBoxMargin(Side side, double fontScale) const
{
  return cssPadding(side, fontScale)
    + cssMargin(side, fontScale)
    + cssBorderWidth(side, fontScale);
}

WFont::Style Block::cssFontStyle() const
{
  if (!node_ && parent_)
    return parent_->cssFontStyle();

  std::string v = cssProperty(PropertyStyleFontStyle);

  if (v.empty() &&
      (type_ == DomElement_EM || type_ == DomElement_I))
    return WFont::Italic;
  else if (v == "normal")
    return WFont::NormalStyle;
  else if (v == "italic")
    return WFont::Italic;
  else if (v == "oblique")
    return WFont::Oblique;
  else
    if (parent_)
      return parent_->cssFontStyle();
    else
      return WFont::NormalStyle;
}

int Block::cssFontWeight() const
{
  if (!node_ && parent_)
    return parent_->cssFontWeight();

  std::string v = cssProperty(PropertyStyleFontWeight);

  if (v.empty() && (type_ == DomElement_B
		    || type_ == DomElement_STRONG
		    || type_ == DomElement_TH
		    || (type_ >= DomElement_H1
			&& type_ <= DomElement_H6)))
    v = "bolder";

  if (!v.empty() && v != "bolder" && v != "lighter") {
    if (v == "normal")
      return 400;
    else if (v == "bold")
      return 700;
    else {
      try {
	return boost::lexical_cast<int>(v);
      } catch (boost::bad_lexical_cast& blc) {
      }
    }
  }

  int parentWeight = parent_ ? parent_->cssFontWeight() : 400;

  if (v == "bolder") {
    if (parentWeight < 300)
      return 400;
    else if (parentWeight < 600)
      return 700;
    else
      return 900;
  } else if (v == "lighter") {
    if (parentWeight < 600)
      return 100;
    else if (parentWeight < 800)
      return 400;
    else
      return 700;
  } else
    return parentWeight;
  }

double Block::cssFontSize(double fontScale) const
{
  if (!node_ && parent_)
    return fontScale * parent_->cssFontSize();

  std::string v = cssProperty(PropertyStyleFontSize);

  double Medium = 16;
  double parentSize = parent_ ? parent_->cssFontSize() : Medium;

  double result;

  if (!v.empty()) {
    if (v == "xx-small")
      result = Medium / 1.2 / 1.2 / 1.2;
    else if (v == "x-small")
      result = Medium / 1.2 / 1.2;
    else if (v == "small")
      result = Medium / 1.2;
    else if (v == "medium")
      result = Medium;
    else if (v == "large")
      result = Medium * 1.2;
    else if (v == "x-large")
      result = Medium * 1.2 * 1.2;
    else if (v == "xx-large")
      result = Medium * 1.2 * 1.2 * 1.2;
    else if (v == "larger")
      result = parentSize * 1.2;
    else if (v == "smaller")
      result = parentSize / 1.2;
    else {
      WLength l(v.c_str());

      if (l.unit() == WLength::Percentage)
	result = parentSize * l.value() / 100;
      else if (l.unit() == WLength::FontEm)
	result = parentSize * l.value();
      else
	result = l.toPixels();
    }
  } else {
    if (type_ == DomElement_H1)
      result = parentSize * 2;
    else if (type_ == DomElement_H2)
      result = parentSize * 1.5;
    else if (type_ == DomElement_H3)
      result = parentSize * 1.17;
    else if (type_ == DomElement_H5)
      result = parentSize * 0.83;
    else if (type_ == DomElement_H6)
      result = parentSize * 0.75;
    else
      result = parentSize;
  }

  return result * fontScale;
}

double Block::cssLineHeight(double fontLineHeight, double fontScale) const
{
  if (!node_ && parent_)
    return parent_->cssLineHeight(fontLineHeight, fontScale);

  std::string v = cssProperty(PropertyStyleLineHeight);

  if (!v.empty()) {
    if (v == "normal")
      return fontLineHeight;
    else {
      try {
	return boost::lexical_cast<double>(v);
      } catch (boost::bad_lexical_cast& e) {
	WLength l(v.c_str());

	if (l.unit() == WLength::Percentage)
	  return cssFontSize(fontScale) * l.value() / 100;
	else
	  return l.toPixels(parent_->cssFontSize(fontScale));
      }
    }
  } else {
    if (parent_)
      return parent_->cssLineHeight(fontLineHeight, fontScale);
    else
      return fontLineHeight;
  }
}

void Block::inlinePageBreak(const std::string& pageBreak,
			    Line& line, BlockList& floats,
			    double minX, double maxX,
			    const WTextRenderer& renderer)
{
  if (pageBreak == "always") {
    if (inlineLayout.empty()) {
      inlineLayout.push_back(InlineBox());
      InlineBox& b = inlineLayout.back();

      b.page = line.page();
      b.x = line.x();
      b.width = 1;
      b.y = line.y();
      b.height = 1;
      b.baseline = 0;

      b.utf8Count = 0;
      b.utf8Pos = 0;
      b.whitespaceWidth = 0;

      line.adjustHeight(1, 0, 0);
      line.setX(line.x() + b.width);
      line.addBlock(this);
    }

    line.newLine(minX, line.y() + line.height(), line.page());
    line.moveToNextPage(floats, minX, maxX, renderer);
  }
}

double Block::layoutInline(Line& line, BlockList& floats,
			   double minX, double maxX, bool canIncreaseWidth,
			   const WTextRenderer& renderer)
{
  inlineLayout.clear();

  inlinePageBreak(cssProperty(PropertyStylePageBreakBefore),
		  line, floats, minX, maxX, renderer);

  if (isText() || type_ == DomElement_IMG || type_ == DomElement_BR) {
    std::string s;
    unsigned utf8Pos = 0, utf8Count = 0;
    double whitespaceWidth = 0;

    renderer.painter()->setFont(cssFont(renderer.fontScale()));

    WPaintDevice *device = renderer.painter()->device();
    WFontMetrics metrics = device->fontMetrics();

    double lineHeight = cssLineHeight(metrics.height(), renderer.fontScale());
    double fontHeight = metrics.size();
    double baseline = (lineHeight - fontHeight)/2.0 + metrics.ascent();

    if (isText()) {
      s = text();
      whitespaceWidth = device->measureText(WString::fromUTF8(" ")).width();
    }

    for (;;) {
      Range rangeX(minX, maxX);
      adjustAvailableWidth(line.y(), line.page(), floats, rangeX);

      if (rangeX.start > line.x())
	line.setX(rangeX.start);

      double w = 0, h = 0;
      bool lineBreak = false;

      if (isText()) {
	/*
	 * Skip whitespace at the start of a line
	 */
	if (utf8Pos < s.length()
	    && line.x() == rangeX.start
	    && isWhitespace(s[utf8Pos]))
	  ++utf8Pos;

	if (utf8Pos < s.length()) {
	  double maxWidth;

	  if (canIncreaseWidth)
	    maxWidth = std::numeric_limits<double>::max();
	  else
	    maxWidth = rangeX.end - line.x();

	  WString text = WString::fromUTF8(s.substr(utf8Pos));

	  WTextItem item
	    = renderer.painter()->device()->measureText(text, maxWidth, true);

	  utf8Count = item.text().toUTF8().length();

	  w = item.width();

	  /*
	   * The measured text width excludes the trailing space, add
	   * this is so that it matches the measured word width.
	   */
	  if (utf8Count > 0
	      && utf8Pos + utf8Count < s.length()
	      && isWhitespace(s[utf8Pos + utf8Count - 1])) {
	    w += whitespaceWidth;
	    rangeX.end += whitespaceWidth; // to avoid an artificial overflow
	  }

	  if (canIncreaseWidth &&
	      isEpsilonMore(item.width(), rangeX.end - line.x())) {
	    maxX += w - (rangeX.end - line.x());
	    rangeX.end += w - (rangeX.end - line.x());
	  }

	  if (w == 0) {
	    /*
	     * Start a new line
	     */
	    lineBreak = true;

	    /*
	     * We need at least room for one word.
	     */
	    if (line.x() == rangeX.start) {
	      if (item.nextWidth() < 0) {
		for (unsigned i = utf8Pos; i <= s.length(); ++i) {
		  if (i == s.length() || isWhitespace(s[i])) {
		    WString word = s.substr(utf8Pos, i - utf8Pos);
		    double wordWidth = device->measureText(word).width();

		    w = wordWidth;

		    break;
		  }
		}
	      } else
		w = item.nextWidth();
	    }
	  } else {
	    if (utf8Count <= 0)
	      throw WException("Internal error: utf8Count <= 0!");
	    h = fontHeight;
	  }
	} else
	  break; // All text has been processed
      } else if (type_ == DomElement_BR) {
	if (inlineLayout.empty()) {
	  inlineLayout.push_back(InlineBox());

	  lineBreak = true;

	  line.adjustHeight(fontHeight, baseline, lineHeight);
	} else {
	  inlineLayout.clear();
	  break;
	}
      } else { // type_ == DomElement_IMG
	w = cssWidth(renderer.fontScale());
	h = cssHeight(renderer.fontScale());

	std::string src = attributeValue("src");

	if (w <= 0 || h <= 0) {
	  WPainter::Image image(src, src);

	  if (w <= 0)
	    w = image.width();

	  if (h <= 0)
	    h = image.height();
	}

	w += cssBoxMargin(Left, renderer.fontScale())
	  + cssBoxMargin(Right, renderer.fontScale());
	h += cssBoxMargin(Top, renderer.fontScale())
	  + cssBoxMargin(Bottom, renderer.fontScale());

	std::string va = cssProperty(PropertyStyleVerticalAlign);

	if (va == "middle")
	  baseline = h/2 + fontHeight / 2;
	else if (va == "text-top") {
	  // keep font-baseline
	} else
	  baseline = h;
      }

      if (lineBreak || isEpsilonMore(w, rangeX.end - line.x())) {
	/*
	 * Content does not fit on this line.
	 */
	line.setLineBreak(type_ == DomElement_BR);
	line.finish(cssTextAlign(), floats, minX, maxX, renderer);

	if (w == 0 || line.x() > rangeX.start) {
	  if (w > 0 && canIncreaseWidth) {
	    maxX += w - (maxX - line.x());
	    rangeX.end += w - (maxX - line.x());
	  }

	  /*
	   * Desired width is 0 or the thing didn't fit on a started line:
	   * we should simply break the line.
	   */
	  line.newLine(minX, line.y() + line.height(), line.page());
	  h = 0;
	} else if (isEpsilonMore(w, maxX - minX)) {
	  /*
	   * Wider than the box width without floats
	   */
	  maxX += w - (maxX - minX);
	  rangeX.end += w - (maxX - minX);
	} else {
	  /*
	   * Not wider than the box width without floats:
	   * clear the blocking float and see if it will then fit.
	   */
	  PageState linePs;
	  linePs.y = line.y();
	  linePs.page = line.page();
	  linePs.minX = minX;
	  linePs.maxX = maxX;
	  linePs.floats = floats;
	  clearFloats(linePs, w);
	  line.newLine(linePs.minX, linePs.y, linePs.page);
	}

	h = 0;
      }

      if (w > 0 && h > 0) {
	inlineLayout.push_back(InlineBox());
	InlineBox& b = inlineLayout.back();

	double marginLeft = 0, marginRight = 0, marginBottom = 0, marginTop = 0;
	if (type_ == DomElement_IMG) {
	  marginLeft = cssMargin(Left, renderer.fontScale());
	  marginRight = cssMargin(Right, renderer.fontScale());
	  marginBottom = cssMargin(Bottom, renderer.fontScale());
	  marginTop = cssMargin(Top, renderer.fontScale());
	}

	b.page = line.page();
	b.x = line.x() + marginLeft;
	b.width = w - marginLeft - marginRight;
	b.y = line.y() + marginTop;
	b.height = h - marginTop - marginBottom;
	b.baseline = baseline - marginTop - marginBottom;

	b.utf8Count = utf8Count;
	b.utf8Pos = utf8Pos;
	b.whitespaceWidth = whitespaceWidth;

	utf8Pos += utf8Count;

	line.adjustHeight(h, baseline, lineHeight);
	line.setX(line.x() + w);
	line.addBlock(this);

	if (line.bottom() >= renderer.textHeight(line.page()))
	  line.moveToNextPage(floats, minX, maxX, renderer);

	if (!isText() || (isText() && utf8Pos == s.length()))
	  break;
      }
    }
  }

  if (inlineChildren()) {
    if (type_ == DomElement_LI) {
      inlineLayout[0].x = MARGINX; // move it to the the margin
      line.setX(minX);
    }

    if (!children_.empty()) {
      // FIXME, we should somehow take into account left/right box margin
      // This can only be done here since only actual elements like <span>
      // can have margins, borders and paddings

      for (unsigned i = 0; i < children_.size(); ++i) {
	Block *c = children_[i];

	if (c->isFloat()) {
	  maxX = c->layoutFloat(line.y(), line.page(), floats, line.x(),
				line.height(), 
				minX, maxX, canIncreaseWidth, renderer);

	  line.reflow(c);
	  line.addBlock(c);
	} else if (c->isPositionedAbsolutely()) {
	  if (!c->offsetParent_)
	    c->setOffsetParent();

	  /*
	   * A hypothetical static layout, good enough for LTR text
	   */
	  c->inlineLayout.clear();
	  c->inlineLayout.push_back(InlineBox());
	  InlineBox& box = c->inlineLayout.back();
	  box.page = line.page();
	  box.x = line.x();
	  box.y = line.y();
	  box.width = 0;
	  box.height = 0;
	  /* other fields are unused */
	} else
	  maxX = c->layoutInline(line, floats, minX, maxX, canIncreaseWidth,
				 renderer);
      }
    }
  }

  if (isInline())
    for (unsigned i = 0; i < offsetChildren_.size(); ++i)
      offsetChildren_[i]->layoutAbsolute(renderer);

  inlinePageBreak(cssProperty(PropertyStylePageBreakAfter),
		  line, floats, minX, maxX, renderer);

  return maxX;
}

void Block::layoutTable(PageState &ps,
			bool canIncreaseWidth,
			const WTextRenderer& renderer,
			double cssSetWidth)
{
  // used to calculate minimumColumnWidths
  currentWidth_ = std::max(0.0, cssSetWidth);

  std::vector<double> minimumColumnWidths;
  std::vector<double> maximumColumnWidths;
  std::vector<double> setColumnWidths;

  for (unsigned i = 0; i < children_.size(); ++i) {
    Block *c = children_[i];
    c->tableComputeColumnWidths(minimumColumnWidths, maximumColumnWidths,
				setColumnWidths, renderer, this);
  }

  currentWidth_ = 0;

  unsigned colCount = minimumColumnWidths.size();

  for (unsigned i = 0; i < colCount; ++i) {
    if (setColumnWidths[i] >= 0) {
      setColumnWidths[i] = std::max(setColumnWidths[i], minimumColumnWidths[i]);
      maximumColumnWidths[i] = minimumColumnWidths[i] = setColumnWidths[i];
    }
  }

  double cellSpacing = cssBorderSpacing(renderer.fontScale());
  double totalSpacing = (colCount + 1) * cellSpacing;
  double totalMinWidth = sum(minimumColumnWidths) + totalSpacing;
  double totalMaxWidth = sum(maximumColumnWidths) + totalSpacing;

  double desiredMinWidth = std::max(totalMinWidth, cssSetWidth);
  double desiredMaxWidth = totalMaxWidth;

  if (cssSetWidth > 0 && cssSetWidth < desiredMaxWidth)
    desiredMaxWidth = std::max(desiredMinWidth, cssSetWidth);

  double availableWidth;

  for (;;) {
    Range rangeX(ps.minX, ps.maxX);
    adjustAvailableWidth(ps.y, ps.page, ps.floats, rangeX);
    ps.maxX = rangeX.end;

    double border = cssBorderWidth(Left, renderer.fontScale())
      + cssBorderWidth(Right, renderer.fontScale());
    availableWidth = rangeX.end - rangeX.start - border;

    /*
     * If we can increase the available width without clearing floats
     * to fit the table at its widest, then try so
     */
    if (canIncreaseWidth && isEpsilonLess(availableWidth, desiredMaxWidth)) {
      ps.maxX += desiredMaxWidth - availableWidth;
      availableWidth = desiredMaxWidth;
    }

    if (!isEpsilonLess(availableWidth, desiredMinWidth))
      break;
    else {
      if (isEpsilonLess(desiredMinWidth, ps.maxX - ps.minX - border)) {
	clearFloats(ps, desiredMinWidth);
      } else {
	ps.maxX += desiredMinWidth - availableWidth;
	availableWidth = desiredMinWidth;
      }
    }
  }

  double width = desiredMinWidth;

  if (width <= availableWidth) {
    if (desiredMaxWidth > availableWidth)
      width = availableWidth;
    else
      width = std::max(desiredMaxWidth, width);
  } else {
    maximumColumnWidths = minimumColumnWidths;
  }

  std::vector<double> widths = minimumColumnWidths;

  if (width > totalMaxWidth) {
    widths = maximumColumnWidths;

    double rWidth = width - totalSpacing;
    double rTotalMaxWidth = totalMaxWidth - totalSpacing;
    for (unsigned i = 0; i < colCount; ++i) {
      if (setColumnWidths[i] >= 0) {
	rWidth -= widths[i];
	rTotalMaxWidth -= widths[i];
      }
    }

    if (rTotalMaxWidth <= 0) {
      rWidth = width - totalSpacing;
      rTotalMaxWidth = totalMaxWidth - totalSpacing;

      for (unsigned i = 0; i < widths.size(); ++i)
	setColumnWidths[i] = -1.0;
    }

    if (rTotalMaxWidth > 0) {
      double factor = rWidth / rTotalMaxWidth;

      for (unsigned i = 0; i < widths.size(); ++i) {
	if (setColumnWidths[i] < 0)
	  widths[i] *= factor;
      }
    } else { /* degenerate case: all columns have width 0 */
      double widthPerColumn = rWidth / colCount;

      for (unsigned i = 0; i < colCount; ++i)
	widths[i] = widthPerColumn;
    }
  } else if (width > totalMinWidth) {
    double totalStretch = 0;

    for (unsigned i = 0; i < widths.size(); ++i)
      totalStretch += maximumColumnWidths[i] - minimumColumnWidths[i];

    double room = width - totalMinWidth;
    double factor = room / totalStretch;

    for (unsigned i = 0; i < widths.size(); ++i) {
      double stretch = maximumColumnWidths[i] - minimumColumnWidths[i];
      widths[i] += factor * stretch;
    }
  }

  AlignmentFlag hAlign = horizontalAlignment();

  switch (hAlign) {
  case AlignLeft:
  case AlignJustify:
    ps.maxX = ps.minX + width;
    break;
  case AlignCenter:
    ps.minX = ps.minX + (ps.maxX - ps.minX - width) / 2;
    ps.maxX = ps.minX + width;
    break;
  case AlignRight:
    ps.minX = ps.maxX - width;
    break;
  default:
    break;
  }

  Block *repeatHead = 0;

  for (unsigned i = 0; i < children_.size(); ++i) {
    if (children_[i]->type_ == DomElement_THEAD) {
      // Note: should actually interpret CSS 'table-header-group' value for
      // display
      repeatHead = children_[i];
      break;
    } else if (children_[i]->type_ == DomElement_TBODY ||
	       children_[i]->type_ == DomElement_TR)
      break;
  }
  bool protectRows = repeatHead != 0;

  std::vector<CellState> rowSpanBackLog;
  tableDoLayout(ps.minX, ps, cellSpacing, widths, rowSpanBackLog,
		protectRows, repeatHead, renderer);

  ps.minX -= cssBorderWidth(Left, renderer.fontScale());
  ps.maxX += cssBorderWidth(Right, renderer.fontScale());

  ps.y += cellSpacing;
}

void Block::tableDoLayout(double x, PageState &ps, double cellSpacing,
			  const std::vector<double>& widths,
			  std::vector<CellState>& rowSpanBackLog,
			  bool protectRows, Block *repeatHead,
			  const WTextRenderer& renderer)
{
  if (   type_ == DomElement_TABLE
      || type_ == DomElement_TBODY
      || type_ == DomElement_THEAD
      || type_ == DomElement_TFOOT) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      c->tableDoLayout(x, ps, cellSpacing, widths, rowSpanBackLog,
		       protectRows,
		       (type_ != DomElement_THEAD ? repeatHead : 0),
		       renderer);
    }

    if (repeatHead && type_ == DomElement_THEAD) {
      blockLayout.clear();

      BlockBox bb;
      bb.page = ps.page;
      bb.y = minChildrenLayoutY(ps.page);
      bb.height = childrenLayoutHeight(ps.page);
      bb.x = x;
      bb.width = 0; // Not really used

      blockLayout.push_back(bb);
    }
  } else if (type_ == DomElement_TR) {
    double startY = ps.y;
    int startPage = ps.page;
    tableRowDoLayout(x, ps, cellSpacing, widths, rowSpanBackLog, renderer, -1);

    if (protectRows && ps.page != startPage) {
      ps.y = startY;
      ps.page = startPage;

      pageBreak(ps);

      if (repeatHead) {
	/*
	 * Add a blocklayout element to THEAD
	 * These are handled during render() to repeat the head element
	 */
	BlockBox bb;
	bb.page = ps.page;
	bb.y = ps.y;
	bb.height = repeatHead->blockLayout[0].height;
	bb.x = x;
	bb.width = 0; // not really used

	repeatHead->blockLayout.push_back(bb);

	ps.y += bb.height;
      }

      startY = ps.y;
      startPage = ps.page;
      tableRowDoLayout(x, ps, cellSpacing, widths, rowSpanBackLog,
		       renderer, -1);
    }

    double rowHeight = (ps.page - startPage) 
      * renderer.textHeight(ps.page) // XXX
      + (ps.y - startY) - cellSpacing;

    ps.y = startY;
    ps.page = startPage;
    tableRowDoLayout(x, ps, cellSpacing, widths, rowSpanBackLog,
		     renderer, rowHeight);
  }
}

double Block::tableCellX(const std::vector<double>& widths,
			 double cellSpacing) const
{
  double result = 0;
  for (int j = 0; j < cellCol_; ++j)
    result += widths[j] + cellSpacing;

  return result;
}

double Block::tableCellWidth(const std::vector<double>& widths,
			     double cellSpacing) const
{
  int colSpan = attributeValue("colspan", 1);

  double width = 0;
  for (int j = cellCol_; j < cellCol_ + colSpan; ++j)
    width += widths[j];

  return width + (colSpan - 1) * cellSpacing;
}

void Block::tableCellDoLayout(double x, const PageState& ps,
			      double cellSpacing, PageState& rowEnd,
			      const std::vector<double>& widths,
			      const WTextRenderer& renderer,
			      double rowHeight)
{
  x += tableCellX(widths, cellSpacing);
  double width = tableCellWidth(widths, cellSpacing);

  PageState cellPs;
  cellPs.y = ps.y + cellSpacing;
  cellPs.page = ps.page;
  cellPs.minX = x;
  cellPs.maxX = x + width;

  double collapseMarginBottom = 0;
  double collapseMarginTop = std::numeric_limits<double>::max();

  std::string s = cssProperty(PropertyStyleBackgroundColor);

  collapseMarginBottom = layoutBlock(cellPs, false, renderer,
				     collapseMarginTop, 
				     collapseMarginBottom, 
				     rowHeight);

  if (collapseMarginBottom < collapseMarginTop)
    cellPs.y -= collapseMarginBottom;

  cellPs.minX = x;
  cellPs.maxX = x + width;
  Block::clearFloats(cellPs, width);

  if (cellPs.page > rowEnd.page
      || (cellPs.page == rowEnd.page && cellPs.y > rowEnd.y)) {
    rowEnd.page = cellPs.page;
    rowEnd.y = cellPs.y;
  }
}

void Block::tableRowDoLayout(double x, PageState &ps,
			     double cellSpacing,
			     const std::vector<double>& widths,
			     std::vector<CellState>& rowSpanBackLog,
			     const WTextRenderer& renderer,
			     double rowHeight)
{
  PageState rowEnd;
  rowEnd.y = ps.y;
  rowEnd.page = ps.page;

  if (rowHeight == -1) {
    double height = cssHeight(renderer.fontScale());
    if (height > 0)
      advance(rowEnd, height, renderer);
  }

  x += cellSpacing;

  for (unsigned i = 0; i < children_.size(); ++i) {
    Block *c = children_[i];

    if (c->isTableCell()) {
      int rowSpan = c->attributeValue("rowspan", 1);
      if (rowSpan > 1) {
	if (rowHeight == -1) {
	  CellState cs;
	  cs.lastRow = c->cellRow_ + rowSpan - 1;
	  cs.y = ps.y;
	  cs.page = ps.page;
	  cs.cell = c;
	  rowSpanBackLog.push_back(cs);
	}
      } else
	c->tableCellDoLayout(x, ps, cellSpacing, rowEnd,
			     widths, renderer, rowHeight);
    }
  }

  for (unsigned i = 0; i < rowSpanBackLog.size(); ++i) {
    if (rowSpanBackLog[i].lastRow == cellRow_) {
      const CellState& cs = rowSpanBackLog[i];
      double rh = rowHeight;

      if (rh >= 0)
	rh += (ps.page - cs.page) 
	  * renderer.textHeight(cs.page) // XXX
	  + (ps.y - cs.y);

      cs.cell->tableCellDoLayout(x, cs, cellSpacing, rowEnd,
				 widths, renderer, rh);
    }
  }

  ps.y = rowEnd.y;
  ps.page = rowEnd.page;
}

void Block::tableComputeColumnWidths(std::vector<double>& minima,
				     std::vector<double>& maxima,
				     std::vector<double>& asSet,
				     const WTextRenderer& renderer,
				     Block *table)
{
  /*
   * Current limitations:
   * - we currently ignore column/column group widths
   */
  if (   type_ == DomElement_TBODY
      || type_ == DomElement_THEAD
      || type_ == DomElement_TFOOT) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      c->tableComputeColumnWidths(minima, maxima, asSet, renderer, table);
    }
  } else if (type_ == DomElement_TR) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      if (c->isTableCell()) {
	c->cellComputeColumnWidths(AsSetWidth, asSet, renderer, table);
	c->cellComputeColumnWidths(MinimumWidth, minima, renderer, table);
	c->cellComputeColumnWidths(MaximumWidth, maxima, renderer, table);
      }
    }
  }
}

int Block::attributeValue(const char *attribute, int defaultValue) const
{
  std::string valueStr = attributeValue(attribute);
  if (!valueStr.empty())
    return boost::lexical_cast<int>(valueStr);
  else
    return defaultValue;
}

void Block::cellComputeColumnWidths(WidthType type,
				    std::vector<double>& values,
				    const WTextRenderer& renderer,
				    Block *table)
{
  double currentWidth = 0;

  int col = cellCol_;
  int colSpan = attributeValue("colspan", 1);

  double defaultWidth = 0;
  if (type == AsSetWidth)
    defaultWidth = -1;

  while (col + colSpan > (int)values.size())
    values.push_back(defaultWidth);

  for (int i = 0; i < colSpan; ++i) {
    if (values[col + i] > 0)
      currentWidth += values[col + i];
  }

  double width = currentWidth;

  switch (type) {
  case AsSetWidth:
    width = cssWidth(renderer.fontScale());
    break;
  case MinimumWidth:
  case MaximumWidth:
    {
      PageState ps;
      ps.y = 0;
      ps.page = 0;
      ps.minX = 0;
      ps.maxX = width;

      double origTableWidth = table->currentWidth_;
      if (type == MinimumWidth)
	table->currentWidth_ = 0;

      layoutBlock(ps, type == MaximumWidth, renderer, 0, 0);

      table->currentWidth_ = origTableWidth;

      width = ps.maxX;
    }
  }

  if (width > currentWidth) {
    double extraPerColumn = (width - currentWidth) / colSpan;

    for (int i = 0; i < colSpan; ++i)
      values[col + i] += extraPerColumn;
  }
}

bool Block::isPercentageLength(const std::string& length)
{
  return !length.empty()
    && WLength(length.c_str()).unit() == WLength::Percentage;
}

double Block::cssDecodeLength(const std::string& length,
			      double fontScale, double defaultValue,
			      PercentageRule percentage,
			      double parentSize) const
{
  if (!length.empty()) {
    WLength l(length.c_str());
    if (l.unit() == WLength::Percentage) {
      switch (percentage) {
      case PercentageOfFontSize:
	return l.toPixels(cssFontSize(fontScale));
      case PercentageOfParentSize:
	return l.value() / 100.0 * parentSize;
      case IgnorePercentage:
	return defaultValue;
      }

      return defaultValue;
    } else
      return l.toPixels(cssFontSize(fontScale));
  } else
    return defaultValue;
}

double Block::currentParentWidth() const
{
  if (parent_) {
    switch (parent_->type_) {
    case DomElement_TR:
    case DomElement_TBODY:
    case DomElement_THEAD:
    case DomElement_TFOOT:
      return parent_->currentParentWidth();
    default:
      return parent_->currentWidth_;
    }
  } else
    return 0;
}

double Block::cssWidth(double fontScale) const
{
  double result = -1;

  if (node_) {
    result = cssDecodeLength(cssProperty(PropertyStyleWidth),
			     fontScale, result, PercentageOfParentSize,
			     currentParentWidth());

    if (type_ == DomElement_IMG ||
	type_ == DomElement_TABLE ||
	type_ == DomElement_TD ||
	type_ == DomElement_TH)
      result = cssDecodeLength(attributeValue("width"),
			       fontScale, result, PercentageOfParentSize,
			       currentParentWidth());
  }

  return result;
}

double Block::cssHeight(double fontScale) const
{
  double result = -1;

  if (node_) {
    result = cssDecodeLength(cssProperty(PropertyStyleHeight),
			     fontScale, result, IgnorePercentage);

    if (type_ == DomElement_IMG)
      result = cssDecodeLength(attributeValue("height"),
			       fontScale, result, IgnorePercentage);
  }

  return result;
}

double Block::diff(double y, int page, double startY, int startPage,
		   const WTextRenderer& renderer)
{
  double result = y - startY;

  while (page > startPage) {
    result += renderer.textHeight(page);
    --page;
  }

  return result;
}

void Block::advance(PageState &ps, double height,
		    const WTextRenderer& renderer)
{
  while (ps.y + height > renderer.textHeight(ps.page)) {
    ++ps.page;
    ps.y = 0;
    height -= (renderer.textHeight(ps.page) - ps.y);
    if (height < 0)
      height = 0;
    if ((renderer.textHeight(ps.page) - ps.y) < 0 && height >= 0)
      throw new WException("The margin is too large");
  }

  ps.y += height;
}

void Block::pageBreak(PageState& ps)
{
  clearFloats(ps);

  ++ps.page;
  ps.y = 0;
}

double Block::maxChildrenLayoutY(int page) const
{
  double result = 0;

  for (unsigned i = 0; i < children_.size(); ++i)
    result = std::max(result, children_[i]->maxLayoutY(page));

  return result;
}

double Block::minChildrenLayoutY(int page) const
{
  double result = 1E9;

  for (unsigned i = 0; i < children_.size(); ++i)
    result = std::min(result, children_[i]->minLayoutY(page));

  return result;
}

double Block::maxLayoutY(int page) const
{
  double result = 0;

  for (unsigned i = 0; i < inlineLayout.size(); ++i) {
    const InlineBox& ib = inlineLayout[i];
  
    if (page == -1 || ib.page == page) {
      /* to be accurate, we need font metrics, see renderText() ! */
      result = std::max(result, ib.y + ib.height);
    }
  }

  for (unsigned i = 0; i < blockLayout.size(); ++i) {
    const BlockBox& lb = blockLayout[i];

    if (page == -1 || lb.page == page) {
      /* to be accurate, we need to call toBorderBox()) */
      result = std::max(result, lb.y + lb.height);
    }
  }

  if (inlineLayout.empty() && blockLayout.empty()) {
    for (unsigned i = 0; i < children_.size(); ++i)
      result = std::max(result, children_[i]->maxLayoutY(page));
  }

  return result;
}

double Block::minLayoutY(int page) const
{
  double result = 1E9;

  for (unsigned i = 0; i < inlineLayout.size(); ++i) {
    const InlineBox& ib = inlineLayout[i];
  
    if (page == -1 || ib.page == page) {
      /* to be accurate, we need font metrics, see renderText() ! */
      result = std::min(result, ib.y);
    }
  }

  for (unsigned i = 0; i < blockLayout.size(); ++i) {
    const BlockBox& lb = blockLayout[i];

    if (page == -1 || lb.page == page) {
      /* to be accurate, we need to call toBorderBox()) */
      result = std::min(result, lb.y);
    }
  }

  if (inlineLayout.empty() && blockLayout.empty()) {
    for (unsigned i = 0; i < children_.size(); ++i)
      result = std::min(result, children_[i]->minLayoutY(page));
  }

  return result;
}

double Block::childrenLayoutHeight(int page) const
{
  return maxChildrenLayoutY(page) - minChildrenLayoutY(page);
}

double Block::layoutBlock(PageState &ps,
			  bool canIncreaseWidth,
			  const WTextRenderer& renderer,
			  double collapseMarginTop,
			  double collapseMarginBottom,
			  double cellHeight)
{
  std::string pageBreakBefore = cssProperty(PropertyStylePageBreakBefore);
  if (pageBreakBefore == "always") {
    pageBreak(ps);
    collapseMarginTop = 0;
  }

  double origMinX = ps.minX;
  double origMaxX = ps.maxX;

  double inCollapseMarginTop = collapseMarginTop;
  double inY = ps.y;
  double spacerTop = 0, spacerBottom = 0;

  if (cellHeight >= 0) {
    double ch = contentsHeight_;
    AlignmentFlag va = verticalAlignment();

    switch (va) {
    case AlignTop:
      spacerBottom = cellHeight - ch;
      break;
    case AlignMiddle:
      spacerTop = spacerBottom = (cellHeight - ch) / 2;
      break;
    case AlignBottom:
      spacerTop = cellHeight - ch;
      break;
    default:
      break;
    }
  }

  blockLayout.clear();

  double startY;

  double marginTop = cssMargin(Top, renderer.fontScale());

  ps.y -= std::min(marginTop, collapseMarginTop);

  collapseMarginTop = std::max(marginTop, collapseMarginTop);
  collapseMarginBottom = 0;

  startY = ps.y;

  ps.y += marginTop;

  if (!isFloat())
    startY = ps.y;

  int startPage = ps.page;

  ps.y += cssBorderWidth(Top, renderer.fontScale());

  ps.minX += cssMargin(Left, renderer.fontScale());
  ps.maxX -= cssMargin(Right, renderer.fontScale());

  double cssSetWidth = cssWidth(renderer.fontScale());

  if (isTable()) {
    /*
     * A table will apply the width to it's border box, unlike a block level
     * element which applies the width to it's padding box !
     */
    if (cssSetWidth > 0) {
      cssSetWidth -= cssBorderWidth(Left, renderer.fontScale())
	+ cssBorderWidth(Right, renderer.fontScale());
      cssSetWidth = std::max(0.0, cssSetWidth);
    }

    layoutTable(ps, canIncreaseWidth, renderer, cssSetWidth);
  } else {
    double width = cssSetWidth;

    bool paddingBorderWithinWidth
      = isTableCell() && isPercentageLength(cssProperty(PropertyStyleWidth));

    if (width >= 0) {
      if (!paddingBorderWithinWidth)
	width += cssPadding(Left, renderer.fontScale())
	  + cssBorderWidth(Left, renderer.fontScale())
	  + cssPadding(Right, renderer.fontScale())
	  + cssBorderWidth(Right, renderer.fontScale());

      if (isTableCell()) {
	if (width < (ps.maxX - ps.minX))
	  width = ps.maxX - ps.minX;
	/*
	 * A width set on a td or th should be considered as a desired
	 * width -- this cell should not try to consume excess width
	 */
	canIncreaseWidth = false;
      }

      if (width > (ps.maxX - ps.minX))
	ps.maxX = ps.minX + width;

      AlignmentFlag hAlign = horizontalAlignment();
      switch (hAlign) {
      case AlignJustify:
      case AlignLeft:
	ps.maxX = ps.minX + width;
	break;
      case AlignCenter:
	ps.minX = ps.minX + (ps.maxX - ps.minX - width) / 2;
	ps.maxX = ps.minX + width;
	break;
      case AlignRight:
	ps.minX = ps.maxX - width;
	break;
      default:
	break;
      }
    }

    if (type_ == DomElement_IMG) {
      double height = cssHeight(renderer.fontScale());
      std::string src = attributeValue("src");

      if (width <= 0 || height <= 0) {
	WPainter::Image image(src, src);

	if (height <= 0)
	  height = image.height();

	if (width <= 0)
	  width = image.width();
      }

      if (ps.y + height > renderer.textHeight(ps.page)) {
	pageBreak(ps);

	startPage = ps.page;
	startY = ps.y;

	ps.y += cssBorderWidth(Top, renderer.fontScale());
      }

      ps.y += height;

      // FIXME this should be reviewed for margin/border implications
      ps.maxX = std::max(ps.minX + width, ps.maxX);
    } else {
      double borderFactor = 1.0;

      if (isTableCell()) {
	Block *t = table();
	if (t && t->tableCollapseBorders())
	  borderFactor = 0.5;
      }

      double cMinX = ps.minX + cssPadding(Left, renderer.fontScale())
	+ borderFactor * cssBorderWidth(Left, renderer.fontScale());
      double cMaxX = ps.maxX - cssPadding(Right, renderer.fontScale())
	- borderFactor * cssBorderWidth(Right, renderer.fontScale());

      cMaxX = std::max(cMaxX, cMinX);

      currentWidth_ = cMaxX - cMinX;

      ps.y += cssPadding(Top, renderer.fontScale());

      advance(ps, spacerTop, renderer);

      if (inlineChildren()) {
	Line line(cMinX, ps.y, ps.page);

	renderer.painter()->setFont(cssFont(renderer.fontScale()));

	cMaxX = layoutInline(line, ps.floats, 
			     cMinX, cMaxX, canIncreaseWidth, renderer);

	line.setLineBreak(true);
	line.finish(cssTextAlign(), ps.floats, cMinX, cMaxX, renderer);

	// FIXME deal with the fact that the first line may have moved to
	// the next page. In fact, we cannot do this now: there may have been
	// border and padding to be added on top of it.

	ps.y = line.bottom();
	ps.page = line.page();
      } else {
	double minY = ps.y;
	int minPage = ps.page;
	if (type_ == DomElement_LI) {
	  Line line(0, ps.y, ps.page);

	  double x2 = 1000;
	  x2 = layoutInline(line, ps.floats, cMinX, x2, false, renderer);

	  line.setLineBreak(true);
	  line.finish(AlignLeft, ps.floats, cMinX, x2, renderer);

	  inlineLayout[0].x -= inlineLayout[0].width;
	  minY = line.bottom();
	  minPage = line.page();

	  ps.y = line.y();
	  ps.page = line.page();
	}

	for (unsigned i = 0; i < children_.size(); ++i) {
	  Block *c = children_[i];

	  if (c->isFloat())
	    cMaxX = c->layoutFloat(ps.y, ps.page, ps.floats, 
				   cMinX, 0, cMinX, cMaxX,
				   canIncreaseWidth, renderer);
	  else if (c->isPositionedAbsolutely()) {
	    if (!c->offsetParent_)
	      c->setOffsetParent();

	    /*
	     * This layout computes the 'hypothetical' static layout
	     * properties
	     */
	    PageState absolutePs;
	    absolutePs.y = ps.y;
	    absolutePs.page = ps.page;
	    absolutePs.minX = ps.minX;
	    absolutePs.maxX = ps.maxX;
	    absolutePs.floats = ps.floats;
	    c->layoutBlock(absolutePs, false, renderer, 0, 0);
	  } else {
	    double copyMinX = ps.minX;
	    double copyMaxX = ps.maxX;

	    ps.minX = cMinX;
	    ps.maxX = cMaxX;

	    collapseMarginBottom 
	      = c->layoutBlock(ps, canIncreaseWidth,
			       renderer, 
			       collapseMarginTop, collapseMarginBottom);
	    collapseMarginTop = collapseMarginBottom;

	    cMaxX = ps.maxX;
	    ps.minX = copyMinX;
	    ps.maxX = copyMaxX;
	  }
	}

	if (ps.y < minY && ps.page == minPage)
	  ps.y = minY;
      }

      ps.maxX = cMaxX + cssPadding(Right, renderer.fontScale())
	+ borderFactor * cssBorderWidth(Right, renderer.fontScale());

      advance(ps, spacerBottom, renderer);

      ps.y += cssPadding(Bottom, renderer.fontScale());
    }
  }

  ps.y += cssBorderWidth(Bottom, renderer.fontScale());

  double marginBottom = cssMargin(Bottom, renderer.fontScale());

  ps.y -= collapseMarginBottom;

  double height = cssHeight(renderer.fontScale());

  if (isTableCell())
    contentsHeight_ = std::max(0.0, diff(ps.y, ps.page, startY,
					 startPage, renderer));

  if (height >= 0) {
    int prevPage = ps.page;
    double prevY = ps.y;

    ps.page = startPage;
    ps.y = startY;

    if (isFloat()) // see supra, startY includes the margin
      ps.y += marginTop;

    advance(ps, height, renderer);

    if (isTable() || isTableCell()) {
      // A table or table cell will overflow if necessary
      if (prevPage > ps.page ||
	  (prevPage == ps.page && prevY > ps.y)) {
	ps.page = prevPage;
	ps.y = prevY;
      }
    }
  }

  collapseMarginBottom = std::max(marginBottom, collapseMarginBottom);

  if (isFloat()) {
    ps.minX -= cssMargin(Left, renderer.fontScale());
    ps.maxX += cssMargin(Right, renderer.fontScale());
    ps.y += collapseMarginBottom;
    collapseMarginBottom = 0;
  }

  for (int i = startPage; i <= ps.page; ++i) {
    double boxY = (i == startPage) ? startY : 0;
    double boxH;
    
    if (i == ps.page)
      boxH = ps.y - boxY;
    else
      boxH = std::max(0.0, maxChildrenLayoutY(i) - boxY);

    if (boxH > 0) {
      blockLayout.push_back(BlockBox());
      BlockBox& box = blockLayout.back();

      box.page = i;
      box.x = ps.minX;
      box.width = ps.maxX - ps.minX;
      box.y = boxY;
      box.height = boxH;
    }
  }

  if (ps.y != 0)
    ps.y += collapseMarginBottom;
  else
    collapseMarginBottom = 0;

  if (blockLayout.empty()) {
    ps.page = startPage;
    ps.y = inY;
    collapseMarginBottom = inCollapseMarginTop;

    /*
     * Record an empty layout box, since this is used for offset
     * children for example. And we need this for borders etc...
     * anyway ?
     */
    blockLayout.push_back(BlockBox());
    BlockBox& box = blockLayout.back();

    box.page = startPage;
    box.x = ps.minX;
    box.width = ps.maxX - ps.minX;
    box.y = inY;
    box.height = 0;
  }

  /*
   * Layout dependent absolute/fixed positioned blocks.
   */
  for (unsigned i = 0; i < offsetChildren_.size(); ++i)
    offsetChildren_[i]->layoutAbsolute(renderer);

  /*
   * For a percentage length we will not try to overflow the parent,
   * unless we are a table cell
   */
  if (!isTableCell()
      && (ps.maxX - ps.minX == cssSetWidth)
      && isPercentageLength(cssProperty(PropertyStyleWidth)))
    ps.maxX = origMaxX;
  else if (ps.maxX < origMaxX)
    ps.maxX = origMaxX;
  else {
    if (!isFloat()) {
      ps.minX -= cssMargin(Left, renderer.fontScale());
      ps.maxX += cssMargin(Right, renderer.fontScale());
    }
  }

  ps.minX = origMinX;

  std::string pageBreakAfter = cssProperty(PropertyStylePageBreakAfter);
  if (pageBreakAfter == "always") {
    pageBreak(ps);
    return 0;
  } else
    return collapseMarginBottom;
}

WString Block::generateItem() const
{
  bool numbered = parent_ && parent_->type_ == DomElement_OL;

  if (numbered) {
    int counter = 0;

    for (unsigned i = 0; i < parent_->children_.size(); ++i) {
      Block *child = parent_->children_[i];

      if (child->type_ == DomElement_LI)
	++counter;

      if (child == this)
	break;
    }

    return boost::lexical_cast<std::string>(counter) + ". ";
  } else
    return "- ";
}

Block *Block::findOffsetParent()
{
  if (parent_) {
    std::string pos = parent_->cssProperty(PropertyStylePosition);
    if (pos == "absolute" ||
	pos == "fixed" ||
	pos == "relative")
      return parent_;
    else
      return parent_->findOffsetParent();
  } else
    return this;
}

bool Block::isPositionedAbsolutely() const
{
  std::string pos = cssProperty(PropertyStylePosition);
  return (pos == "absolute" || pos == "fixed");
}

void Block::setOffsetParent()
{
  offsetParent_ = findOffsetParent();
  offsetParent_->offsetChildren_.push_back(this);
}

LayoutBox Block::firstInlineLayoutBox()
{
  if (!inlineLayout.empty())
    return inlineLayout.front();
  else {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      LayoutBox b = c->firstInlineLayoutBox();
      if (!b.isNull())
	return b;
    }

    return LayoutBox();
  }
}

LayoutBox Block::layoutTotal()
{
  if (isInline()) {
    return firstInlineLayoutBox();
  } else {
    LayoutBox result;
    LayoutBox& first = blockLayout.front();
    result.x = first.x;
    result.y = first.y;
    result.page = first.page;
    result.width = first.width;
    result.height = first.height;

    for (unsigned i = 1; i < blockLayout.size(); ++i)
      result.height += blockLayout[i].height;

    return result;
  }
}

bool isOffsetAuto(const std::string& s) {
  return s.empty() || s == "auto";
}

void Block::layoutAbsolute(const WTextRenderer& renderer)
{
  /*
   * This is lacking logic for 'auto' margins still, although the main
   * use case of it (centering horizontally) is dealt with I believe in
   * layoutBlock() itself ?
   */

  /*
   * the static position: this we need to record from a hypothetical
   * static layout, which has been done before getting here
   */
  LayoutBox staticLayout = layoutTotal();
  LayoutBox containingLayout = offsetParent_->layoutTotal();

  bool leftAuto = isOffsetAuto(cssProperty(PropertyStyleLeft));
  bool widthAuto = isOffsetAuto(cssProperty(PropertyStyleWidth));
  bool rightAuto = isOffsetAuto(cssProperty(PropertyStyleRight));

  double staticLeft = staticLayout.x - containingLayout.x;
  /*
    useful only for RTL
    
    double staticRight = (containingLayout.x + containingLayout.width)
      - (staticLayout.x + staticLayout.width);  
   */

  /*
   * This is not entirely accurate...
   *
   *   we should be doing a table-cell like layout for maximum and minimum
   *   preferred widths, and as maximum the containing width
   */
  PageState ps;
  layoutBlock(ps, false, renderer, 0, 0);
  double preferredMinWidth = ps.maxX;

  ps = PageState();
  layoutBlock(ps, true, renderer, 0, 0);
  double preferredWidth = ps.maxX;

  // FIXME containing box: add border, padding widths
  double availableWidth = containingLayout.width;

  double shrinkToFitWidth = 
    std::min(std::max(preferredMinWidth, availableWidth), preferredWidth);

  double left = 0, width = 0, right = 0;

  if (!leftAuto)
    left = cssDecodeLength(cssProperty(PropertyStyleLeft),
			   renderer.fontScale(), 0,
			   PercentageOfParentSize,
			   containingLayout.width);

  if (!rightAuto)
    right = cssDecodeLength(cssProperty(PropertyStyleRight),
			    renderer.fontScale(), 0,
			    PercentageOfParentSize,
			    containingLayout.width);

  if (!widthAuto)
    width = cssWidth(renderer.fontScale());

  /*
   * compute final values for left and width according to the rules of
   * W3C CSS2 10.3.7
   */
  if (leftAuto && widthAuto && rightAuto) {
    /* assuming ltr, 10.3.7 rule 3 */
    left = staticLeft;
    width = shrinkToFitWidth;
  } else if (!leftAuto && !widthAuto && !rightAuto) {
    /* assuming ltr */
    // keep left and width
  } else if (leftAuto && widthAuto && !rightAuto) {
    // 10.3.7 rule 1.
    width = shrinkToFitWidth;
    left = containingLayout.width - right - width;
  } else if (leftAuto && !widthAuto && rightAuto) {
    // 10.3.7 rule 2.
    /* assuming ltr */
    // keep left and width
  } else if (!leftAuto && widthAuto && rightAuto) {
    // 10.3.7 rule 3.
    width = shrinkToFitWidth;
  } else if (leftAuto && !widthAuto && !rightAuto) {
    // 10.3.7 rule 4.
    left = containingLayout.width - right - width;
  } else if (!leftAuto && widthAuto && !rightAuto) {
    // 10.3.7 rule 5.
    width = std::max(0.0, containingLayout.width - left - right);
  } else if (!leftAuto && !widthAuto && rightAuto) {
    // 10.3.7 rule 6.
    // keep left and width
  }

  double staticTop = staticLayout.y - containingLayout.y;

  /*
   * correct for page differences; XXX this will only work if
   * textHeight() is the same for all pages
   */
  staticTop += (staticLayout.page - containingLayout.page)
    * renderer.textHeight(containingLayout.page);

  bool topAuto = isOffsetAuto(cssProperty(PropertyStyleTop));
  bool heightAuto = isOffsetAuto(cssProperty(PropertyStyleHeight));
  bool bottomAuto = isOffsetAuto(cssProperty(PropertyStyleBottom));

  double top = 0, height = 0, bottom = 0;

  if (!topAuto)
    top = cssDecodeLength(cssProperty(PropertyStyleTop),
			   renderer.fontScale(), 0,
			   PercentageOfParentSize,
			   containingLayout.height);

  if (!bottomAuto)
    right = cssDecodeLength(cssProperty(PropertyStyleBottom),
			    renderer.fontScale(), 0,
			    PercentageOfParentSize,
			    containingLayout.height);

  if (!heightAuto)
    height = cssWidth(renderer.fontScale());

  /*
   * Perform a layout to compute the contents height
   */
  ps = PageState();
  ps.minX = containingLayout.x + left;
  ps.maxX = containingLayout.x + left + width;
  layoutBlock(ps, false, renderer, 0, 0);
  double contentHeight = layoutTotal().height;

  /*
   * compute final values for top and height according to the rules of
   * W3C CSS2 10.6.4
   */
  if (topAuto && heightAuto && bottomAuto) {
    /* 10.6.4 rule 3 */
    top = staticTop;
    height = contentHeight;
  } else if (!topAuto && !heightAuto && !bottomAuto) {
    // keep top and height
  } else if (topAuto && heightAuto && !bottomAuto) {
    /* 10.6.4 rule 1 */
    height = contentHeight;
    top = containingLayout.height - bottom - height;
  } else if (topAuto && !heightAuto && bottomAuto) {
    /* 10.6.4 rule 2 */
    top = staticTop;
  } else if (!topAuto && heightAuto && bottomAuto) {
    /* 10.6.4 rule 3 */
    height = contentHeight;
  } else if (topAuto && !heightAuto && !bottomAuto) {
    /* 10.6.4 rule 4 */
    top = containingLayout.height - bottom - height;
  } else if (!topAuto && heightAuto && !bottomAuto) {
    /* 10.6.4 rule 5 */
    height = containingLayout.height - top - bottom;
  } else if (!topAuto && !heightAuto && bottomAuto) {
    /* 10.6.4 rule 6 */
    // keep top and height
  }

  /*
   * Perform final layout
   */
  ps = PageState();
  ps.y = containingLayout.y + top;
  ps.page = containingLayout.page;

  /* 
   * XXX this will only work if textHeight() is the same for all pages
   */
  while (ps.y > renderer.pageHeight(ps.page)) {
    ++ps.page;
    ps.y -= renderer.pageHeight(ps.page);
  }
  ps.minX = containingLayout.x + left;
  ps.maxX = containingLayout.x + left + width;
 
  layoutBlock(ps, false, renderer, 0, 0);
}

double Block::layoutFloat(double y, int page, BlockList& floats,
			  double lineX, double lineHeight,
			  double minX, double maxX, bool canIncreaseWidth,
			  const WTextRenderer& renderer)
{
  if (Wt::Utils::indexOf(floats, this) != -1)
    return maxX;

  double blockCssWidth = cssWidth(renderer.fontScale());

  double currentWidth = std::max(0.0, blockCssWidth)
    + cssBoxMargin(Left, renderer.fontScale())
    + cssBoxMargin(Right, renderer.fontScale());

  PageState floatPs;
  floatPs.floats = floats;

  /*
   * Here we iterate while trying to position the float horizontally,
   * iterating over:
   *  - finding a suitable X position that fits the current assumed width
   *  - determining the actual width at that position
   */
  for (;;) {
    floatPs.page = page;
    floatPs.y = y;
    floatPs.minX = minX;
    floatPs.maxX = maxX;

    double floatX = positionFloat(lineX, floatPs, lineHeight, currentWidth,
				  canIncreaseWidth,
				  renderer, floatSide());

    if (floatPs.maxX > maxX)
      return floatPs.maxX;

    /*
     * We need to determine the block width to be able to position
     * it. But to know the block width, we may need to lay it out
     * entirely, when its width is not fixed.
     *
     * What if the line moves to a next page later? We will then relayout
     * the float.
     */
    BlockList innerFloats;

    bool unknownWidth = blockCssWidth < 0 &&
      isEpsilonLess(currentWidth, maxX - minX);

    double collapseMarginBottom = 0; // does not apply to a float

    floatPs.minX = floatX;
    floatPs.maxX = floatX + currentWidth;
    collapseMarginBottom = layoutBlock(floatPs,
				       unknownWidth || canIncreaseWidth, 
				       renderer, 
				       0, collapseMarginBottom);

    double pw = floatPs.maxX - (floatPs.minX + currentWidth);
    if (pw > 0) {
      if (blockCssWidth < 0) {
	currentWidth = std::min(maxX - minX, currentWidth + pw);
	continue;
      } else {
	if (!canIncreaseWidth)
	  throw WException("Internal error: !canIncreaseWidth");
	return maxX + pw;
      }
    }

    floats.push_back(this);

    return maxX;
  }
}

void Block::adjustAvailableWidth(double y, int page, 
				 const BlockList& floats,
				 Range &rangeX)
{
  for (unsigned i = 0; i < floats.size(); ++i) {
    Block *b = floats[i];

    for (unsigned j = 0; j < b->blockLayout.size(); ++j) {
      const BlockBox& block = b->blockLayout[j];

      if (block.page == page) {
	if (block.y <= y && y < block.y + block.height) {
	  if (floats[i]->floatSide() == Left)
	    rangeX.start = std::max(rangeX.start, block.x + block.width);
	  else
	    rangeX.end = std::min(rangeX.end, block.x);

	  if (rangeX.end <= rangeX.start)
	    return;
	}
      }
    }
  }
}

void Block::clearFloats(PageState &ps, double minWidth)
{
  /* Floats need to be cleared in order */
  for (; !ps.floats.empty();) {
    Block *b = ps.floats[0];

    ps.y = b->blockLayout.back().y + b->blockLayout.back().height;
    ps.page = b->blockLayout.back().page;

    ps.floats.erase(ps.floats.begin());

    Range rangeX(ps.minX, ps.maxX);
    adjustAvailableWidth(ps.y, ps.page, ps.floats, rangeX);

    if (!isEpsilonMore(minWidth, rangeX.end - rangeX.start))
      break;
  }
}

double Block::positionFloat(double x, PageState &ps,
			    double lineHeight, double width,
			    bool canIncreaseWidth,
			    const WTextRenderer& renderer,
			    Side floatSide)
{
  if (!ps.floats.empty()) {
    double minY = ps.floats.back()->blockLayout[0].y;

    if (minY > ps.y) {
      if (minY < ps.y + lineHeight)
	lineHeight -= (minY - ps.y); // we've cleared the current line partially
      else
	x = ps.minX; // we've cleared the current line
      ps.y = minY;
    }
  }

  BlockList floats = ps.floats;

  for (;;) {
    Range rangeX(ps.minX, ps.maxX);
    adjustAvailableWidth(ps.y, ps.page, ps.floats, rangeX);
    ps.maxX = rangeX.end;

    double availableWidth = rangeX.end - std::max(x, rangeX.start);

    if (!isEpsilonLess(availableWidth, width))
      break;
    else {
      if (canIncreaseWidth) {
	ps.maxX += width - availableWidth;
	break;
      } else if (x > rangeX.start) {
	ps.y += lineHeight;
	x = ps.minX;
      } else {
	clearFloats(ps, width);
	break;
      }
    }
  }

  ps.floats = floats;

  Range rangeX(ps.minX, ps.maxX);
  adjustAvailableWidth(ps.y, ps.page, ps.floats, rangeX);
  ps.maxX = rangeX.end;

  if (floatSide == Left)
    x = rangeX.start;
  else
    x = rangeX.end - width;

  return x;
}

void Block::clearFloats(PageState &ps)
{
  for (unsigned i = 0; i < ps.floats.size(); ++i) {
    Block *b = ps.floats[i];

    BlockBox& bb = b->blockLayout.back();
    if (bb.page <= ps.page) {
      ps.floats.erase(ps.floats.begin() + i);
      --i;
    }
  }
}

AlignmentFlag Block::cssTextAlign() const
{
  if (node_ && !isInline()) {
    std::string s = cssProperty(PropertyStyleTextAlign);

    if (s.empty() && !isTable())
      s = attributeValue("align");

    if (s.empty() || s == "inherit") {
      if (type_ == DomElement_TH)
	return AlignCenter;
      else if (parent_)
	return parent_->cssTextAlign();
      else
	return AlignLeft;
    } else {
      if (s == "left")
	return AlignLeft;
      else if (s == "center")
	return AlignCenter;
      else if (s == "right")
	return AlignRight;
      else if (s == "justify")
	return AlignJustify;
      else {
	unsupportedCssValue(PropertyStyleTextAlign, s);
	return AlignLeft;
      }
    }
  } else if (parent_)
    return parent_->cssTextAlign();
  else
    return AlignLeft;
}

WFont Block::cssFont(double fontScale) const
{
  if (font_.genericFamily() != WFont::Default)
    return font_;

  WFont::GenericFamily genericFamily = WFont::SansSerif;
  WString specificFamilies;

  std::string family = inheritedCssProperty(PropertyStyleFontFamily);

  if (!family.empty()) {
    std::vector<std::string> values;
    boost::split(values, family, boost::is_any_of(","));

    for (unsigned i = 0; i < values.size(); ++i) {
      std::string name = values[i];
      boost::trim(name);
      boost::trim_if(name, boost::is_any_of("'\""));
      name = Wt::Utils::lowerCase(name);

      if (name == "sans-serif")
	genericFamily = WFont::SansSerif;
      else if (name == "serif")
	genericFamily = WFont::Serif;
      else if (name == "cursive")
	genericFamily = WFont::Cursive;
      else if (name == "fantasy")
	genericFamily = WFont::Fantasy;
      else if (name == "monospace")
	genericFamily = WFont::Monospace;
      else {
	if (   name == "times"
	    || name == "palatino")
	  genericFamily = WFont::Serif;
	else if (   name == "arial"
		 || name == "helvetica")
	  genericFamily = WFont::SansSerif;
	else if (name == "courier")
	  genericFamily = WFont::Monospace;
	else if (name == "symbol")
	  genericFamily = WFont::Fantasy; // XXX
	else if (name == "zapf dingbats")
	  genericFamily = WFont::Cursive;

	if (!specificFamilies.empty())
	  specificFamilies += ", ";
	specificFamilies += name;
      }
    }
  }

  font_.setFamily(genericFamily, specificFamilies);
  font_.setSize(WFont::FixedSize,
		WLength(cssFontSize(fontScale), WLength::Pixel));
  font_.setWeight(WFont::Value, cssFontWeight());
  font_.setStyle(cssFontStyle());

  return font_;
}

std::string Block::cssTextDecoration() const
{
  std::string v = cssProperty(PropertyStyleTextDecoration);

  if (v.empty() || v == "inherit")
    if (parent_)
      return parent_->cssTextDecoration();
    else
      return std::string();
  else
    return v;
}

void Block::reLayout(const LayoutBox &from, const LayoutBox &to)
{
  for (unsigned i = 0; i < inlineLayout.size(); ++i) {
    InlineBox& ib = inlineLayout[i];

    ib.page = to.page;
    ib.x += to.x - from.x;
    ib.y += to.y - from.y;
  }

  for (unsigned i = 0; i < blockLayout.size(); ++i) {
    BlockBox& bb = blockLayout[i];

    bb.page = to.page;
    bb.x += to.x - from.x;
    bb.y += to.y - from.y;
  }

  for (unsigned i = 0; i < children_.size(); ++i)
    children_[i]->reLayout(from, to);
}

void Block::render(WTextRenderer& renderer, WPainter& painter, int page)
{
  bool painterTranslated = false;

  if (cssProperty(PropertyStylePosition) == "relative") {
    painter.save();
    painterTranslated = true;

    LayoutBox box = layoutTotal();
    double left = cssDecodeLength(cssProperty(PropertyStyleLeft),
				  renderer.fontScale(), 0,
				  PercentageOfParentSize,
				  box.width);
    double top = cssDecodeLength(cssProperty(PropertyStyleTop),
				 renderer.fontScale(), 0,
				 PercentageOfParentSize,
				 box.height);

    painter.translate(left, top);
  }

  if (isText()) {
    renderText(text(), renderer, painter, page);

    if (type_ != DomElement_LI) { // there are inline children
      if (painterTranslated)
	painter.restore();
      return;
    }
  }

  unsigned first = (type_ == DomElement_LI) ? 1 : 0;
  for (unsigned i = first; i < inlineLayout.size(); ++i) {
    LayoutBox& lb = inlineLayout[i];

    if (lb.page == page)
      renderer.paintNode(painter, WTextRenderer::Node(*this, lb, renderer));
  }

  for (unsigned i = 0; i < blockLayout.size(); ++i) {
    LayoutBox& lb = blockLayout[i];

    if (lb.page == page)
      renderer.paintNode(painter, WTextRenderer::Node(*this, lb, renderer));
  }

  if (inlineLayout.empty() && blockLayout.empty()) {
    for (unsigned i = 0; i < children_.size(); ++i)
      children_[i]->render(renderer, painter, page);
  }

  if (painterTranslated)
    painter.restore();
}

int Block::firstLayoutPage() const
{
  if (!inlineLayout.empty())
    return inlineLayout.front().page;

  if (!blockLayout.empty())
    return blockLayout.front().page;

  return -1;
}

int Block::lastLayoutPage() const
{
  if (!inlineLayout.empty())
    return inlineLayout.back().page;

  if (!blockLayout.empty())
    return blockLayout.back().page;

  return -1;
}

void Block::actualRender(WTextRenderer& renderer, WPainter& painter,
                         LayoutBox& lb)
{
  if (type_ == DomElement_IMG) {
    LayoutBox bb = toBorderBox(lb, renderer.fontScale());

    renderBorders(bb, renderer, painter, Top | Bottom);

    double left = renderer.margin(Left) + bb.x
     + cssBorderWidth(Left, renderer.fontScale());
    double top = renderer.margin(Top) + bb.y
     + cssBorderWidth(Top, renderer.fontScale());

    double width = bb.width;
    double height = bb.height;

    WRectF rect(left, top, width, height);

#ifdef DEBUG_LAYOUT
    painter.setPen(WPen(red));
    painter.drawRect(rect);
#endif // DEBUG_LAYOUT

    painter.drawImage(rect, WPainter::Image(attributeValue("src"),
					    (int)width, (int)height));
  } else {
    LayoutBox bb = toBorderBox(lb, renderer.fontScale());

    WRectF rect(bb.x + renderer.margin(Left), bb.y + renderer.margin(Top),
                bb.width, bb.height);

    std::string s = cssProperty(PropertyStyleBackgroundColor);
    if (!s.empty()) {
      WColor c(WString::fromUTF8(s));
      painter.fillRect(rect, WBrush(c));
    }

    WFlags<Side> verticals;
    if (lb.page == firstLayoutPage())
      verticals |= Top;
    if (lb.page == lastLayoutPage())
      verticals |= Bottom;

    renderBorders(bb, renderer, painter, verticals);

#ifdef DEBUG_LAYOUT
    painter.setPen(WPen(green));
    painter.drawRect(rect);
#endif // DEBUG_LAYOUT

    if (type_ == DomElement_THEAD) {
      if (currentTheadBlock_ == 0 && !blockLayout.empty())
	currentTheadBlock_ = &blockLayout.front();

      for (unsigned j = 0; j < children_.size(); ++j) {
        if (currentTheadBlock_ != &lb)
          children_[j]->reLayout(*currentTheadBlock_, lb);

        children_[j]->render(renderer, painter, lb.page);
      }

      currentTheadBlock_ = &lb;
    }
  }

  if (type_ != DomElement_THEAD)
    for (unsigned i = 0; i < children_.size(); ++i)
      children_[i]->render(renderer, painter, lb.page);
}

void Block::renderText(const std::string& text, WTextRenderer& renderer,
                       WPainter& painter, int page)
{
  WPaintDevice *device = painter.device();

  painter.setFont(cssFont(renderer.fontScale()));

  WFontMetrics metrics = device->fontMetrics();
  double lineHeight = cssLineHeight(metrics.height(), renderer.fontScale());
  double fontHeight = metrics.size();

  std::string decoration = cssTextDecoration();

  for (unsigned i = 0; i < inlineLayout.size(); ++i) {
    InlineBox& ib = inlineLayout[i];
  
    if (ib.page == page) {
      double y = renderer.margin(Top) + ib.y - metrics.leading()
	+ (lineHeight - fontHeight)/2.0;
      WRectF rect(renderer.margin(Left) + ib.x, y, ib.width, ib.height);

#ifdef DEBUG_LAYOUT
      painter.save();
      painter.setPen(WPen(gray));
      painter.drawRect(WRectF(rect.left(), rect.top() + metrics.leading(),
			      rect.width(), rect.height()));
      painter.setPen(WPen(blue));
      double baseline = y + metrics.leading() + metrics.ascent();
      painter.drawLine(rect.left(), baseline, rect.right(), baseline);
      painter.restore();
#endif // DEBUG_LAYOUT

      /*
      std::string s = inheritedCssProperty(PropertyStyleBackgroundColor);
      if (!s.empty()) {
	WColor c(WString::fromUTF8(s));

	WRectF hrect(rect.left(), renderer.margin(Top) + ib.lineTop,
		     rect.width(), ib.lineHeight);

	renderer.painter()->fillRect(hrect, WBrush(c));
      }
      */

      painter.setPen(WPen(cssColor()));

      if (ib.whitespaceWidth == device->measureText(" ").width()) {
	WString t = WString::fromUTF8(text.substr(ib.utf8Pos, ib.utf8Count));

	painter.drawText(WRectF(rect.x(), rect.y(), rect.width(),
				rect.height() + metrics.leading()),
			 AlignLeft | AlignTop, t);
      } else {
	double x = rect.left();

	int wordStart = 0;
	double wordTotal = 0;
	for (int j = 0; j <= ib.utf8Count; ++j) {
	  if (j == ib.utf8Count || isWhitespace(text[ib.utf8Pos + j])) {
	    if (j > wordStart) {
	      WString word = WString::fromUTF8
		(text.substr(ib.utf8Pos + wordStart, j - wordStart));
	      double wordWidth = device->measureText(word).width();

	      wordTotal += wordWidth;

	      painter.drawText(WRectF(x, rect.top(),
				      wordWidth, rect.height()),
			       AlignLeft | AlignTop, word);

#ifdef DEBUG_LAYOUT
	      painter.save();
	      painter.setPen(WPen(cyan));
	      painter.drawRect(WRectF(x, rect.top(),
				      wordWidth, rect.height()));
	      painter.restore();
#endif // DEBUG_LAYOUT

	      x += wordWidth;
	    }

	    x += ib.whitespaceWidth;
	    wordStart = j + 1;
	  }
	}
      }

      if (decoration == "underline") {
	double below = y + metrics.leading() + metrics.ascent() + 2;
	painter.drawLine(rect.left(), below, rect.right(), below);
      } else if (decoration == "overline") {
	// FIXME this will depend on font metrics, but shouldn't
	double over = renderer.margin(Top) + ib.y + 2;
	painter.drawLine(rect.left(), over, rect.right(), over);
      } else if (decoration == "line-through") {
	double through = y + metrics.leading() + metrics.ascent() - 3;
	painter.drawLine(rect.left(), through, rect.right(), through);
      }
    } else if (ib.page > page)
      break;
  }
}

LayoutBox Block::toBorderBox(const LayoutBox& bb, double fontScale) const
{
  LayoutBox result = bb;

  if (isFloat()) {
    result.x += cssMargin(Left, fontScale);
    result.y += cssMargin(Top, fontScale);
    result.width -= cssMargin(Left, fontScale) + cssMargin(Right, fontScale);
    result.height -= cssMargin(Top, fontScale) + cssMargin(Bottom, fontScale);
  }

  return result;
}

double Block::maxBorderWidth(Block *b1, Side s1,
			     Block *b2, Side s2,
			     Block *b3, Side s3,
			     Block *b4, Side s4,
			     double fontScale)
{
  double result = 0;

  if (b1)
    result = std::max(result, b1->collapsedBorderWidth(s1, fontScale));

  if (b2)
    result = std::max(result, b2->collapsedBorderWidth(s2, fontScale));

  if (b3)
    result = std::max(result, b3->collapsedBorderWidth(s3, fontScale));

  if (b4)
    result = std::max(result, b4->collapsedBorderWidth(s4, fontScale));

  return result;
}

void Block::renderBorders(const LayoutBox& bb, WTextRenderer& renderer,
                          WPainter &painter, WFlags<Side> verticals)
{
  if (!node_)
    return;

  double left = renderer.margin(Left) + bb.x;
  double top = renderer.margin(Top) + bb.y;
  double right = left + bb.width;
  double bottom = top + bb.height;

  double borderWidth[4];
  WColor borderColor[4];

  Side sides[4] = { Top, Right, Bottom, Left };
  for (int i = 0; i < 4; ++i) {
    borderWidth[i] = cssBorderWidth(sides[i], renderer.fontScale());
    borderColor[i] = cssBorderColor(sides[i]);
  }

  double offsetFactor = 1;

  /*
   * For a table-collapsed cell, do not add the borderWidth offsets
   */
  if (isTableCell()) {
    Block *t = table();
    if (t && t->tableCollapseBorders())
      offsetFactor = 0;
  }

  double cornerMaxWidth[4] = { 0, 0, 0, 0 };

  if (offsetFactor == 0) {
    Block *siblings[4];
    for (unsigned i = 0; i < 4; ++i)
      siblings[i] = siblingTableCell(sides[i]);

    cornerMaxWidth[TopLeft] = maxBorderWidth(siblings[3], Top,
					     this, Top,
					     siblings[0], Left,
					     this, Left,
					     renderer.fontScale());

    cornerMaxWidth[TopRight] = maxBorderWidth(siblings[1], Top,
					      this, Top,
					      siblings[0], Right,
					      this, Right,
					      renderer.fontScale());

    cornerMaxWidth[BottomLeft] = maxBorderWidth(siblings[3], Bottom,
						this, Bottom,
						siblings[2], Left,
						this, Left,
						renderer.fontScale());

    cornerMaxWidth[BottomRight] = maxBorderWidth(siblings[1], Bottom,
						 this, Bottom,
						 siblings[2], Right,
						 this, Right,
						 renderer.fontScale());
  }

  for (unsigned i = 0; i < 4; ++i) {
    if (borderWidth[i] != 0) {
      WPen borderPen;
      borderPen.setCapStyle(FlatCap);
      borderPen.setWidth(borderWidth[i]);
      borderPen.setColor(borderColor[i]);
      painter.setPen(borderPen);

      switch (sides[i]) {
      case Top:
	if (verticals & Top) {
	  double leftOffset = 0, rightOffset = 0;

	  if (borderWidth[i] < cornerMaxWidth[TopLeft])
	    leftOffset = cornerMaxWidth[TopLeft] / 2;
	  else if (offsetFactor == 0)
	    leftOffset = -borderWidth[i] / 2;

	  if (borderWidth[i] < cornerMaxWidth[TopRight])
	    rightOffset = cornerMaxWidth[TopRight] / 2;
	  else if (offsetFactor == 0)
	    rightOffset = -borderWidth[i] / 2;

	  painter.drawLine(left + leftOffset, 
			   top + offsetFactor * borderWidth[i]/2,
			   right - rightOffset,
			   top + offsetFactor * borderWidth[i]/2);

	}

	break;
      case Right:
	{
	  double topOffset = 0, bottomOffset = 0;

	  if (borderWidth[i] < cornerMaxWidth[TopRight])
	    topOffset = cornerMaxWidth[TopRight] / 2;
	  else if (offsetFactor == 0)
	    topOffset = -borderWidth[i] / 2;

	  if (borderWidth[i] < cornerMaxWidth[BottomRight])
	    bottomOffset = cornerMaxWidth[BottomRight] / 2;
	  else if (offsetFactor == 0)
	    bottomOffset = -borderWidth[i] / 2;

	  painter.drawLine(right - offsetFactor * borderWidth[i]/2,
			   top + topOffset,
			   right - offsetFactor * borderWidth[i]/2,
			   bottom - bottomOffset);
	}

	break;
      case Bottom:
	if (verticals & Bottom) {
	  double leftOffset = 0, rightOffset = 0;

	  if (borderWidth[i] < cornerMaxWidth[BottomLeft])
	    leftOffset = cornerMaxWidth[BottomLeft] / 2;
	  else if (offsetFactor == 0)
	    leftOffset = -borderWidth[i] / 2;

	  if (borderWidth[i] < cornerMaxWidth[TopRight])
	    rightOffset = cornerMaxWidth[BottomRight] / 2;
	  else if (offsetFactor == 0)
	    rightOffset = -borderWidth[i] / 2;

	  painter.drawLine(left + leftOffset,
			   bottom - offsetFactor * borderWidth[i]/2,
			   right - rightOffset,
			   bottom - offsetFactor * borderWidth[i]/2);
	}

	break;
      case Left:
	{
	  double topOffset = 0, bottomOffset = 0;

	  if (borderWidth[i] < cornerMaxWidth[TopLeft])
	    topOffset = cornerMaxWidth[TopLeft] / 2;
	  else if (offsetFactor == 0)
	    topOffset = -borderWidth[i] / 2;

	  if (borderWidth[i] < cornerMaxWidth[BottomLeft])
	    bottomOffset = cornerMaxWidth[BottomLeft] / 2;
	  else if (offsetFactor == 0)
	    bottomOffset = -borderWidth[i] / 2;

	  painter.drawLine(left + offsetFactor * borderWidth[i]/2,
			   top + topOffset,
			   left + offsetFactor * borderWidth[i]/2,
			   bottom - bottomOffset);
	}

	break;
      default:
	break;
      }
    }
  }
}

std::string Block::inheritedCssProperty(Property property) const
{
  if (node_) {
    std::string s = cssProperty(property);

    if (!s.empty())
      return s;
  }

  if (parent_)
    return parent_->inheritedCssProperty(property);
  else
    return std::string();
}

bool Block::isAggregate(const std::string& cssProperty)
{
  return cssProperty == "margin"
    || cssProperty == "border"
    || cssProperty == "padding"
    || cssProperty == "border-color"
    || cssProperty == "border-width";
}

void Block::updateAggregateProperty(const std::string& property,
                                    const std::string& aggregate,
                                    const Specificity& spec,
                                    const std::string& value) const
{
  if (css_.find(property + aggregate) == css_.end()
     || css_[property + aggregate].s_.isSmallerOrEqualThen(spec))
    css_[property + aggregate] = PropertyValue(value, spec);
}

void Block::fillinStyle(const std::string& style,
                        const Specificity& specificity) const
{
  if (style.empty())
    return;

  Wt::Utils::SplitVector values;
  boost::split(values, style, boost::is_any_of(";"));

  for (unsigned i = 0; i < values.size(); ++i) {
    Wt::Utils::SplitVector namevalue;

    boost::split(namevalue, values[i], boost::is_any_of(":"));
    if (namevalue.size() == 2) {
      std::string n = Wt::Utils::splitEntryToString(namevalue[0]);
      std::string v = Wt::Utils::splitEntryToString(namevalue[1]);

      boost::trim(n);
      boost::trim(v);

      updateAggregateProperty(n, "", specificity, v);

      if (isAggregate(n)) {
        Wt::Utils::SplitVector allvalues;
        boost::split(allvalues, v, boost::is_any_of(" "));

        /*
         * count up to first value that does not start with a digit,
         *  we want to interpret '1px solid rgb(...)' as '1px'
         */
        unsigned int count = 0;
        for (unsigned j = 0; j < allvalues.size(); ++j) {
          std::string vj = Wt::Utils::splitEntryToString(allvalues[j]);
          if (vj[0] < '0' || vj[0] > '9')
            break;

          ++count;
        }

        if (count == 0)
	  count = allvalues.size();

        if (count == 1) {
          updateAggregateProperty(n, "-top",    specificity, v);
          updateAggregateProperty(n, "-right",  specificity, v);
          updateAggregateProperty(n, "-bottom", specificity, v);
          updateAggregateProperty(n, "-left",   specificity, v);
        } else if (count == 2) {
          std::string v1 = Wt::Utils::splitEntryToString(allvalues[0]);
          updateAggregateProperty(n, "-top",    specificity, v1);
          updateAggregateProperty(n, "-bottom", specificity, v1);

          std::string v2 = Wt::Utils::splitEntryToString(allvalues[1]);
          updateAggregateProperty(n, "-right",  specificity, v2);
          updateAggregateProperty(n, "-left",   specificity, v2);
        } else if (count == 3) {
          std::string v1 = Wt::Utils::splitEntryToString(allvalues[0]);
          updateAggregateProperty(n, "-top",    specificity, v1);

          std::string v2 = Wt::Utils::splitEntryToString(allvalues[1]);
          updateAggregateProperty(n, "-right",  specificity, v2);
          updateAggregateProperty(n, "-left",   specificity, v2);

          std::string v3 = Wt::Utils::splitEntryToString(allvalues[2]);
          updateAggregateProperty(n, "-bottom", specificity, v3);
        } else {
          std::string v1 = Wt::Utils::splitEntryToString(allvalues[0]);
          updateAggregateProperty(n, "-top",    specificity, v1);

          std::string v2 = Wt::Utils::splitEntryToString(allvalues[1]);
          updateAggregateProperty(n, "-right",  specificity, v2);

          std::string v3 = Wt::Utils::splitEntryToString(allvalues[2]);
          updateAggregateProperty(n, "-bottom", specificity, v3);

          std::string v4 = Wt::Utils::splitEntryToString(allvalues[3]);
          updateAggregateProperty(n, "-left",   specificity, v4);
        }
      }
    }
  }
}

std::string Block::cssProperty(Property property) const
{
  if (!node_)
    return std::string();

  if (css_.empty()) {
    if (styleSheet_) {
      for (unsigned int i = 0; i < styleSheet_->rulesetSize(); ++i) {
        Specificity s = Match::isMatch(this,
				       styleSheet_->rulesetAt(i).selector());
        if (s.isValid()) {
          fillinStyle(styleSheet_->rulesetAt(i).declarationBlock()
		      .declarationString(),
                      s);
        }
      }
    }

    // The "style" attribute has Specificity(1,0,0,0)
    fillinStyle(attributeValue("style"), Specificity(1,0,0,0));
  }

  std::map<std::string, PropertyValue>::const_iterator i
    = css_.find(DomElement::cssName(property));

  if (i != css_.end())
    return i->second.value_;
  else
    return std::string();
}

std::string Block::attributeValue(const char *attribute) const
{
  if (!node_)
    return std::string();

  xml_attribute<> *attr = node_->first_attribute(attribute);

  if (attr)
    return attr->value();
  else
    return std::string();
}

void Block::unsupportedAttributeValue(const char *attribute,
				      const std::string& value)
{
  LOG_ERROR("unsupported value '" << value << "' for attribute " << attribute);
}

void Block::unsupportedCssValue(Property property, const std::string& value)
{
  LOG_ERROR("unsupported value '" << value
	    << "'for CSS style property " << DomElement::cssName(property));
}

  }
}
