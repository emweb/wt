/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>

#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WResource"
#include "Wt/WText"

#include "DomElement.h"
#include "Utils.h"
#include "WebSession.h"

namespace Wt {

WAnchor::WAnchor(WContainerWidget *parent)
  : WContainerWidget(parent),
    resource_(0),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{
  setInline(true);
}

WAnchor::WAnchor(const std::string& ref, WContainerWidget *parent)
  : WContainerWidget(parent),
    ref_(ref),
    resource_(0),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{
  setInline(true);
}

WAnchor::WAnchor(WResource *resource, WContainerWidget *parent)
  : WContainerWidget(parent),
    resource_(0),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{
  setInline(true);
  setResource(resource);
}

WAnchor::WAnchor(const std::string& ref, const WString& text,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    ref_(ref),
    resource_(0),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{ 
  setInline(true);
  text_ = new WText(text, this);
}

WAnchor::WAnchor(WResource *resource, const WString& text,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    resource_(0),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{ 
  setInline(true);
  text_ = new WText(text, this);
  setResource(resource);
}

WAnchor::WAnchor(const std::string& ref, WImage *image,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    ref_(ref),
    resource_(0),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{ 
  setInline(true);
  image_ = image;

  if (image_)
    addWidget(image_);
}

WAnchor::WAnchor(WResource *resource, WImage *image,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    resource_(0),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{ 
  setInline(true);
  image_ = image;

  if (image_)
    addWidget(image_);

  setResource(resource);
}

WAnchor::~WAnchor()
{
  delete changeInternalPathJS_;
}

void WAnchor::setRef(const std::string& url)
{
  if (!flags_.test(BIT_REF_INTERNAL_PATH) && ref_ == url)
    return;

  flags_.reset(BIT_REF_INTERNAL_PATH);
  ref_ = url;

  flags_.set(BIT_REF_CHANGED);

  repaint(RepaintPropertyIEMobile);
}

void WAnchor::setRefInternalPath(const std::string& path)
{
  if (flags_.test(BIT_REF_INTERNAL_PATH) && path == ref_)
    return;

  flags_.set(BIT_REF_INTERNAL_PATH);
  ref_ = path;

  flags_.set(BIT_REF_CHANGED);

  repaint(RepaintPropertyIEMobile);
}

void WAnchor::setResource(WResource *resource)
{
  resource_ = resource;

  if (resource_) {
    resource_->dataChanged().connect(this, &WAnchor::resourceChanged);
    resourceChanged();
  }
}

void WAnchor::setTarget(AnchorTarget target)
{
  if (target_ != target) {
    target_ = target;
    flags_.set(BIT_TARGET_CHANGED);
  }
}

const WString& WAnchor::text() const
{
  static WString empty("");
  if (text_)
    return text_->text();
  else
    return empty;
}

void WAnchor::setText(const WString& text)
{
  if (!text_) {
    text_ = new WText(text, this);
  } else
    if (!text.empty())
      text_->setText(text);
    else {
      delete text_;
      text_ = 0;
    }
}

void WAnchor::setWordWrap(bool wordWrap)
{
  if (!text_)
    text_ = new WText(this);

  text_->setWordWrap(wordWrap);
}

bool WAnchor::wordWrap() const
{
  return text_ ? text_->wordWrap() : true;
}

void WAnchor::setImage(WImage *image)
{
  delete image_;
  image_ = image;

  if (image_)
    addWidget(image_);
}

void WAnchor::resourceChanged()
{
  setRef(resource_->url());
}

void WAnchor::enableAjax()
{
  if (flags_.test(BIT_REF_INTERNAL_PATH)) {
    flags_.set(BIT_REF_CHANGED);
    repaint(RepaintPropertyIEMobile);
  }

  WContainerWidget::enableAjax();
}

void WAnchor::updateDom(DomElement& element, bool all)
{
  if (flags_.test(BIT_REF_CHANGED) || all) {
    std::string url;
    if (flags_.test(BIT_REF_INTERNAL_PATH)) {
      WApplication *app = WApplication::instance();

      if (app->environment().ajax()) {
	url = app->bookmarkUrl(ref_);

	/*
	 * From 但浩亮: setRefInternalPath() and setTarget(TargetNewWindow)
	 * does not work without the check below:
	 */
	if (target_ == TargetSelf) {
	  if (!changeInternalPathJS_) {
	    changeInternalPathJS_ = new JSlot();
	    clicked().connect(*changeInternalPathJS_);
	    clicked().preventDefaultAction();
	  }

	  changeInternalPathJS_->setJavaScript
	    ("function(){"
	     "window.location.hash='#" + Utils::urlEncode(ref_) + "';"
	     "}");
	  clicked().senderRepaint(); // XXX only for Java port necessary
	}
      } else {
	if (app->environment().agentIsSpiderBot())
	  url = app->bookmarkUrl(ref_);
	else {
	  // If no JavaScript is available, then we still add the session
	  // so that when used in WAnchor it will be handled by the same
	  // session.
	  url = app->session()->mostRelativeUrl(ref_);
	}
      }
    } else {
      url = ref_;

      delete changeInternalPathJS_;
      changeInternalPathJS_ = 0;
    }

    element.setAttribute("href", fixRelativeUrl(url));

    flags_.reset(BIT_REF_CHANGED);
  }

  if (flags_.test(BIT_TARGET_CHANGED) || all) {
    switch (target_) {
    case TargetSelf:
      if (!all)
	element.setProperty(PropertyTarget, "_self");
      break;
    case TargetThisWindow:
      element.setProperty(PropertyTarget, "_top");
      break;
    case TargetNewWindow:
      element.setProperty(PropertyTarget, "_blank");
    }
    flags_.reset(BIT_TARGET_CHANGED);
  }

  WContainerWidget::updateDom(element, all);
}

void WAnchor::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_REF_CHANGED);
  flags_.reset(BIT_TARGET_CHANGED);

  WContainerWidget::propagateRenderOk(deep);
}

DomElementType WAnchor::domElementType() const
{
  return DomElement_A;
}

}
