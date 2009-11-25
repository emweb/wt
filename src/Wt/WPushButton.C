/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WPushButton"

#include "DomElement.h"

namespace Wt {

WPushButton::WPushButton(WContainerWidget *parent)
  : WFormWidget(parent)
{ }

WPushButton::WPushButton(const WString& text, WContainerWidget *parent)
  : WFormWidget(parent),
    text_(text)
{ }

void WPushButton::setText(const WString& text)
{
  if (canOptimizeUpdates() && (text == text_))
    return;

  text_ = text;
  flags_.set(BIT_TEXT_CHANGED);

  repaint(RepaintInnerHtml);
}

void WPushButton::setIcon(const std::string& url)
{
  if (canOptimizeUpdates() && (url == icon_))
    return;

  icon_ = url;
  flags_.set(BIT_ICON_CHANGED);

  repaint(RepaintInnerHtml);
}

DomElementType WPushButton::domElementType() const
{
  return DomElement_BUTTON;
}

void WPushButton::updateDom(DomElement& element, bool all)
{
  if (all) {
    element.setAttribute("type", "button");
    element.setProperty(PropertyClass, "Wt-btn");
  }

  if (flags_.test(BIT_ICON_CHANGED) || (all && !icon_.empty())) {
    DomElement *image = DomElement::createNew(DomElement_IMG);
    image->setProperty(PropertySrc, icon_);
    image->setId("im" + formName());
    element.insertChildAt(image, 0);
    flags_.set(BIT_ICON_RENDERED);
  }

  if (flags_.test(BIT_TEXT_CHANGED) || all) {
    element
      .setProperty(Wt::PropertyInnerHTML,
		   text_.literal() ? escapeText(text_, true).toUTF8()
		   : text_.toUTF8());
    flags_.reset(BIT_TEXT_CHANGED);
  }

  WFormWidget::updateDom(element, all);
}

void WPushButton::getDomChanges(std::vector<DomElement *>& result,
				WApplication *app)
{
  if (flags_.test(BIT_ICON_CHANGED) && flags_.test(BIT_ICON_RENDERED)) {
    DomElement *image = DomElement::getForUpdate("im" + formName(),
						 DomElement_IMG);
    if (icon_.empty()) {
      image->removeFromParent();
      flags_.reset(BIT_ICON_RENDERED);
    } else
      image->setProperty(PropertySrc, icon_);

    result.push_back(image);

    flags_.reset(BIT_ICON_CHANGED);
  }

  WFormWidget::getDomChanges(result, app);
}

void WPushButton::propagateRenderOk(bool deep)
{
  flags_.reset();

  WFormWidget::propagateRenderOk(deep);
}

void WPushButton::refresh()
{
  if (text_.refresh()) {
    flags_.set(BIT_TEXT_CHANGED);
    repaint(RepaintInnerHtml);
  }

  WFormWidget::refresh();
}

}
