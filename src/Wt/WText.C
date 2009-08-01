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

namespace Wt {

WText::WText(WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(XHTMLText),
    wordWrap_(true),
    textChanged_(false),
    wordWrapChanged_(false)
{
  WT_DEBUG(setObjectName("WText"));
}

WText::WText(const WString& text, WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(XHTMLText),
    wordWrap_(true),
    textChanged_(false),
    wordWrapChanged_(false)
{
  WT_DEBUG(setObjectName("WText"));
  setText(text);
}

WText::WText(const WString& text, TextFormat format, WContainerWidget *parent)
  : WInteractWidget(parent),
    textFormat_(format),
    wordWrap_(true),
    textChanged_(false),
    wordWrapChanged_(false)
{
  WT_DEBUG(setObjectName("WText"));
  setText(text);
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

  autoAdjustInline();

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

void WText::refresh()
{
  if (text_.refresh()) {
    textChanged_ = true;
    autoAdjustInline();
    repaint(RepaintInnerHtml);
  }

  WInteractWidget::refresh();
}

}
