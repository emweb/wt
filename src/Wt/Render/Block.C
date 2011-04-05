/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// #define DEBUG_LAYOUT

#include "Wt/WFontMetrics"
#include "Wt/WPaintDevice"
#include "Wt/WPainter"
#include "Wt/Render/WTextRenderer"

#include "Block.h"
#include "Line.h"
#include "Utils.h"
#include "DomElement.h"
#include "WtException.h"

#include <iostream>
#include <math.h>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string.hpp>

using namespace rapidxml;

namespace Wt {
  namespace Render {

int sideToIndex(Wt::Side side)
{
  switch (side) {
  case Wt::Top: return 0;
  case Wt::Right: return 1;
  case Wt::Bottom: return 2;
  case Wt::Left: return 3;
  default:
    throw std::runtime_error("Unexpected side: " + side);
  }
}

double sum(const std::vector<double>& v)
{
  double result = 0;
  for (unsigned i = 0; i < v.size(); ++i)
    result += v[i];
  return result;
}

const double MARGINX = -1;

Block::Block(xml_node<> *node, Block *parent)
  : node_(node),
    parent_(parent),
    type_(DomElement_UNKNOWN),
    inline_(false),
    float_(None)
{
  if (node) {
    switch (node->type()) {
    case node_element:
      type_ = DomElement::parseTagName(node->name());
      if (type_ == DomElement_UNKNOWN)
	unsupportedElement(node->name());
      break;
    default:
      ;
    }

    for (xml_node<> *child = node->first_node(); child;
	 child = child->next_sibling())
      children_.push_back(new Block(child, this));
  }
}

Block::~Block()
{
  for (unsigned i = 0; i < children_.size(); ++i)
    delete children_[i];
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
    return std::string(node_->value(), node_->value_size());
}

void Block::determineDisplay()
{
  std::string fl = cssProperty(PropertyStyleFloat);
  if (!fl.empty()) {
    if (fl == "left")
      float_ = Left;
    else if (fl == "right")
      float_ = Right;
    else
      unsupportedCssValue(PropertyStyleFloat, fl);
  } else if (type_ == DomElement_IMG || type_ == DomElement_TABLE) {
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
	else
	  throw std::logic_error("Display '" + display + "' is not supported.");
      else
	inline_ = DomElement::isDefaultInline(type_);

      if (inline_ && !allChildrenInline)
	throw std::logic_error("Inline element cannot contain block elements");
    } else
      inline_ = false;
  }
}

/*
 * Normalizes whitespace inbetween nodes.
 */
bool Block::normalizeWhitespace(bool haveWhitespace, memory_pool<>& pool)
{
  bool whitespaceIn = haveWhitespace;

  if (!isInline())
    haveWhitespace = true;

  if (type_ == DomElement_UNKNOWN && isText()) {
    char *v = node_->value();

    unsigned len = node_->value_size();

    std::string s;
    s.reserve(len);

    for (unsigned i = 0; i < len; ++i) {
      if (isWhitespace(v[i])) {
	if (!haveWhitespace)
	  s += ' ';
	haveWhitespace = true;
      } else if (i < len - 1 && v[i] == (char)0xC2 && v[i+1] == (char)0xA0) {
	/*
	 * This wrong but will work temporarily. We are treating &nbsp;
	 * (resolved to UTF-8 0xC2 0xA0) here equal to normal space.
	 */
	if (!haveWhitespace)
	  s += ' ';
	haveWhitespace = true;
	++i;
      } else {
	s += v[i];
	haveWhitespace = false;
      }
    }

    char *nv = pool.allocate_string(s.c_str(), s.length());
    node_->value(nv, s.length());
  } else
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *b = children_[i];
      haveWhitespace = b->normalizeWhitespace(haveWhitespace, pool);
    }

  if (!isInline())
    return whitespaceIn;
  else
    return haveWhitespace;
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
  std::string marginLeft = cssProperty(PropertyStyleMarginLeft, "margin", 3);
  std::string marginRight = cssProperty(PropertyStyleMarginRight, "margin", 1);

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

double Block::cssLength(Property top, const char *aggregate, Side side,
			double fontScale, bool& defined) const
{
  if (!node_) {
    defined = false;
    return 0;
  }

  int index = sideToIndex(side);
  Property property = (Property)(top + index);

  std::string value = cssProperty(property, aggregate, index);

  if (!value.empty()) {
    WLength l(value.c_str());
    defined = true;
    return l.toPixels(cssFontSize(fontScale));
  } else {
    defined = false;
    return 0;
  }
}

 double Block::cssPadding(Side side, double fontScale) const
{
  bool defined;
  double result = cssLength(PropertyStylePaddingTop, "padding", side,
			    fontScale, defined);

  if (!defined) {
    if (type_ == DomElement_TD || type_ == DomElement_TH)
      return 4;
    else if ((type_ == DomElement_UL || type_ == DomElement_OL) && side == Left)
      return 40;
  }

  return result;
}

double Block::cssMargin(Side side, double fontScale) const
{
  bool defined;
  double result = 0;

  try {
    result = cssLength(PropertyStyleMarginTop, "margin", side, fontScale,
		       defined);
  } catch (std::exception& e) {
    /* catches 'auto' margin length */
  }

  if (!defined) {
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
    }
  }

  return result;
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

double Block::cssBorderWidth(Side side, double fontScale) const
{
  if (!node_)
    return 0;

  int index = sideToIndex(side);
  Property property = (Property)(PropertyStyleBorderTop + index);

  std::string borderStr = cssProperty(property, "border");

  double result = 0;
  if (!borderStr.empty()) {
    std::vector<std::string> values;
    boost::split(values, borderStr, boost::is_any_of(" "));

    WLength l(values[0].c_str());
    result = l.toPixels(cssFontSize(fontScale));
  }

  if (result == 0) {
    if (type_ == DomElement_TABLE) {
      result = attributeValue("border", 0);
    } else if (type_ == DomElement_TD) {
      /*
       * If the table has a border, then we have a default border of 1px
       */
      Block *table = parent_;
      while (table && table->type_ != DomElement_TABLE)
	table = table->parent_;

      if (table)
	result = table->attributeValue("border", 0) ? 1 : 0;
    }
  }

  return result;
}

WColor Block::cssBorderColor(Side side) const
{
  int index = sideToIndex(side);
  Property property = (Property)(PropertyStyleBorderTop + index);

  std::string borderStr = cssProperty(property, "border");

  if (!borderStr.empty()) {
    std::vector<std::string> values;
    boost::split(values, borderStr, boost::is_any_of(" "));

    if (values.size() > 2)
      return WColor(WString::fromUTF8(values[2]));
  }

  return black;
}

WColor Block::cssColor() const
{
  if (!node_)
    return parent_->cssColor();

  std::string colorStr = cssProperty(PropertyStyleColor);

  if (!colorStr.empty())
    return WColor(WString::fromUTF8(colorStr));
  else if (parent_)
    return parent_->cssColor();
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
  if (!node_)
    return parent_->cssFontStyle();

  std::string v = cssProperty(PropertyStyleFontStyle);

  if (v.empty() && type_ == DomElement_EM)
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
  if (!node_)
    return parent_->cssFontWeight();

  std::string v = cssProperty(PropertyStyleFontWeight);

  if (v.empty() && (type_ == DomElement_STRONG
		    || (type_ >= DomElement_H1
			&& type_ <= DomElement_H6)))
    v = "bolder";

  if (!v.empty()) {
    try {
      return boost::lexical_cast<int>(v);
    } catch (boost::bad_lexical_cast&) {
      if (v == "normal")
	return 400;
      else if (v == "bold")
	return 700;
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
  if (!node_)
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
  if (!node_)
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

void Block::layoutInline(Line& line, BlockList& floats,
			 double minX, double maxX, bool canIncreaseWidth,
			 const WTextRenderer& renderer)
{
  inlineLayout.clear();

  if (isText() || type_ == DomElement_IMG) {
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
      double startX = minX, endX = maxX;
      adjustAvailableWidth(line.y(), line.page(), startX, endX, floats);

      if (startX > line.x())
	line.setX(startX);

      double w = 0, h = 0;
      bool lineBreak = false;

      if (isText()) {
	/*
	 * Skip whitespace at the start of a line
	 */
	if (utf8Pos < s.length()
	    && line.x() == startX
	    && isWhitespace(s[utf8Pos]))
	  ++utf8Pos;

	if (utf8Pos < s.length()) {
	  double maxWidth;

	  if (canIncreaseWidth)
	    maxWidth = std::numeric_limits<double>::max();
	  else
	    maxWidth = endX - line.x();

	  WString text = WString::fromUTF8(s.substr(utf8Pos));

	  /*
	   * TODO: if canIncreaseWidth, we should immediately measure the whole
	   * line.
	   */
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
	    endX += whitespaceWidth; // to avoid an artificial overflow
	  }

	  if (canIncreaseWidth && item.width() > endX - line.x())
	    throw PleaseWiden(w - (endX - line.x()));

	  if (w == 0) {
	    /*
	     * Start a new line
	     */
	    lineBreak = true;

	    /*
	     * We need at least room for one word.
	     */
	    if (line.x() == startX) {
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
	    assert(utf8Count > 0);
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

	if (w <= 0 || h <= 0)
	  throw std::runtime_error("Image with unknown width/height");

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

      if (lineBreak || w > endX - line.x()) {
	/*
	 * Content does not fit on this line.
	 */
	line.setLineBreak(type_ == DomElement_BR);
	line.finish(cssTextAlign(), floats, minX, maxX, renderer);

	if (w == 0 || line.x() > startX) {
	  if (w > 0 && canIncreaseWidth)
	    throw PleaseWiden(w - (maxX - line.x()));
	  /*
	   * Desired width is 0 or the thing didn't fit on a started line:
	   * we should simply break the line.
	   */
	  line.newLine(minX, line.y() + line.height(), line.page());
	  h = 0;
	} else if (w > maxX - minX) {
	  /*
	   * Wider than the box width without floats
	   */
	  throw PleaseWiden(w - (maxX - minX));
	} else {
	  /*
	   * Not wider than the box width without floats:
	   * clear the blocking float and see if it will then fit.
	   */
	  double y = line.y();
	  int page = line.page();
	  clearFloats(y, page, floats, minX, maxX, w);
	  line.newLine(minX, y, page);
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

	if (!isText() || (isText() && utf8Count == s.length()))
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
	  c->layoutFloat(line.y(), line.page(), floats, line.x(),
			 line.height(), minX, maxX, canIncreaseWidth, renderer);

	  line.reflow(c);
	  line.addBlock(c);
	} else
	  c->layoutInline(line, floats, minX, maxX, canIncreaseWidth,
			  renderer);
      }
    }
  }
}

void Block::layoutTable(double& y, int& page, BlockList& floats,
			double& minX, double& maxX, bool canIncreaseWidth,
			const WTextRenderer& renderer)
{
  int cellSpacing = attributeValue("cellspacing", 2);

  std::vector<double> minimumColumnWidths;
  std::vector<double> maximumColumnWidths;

  for (unsigned i = 0; i < children_.size(); ++i) {
    Block *c = children_[i];

    c->tableComputeColumnWidths(minimumColumnWidths, maximumColumnWidths,
				renderer);
  }

  unsigned colCount = minimumColumnWidths.size();

  double totalSpacing = (colCount + 1) * cellSpacing;
  double totalMinWidth = sum(minimumColumnWidths) + totalSpacing;
  double totalMaxWidth = sum(maximumColumnWidths) + totalSpacing;

  double width = cssWidth(renderer.fontScale());

  width = std::max(totalMinWidth, width);

  double availableWidth;
  for (;;) {
    double startX = minX, endX = maxX;
    adjustAvailableWidth(y, page, startX, endX, floats);

    availableWidth = endX - startX
      - cssBorderWidth(Left, renderer.fontScale())
      - cssBorderWidth(Right, renderer.fontScale());

    /*
     * If we can increase the available width without clearing floats
     * to fit the table at its widest, then try so
     */
    if (canIncreaseWidth && availableWidth < totalMaxWidth)
      throw PleaseWiden(totalMaxWidth - availableWidth);

    if (availableWidth >= width)
      break;
    else {
      if (width < maxX - minX)
	clearFloats(y, page, floats, minX, maxX, width);
      else
	throw PleaseWiden(width - availableWidth);
    }
  }

  if (width <= availableWidth) {
    if (totalMaxWidth > availableWidth)
      width = availableWidth;
    else
      width = std::max(totalMaxWidth, width);
  } else {
    maximumColumnWidths = minimumColumnWidths;
  }

  if (width > totalMinWidth) {
    double ww = width;

    int resizableColumns = maximumColumnWidths.size();

    double remainWidth = totalMaxWidth - totalSpacing;
    for (;;) {
      double factor = (ww - totalSpacing) / remainWidth;

      int columnsResized = 0;

      remainWidth = 0;
      for (unsigned i = 0; i < maximumColumnWidths.size(); ++i) {
	double w = factor * maximumColumnWidths[i];

	if (w > minimumColumnWidths[i]) {
	  ++columnsResized;
	  maximumColumnWidths[i] = w;
	  remainWidth += w;
	} else {
	  maximumColumnWidths[i] = minimumColumnWidths[i];
	  ww -= minimumColumnWidths[i];
	}
      }

      if (columnsResized == resizableColumns || fabs(remainWidth) < 1E-2)
	break;
      else
	resizableColumns = columnsResized;
    }
  } else
    maximumColumnWidths = minimumColumnWidths;

  width += cssBoxMargin(Left, renderer.fontScale())
    + cssBoxMargin(Right, renderer.fontScale());

  AlignmentFlag hAlign = horizontalAlignment();

  switch (hAlign) {
  case AlignLeft:
  case AlignJustify:
    maxX = minX + width;
    break;
  case AlignCenter:
    minX = minX + (maxX - minX - width) / 2;
    maxX = minX + width;
    break;
  case AlignRight:
    minX = maxX - width;
    break;
  default:
    break;
  }

  minX += cssBoxMargin(Left, renderer.fontScale());
  maxX -= cssBoxMargin(Right, renderer.fontScale());

  tableDoLayout(minX, y, page, cellSpacing, maximumColumnWidths, renderer);

  minX -= cssBorderWidth(Left, renderer.fontScale());
  maxX += cssBorderWidth(Right, renderer.fontScale());

  y += cellSpacing;
}

void Block::tableDoLayout(double x, double& y, int& page, int cellSpacing,
			  const std::vector<double>& widths,
			  const WTextRenderer& renderer)
{
  if (   type_ == DomElement_TABLE
      || type_ == DomElement_TBODY
      || type_ == DomElement_THEAD
      || type_ == DomElement_TFOOT) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      c->tableDoLayout(x, y, page, cellSpacing, widths, renderer);
    }
  } else if (   type_ == DomElement_TR
	     || type_ == DomElement_TH) {
    double startY = y;
    int startPage = page;
    tableRowDoLayout(x, y, page, cellSpacing, widths, renderer, -1);
    double rowHeight = (page - startPage) * renderer.textHeight(page) // XXX
      + (y - startY) - cellSpacing;

    y = startY;
    page = startPage;
    tableRowDoLayout(x, y, page, cellSpacing, widths, renderer, rowHeight);
  }
}

void Block::tableRowDoLayout(double x, double& y, int& page,
			     int cellSpacing,
			     const std::vector<double>& widths,
			     const WTextRenderer& renderer,
			     double rowHeight)
{
  double endY = y;
  int endPage = page;

  unsigned col = 0;

  x += cellSpacing;

  for (unsigned i = 0; i < children_.size(); ++i) {
    Block *c = children_[i];

    // TODO skip row-overspanned cells: we need to keep a list of
    // cells that are overspanning, and then we need to adjust their
    // height once done.

    if (c->type_ == DomElement_TD) {
      int colSpan = c->attributeValue("colspan", 1);

      double width = 0;
      for (unsigned i = col; i < col + colSpan; ++i)
	width += widths[col];

      width += (colSpan - 1) * cellSpacing;

      double cellY = y + cellSpacing;
      int cellPage = page;

      BlockList floats;

      double collapseMarginBottom = 0;
      double collapseMarginTop = std::numeric_limits<double>::max();
      c->layoutBlock(cellY, cellPage, floats, x, x + width, false,
		     renderer, collapseMarginTop,
		     collapseMarginBottom, rowHeight);
      if (collapseMarginBottom < collapseMarginTop)
	cellY -= collapseMarginBottom;

      Block::clearFloats(cellY, cellPage, floats, x, x + width, width);

      if (cellPage > endPage
	  || (cellPage == endPage && cellY > endY)) {
	endPage = cellPage;
	endY = cellY;
      }

      col += colSpan;
      x += width + cellSpacing;
    }
  }

  y = endY;
  page = endPage;
}

void Block::tableComputeColumnWidths(std::vector<double>& minima,
				     std::vector<double>& maxima,
				     const WTextRenderer& renderer)
{
  /*
   * Current limitations:
   * - we currently ignore column/column group widths
   * - we treat a cell width as a hard constraint, not a minimum width
   *   (this should be fixed in layoutBlock()).
   * - we do not interpret percentage values for cell/column widths
   */
  if (   type_ == DomElement_TBODY
      || type_ == DomElement_THEAD
      || type_ == DomElement_TFOOT) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      c->tableComputeColumnWidths(minima, maxima, renderer);
    }
  } else if (type_ == DomElement_TR) {
    int col = 0;

    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      if (c->type_ == DomElement_TD) {
	c->cellComputeColumnWidths(col, false, minima, renderer);
	col = c->cellComputeColumnWidths(col, true, maxima, renderer);
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

int Block::cellComputeColumnWidths(int col, bool maximum,
				   std::vector<double>& values,
				   const WTextRenderer& renderer)
{
  double currentWidth = 0;

  int colSpan = attributeValue("colspan", 1);

  while (col + colSpan > (int)values.size())
    values.push_back(0);

  for (int i = 0; i < colSpan; ++i)
    currentWidth += values[col + i];

  double width = currentWidth;

  for (;;) {
    try {
      BlockList innerFloats;

      double y = 0, collapseMarginBottom = 0;
      int page = 0;
    
      layoutBlock(y, page, innerFloats, 0, width, maximum, renderer,
		  0, collapseMarginBottom);

      break;
    } catch (PleaseWiden& pw) {
      width += pw.width;
    }
  }

  if (width > currentWidth) {
    double extraPerColumn = (width - currentWidth) / colSpan;

    for (int i = 0; i < colSpan; ++i) {
      values[col + i] += extraPerColumn;
    }
  }

  return col + colSpan;
}

double Block::cssDecodeLength(const std::string& length,
			      double fontScale, double defaultValue) const
{
  if (!length.empty()) {
    WLength l(length.c_str());
    return l.toPixels(cssFontSize(fontScale));
  } else
    return defaultValue;
}

double Block::cssWidth(double fontScale) const
{
  double result = -1;

  if (node_) {
    result = cssDecodeLength(cssProperty(PropertyStyleWidth),
			     fontScale, result);

    if (type_ == DomElement_IMG)
      result = cssDecodeLength(attributeValue("width"), fontScale, result);
  }

  return result;
}

double Block::cssHeight(double fontScale) const
{
  double result = -1;

  if (node_) {
    result = cssDecodeLength(cssProperty(PropertyStyleHeight),
			     fontScale, result);

    if (type_ == DomElement_IMG)
      result = cssDecodeLength(attributeValue("height"), fontScale, result);
  }

  return result;
}

double Block::layoutHeight() const
{
  double h = 0;

  for (unsigned i = 0; i < blockLayout.size(); ++i)
    h += blockLayout[i].height;

  return h;
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

void Block::advance(double& y, int& page, double height,
		    const WTextRenderer& renderer)
{
  while (y + height > renderer.textHeight(page)) {
    ++page;
    y = 0;
    height -= (renderer.textHeight(page) - y);
  }

  y += height;
}

void Block::layoutBlock(double& y, int& page, BlockList& floats,
			double minX, double maxX, bool canIncreaseWidth,
			const WTextRenderer& renderer,
			double collapseMarginTop,
			double& collapseMarginBottom,
			double cellHeight)
{
  double inCollapseMarginTop = collapseMarginTop;
  double inY = y;
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

  y -= std::min(marginTop, collapseMarginTop);

  collapseMarginTop = std::max(marginTop, collapseMarginTop);
  collapseMarginBottom = 0;

  startY = y;

  y += marginTop;

  if (!isFloat())
    startY = y;

  int startPage = page;

  y += cssBorderWidth(Top, renderer.fontScale());

  minX += cssMargin(Left, renderer.fontScale());
  maxX -= cssMargin(Right, renderer.fontScale());

  if (type_ == DomElement_TABLE) {
    layoutTable(y, page, floats, minX, maxX, canIncreaseWidth, renderer);
  } else {
    double width = cssWidth(renderer.fontScale());

    if (width >= 0) {
      width += cssPadding(Left, renderer.fontScale())
	+ cssBorderWidth(Left, renderer.fontScale())
	+ cssPadding(Right, renderer.fontScale())
	+ cssBorderWidth(Right, renderer.fontScale());

      AlignmentFlag hAlign = horizontalAlignment();
      switch (hAlign) {
      case AlignLeft:
	maxX = minX + width;
	break;
      case AlignCenter:
	minX = minX + (maxX - minX - width) / 2;
	maxX = minX + width;
	break;
      case AlignRight:
	minX = maxX - width;
	break;
      default:
	break;
      }
    }

    if (type_ == DomElement_IMG) {
      // FIXME deal with page break
      double height = cssHeight(renderer.fontScale());

      if (y + height > renderer.textHeight(page)) {
	clearFloats(floats, page);

	startY = 0;
	++startPage;

	y = startY + cssBorderWidth(Top, renderer.fontScale());
	page = startPage;
      }

      y += height;
    } else {
      double cMinX = minX + cssPadding(Left, renderer.fontScale())
	+ cssBorderWidth(Left, renderer.fontScale());
      double cMaxX = maxX - cssPadding(Right, renderer.fontScale())
	- cssBorderWidth(Right, renderer.fontScale());

      y += cssPadding(Top, renderer.fontScale());

      advance(y, page, spacerTop, renderer);

      if (inlineChildren()) {
	Line line(cMinX, y, page);

	renderer.painter()->setFont(cssFont(renderer.fontScale()));

	layoutInline(line, floats, cMinX, cMaxX, canIncreaseWidth, renderer);

	line.setLineBreak(true);
	line.finish(cssTextAlign(), floats, cMinX, cMaxX, renderer);

	// FIXME deal with the fact that the first line may have moved to
	// the next page. In fact, we cannot do this now: there may have been
	// border and padding to be added on top of it.

	y = line.bottom();
	page = line.page();
      } else {
	double minY = y;
	int minPage = page;
	if (type_ == DomElement_LI) {
	  Line line(0, y, page);

	  layoutInline(line, floats, cMinX, 1000, false, renderer);

	  line.setLineBreak(true);
	  line.finish(AlignLeft, floats, cMinX, 1000, renderer);

	  inlineLayout[0].x -= inlineLayout[0].width;
	  minY = line.bottom();
	  minPage = line.page();

	  y = line.y();
	  page = line.page();
	}

	for (unsigned i = 0; i < children_.size(); ++i) {
	  Block *c = children_[i];

	  if (c->isFloat())
	    c->layoutFloat(y, page, floats, cMinX, 0, cMinX, cMaxX,
			   canIncreaseWidth, renderer);
	  else {
	    c->layoutBlock(y, page, floats, cMinX, cMaxX, canIncreaseWidth,
			   renderer, collapseMarginTop, collapseMarginBottom);
	    collapseMarginTop = collapseMarginBottom;
	  }
	}

	if (y < minY && page == minPage)
	  y = minY;
      }

      advance(y, page, spacerBottom, renderer);

      y += cssPadding(Bottom, renderer.fontScale());
    }
  }

  y += cssBorderWidth(Bottom, renderer.fontScale());

  double marginBottom = cssMargin(Bottom, renderer.fontScale());

  y -= collapseMarginBottom;

  double height = cssHeight(renderer.fontScale());

  if (type_ == DomElement_TD || type_ == DomElement_TH) {
    contentsHeight_ = diff(y, page, startY, startPage, renderer);
  }

  if (height >= 0) {
    page = startPage;
    y = startY;

    if (isFloat()) // see supra, startY includes the margin
      y += marginTop;

    advance(y, page, height, renderer);
  }

  collapseMarginBottom = std::max(marginBottom, collapseMarginBottom);

  if (isFloat()) {
    minX -= cssMargin(Left, renderer.fontScale());
    maxX += cssMargin(Right, renderer.fontScale());
    y += collapseMarginBottom;
    collapseMarginBottom = 0;
  }

  for (int i = startPage; i <= page; ++i) {
    double boxY =  (i == startPage) ? startY : 0;
    double boxH = (i == page ? y : renderer.textHeight(i)) - boxY;

    if (boxH > 0) {
      blockLayout.push_back(BlockBox());
      BlockBox& box = blockLayout.back();

      box.page = i;
      box.x = minX;
      box.width = maxX - minX;
      box.y = boxY;
      box.height = boxH;
    }
  }

  y += collapseMarginBottom;

  if (blockLayout.empty()) {
    y = inY;
    collapseMarginBottom = inCollapseMarginTop;
  }
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

void Block::layoutFloat(double y, int page, BlockList& floats,
			double lineX, double lineHeight,
			double minX, double maxX, bool canIncreaseWidth,
			const WTextRenderer& renderer)
{
  if (Utils::indexOf(floats, this) != -1)
    return;

  double blockCssWidth = cssWidth(renderer.fontScale());

  double currentWidth = std::max(0.0, blockCssWidth)
    + cssBoxMargin(Left, renderer.fontScale())
    + cssBoxMargin(Right, renderer.fontScale());

  for (;;) {
    int floatPage = page;
    double floatX = lineX, floatY = y;

    positionFloat(floatX, floatY, floatPage, lineHeight, currentWidth,
		  floats, minX, maxX, canIncreaseWidth,
		  renderer, floatSide());

    /*
     * We need to determine the block width to be able to position
     * it. But to know the block width, we may need to lay it out
     * entirely, when its width is not fixed.
     *
     * What if the line moves to a next page later? We will then relayout
     * the float.
     */
    try {
      BlockList innerFloats;

      bool unknownWidth = blockCssWidth < 0 && currentWidth < (maxX - minX);

      double collapseMarginBottom; // does not apply to a float
      layoutBlock(floatY, floatPage, innerFloats, floatX, floatX + currentWidth,
		  unknownWidth || canIncreaseWidth, renderer, 0,
		  collapseMarginBottom);
      floats.push_back(this);

      return;
    } catch (PleaseWiden& pw) {
      if (blockCssWidth < 0) {
	currentWidth = std::min(maxX - minX, currentWidth + pw.width);
      } else {
	assert(canIncreaseWidth);
	throw;
      }
    }
  }
}

void Block::adjustAvailableWidth(double y, int page, double& minX,
				 double& maxX, const BlockList& floats)
{
  for (unsigned i = 0; i < floats.size(); ++i) {
    Block *b = floats[i];

    for (unsigned j = 0; j < b->blockLayout.size(); ++j) {
      const BlockBox& block = b->blockLayout[j];

      if (block.page == page) {
	if (block.y <= y && y < block.y + block.height) {
	  if (floats[i]->floatSide() == Left)
	    minX = std::max(minX, block.x + block.width);
	  else
	    maxX = std::min(maxX, block.x);

	  if (maxX <= minX)
	    return;
	}
      }
    }
  }
}

void Block::clearFloats(double& y, int& page, BlockList& floats,
			double minX, double maxX, double minWidth)
{
  /* Floats need to be cleared in order */
  for (; !floats.empty();) {
    Block *b = floats[0];

    y = b->blockLayout.back().y + b->blockLayout.back().height;
    page = b->blockLayout.back().page;

    floats.erase(floats.begin());

    double startX = minX, endX = maxX;

    adjustAvailableWidth(y, page, startX, endX, floats);

    if (endX - startX >= minWidth)
      break;
  }
}

void Block::positionFloat(double& x, double& y, int& page,
			  double lineHeight, double width,
			  const BlockList& floats,
			  double minX, double maxX, bool canIncreaseWidth,
			  const WTextRenderer& renderer,
			  Side floatSide)
{
  if (!floats.empty()) {
    double minY = floats.back()->blockLayout[0].y;

    if (minY > y) {
      if (minY < y + lineHeight)
	lineHeight -= (minY - y); // we've cleared the current line partially
      else
	x = minX; // we've cleared the current line
      y = minY;
    }
  }

  BlockList floatsToClear = floats;

  for (;;) {
    double startX = minX;
    double endX = maxX;

    adjustAvailableWidth(y, page, startX, endX, floatsToClear);

    double availableWidth = endX - std::max(x, startX);

    if (availableWidth >= width)
      break;
    else {
      if (canIncreaseWidth)
	throw PleaseWiden(width - availableWidth);
      if (x > startX) {
	y += lineHeight;
	x = minX;
      } else {
	clearFloats(y, page, floatsToClear, minX, maxX, width);
	break;
      }
    }
  }

  double startX = minX;
  double endX = maxX;

  adjustAvailableWidth(y, page, startX, endX, floats);

  if (floatSide == Left)
    x = startX;
  else
    x = endX - width;
}

void Block::clearFloats(BlockList& floats, int page)
{
  for (unsigned i = 0; i < floats.size(); ++i) {
    Block *b = floats[i];

    BlockBox& bb = b->blockLayout.back();
    if (bb.page <= page) {
      floats.erase(floats.begin() + i);
      --i;
    }
  }
}

AlignmentFlag Block::cssTextAlign() const
{
  if (node_ && !isInline()) {
    std::string s = cssProperty(PropertyStyleTextAlign);

    if (s.empty() && type_ != DomElement_TABLE)
      s = attributeValue("align");

    if (s.empty() || s == "inherit") {
      if (parent_)
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
      else
	throw std::runtime_error("Unsupported value for text-align");
    }
  } else
    return parent_->cssTextAlign();
}

WFont Block::cssFont(double fontScale) const
{
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
      name = Utils::lowerCase(name);

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

  WFont result;

  result.setFamily(genericFamily, specificFamilies);
  result.setSize(WFont::FixedSize,
		 WLength(cssFontSize(fontScale), WLength::Pixel));
  result.setWeight(WFont::Value, cssFontWeight());
  result.setStyle(cssFontStyle());
  return result;
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

void Block::render(WTextRenderer& renderer, int page)
{
  if (isText())
    renderText(text(), renderer, page);

  if (type_ == DomElement_IMG) {
    LayoutBox *lb;

    if (!blockLayout.empty())
      lb = &blockLayout[0];
    else
      lb = &inlineLayout[0];

    if (lb->page == page) {
      LayoutBox bb = toBorderBox(*lb, renderer.fontScale());

      renderBorders(bb, renderer, Top | Bottom);

      double left = renderer.margin(Left) + bb.x
	+ cssBorderWidth(Left, renderer.fontScale());
      double top = renderer.margin(Top) + bb.y
	+ cssBorderWidth(Top, renderer.fontScale());
      double width = cssWidth(renderer.fontScale());
      double height = cssHeight(renderer.fontScale());

      WRectF rect(left, top, width, height);

#ifdef DEBUG_LAYOUT
      renderer.painter()->setPen(WPen(red));      
      renderer.painter()->drawRect(rect);
#endif // DEBUG_LAYOUT

      renderer.painter()->drawImage(rect,
				    WPainter::Image(attributeValue("src"),
						    (int)width, (int)height));
    }
  }

  for (unsigned i = 0; i < blockLayout.size(); ++i) {
    BlockBox& lb = blockLayout[i];
    if (lb.page == page) {
      LayoutBox bb = toBorderBox(lb, renderer.fontScale());

      WFlags<Side> verticals;
      if (i == 0)
	verticals |= Top;
      if (i == blockLayout.size() - 1)
	verticals |= Bottom;

      WRectF rect(bb.x + renderer.margin(Left), bb.y + renderer.margin(Top),
		  bb.width, bb.height);

      std::string s = cssProperty(PropertyStyleBackgroundColor);
      if (!s.empty()) {
	WColor c(WString::fromUTF8(s));
	renderer.painter()->fillRect(rect, WBrush(c));
      }

      renderBorders(bb, renderer, verticals);

#ifdef DEBUG_LAYOUT
      renderer.painter()->setPen(WPen(green));
      renderer.painter()->drawRect(rect);
#endif // DEBUG_LAYOUT
    }
  }

  for (unsigned i = 0; i < children_.size(); ++i)
    children_[i]->render(renderer, page);
}

void Block::renderText(const std::string& text, WTextRenderer& renderer,
		       int page)
{
  WPainter& painter = *renderer.painter();
  WPaintDevice *device = painter.device();

  renderer.painter()->setFont(cssFont(renderer.fontScale()));

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

      renderer.painter()->setPen(WPen(cssColor()));

      if (ib.whitespaceWidth == device->measureText(" ").width()) {
	WString t = WString::fromUTF8(text.substr(ib.utf8Pos, ib.utf8Count));

	painter.drawText(WRectF(rect.x(), rect.y(), rect.width(),
				rect.height() + metrics.leading()),
			 AlignLeft | AlignTop, t);
      } else {
	double x = rect.left();

	int wordStart = 0;
	double wordTotal = 0;
	for (int i = 0; i <= ib.utf8Count; ++i) {
	  if (i == ib.utf8Count || isWhitespace(text[ib.utf8Pos + i])) {
	    if (i > wordStart) {
	      WString word = WString::fromUTF8
		(text.substr(ib.utf8Pos + wordStart, i - wordStart));
	      double wordWidth = device->measureText(word).width();

	      wordTotal += wordWidth;

	      painter.drawText(WRectF(x, rect.top(),
				      wordWidth, rect.height()),
			       AlignLeft | AlignTop, word);

	      x += wordWidth;
	    }

	    x += ib.whitespaceWidth;
	    wordStart = i + 1;
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

void Block::renderBorders(const LayoutBox& bb, WTextRenderer& renderer,
			  WFlags<Side> verticals)
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

  WPainter& painter = *renderer.painter();
  WPen borderPen;
  borderPen.setCapStyle(FlatCap);

  for (unsigned i = 0; i < 4; ++i) {
    if (borderWidth[i]) {
      borderPen.setWidth(borderWidth[i]);
      borderPen.setColor(borderColor[i]);
      painter.setPen(borderPen);

      switch (sides[i]) {
      case Top:
	if (verticals & Top)
	  painter.drawLine(left, 
			   top + borderWidth[0]/2,
			   right,
			   top + borderWidth[0]/2);
	break;
      case Right:
	painter.drawLine(right - borderWidth[1]/2,
			 top,
			 right - borderWidth[1]/2,
			 bottom);
	break;
      case Bottom:
	if (verticals & Bottom)
	  painter.drawLine(left,
			   bottom - borderWidth[2]/2,
			   right,
			   bottom - borderWidth[2]/2);
	break;
      case Left:
	painter.drawLine(left + borderWidth[3]/2,
			 top,
			 left + borderWidth[3]/2,
			 bottom);
	break;
      default:
	break;
      }
    }
  }
}

std::string Block::cssProperty(Property property) const
{
  return cssProperty(property, 0, 0);
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

std::string Block::cssProperty(Property property, const char *aggregate,
			       int aggregateIndex) const
{
  if (!node_)
    return std::string();

  assert (aggregateIndex < 4);

  std::string style = attributeValue("style");

  std::vector<std::string> values;
  boost::split(values, style, boost::is_any_of(";"));

  std::string needle = DomElement::cssName(property);

  std::string result;

  for (unsigned i = 0; i < values.size(); ++i) {
    std::vector<std::string> namevalue;

    boost::split(namevalue, values[i], boost::is_any_of(":"));
    if (namevalue.size() == 2) {
      boost::trim(namevalue[0]);
      boost::trim(namevalue[1]);

      if (namevalue[0] == needle)
	result = namevalue[1];
      else if (aggregate && namevalue[0] == aggregate) {
	if (aggregateIndex == -1)
	  result = namevalue[1];
	else {
	  std::vector<std::string> allvalues;
	  boost::split(allvalues, namevalue[1], boost::is_any_of(" \t\n"));

	  if (aggregateIndex < (int)allvalues.size())
	    result = allvalues[aggregateIndex];
	  else {
	    if (allvalues.size() == 1)
	      result = allvalues[0];
	    else if (allvalues.size() == 2)
	      result = allvalues[aggregateIndex - 2];
	    else /* allvalues.size() == 3 */
	    result = allvalues[1];
	  }
	}
      }
    }
  }

  return result;
}

std::string Block::attributeValue(const char *attribute) const
{
  xml_attribute<> *attr = node_->first_attribute(attribute);

  if (attr)
    return attr->value();
  else
    return std::string();
}

void Block::unsupportedAttributeValue(const char *attribute,
				      const std::string& value)
{
  throw std::runtime_error("Unsupported value '" + value + "' for attribute "
			   + attribute);
}

void Block::unsupportedCssValue(Property property, const std::string& value)
{
  throw std::runtime_error("Unsupported value '" + value + "' for CSS style "
			   "property " + DomElement::cssName(property));
}

void Block::unsupportedElement(const std::string& tag)
{
  throw std::runtime_error("Unsupported element '" + tag + "'");
}

  }
}
