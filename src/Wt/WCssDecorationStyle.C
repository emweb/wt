/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WCssDecorationStyle.h"

#include "Wt/WApplication.h"
#include "Wt/WWebWidget.h"
#include "Wt/WResource.h"

#include "DomElement.h"

using namespace Wt;

WCssDecorationStyle::WCssDecorationStyle()
  : widget_(nullptr),
    cursor_(Cursor::Auto),
    backgroundImageRepeat_(Orientation::Horizontal | Orientation::Vertical),
    backgroundImageLocation_(None),
    textDecoration_(None),
    cursorChanged_(false),
    borderChanged_(false),
    foregroundColorChanged_(false),
    backgroundColorChanged_(false),
    backgroundImageChanged_(false),
    fontChanged_(false),
    textDecorationChanged_(false)
{ }

WCssDecorationStyle::~WCssDecorationStyle()
{ }

WCssDecorationStyle::WCssDecorationStyle(const WCssDecorationStyle& other)
  : WObject(),
    widget_(0)
{
  copy(other);
}

WCssDecorationStyle&
WCssDecorationStyle::operator= (const WCssDecorationStyle& other)
{
  copy(other);

  return *this;
}

void WCssDecorationStyle::copy(const WCssDecorationStyle& other)
{
  if (this == &other)
    return;

  setCursor(other.cursor_);
  setBackgroundColor(other.backgroundColor());
  setBackgroundImage(other.backgroundImage(),
		     other.backgroundImageRepeat(),
		     other.backgroundImageLocation_);
  setForegroundColor(other.foregroundColor());

  for (unsigned i = 0; i < 4; ++i) {
    if (other.border_[i])
      border_[i] = std::unique_ptr<WBorder>(new WBorder(*other.border_[i]));
    else
      border_[i] = nullptr;
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
      || !cursorImage_.empty()
      || cursor_ != c) {
    cursorImage_.clear();
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
    changed(RepaintFlag::SizeAffected);
  }
}

void WCssDecorationStyle::setBackgroundImage(const WLink& image,
					     WFlags<Orientation> repeat,
					     WFlags<Side> sides)
{
  if (image.type() == LinkType::Resource)
    image.resource()->dataChanged().connect
      (this, &WCssDecorationStyle::backgroundImageResourceChanged);

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
					     WFlags<Orientation> repeat,
					     WFlags<Side> sides)
{
  setBackgroundImage(WLink(url), repeat, sides);
}
#endif // WT_TARGET_JAVA

void WCssDecorationStyle::backgroundImageResourceChanged()
{
  if (backgroundImage_.type() == LinkType::Resource) {
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
  Side theSides[4] = { Side::Top, Side::Right, Side::Bottom, Side::Left };

  for (unsigned i = 0; i < 4; ++i) {
    if (sides.test(theSides[i])) {
      border_[i] = std::unique_ptr<WBorder>(new WBorder(border));
    }

    borderChanged_ = true;
  }

  if (borderChanged_)
    changed(RepaintFlag::SizeAffected);
}

WBorder WCssDecorationStyle::border(Side side) const
{
  switch (side) {
  case Side::Top: return borderI(0);
  case Side::Right: return borderI(1);
  case Side::Bottom: return borderI(2);
  case Side::Left: return borderI(3);
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
    case Cursor::Auto:
      if (cursorChanged_)
	element.setProperty(Property::StyleCursor, "auto");
      break;
    case Cursor::Arrow:
      element.setProperty(Property::StyleCursor, "default"); break;
    case Cursor::Cross:
      element.setProperty(Property::StyleCursor, "crosshair"); break;
    case Cursor::PointingHand:
      element.setProperty(Property::StyleCursor, "pointer"); break;
    case Cursor::OpenHand:
      element.setProperty(Property::StyleCursor, "move"); break;
    case Cursor::Wait:
      element.setProperty(Property::StyleCursor, "wait"); break;
    case Cursor::IBeam:
      element.setProperty(Property::StyleCursor, "text"); break;
    case Cursor::WhatsThis:
      element.setProperty(Property::StyleCursor, "help"); break;
    }

    if (!cursorImage_.empty()) {
      element.setProperty(Property::StyleCursor, 
			  "url(" + cursorImage_ + "),"
			  + element.getProperty(Property::StyleCursor));
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
    = { Property::StyleBorderTop,
	Property::StyleBorderRight,
	Property::StyleBorderBottom,
	Property::StyleBorderLeft };

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
      element.setProperty(Property::StyleColor, foregroundColor_.cssText());
    foregroundColorChanged_ = false;
  }

  if (backgroundColorChanged_ || all) {
    if ((all && !backgroundColor_.isDefault()) ||
	backgroundColorChanged_)
      element.setProperty(Property::StyleBackgroundColor,
			  backgroundColor_.cssText());
    backgroundColorChanged_ = false;
  }

  if (backgroundImageChanged_ || all) {
    if (!backgroundImage_.isNull() || backgroundImageChanged_) {
      if (backgroundImage_.isNull())
	element.setProperty(Property::StyleBackgroundImage, "none");
      else {
	Wt::WApplication *app = Wt::WApplication::instance();

	std::string url = app->encodeUntrustedUrl
	  (app->resolveRelativeUrl(backgroundImage_.url()));

	element.setProperty(Property::StyleBackgroundImage,
			    "url(" + WWebWidget::jsStringLiteral(url, '"')
			    + ")");
      }

      if (backgroundImageRepeat_ != 
	  (Orientation::Horizontal | Orientation::Vertical) ||
	  !backgroundImageLocation_.empty()) {
	if (backgroundImageRepeat_ == (Orientation::Horizontal |
				       Orientation::Vertical))
	  element.setProperty(Property::StyleBackgroundRepeat, "repeat");
	else if (backgroundImageRepeat_ == Orientation::Horizontal)
	  element.setProperty(Property::StyleBackgroundRepeat, "repeat-x");
	else if (backgroundImageRepeat_ == Orientation::Vertical)
	  element.setProperty(Property::StyleBackgroundRepeat, "repeat-y");
	else
	  element.setProperty(Property::StyleBackgroundRepeat, "no-repeat");

	if (!backgroundImageLocation_.empty()) {
	  // www3schools claims this is needed for mozilla -- but not true ?
	  //element.setProperty(Property::StyleBackgroundAttachment, "fixed");

	  std::string location;
	  if (backgroundImageLocation_.test(Side::CenterY))
	    location += " center";
	  else if (backgroundImageLocation_.test(Side::Bottom))
	    location += " bottom";
	  else
	    location += " top";

	  if (backgroundImageLocation_.test(Side::CenterX))
	    location += " center";
	  else if (backgroundImageLocation_.test(Side::Right))
	    location += " right";
	  else 
	    location += " left";

	  element.setProperty(Property::StyleBackgroundPosition, location);
	}
      }
    }

    backgroundImageChanged_ = false;
  }

  if (textDecorationChanged_ ||  all) {
    std::string options;

    if (textDecoration_.test(TextDecoration::Underline))
      options += " underline";
    if (textDecoration_.test(TextDecoration::Overline))
      options += " overline";
    if (textDecoration_.test(TextDecoration::LineThrough))
      options += " line-through";
    if (textDecoration_.test(TextDecoration::Blink))
      options += " blink";

    if (!options.empty() || textDecorationChanged_)
      element.setProperty(Property::StyleTextDecoration, options);

    textDecorationChanged_ = false;
  }
}

std::string WCssDecorationStyle::cssText()
{
  DomElement e(DomElement::Mode::Create, DomElementType::A);
  updateDomElement(e, true);

  return e.cssStyle();
}
