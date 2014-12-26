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

WAnchor::LinkState::LinkState()
  : target(TargetSelf),
    clickJS(0)
{ }

WAnchor::LinkState::~LinkState()
{
  delete clickJS;
}

WAnchor::WAnchor(WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
{
  setInline(true);
}

WAnchor::WAnchor(const WLink& link, WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
{
  setInline(true);

  setLink(link);
}

#ifdef WT_TARGET_JAVA
WAnchor::WAnchor(const std::string& ref, WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
{
  setInline(true);

  linkState_.link = WLink(WLink::Url, ref);
}

WAnchor::WAnchor(WResource *resource, WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
{
  setInline(true);

  setResource(resource);
}
#endif // WT_TARGET_JAVA

WAnchor::WAnchor(const WLink& link, const WString& text,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
{
  setInline(true);

  setLink(link);

  text_ = new WText(text, this);
}

#ifdef WT_TARGET_JAVA
WAnchor::WAnchor(const std::string& ref, const WString& text,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
{ 
  setInline(true);

  setLink(WLink(WLink::Url, ref));

  text_ = new WText(text, this);
}

WAnchor::WAnchor(WResource *resource, const WString& text,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
{ 
  setInline(true);

  setResource(resource);

  text_ = new WText(text, this);
}
#endif // WT_TARGET_JAVA

WAnchor::WAnchor(const WLink& link, WImage *image, WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
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
    text_(0),
    image_(0)
{ 
  setInline(true);

  linkState_.link = WLink(WLink::Url, ref);

  image_ = image;

  if (image_)
    addWidget(image_);
}

WAnchor::WAnchor(WResource *resource, WImage *image,
		 WContainerWidget *parent)
  : WContainerWidget(parent),
    text_(0),
    image_(0)
{ 
  setInline(true);

  setResource(resource);

  image_ = image;
  if (image_)
    addWidget(image_);
}
#endif // WT_TARGET_JAVA

void WAnchor::setLink(const WLink& link)
{
  if (linkState_.link.type() != WLink::Resource && linkState_.link == link)
    return;

  linkState_.link = link;

  flags_.set(BIT_LINK_CHANGED);

  repaint();

  switch (linkState_.link.type()) {
  case WLink::Resource:
    linkState_.link.resource()->dataChanged().connect
      (this, &WAnchor::resourceChanged);
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

WResource *WAnchor::resource() const
{
  if (linkState_.link.type() == WLink::Resource)
    return linkState_.link.resource();
  else
    return 0;
}

void WAnchor::setTarget(AnchorTarget target)
{
  if (linkState_.target != target) {
    linkState_.target = target;
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
  repaint();
}

void WAnchor::enableAjax()
{
  if (linkState_.link.type() == WLink::InternalPath) {
    flags_.set(BIT_LINK_CHANGED);
    repaint();
  }

  WContainerWidget::enableAjax();
}

bool WAnchor::canReceiveFocus() const
{
  return true;
}

int WAnchor::tabIndex() const
{
  int result = WContainerWidget::tabIndex();

  if (result == std::numeric_limits<int>::min())
    return 0;
  else
    return result;
}

bool WAnchor::setFirstFocus()
{
  return false;
}

void WAnchor::updateDom(DomElement& element, bool all)
{
  bool needsUrlResolution = false;

  if (flags_.test(BIT_LINK_CHANGED) || all) {
    needsUrlResolution = renderHRef(this, linkState_, element);
    flags_.reset(BIT_LINK_CHANGED);
  }

  if (flags_.test(BIT_TARGET_CHANGED) || all) {
    renderHTarget(linkState_, element, all);
    flags_.reset(BIT_TARGET_CHANGED);
  }

  WContainerWidget::updateDom(element, all);

  if (needsUrlResolution)
    renderUrlResolution(this, element, all);
}

bool WAnchor::renderHRef(WInteractWidget *widget,
			 LinkState& linkState, DomElement& element)
{
  WApplication *app = WApplication::instance();

  if (linkState.link.isNull() || widget->isDisabled())
    element.removeAttribute("href");
  else {
    std::string url = linkState.link.resolveUrl(app);

    /*
     * From 但浩亮: setRefInternalPath() and setTarget(TargetNewWindow)
     * does not work without the check below:
     */
    if (linkState.target == TargetSelf) {
      linkState.clickJS
	= linkState.link.manageInternalPathChange(app, widget,
						  linkState.clickJS);
    } else {
      delete linkState.clickJS;
      linkState.clickJS = 0;
    }

    url = app->encodeUntrustedUrl(url);

    std::string href = url;
    element.setAttribute("href", href);
    return !app->environment().internalPathUsingFragments()
      && href.find("://") == std::string::npos && href[0] != '/';
  }

  return false;
}

void WAnchor::renderHTarget(LinkState& linkState, DomElement& element, bool all)
{
  switch (linkState.target) {
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
}

void WAnchor::renderUrlResolution(WWidget *widget, DomElement& element,
				  bool all)
{
  if (all)
    element.setProperty(PropertyClass,
			Utils::addWord(widget->styleClass().toUTF8(), "Wt-rr"));
  else
    element.callJavaScript("$('#" + widget->id() + "').addClass('Wt-rr');");
}

void WAnchor::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_LINK_CHANGED);
  flags_.reset(BIT_TARGET_CHANGED);

  WContainerWidget::propagateRenderOk(deep);
}

void WAnchor::propagateSetEnabled(bool enabled)
{
  WContainerWidget::propagateSetEnabled(enabled);

  resourceChanged();
}

DomElementType WAnchor::domElementType() const
{
  return DomElement_A;
}

}
