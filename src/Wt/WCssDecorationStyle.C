/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WCssDecorationStyle"

#include "Wt/WApplication"
#include "Wt/WWidget"
#include "Wt/WWebWidget"

#include "DomElement.h"

using namespace Wt;

WCssDecorationStyle::WCssDecorationStyle()
  : widget_(0),
    cursor_(Auto),
    backgroundImageRepeat_(RepeatXY),
    backgroundImageLocation_(0),
    textDecoration_(0),
    borderPosition_(0),
    cursorChanged_(false),
    borderChanged_(false),
    foregroundColorChanged_(false),
    backgroundColorChanged_(false),
    backgroundImageChanged_(false),
    fontChanged_(false),
    textDecorationChanged_(false)
{ }

WCssDecorationStyle&
WCssDecorationStyle::operator= (const WCssDecorationStyle& other)
{
  setCursor(other.cursor_);
  setBackgroundColor(other.backgroundColor());
  setBackgroundImage(other.backgroundImage(),
		     other.backgroundImageRepeat(),
		     other.backgroundImageLocation_);
  setForegroundColor(other.foregroundColor());
  setBorder(other.border());
  setFont(other.font_);
  setTextDecoration(other.textDecoration());

  return *this;
}

void WCssDecorationStyle::setWebWidget(WWebWidget *w)
{
  widget_ = w;
  font_.setWebWidget(w);
}

void WCssDecorationStyle::changed()
{
  if (widget_) widget_->repaint(WWebWidget::RepaintPropertyAttribute);
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

void WCssDecorationStyle::setFont(const WFont& font)
{
  if (!WWebWidget::canOptimizeUpdates()
      || font_ != font) {
    font_ = font;
    fontChanged_ = true;
    changed();
  }
}

void WCssDecorationStyle::setBackgroundImage(const std::string& image,
					     Repeat repeat,
					     int sides)
{
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

void WCssDecorationStyle::setBorder(WBorder border, int sides)
{
  if (!WWebWidget::canOptimizeUpdates()
      || border_ != border
      || borderPosition_ != sides) {
    border_ = border;
    borderPosition_ = sides;
    borderChanged_ = true;
    changed();
  }
}

void WCssDecorationStyle::setTextDecoration(int options)
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
    case Auto:
      if (cursorChanged_)
	element.setProperty(PropertyStyleCursor, "auto");
      break;
    case Default:
      element.setProperty(PropertyStyleCursor, "default"); break;
    case CrossHair:
      element.setProperty(PropertyStyleCursor, "crosshair"); break;
    case Pointer:
      element.setProperty(PropertyStyleCursor, "pointer"); break;
    case Move:
      element.setProperty(PropertyStyleCursor, "move"); break;
    case Wait:
      element.setProperty(PropertyStyleCursor, "wait"); break;
    case Text:
      element.setProperty(PropertyStyleCursor, "text"); break;
    case Help:
      element.setProperty(PropertyStyleCursor, "help"); break;
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
  if (borderChanged_ || all) {
    if (borderChanged_ || (element.type() == DomElement_IFRAME)
	|| (border_.style() != WBorder::None)) {
	
      if (borderPosition_ & Top)
	  element.setProperty(PropertyStyleBorderTop, border_.cssText());
      if (borderPosition_ & Left)
	  element.setProperty(PropertyStyleBorderLeft, border_.cssText());
      if (borderPosition_ & Right)
	  element.setProperty(PropertyStyleBorderRight, border_.cssText());
      if (borderPosition_ & Bottom)
	  element.setProperty(PropertyStyleBorderBottom, border_.cssText());
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
    if ((all && !backgroundColor_.isDefault())
	|| backgroundColorChanged_)
      element.setProperty(PropertyStyleBackgroundColor,
			  backgroundColor_.cssText());
    backgroundColorChanged_ = false;
  }

  if (backgroundImageChanged_ || all) {
    if ((backgroundImage_.length() != 0) || backgroundImageChanged_) {
      element.setProperty(PropertyStyleBackgroundImage,
			  backgroundImage_.length() > 0
			  ? "url("
	  + WApplication::instance()->fixRelativeUrl(backgroundImage_) + ")" 
			  : "none");
      switch (backgroundImageRepeat_) {
      case RepeatXY:
	element.setProperty(PropertyStyleBackgroundRepeat, "repeat"); break;
      case RepeatX:
	element.setProperty(PropertyStyleBackgroundRepeat, "repeat-x"); break;
      case RepeatY:
	element.setProperty(PropertyStyleBackgroundRepeat, "repeat-y"); break;
      case NoRepeat:
	element.setProperty(PropertyStyleBackgroundRepeat, "no-repeat");break;
      }

      if (backgroundImageLocation_) {
	// www3schools claims this is needed for mozilla -- but not true ?
	//element.setProperty(PropertyStyleBackgroundAttachment, "fixed");

	std::string location;
	if (backgroundImageLocation_ & WWidget::CenterY)
	  location += " center";
	else if (backgroundImageLocation_ & WWidget::Bottom)
	  location += " bottom";
	else
	  location += " top";

	if (backgroundImageLocation_ & WWidget::CenterX)
	  location += " center";
	else if (backgroundImageLocation_ & WWidget::Right)
	  location += " right";
	else 
	  location += " left";

	element.setProperty(PropertyStyleBackgroundPosition, location);
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
