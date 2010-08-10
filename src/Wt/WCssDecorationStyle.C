/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WCssDecorationStyle"

#include "Wt/WApplication"
#include "Wt/WWidget"
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
  if (widget_) widget_->repaint(RepaintPropertyAttribute);
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
    changed();
  }
}

void WCssDecorationStyle::setBackgroundImage(const std::string& image,
					     Repeat repeat,
					     WFlags<Side> sides)
{
  if (!WWebWidget::canOptimizeUpdates()
      || backgroundImage_ != image
      || backgroundImageRepeat_ != repeat
      || backgroundImageLocation_ != sides) {
    backgroundImage_ = image;
    backgroundImageResource_ = 0;
    backgroundImageRepeat_ = repeat;
    backgroundImageLocation_ = sides;
    backgroundImageChanged_ = true;
    changed();
  }
}

void WCssDecorationStyle::setBackgroundImage(WResource *resource,
					     Repeat repeat,
					     WFlags<Side> sides)
{
  backgroundImageResource_ = resource;
  resource->dataChanged().
    connect(this, &WCssDecorationStyle::backgroundImageResourceChanged);
  setBackgroundImage(resource->url(), repeat, sides);
}

void WCssDecorationStyle::backgroundImageResourceChanged()
{
  if (backgroundImageResource_) {
    setBackgroundImage(backgroundImageResource_->url(),
                       backgroundImageRepeat_, backgroundImageLocation_);
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
  if (!WWebWidget::canOptimizeUpdates()
      || border_ != border
      || borderPosition_ != sides) {
    border_ = border;
    borderPosition_ = sides;
    borderChanged_ = true;
    changed();
  }
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
  if (borderChanged_ || all) {
    bool elementHasDefaultBorder
      = element.type() == DomElement_IFRAME
      || element.type() == DomElement_INPUT
      || element.type() == DomElement_SELECT
      || element.type() == DomElement_TEXTAREA;

    if (borderChanged_
	|| elementHasDefaultBorder
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
