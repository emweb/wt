/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WCssDecorationStyle"

#include "Wt/WApplication"
#include "Wt/WWebWidget"
#include "Wt/WResource"

#include "DomElement.h"

using namespace Wt;

WCssDecorationStyle::WCssDecorationStyle()
  : widget_(0),
    cursor_(AutoCursor),
    backgroundImageRepeat_(RepeatXY),
    backgroundImageLocation_(0),
    textDecoration_(0),
    cursorChanged_(false),
    borderChanged_(false),
    foregroundColorChanged_(false),
    backgroundColorChanged_(false),
    backgroundImageChanged_(false),
    fontChanged_(false),
    textDecorationChanged_(false)
{
  for (unsigned i = 0; i < 4; ++i)
    border_[i] = 0;
}

WCssDecorationStyle::~WCssDecorationStyle()
{
  for (unsigned i = 0; i < 4; ++i)
    delete border_[i];
}

WCssDecorationStyle::WCssDecorationStyle(const WCssDecorationStyle& other):
  widget_(0)
{
  for (unsigned i = 0; i < 4; ++i)
    border_[i] = 0;

  copy(other);
  // *this = other;
}

WCssDecorationStyle&
WCssDecorationStyle::operator= (const WCssDecorationStyle& other)
{
  copy(other);
  // setCursor(other.cursor_);
  // setBackgroundColor(other.backgroundColor());
  // setBackgroundImage(other.backgroundImage(),
  // 		     other.backgroundImageRepeat(),
  // 		     other.backgroundImageLocation_);
  // setForegroundColor(other.foregroundColor());

  // for (unsigned i = 0; i < 4; ++i) {
  //   delete border_[i];
  //   if (other.border_[i])
  //     border_[i] = new WBorder(*other.border_[i]);
  //   else
  //     border_[i] = 0;
  // }

  // borderChanged_ = true;

  // setFont(other.font_);
  // setTextDecoration(other.textDecoration());

  return *this;
}

void WCssDecorationStyle::copy(const WCssDecorationStyle& other)
{
  setCursor(other.cursor_);
  setBackgroundColor(other.backgroundColor());
  setBackgroundImage(other.backgroundImage(),
		     other.backgroundImageRepeat(),
		     other.backgroundImageLocation_);
  setForegroundColor(other.foregroundColor());

  for (unsigned i = 0; i < 4; ++i) {
    delete border_[i];
    if (other.border_[i])
      border_[i] = new WBorder(*other.border_[i]);
    else
      border_[i] = 0;
  }

  borderChanged_ = true;

  setFont(other.font_);
  setTextDecoration(other.textDecoration());
}

void WCssDecorationStyle::setWebWidget(WWebWidget *w)
{
  widget_ = w;
  font_.setWebWidget(w);
}

void WCssDecorationStyle::changed(WFlags<RepaintFlag> flags)
{
  if (widget_)
    widget_->repaint(flags);
}

void WCssDecorationStyle::setCursor(Cursor c)
{
  if (!WWebWidget::canOptimizeUpdates()
      || cursor_ != c) {
    cursor_ = c;
    cursorChanged_ = true;
    changed();
  }
}

void WCssDecorationStyle::setCursor(std::string cursorImage, Cursor fallback)
{
  if (!WWebWidget::canOptimizeUpdates()
      || cursorImage_ != cursorImage
      || cursor_ != fallback) {
    cursorImage_ = cursorImage;
    cursor_ = fallback;
    cursorChanged_ = true;
    changed();
  }
}

void WCssDecorationStyle::setFont(const WFont& font)
{
  if (!WWebWidget::canOptimizeUpdates()
      || font_ != font) {
    font_ = font;
    fontChanged_ = true;
    changed(RepaintSizeAffected);
  }
}

void WCssDecorationStyle::setBackgroundImage(const WLink& image,
					     Repeat repeat,
					     WFlags<Side> sides)
{
  if (image.type() == WLink::Resource)
    image.resource()->dataChanged().
      connect(this, &WCssDecorationStyle::backgroundImageResourceChanged);

  if (!WWebWidget::canOptimizeUpdates()
      || backgroundImage_ != image
      || backgroundImageRepeat_ != repeat
      || backgroundImageLocation_ != sides) {
    backgroundImage_ = image;
    backgroundImageRepeat_ = repeat;
    backgroundImageLocation_ = sides;
    backgroundImageChanged_ = true;

    changed();
  }
}

#ifdef WT_TARGET_JAVA
void WCssDecorationStyle::setBackgroundImage(const std::string& url, 
					     Repeat repeat, 
					     WFlags<Side> sides)
{
  setBackgroundImage(WLink(url), repeat, sides);
}
#endif // WT_TARGET_JAVA

void WCssDecorationStyle::backgroundImageResourceChanged()
{
  if (backgroundImage_.type() == WLink::Resource) {
    backgroundImageChanged_ = true;
    changed();
  }
}

void WCssDecorationStyle::setBackgroundColor(WColor color)
{
  if (!WWebWidget::canOptimizeUpdates()
      || backgroundColor_ != color) {
    backgroundColorChanged_ = true;
    backgroundColor_ = color;
    changed();
  }
}

void WCssDecorationStyle::setForegroundColor(WColor color)
{
  if (!WWebWidget::canOptimizeUpdates()
      || foregroundColor_ != color) {
    foregroundColor_ = color;
    foregroundColorChanged_ = true;
    changed();
  }
}

void WCssDecorationStyle::setBorder(WBorder border, WFlags<Side> sides)
{
  Side theSides[4] = { Top, Right, Bottom, Left };

  for (unsigned i = 0; i < 4; ++i) {
    if (sides & theSides[i]) {
      delete border_[i];
      border_[i] = new WBorder(border);
    }

    borderChanged_ = true;
  }

  if (borderChanged_)
    changed(RepaintSizeAffected);
}

WBorder WCssDecorationStyle::border(Side side) const
{
  switch (side) {
  case Top: return borderI(0);
  case Right: return borderI(1);
  case Bottom: return borderI(2);
  case Left: return borderI(3);
  default: break;
  }

  return WBorder();
}

WBorder WCssDecorationStyle::borderI(unsigned i) const
{
  if (border_[i])
    return *border_[i];
  else
    return WBorder();
}

void WCssDecorationStyle::setTextDecoration(WFlags<TextDecoration> options)
{
  if (!WWebWidget::canOptimizeUpdates()
      || textDecoration_ != options) {
    textDecoration_ = options;
    textDecorationChanged_ = true;
    changed();
  }
}

void WCssDecorationStyle::updateDomElement(DomElement& element, bool all)
{
  /*
   * set cursor
   */
  if (cursorChanged_ || all) {
    switch (cursor_) {
    case AutoCursor:
      if (cursorChanged_)
	element.setProperty(PropertyStyleCursor, "auto");
      break;
    case ArrowCursor:
      element.setProperty(PropertyStyleCursor, "default"); break;
    case CrossCursor:
      element.setProperty(PropertyStyleCursor, "crosshair"); break;
    case PointingHandCursor:
      element.setProperty(PropertyStyleCursor, "pointer"); break;
    case OpenHandCursor:
      element.setProperty(PropertyStyleCursor, "move"); break;
    case WaitCursor:
      element.setProperty(PropertyStyleCursor, "wait"); break;
    case IBeamCursor:
      element.setProperty(PropertyStyleCursor, "text"); break;
    case WhatsThisCursor:
      element.setProperty(PropertyStyleCursor, "help"); break;
    }

    if (!cursorImage_.empty()) {
      element.setProperty(PropertyStyleCursor, 
			  "url(" + cursorImage_ + "),"
			  + element.getProperty(PropertyStyleCursor));
    }

    cursorChanged_ = false;
  }

  /*
   * set font
   */
  font_.updateDomElement(element, fontChanged_, all);
  fontChanged_ = false;

  /*
   * set border
   */
  Property properties[4] 
    = { PropertyStyleBorderTop,
	PropertyStyleBorderRight,
	PropertyStyleBorderBottom,
	PropertyStyleBorderLeft };

  if (borderChanged_ || all) {
    for (unsigned i = 0; i < 4; ++i) {
      if (border_[i])
	element.setProperty(properties[i], border_[i]->cssText());
      else if (borderChanged_)
	element.setProperty(properties[i], "");
    }

    borderChanged_ = false;
  }

  /*
   * set colors
   */
  if (foregroundColorChanged_ || all) {
    if ((all && !foregroundColor_.isDefault())
	|| foregroundColorChanged_)
      element.setProperty(PropertyStyleColor, foregroundColor_.cssText());
    foregroundColorChanged_ = false;
  }

  if (backgroundColorChanged_ || all) {
    if ((all && !backgroundColor_.isDefault()) ||
	backgroundColorChanged_)
      element.setProperty(PropertyStyleBackgroundColor,
			  backgroundColor_.cssText());
    backgroundColorChanged_ = false;
  }

  if (backgroundImageChanged_ || all) {
    if (!backgroundImage_.isNull() || backgroundImageChanged_) {
      if (backgroundImage_.isNull())
	element.setProperty(PropertyStyleBackgroundImage, "none");
      else {
	Wt::WApplication *app = Wt::WApplication::instance();

	std::string url = app->encodeUntrustedUrl
	  (app->resolveRelativeUrl(backgroundImage_.url()));

	element.setProperty(PropertyStyleBackgroundImage,
			    "url(" + WWebWidget::jsStringLiteral(url, '"')
			    + ")");
      }

      if (backgroundImageRepeat_ != RepeatXY ||
	  backgroundImageLocation_ != 0) {
	switch (backgroundImageRepeat_) {
	case RepeatXY:
	  element.setProperty(PropertyStyleBackgroundRepeat, "repeat");
	  break;
	case RepeatX:
	  element.setProperty(PropertyStyleBackgroundRepeat, "repeat-x");
	  break;
	case RepeatY:
	  element.setProperty(PropertyStyleBackgroundRepeat, "repeat-y");
	  break;
	case NoRepeat:
	  element.setProperty(PropertyStyleBackgroundRepeat, "no-repeat");
	  break;
	}

	if (backgroundImageLocation_) {
	  // www3schools claims this is needed for mozilla -- but not true ?
	  //element.setProperty(PropertyStyleBackgroundAttachment, "fixed");

	  std::string location;
	  if (backgroundImageLocation_ & CenterY)
	    location += " center";
	  else if (backgroundImageLocation_ & Bottom)
	    location += " bottom";
	  else
	    location += " top";

	  if (backgroundImageLocation_ & CenterX)
	    location += " center";
	  else if (backgroundImageLocation_ & Right)
	    location += " right";
	  else 
	    location += " left";

	  element.setProperty(PropertyStyleBackgroundPosition, location);
	}
      }
    }

    backgroundImageChanged_ = false;
  }

  if (textDecorationChanged_ ||  all) {
    std::string options;

    if (textDecoration_ & Underline)
      options += " underline";
    if (textDecoration_ & Overline)
      options += " overline";
    if (textDecoration_ & LineThrough)
      options += " line-through";
    if (textDecoration_ & Blink)
      options += " blink";

    if (!options.empty() || textDecorationChanged_)
      element.setProperty(PropertyStyleTextDecoration, options);

    textDecorationChanged_ = false;
  }
}

std::string WCssDecorationStyle::cssText()
{
  DomElement e(DomElement::ModeCreate, DomElement_A);
  updateDomElement(e, true);

  return e.cssStyle();
}
