/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WResource"
#include "Wt/WText"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

WAnchor::WAnchor(WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{
  setInline(true);
}

WAnchor::WAnchor(const WLink& link, WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{
  setInline(true);

  setLink(link);
}

#ifdef WT_TARGET_JAVA
WAnchor::WAnchor(const std::string& ref, WContainerWidget *parent)
  : WContainerWidget(parent),
    link_(WLink::Url, ref),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{
  setInline(true);
}

WAnchor::WAnchor(WResource *resource, WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{
  setInline(true);

  setResource(resource);
}
#endif // WT_TARGET_JAVA

WAnchor::WAnchor(const WLink& link, const WString& text,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{
  setInline(true);

  setLink(link);

  text_ = new WText(text, this);
}

#ifdef WT_TARGET_JAVA
WAnchor::WAnchor(const std::string& ref, const WString& text,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    link_(WLink::Url, ref),
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
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{ 
  setInline(true);

  setResource(resource);

  text_ = new WText(text, this);
}
#endif // WT_TARGET_JAVA

WAnchor::WAnchor(const WLink& link, WImage *image, WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{ 
  setInline(true);

  setLink(link);

  image_ = image;
  if (image_)
    addWidget(image_);
}

#ifdef WT_TARGET_JAVA
WAnchor::WAnchor(const std::string& ref, WImage *image,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    link_(WLink::Url, ref),
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
    text_(0),
    image_(0),
    target_(TargetSelf),
    changeInternalPathJS_(0)
{ 
  setInline(true);

  setResource(resource);

  image_ = image;
  if (image_)
    addWidget(image_);
}
#endif // WT_TARGET_JAVA

WAnchor::~WAnchor()
{
  delete changeInternalPathJS_;
}

void WAnchor::setLink(const WLink& link)
{
  if (link_.type() != WLink::Resource && link_ == link)
    return;

  link_ = link;

  flags_.set(BIT_LINK_CHANGED);

  repaint(RepaintPropertyIEMobile);

  switch (link_.type()) {
  case WLink::Resource:
    link_.resource()->dataChanged().connect(this, &WAnchor::resourceChanged);
    break;
  case WLink::InternalPath:
    WApplication::instance()->enableInternalPaths();
    break;
  default:
    break;
  }
}

void WAnchor::setRef(const std::string& url)
{
  setLink(WLink(WLink::Url, url));
}

void WAnchor::setRefInternalPath(const std::string& path)
{
  setLink(WLink(WLink::InternalPath, path));
}

void WAnchor::setResource(WResource *resource)
{
  setLink(WLink(resource));
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

void WAnchor::setTextFormat(TextFormat textFormat)
{
  if (!text_)
    text_ = new WText(this);

  text_->setTextFormat(textFormat);
}

TextFormat WAnchor::textFormat() const
{
  return text_ ? text_->textFormat() : XHTMLText;
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
  flags_.set(BIT_LINK_CHANGED);
  repaint(RepaintPropertyIEMobile);
}

void WAnchor::enableAjax()
{
  if (link_.type() == WLink::InternalPath) {
    flags_.set(BIT_LINK_CHANGED);
    repaint(RepaintPropertyIEMobile);
  }

  WContainerWidget::enableAjax();
}

void WAnchor::updateDom(DomElement& element, bool all)
{
  bool needsUrlResolution = false;

  if (flags_.test(BIT_LINK_CHANGED) || all) {
    WApplication *app = WApplication::instance();

    if (link_.isNull())
      element.removeAttribute("href");
    else {
      std::string url = link_.resolveUrl(app);

      /*
       * From 但浩亮: setRefInternalPath() and setTarget(TargetNewWindow)
       * does not work without the check below:
       */
      if (target_ == TargetSelf)
	changeInternalPathJS_
	  = link_.manageInternalPathChange(app, this, changeInternalPathJS_);
      else {
	delete changeInternalPathJS_;
	changeInternalPathJS_ = 0;
      }

      url = app->encodeUntrustedUrl(url);

      std::string href = resolveRelativeUrl(url);
      element.setAttribute("href", href);
      needsUrlResolution = !app->environment().hashInternalPaths()
	&& href.find("://") == std::string::npos && href[0] != '/';
    }

    flags_.reset(BIT_LINK_CHANGED);
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

  if (needsUrlResolution) {
    if (all)
      element.setProperty(PropertyClass,
			  Utils::addWord(styleClass().toUTF8(), "Wt-rr"));
    else
      element.callJavaScript("$('#" + id() + "').addClass('Wt-rr');");
  }
}

void WAnchor::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_LINK_CHANGED);
  flags_.reset(BIT_TARGET_CHANGED);

  WContainerWidget::propagateRenderOk(deep);
}

DomElementType WAnchor::domElementType() const
{
  return DomElement_A;
}

}
