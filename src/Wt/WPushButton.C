/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WPushButton"

#include "DomElement.h"

namespace Wt {

WPushButton::WPushButton(WContainerWidget *parent)
  : WFormWidget(parent),
    textChanged_(false)
{ }

WPushButton::WPushButton(const WString& text, WContainerWidget *parent)
  : WFormWidget(parent),
    text_(text),
    textChanged_(false)
{ }

void WPushButton::setText(const WString& text)
{
  if (canOptimizeUpdates() && (text == text_))
    return;

  text_ = text;
  textChanged_ = true;

  repaint(RepaintInnerHtml);
}

DomElementType WPushButton::domElementType() const
{
  return DomElement_BUTTON;
}

void WPushButton::updateDom(DomElement& element, bool all)
{
  if (all)
    element.setAttribute("type", "button");

  if (textChanged_ || all) {
    element
      .setProperty(Wt::PropertyInnerHTML,
		   text_.literal() ? escapeText(text_).toUTF8()
		   : text_.toUTF8());
    textChanged_ = false;
  }

  WFormWidget::updateDom(element, all);
}

void WPushButton::refresh()
{
  if (text_.refresh()) {
    textChanged_ = true;
    repaint(RepaintInnerHtml);
  }

  WFormWidget::refresh();
}

}
