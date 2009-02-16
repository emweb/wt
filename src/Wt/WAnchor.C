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
#include "Wt/WText"
#include "DomElement.h"

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

void WAnchor::setRef(const std::string& ref)
{
  if (ref_ != ref) {
    ref_ = ref;
    flags_.set(BIT_REF_CHANGED);

    repaint(RepaintPropertyIEMobile);
  }
}

void WAnchor::setRefInternalPath(const std::string& path)
{
  WApplication *app = WApplication::instance();
  ref_ = app->bookmarkUrl(path);

  if (app->environment().ajax()) {
    if (!changeInternalPathJS_) {
      changeInternalPathJS_ = new JSlot();
      clicked.connect(*changeInternalPathJS_);
      clicked.setPreventDefault(true);
    }
    changeInternalPathJS_->setJavaScript
      ("function(obj, event){"
       "window.location.hash='#" + DomElement::urlEncode(path) + "';"
       "}");
  } else

  flags_.set(BIT_REF_CHANGED);

  repaint(RepaintPropertyIEMobile);
}

void WAnchor::setResource(WResource *resource)
{
  resource_ = resource;

  if (resource_) {
    resource_->dataChanged.connect(SLOT(this, WAnchor::resourceChanged));
    setRef(resource_->generateUrl());
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

void WAnchor::setWordWrap(bool on)
{
  if (!text_)
    text_ = new WText(this);

  text_->setWordWrap(on);
}

bool WAnchor::wordWrap() const
{
  return text_ ? text_->wordWrap() : true;
}

void WAnchor::setImage(WImage *image)
{
  if (image_)
    delete image_;
  image_ = image;

  if (image_)
    addWidget(image_);
}

void WAnchor::resourceChanged()
{
  setRef(resource_->generateUrl());
}

void WAnchor::updateDom(DomElement& element, bool all)
{
  if (flags_.test(BIT_REF_CHANGED) || all) {
    std::string uri = ref_;
    element.setAttribute("href", fixRelativeUrl(uri));

    flags_.reset(BIT_REF_CHANGED);
  }

  if (flags_.test(BIT_TARGET_CHANGED) || all) {
    switch (target_) {
    case TargetSelf:
      if (!all)
	element.setAttribute("target", "_self");
      break;
    case TargetThisWindow:
      element.setAttribute("target", "_top");
      break;
    case TargetNewWindow:
      element.setAttribute("target", "_blank");
    }
    flags_.reset(BIT_TARGET_CHANGED);
  }

  WContainerWidget::updateDom(element, all);
}

DomElementType WAnchor::domElementType() const
{
  return DomElement_A;
}

}
