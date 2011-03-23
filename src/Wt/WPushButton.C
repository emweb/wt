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
#include "Utils.h"

namespace Wt {

WPushButton::WPushButton(WContainerWidget *parent)
  : WFormWidget(parent),
    resource_(0),
    redirectJS_(0)
{ }

WPushButton::WPushButton(const WString& text, WContainerWidget *parent)
  : WFormWidget(parent),
    text_(text),
    resource_(0),
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

void WPushButton::setIcon(const std::string& url)
{
  if (canOptimizeUpdates() && (url == icon_))
    return;

  icon_ = url;
  flags_.set(BIT_ICON_CHANGED);

  repaint(RepaintInnerHtml);
}

void WPushButton::setRef(const std::string& url)
{
  if (!flags_.test(BIT_REF_INTERNAL_PATH) && ref_ == url)
    return;

  flags_.reset(BIT_REF_INTERNAL_PATH);
  ref_ = url;

  flags_.set(BIT_REF_CHANGED);

  repaint(RepaintPropertyIEMobile);
}

void WPushButton::setRefInternalPath(const std::string& path)
{
  if (flags_.test(BIT_REF_INTERNAL_PATH) && ref_ == path)
    return;

  flags_.set(BIT_REF_INTERNAL_PATH);
  ref_ = path;

  flags_.set(BIT_REF_CHANGED);

  repaint(RepaintPropertyIEMobile);
}

void WPushButton::setResource(WResource *resource)
{
  resource_ = resource;

  if (resource_) {
    resource_->dataChanged().connect(this, &WPushButton::resourceChanged);
    resourceChanged();
  }
}

void WPushButton::resourceChanged()
{
  setRef(resource_->url());
}

void WPushButton::doRedirect()
{
  WApplication *app = WApplication::instance();

  if (!app->environment().ajax()) {
    if (flags_.test(BIT_REF_INTERNAL_PATH))
      app->setInternalPath(ref_, true);
    else
      app->redirect(ref_);
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

  if (flags_.test(BIT_REF_CHANGED) || (all && !ref_.empty())) {
    if (!ref_.empty()) {
      WApplication *app = WApplication::instance();

      if (!redirectJS_) {
	redirectJS_ = new JSlot();
	clicked().connect(*redirectJS_);

	if (!app->environment().ajax())
	  clicked().connect(this, &WPushButton::doRedirect);
      }

      if (flags_.test(BIT_REF_INTERNAL_PATH))
	redirectJS_->setJavaScript
	  ("function(){"
	   WT_CLASS ".history.navigate(" + jsStringLiteral(ref_) + ",true);"
	   "}");
      else
	redirectJS_->setJavaScript
	  ("function(){"
	   "window.location=" + jsStringLiteral(ref_) + ";"
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
