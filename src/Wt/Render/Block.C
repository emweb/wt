/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

//#define DEBUG_LAYOUT

#include "Wt/WFontMetrics"
#include "Wt/WLogger"
#include "Wt/WPaintDevice"
#include "Wt/WPainter"
#include "Wt/Render/WTextRenderer"

#include "Block.h"
#include "Line.h"
#include "WebUtils.h"
#include "RenderUtils.h"
#include "DomElement.h"

#include <boost/algorithm/string.hpp>

using namespace rapidxml;

namespace {
  const double MARGINX = -1;
  const double EPSILON = 1e-4;
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
    currentWidth_(0),
    contentsHeight_(0)
{
  if (node) {
    if (Render::Utils::isXMLElement(node)) {
      type_ = DomElement::parseTagName(node->name());
      if (type_ == DomElement_UNKNOWN) {
	LOG_ERROR("unsupported element: " << node->name());
	type_ = DomElement_DIV;
      }
    }

    Render::Utils::fetchBlockChildren(node, this, children_);
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
    return Render::Utils::nodeValueToString(node_);
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
	else {
	  LOG_ERROR("display '" << display << "' is not supported.");
	  inline_ = false;
	}
      else
	inline_ = DomElement::isDefaultInline(type_);

      if (inline_ && !allChildrenInline)
	LOG_ERROR("inline element cannot contain block elements");
    } else
      inline_ = false;
  }
}

/*
 * Normalizes whitespace inbetween nodes.
 */
bool Block::normalizeWhitespace(bool haveWhitespace, 
				rapidxml::xml_document<> &doc)
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
      return 4;
    else if ((type_ == DomElement_UL || type_ == DomElement_OL) && side == Left)
      return 40;
  }

  return result.length;
}

double Block::cssMargin(Side side, double fontScale) const
{
  CssLength result;
  result.length = 0;

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

double Block::cssBorderWidth(Side side, double fontScale) const
{
  if (!node_)
    return 0;

  int index = sideToIndex(side);
  Property property = (Property)(PropertyStyleBorderTop + index);

  std::string borderStr = cssProperty(property);

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
    } else if (isTableCell()) {
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

  std::string borderStr = cssProperty(property);

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
		    || type_ == DomElement_TH
		    || (type_ >= DomElement_H1
			&& type_ <= DomElement_H6)))
    v = "bolder";

  if (!v.empty()) {
    try {
      return boost::lexical_cast<int>(v);
    } catch (boost::bad_lexical_cast& blc) {
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

double Block::layoutInline(Line& line, BlockList& floats,
			   double minX, double maxX, bool canIncreaseWidth,
			   const WTextRenderer& renderer)
{
  inlineLayout.clear();

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
	      item.width() - EPSILON > rangeX.end - line.x()) {
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

	if (w <= 0) {
	  LOG_ERROR("image with unknown width");
	  w = 10;
	}

	if (h <= 0) {
	  LOG_ERROR("image with unknown height");
	  h = 10;
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

      if (lineBreak || w - EPSILON > rangeX.end - line.x()) {
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
	} else if (w - EPSILON > maxX - minX) {
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
	  maxX = c->layoutFloat(line.y(), line.page(), floats, line.x(),
				line.height(), 
				minX, maxX, canIncreaseWidth, renderer);

	  line.reflow(c);
	  line.addBlock(c);
	} else
	  maxX = c->layoutInline(line, floats, minX, maxX, canIncreaseWidth,
				 renderer);
      }
    }
  }

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

  for (unsigned i = 0; i < children_.size(); ++i) {
    Block *c = children_[i];

    c->tableComputeColumnWidths(minimumColumnWidths, maximumColumnWidths,
				renderer, this);
  }

  currentWidth_ = 0;

  unsigned colCount = minimumColumnWidths.size();

  int cellSpacing = attributeValue("cellspacing", 2);
  double totalSpacing = (colCount + 1) * cellSpacing;
  double totalMinWidth = sum(minimumColumnWidths) + totalSpacing;
  double totalMaxWidth = sum(maximumColumnWidths) + totalSpacing;

  double desiredMinWidth = std::max(totalMinWidth, cssSetWidth);

  double desiredMaxWidth = totalMaxWidth;
  if (cssSetWidth > 0 && cssSetWidth < totalMaxWidth)
    desiredMaxWidth = std::max(desiredMinWidth, cssSetWidth);

  double availableWidth;
  for (;;) {
    Range rangeX(ps.minX, ps.maxX);
    adjustAvailableWidth(ps.y, ps.page, ps.floats, rangeX);
    ps.maxX = rangeX.end;

    availableWidth = rangeX.end - rangeX.start
      - cssBorderWidth(Left, renderer.fontScale())
      - cssBorderWidth(Right, renderer.fontScale());

    /*
     * If we can increase the available width without clearing floats
     * to fit the table at its widest, then try so
     */
    if (canIncreaseWidth && availableWidth < desiredMaxWidth) {
      ps.maxX += desiredMaxWidth - availableWidth;
      availableWidth = desiredMaxWidth;
    }

    if (availableWidth >= desiredMinWidth)
      break;
    else {
      if (desiredMinWidth < ps.maxX - ps.minX) {
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
    double factor = width / totalMaxWidth;

    for (unsigned i = 0; i < widths.size(); ++i)
      widths[i] *= factor;
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

  width += cssBoxMargin(Left, renderer.fontScale())
    + cssBoxMargin(Right, renderer.fontScale());

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

  ps.minX += cssBoxMargin(Left, renderer.fontScale());
  ps.maxX -= cssBoxMargin(Right, renderer.fontScale());

  Block *repeatHead = 0;
  if (!children_.empty() && children_[0]->type_ == DomElement_THEAD) {
    // Note: should actually interpret CSS 'table-header-group' value for
    // display
    repeatHead = children_[0];
  }
  bool protectRows = repeatHead != 0;

  tableDoLayout(ps.minX, ps, cellSpacing, widths,
		protectRows, repeatHead, renderer);

  ps.minX -= cssBorderWidth(Left, renderer.fontScale());
  ps.maxX += cssBorderWidth(Right, renderer.fontScale());

  ps.y += cellSpacing;
}

void Block::tableDoLayout(double x, PageState &ps, int cellSpacing,
			  const std::vector<double>& widths,
			  bool protectRows, Block *repeatHead,
			  const WTextRenderer& renderer)
{
  if (   type_ == DomElement_TABLE
      || type_ == DomElement_TBODY
      || type_ == DomElement_THEAD
      || type_ == DomElement_TFOOT) {
    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      c->tableDoLayout(x, ps, cellSpacing, widths, protectRows,
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
    tableRowDoLayout(x, ps, cellSpacing, widths, renderer, -1);

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
      tableRowDoLayout(x, ps, cellSpacing, widths, renderer, -1);
    }

    double rowHeight = (ps.page - startPage) 
      * renderer.textHeight(ps.page) // XXX
      + (ps.y - startY) - cellSpacing;

    ps.y = startY;
    ps.page = startPage;
    tableRowDoLayout(x, ps, cellSpacing, widths, renderer, rowHeight);
  }
}

void Block::tableRowDoLayout(double x, PageState &ps,
			     int cellSpacing,
			     const std::vector<double>& widths,
			     const WTextRenderer& renderer,
			     double rowHeight)
{
  double endY = ps.y;
  int endPage = ps.page;

  unsigned col = 0;

  x += cellSpacing;

  for (unsigned i = 0; i < children_.size(); ++i) {
    Block *c = children_[i];

    // TODO skip row-overspanned cells: we need to keep a list of
    // cells that are overspanning, and then we need to adjust their
    // height once done.

    if (c->isTableCell()) {
      int colSpan = c->attributeValue("colspan", 1);

      double width = 0;
      for (unsigned j = col; j < col + colSpan; ++j)
	width += widths[col];

      width += (colSpan - 1) * cellSpacing;

      PageState cellPs;
      cellPs.y = ps.y + cellSpacing;
      cellPs.page = ps.page;
      cellPs.minX = x;
      cellPs.maxX = x + width;

      double collapseMarginBottom = 0;
      double collapseMarginTop = std::numeric_limits<double>::max();

      collapseMarginBottom = c->layoutBlock(cellPs, false, renderer,
					    collapseMarginTop, 
					    collapseMarginBottom, 
					    rowHeight);

      if (collapseMarginBottom < collapseMarginTop)
	cellPs.y -= collapseMarginBottom;

      cellPs.minX = x;
      cellPs.maxX = x + width;
      Block::clearFloats(cellPs, width);

      if (cellPs.page > endPage
	  || (cellPs.page == endPage && cellPs.y > endY)) {
	endPage = cellPs.page;
	endY = cellPs.y;
      }

      col += colSpan;
      x += width + cellSpacing;
    }
  }

  ps.y = endY;
  ps.page = endPage;
}

void Block::tableComputeColumnWidths(std::vector<double>& minima,
				     std::vector<double>& maxima,
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

      c->tableComputeColumnWidths(minima, maxima, renderer, table);
    }
  } else if (type_ == DomElement_TR) {
    int col = 0;

    for (unsigned i = 0; i < children_.size(); ++i) {
      Block *c = children_[i];

      if (c->isTableCell()) {
	c->cellComputeColumnWidths(col, false, minima, renderer, table);
	col = c->cellComputeColumnWidths(col, true, maxima, renderer, table);
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
				   const WTextRenderer& renderer,
				   Block *table)
{
  double currentWidth = 0;

  int colSpan = attributeValue("colspan", 1);

  while (col + colSpan > (int)values.size())
    values.push_back(0.0);

  for (int i = 0; i < colSpan; ++i)
    currentWidth += values[col + i];

  double width = currentWidth;

  PageState ps;
  ps.y = 0;
  ps.page = 0;
  ps.minX = 0;
  ps.maxX = width;

  double origTableWidth = table->currentWidth_;
  if (!maximum)
    table->currentWidth_ = 0;

  layoutBlock(ps, maximum, renderer, 0, 0);

  table->currentWidth_ = origTableWidth;

  width = ps.maxX;

  if (width > currentWidth) {
    double extraPerColumn = (width - currentWidth) / colSpan;

    for (int i = 0; i < colSpan; ++i) {
      values[col + i] += extraPerColumn;
    }
  }

  return col + colSpan;
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

  if (type_ == DomElement_TABLE) {
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

      if (ps.y + height > renderer.textHeight(ps.page)) {
	pageBreak(ps);

	startPage = ps.page;
	startY = ps.y;

	ps.y += cssBorderWidth(Top, renderer.fontScale());
      }

      ps.y += height;
    } else {
      double cMinX = ps.minX + cssPadding(Left, renderer.fontScale())
	+ cssBorderWidth(Left, renderer.fontScale());
      double cMaxX = ps.maxX - cssPadding(Right, renderer.fontScale())
	- cssBorderWidth(Right, renderer.fontScale());

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
	  else {
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
	+ cssBorderWidth(Right, renderer.fontScale());

      advance(ps, spacerBottom, renderer);

      ps.y += cssPadding(Bottom, renderer.fontScale());
    }
  }

  ps.y += cssBorderWidth(Bottom, renderer.fontScale());

  double marginBottom = cssMargin(Bottom, renderer.fontScale());

  ps.y -= collapseMarginBottom;

  double height = cssHeight(renderer.fontScale());

  if (isTableCell())
    contentsHeight_ = diff(ps.y, ps.page, startY, startPage, renderer);

  if (height >= 0) {
    ps.page = startPage;
    ps.y = startY;

    if (isFloat()) // see supra, startY includes the margin
      ps.y += marginTop;

    advance(ps, height, renderer);
  }

  collapseMarginBottom = std::max(marginBottom, collapseMarginBottom);

  if (isFloat()) {
    ps.minX -= cssMargin(Left, renderer.fontScale());
    ps.maxX += cssMargin(Right, renderer.fontScale());
    ps.y += collapseMarginBottom;
    collapseMarginBottom = 0;
  }

  for (int i = startPage; i <= ps.page; ++i) {
    double boxY =  (i == startPage) ? startY : 0;
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

  ps.y += collapseMarginBottom;

  if (blockLayout.empty()) {
    ps.page = startPage;
    ps.y = inY;
    collapseMarginBottom = inCollapseMarginTop;
  }

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

    bool unknownWidth = blockCssWidth < 0 && currentWidth < (maxX - minX);

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

void Block::clearFloats(PageState &ps,
			double minWidth)
{
  /* Floats need to be cleared in order */
  for (; !ps.floats.empty();) {
    Block *b = ps.floats[0];

    ps.y = b->blockLayout.back().y + b->blockLayout.back().height;
    ps.page = b->blockLayout.back().page;

    ps.floats.erase(ps.floats.begin());

    Range rangeX(ps.minX, ps.maxX);
    adjustAvailableWidth(ps.y, ps.page, ps.floats, rangeX);

    if (rangeX.end - rangeX.start >= minWidth)
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

    if (availableWidth >= width)
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

    if (s.empty() && type_ != DomElement_TABLE)
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

void Block::reLayout(const BlockBox& from, const BlockBox& to)
{
  std::cerr << "Relayout: " << from.y << " -> " << to.y << std::endl;

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

      if (type_ == DomElement_THEAD) {
	/*
	 * This is the first or a repeated THEAD element
	 * (for a repeated:) move children here + render
	 */
	for (unsigned j = 0; j < children_.size(); ++j) {
	  if (i > 0)
	    children_[j]->reLayout(blockLayout[i-1], lb);
	  children_[j]->render(renderer, page);
	}
      }
    }
  }

  if (type_ != DomElement_THEAD)
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
    if (borderWidth[i] != 0) {
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
    || cssProperty == "padding";
}

std::string Block::cssProperty(Property property) const
{
  if (!node_)
    return std::string();

  if (css_.empty()) {
    std::string style = attributeValue("style");

    if (!style.empty()) {
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

	  css_[n] = v;
	  
	  if (isAggregate(n)) {
	    Wt::Utils::SplitVector allvalues;
	    boost::split(allvalues, v, boost::is_any_of(" \t\n"));

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

	    if (count == 0) {
	      LOG_ERROR("Strange aggregate CSS length property: '" << v << "'");
	    } else if (count == 1) {
	      css_[n + "-top"] = css_[n + "-right"] = css_[n + "-bottom"]
		= css_[n + "-left"]
		= Wt::Utils::splitEntryToString(allvalues[0]);
	    } else if (count == 2) {
	      css_[n + "-top"] = css_[n + "-bottom"] 
		= Wt::Utils::splitEntryToString(allvalues[0]);
	      css_[n + "-right"] = css_[n + "-left"] 
		= Wt::Utils::splitEntryToString(allvalues[1]);
	    } else if (count == 3) {
	      css_[n + "-top"] = Wt::Utils::splitEntryToString(allvalues[0]);
	      css_[n + "-right"] = css_[n + "-left"] 
		= Wt::Utils::splitEntryToString(allvalues[1]);
	      css_[n + "-bottom"] = Wt::Utils::splitEntryToString(allvalues[2]);
	    } else {
	      css_[n + "-top"] = Wt::Utils::splitEntryToString(allvalues[0]);
	      css_[n + "-right"] = Wt::Utils::splitEntryToString(allvalues[1]);
	      css_[n + "-bottom"] = Wt::Utils::splitEntryToString(allvalues[2]);
	      css_[n + "-left"] = Wt::Utils::splitEntryToString(allvalues[3]);
	    }
	  }
	}
      }
    }
  }

  std::map<std::string, std::string>::const_iterator i
    = css_.find(DomElement::cssName(property));

  if (i != css_.end())
    return i->second;
  else
    return std::string();
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
  LOG_ERROR("unsupported value '" << value << "' for attribute " << attribute);
}

void Block::unsupportedCssValue(Property property, const std::string& value)
{
  LOG_ERROR("unsupported value '" << value
	    << "'for CSS style property " << DomElement::cssName(property));
}

  }
}
