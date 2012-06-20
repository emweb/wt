/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WPushButton"
#include "Wt/WResource"

#include "DomElement.h"

namespace Wt {

WPushButton::WPushButton(WContainerWidget *parent)
  : WFormWidget(parent),
    linkTarget_(TargetSelf),
    redirectJS_(0)
{ }

WPushButton::WPushButton(const WString& text, WContainerWidget *parent)
  : WFormWidget(parent),
    text_(text),
    linkTarget_(TargetSelf),
    redirectJS_(0)
{ }

WPushButton::~WPushButton()
{
  delete redirectJS_;
}

void WPushButton::setText(const WString& text)
{
  if (canOptimizeUpdates() && (text == text_))
    return;

  text_ = text;
  flags_.set(BIT_TEXT_CHANGED);

  repaint(RepaintInnerHtml);
}

void WPushButton::setIcon(const WLink& link)
{
  if (canOptimizeUpdates() && (link == icon_))
    return;

  icon_ = link;
  flags_.set(BIT_ICON_CHANGED);

  repaint(RepaintInnerHtml);
}

void WPushButton::setLink(const WLink& link)
{
  if (link == link_)
    return;

  link_ = link;
  flags_.set(BIT_LINK_CHANGED);

  if (link.type() == WLink::Resource)
    link.resource()->dataChanged().connect(this, &WPushButton::resourceChanged);

  repaint(RepaintPropertyIEMobile);
}

void WPushButton::setLinkTarget(AnchorTarget target)
{
  linkTarget_ = target;
}

void WPushButton::setRef(const std::string& url)
{
  setLink(WLink(url));
}

void WPushButton::setResource(WResource *resource)
{
  setLink(WLink(resource));
}

void WPushButton::resourceChanged()
{
  flags_.set(BIT_LINK_CHANGED);
  repaint(RepaintPropertyIEMobile);
}

void WPushButton::doRedirect()
{
  WApplication *app = WApplication::instance();

  if (!app->environment().ajax()) {
    if (link_.type() == WLink::InternalPath)
      app->setInternalPath(link_.internalPath().toUTF8(), true);
    else
      app->redirect(link_.url());
  }
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

  bool updateInnerHtml = !icon_.isNull() && flags_.test(BIT_TEXT_CHANGED);

  if (updateInnerHtml || flags_.test(BIT_ICON_CHANGED)
      || (all && !icon_.isNull())) {
    DomElement *image = DomElement::createNew(DomElement_IMG);
    image->setProperty(PropertySrc, icon_.url());
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

  if (flags_.test(BIT_LINK_CHANGED) || (all && !link_.isNull())) {
    if (!link_.isNull()) {
      WApplication *app = WApplication::instance();

      if (!redirectJS_) {
	redirectJS_ = new JSlot();
	clicked().connect(*redirectJS_);

	if (!app->environment().ajax())
	  clicked().connect(this, &WPushButton::doRedirect);
      }

      if (link_.type() == WLink::InternalPath)
	redirectJS_->setJavaScript
	  ("function(){"
	   WT_CLASS ".history.navigate(" + jsStringLiteral(link_.internalPath())
	   + ",true);"
	   "}");
      else
	if (linkTarget_ == TargetNewWindow)
	  redirectJS_->setJavaScript
	    ("function(){"
	     "window.open(" + jsStringLiteral(link_.url()) + ");"
	     "}");
	else
	  redirectJS_->setJavaScript
	    ("function(){"
	     "window.location=" + jsStringLiteral(link_.url()) + ";"
	     "}");
      clicked().senderRepaint(); // XXX only for Java port necessary
    } else {
      delete redirectJS_;
      redirectJS_ = 0;
    }
  }

  WFormWidget::updateDom(element, all);
}

void WPushButton::getDomChanges(std::vector<DomElement *>& result,
				WApplication *app)
{
  if (flags_.test(BIT_ICON_CHANGED) && flags_.test(BIT_ICON_RENDERED)) {
    DomElement *image
      = DomElement::getForUpdate("im" + formName(), DomElement_IMG);
    if (icon_.isNull()) {
      image->removeFromParent();
      flags_.reset(BIT_ICON_RENDERED);
    } else
      image->setProperty(PropertySrc, icon_.url());

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

WT_USTRING WPushButton::valueText() const
{
  return WT_USTRING();
}

void WPushButton::setValueText(const WT_USTRING& value)
{ }

void WPushButton::refresh()
{
  if (text_.refresh()) {
    flags_.set(BIT_TEXT_CHANGED);
    repaint(RepaintInnerHtml);
  }

  WFormWidget::refresh();
}

}
