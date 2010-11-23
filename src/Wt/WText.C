/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/algorithm/string.hpp>
#include <iostream>

#include "Wt/WApplication"
#include "Wt/WText"
#include "DomElement.h"
#include "WtException.h"

namespace Wt {

WText::WText(WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(XHTMLText),
    wordWrap_(true),
    textChanged_(false),
    wordWrapChanged_(false),
    paddingsChanged_(false),
    padding_(0)
{
  WT_DEBUG(setObjectName("WText"));
}

WText::WText(const WString& text, WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(XHTMLText),
    wordWrap_(true),
    textChanged_(false),
    wordWrapChanged_(false),
    paddingsChanged_(false),
    padding_(0)
{
  WT_DEBUG(setObjectName("WText"));
  setText(text);
}

WText::WText(const WString& text, TextFormat format, WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(format),
    wordWrap_(true),
    textChanged_(false),
    wordWrapChanged_(false),
    paddingsChanged_(false),
    padding_(0)
{
  WT_DEBUG(setObjectName("WText"));
  setText(text);
}

WText::~WText() 
{
  delete[] padding_;
}

bool WText::setText(const WString& text)
{
  if (canOptimizeUpdates() && (text == text_))
    return true;

  text_ = text;

  bool textok = checkWellFormed();
  if (!textok)
    textFormat_ = PlainText;

  textChanged_ = true;
  repaint(RepaintInnerHtml);

  return textok;
}

void WText::autoAdjustInline()
{
  if (textFormat_ != PlainText && isInline()) {
    std::string t = text_.toUTF8();
    boost::trim_left(t);
    if (   boost::istarts_with(t, "<div")
	|| boost::istarts_with(t, "<p")
	|| boost::istarts_with(t, "<h"))
      setInline(false);
  }
}

void WText::setWordWrap(bool wordWrap)
{
  if (wordWrap_ != wordWrap) {
    wordWrap_ = wordWrap;
    wordWrapChanged_ = true;
    repaint(RepaintPropertyAttribute);
  }
}

void WText::updateDom(DomElement& element, bool all)
{
  if (textChanged_ || all) {
    std::string text = formattedText();
    if (textChanged_ || !text.empty())
      element.setProperty(Wt::PropertyInnerHTML, formattedText());
    textChanged_ = false;
  }

  if (wordWrapChanged_ || all) {
    if (!all || !wordWrap_)
      element.setProperty(Wt::PropertyStyleWhiteSpace,
			  wordWrap_ ? "normal" : "nowrap");
    wordWrapChanged_ = false;
  }

  if (paddingsChanged_
      || (all && padding_ &&
	  !(padding_[0].isAuto() && padding_[1].isAuto()))) {
    
    element.setProperty(PropertyStylePaddingRight, padding_[0].cssText());
    element.setProperty(PropertyStylePaddingLeft, padding_[1].cssText());

    paddingsChanged_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

void WText::propagateRenderOk(bool deep)
{
  textChanged_ = false;
  wordWrapChanged_ = false;

  WInteractWidget::propagateRenderOk(deep);
}

bool WText::setTextFormat(TextFormat textFormat)
{
  if (textFormat_ != textFormat) {
    TextFormat oldTextFormat = textFormat_;

    textFormat_ = textFormat;
    bool textok = checkWellFormed();

    if (!textok)
      textFormat_ = oldTextFormat;

    return textok;
  } else
    return true;
}

bool WText::checkWellFormed()
{
  if (textFormat_ == XHTMLText && text_.literal()) {
    return removeScript(text_);
  } else
    return true;
}

void WText::setPadding(const WLength& length, WFlags<Side> sides)
{
  if (!padding_) {
    padding_ = new WLength[2];
#ifdef WT_TARGET_JAVA
    padding_[0] = padding_[1] = WLength::Auto;
#endif // WT_TARGET_JAVA
  }

  if (sides.testFlag(Right))
    padding_[0] = length;
  if (sides.testFlag(Left))
    padding_[1] = length;

  if (sides.testFlag(Top))
    throw WtException("WText::padding on Top is not supported.");
  if (sides.testFlag(Bottom))
    throw WtException("WText::padding on Bottom is not supported.");

  paddingsChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

WLength WText::padding(Side side) const
{
  if (!padding_)
    return WLength::Auto;

  switch (side) {
  case Top:
    throw WtException("WText::padding on Top is not supported.");
  case Right:
    return padding_[1];
  case Bottom:
    throw WtException("WText::padding on Bottom is not supported.");
  case Left:
    return padding_[3];
  default:
    throw WtException("WText::padding(Side) with invalid side.");
  }
}

std::string WText::formattedText() const
{
  if (textFormat_ == PlainText)
    return escapeText(text_, true).toUTF8();
  else
    return text_.toUTF8();
}

DomElementType WText::domElementType() const
{
  return isInline() ? DomElement_SPAN : DomElement_DIV;
}

void WText::render(WFlags<RenderFlag> flags)
{
  if (textChanged_)
    autoAdjustInline();
}

void WText::refresh()
{
  if (text_.refresh()) {
    textChanged_ = true;
    repaint(RepaintInnerHtml);
  }

  WInteractWidget::refresh();
}

}
