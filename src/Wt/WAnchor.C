/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAnchor.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WImage.h"
#include "Wt/WResource.h"
#include "Wt/WText.h"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

WAnchor::LinkState::LinkState()
  : clickJS(nullptr)
{ }

WAnchor::LinkState::~LinkState()
{
  delete clickJS;
}

WAnchor::WAnchor()
{
  setInline(true);
}

WAnchor::WAnchor(const WLink& link)
{
  setInline(true);
  setLink(link);
}

WAnchor::WAnchor(const WLink& link, const WString& text)
{
  setInline(true);
  setLink(link);
  text_ = new WText(text);
  addWidget(std::unique_ptr<WWidget>(text_.get()));
}

WAnchor::WAnchor(const WLink& link, std::unique_ptr<WImage> image)
{ 
  setInline(true);
  setLink(link);

  if (image) {
    image_ = image.get();
    addWidget(std::move(image));
  }
}

void WAnchor::setLink(const WLink& link)
{
  if (linkState_.link.type() != LinkType::Resource && 
      linkState_.link == link)
    return;

  linkState_.link = link;

  flags_.set(BIT_LINK_CHANGED);

  repaint();

  switch (linkState_.link.type()) {
  case LinkType::Resource:
    linkState_.link.resource()->dataChanged().connect
      (this, &WAnchor::resourceChanged);
    break;
  case LinkType::InternalPath:
    WApplication::instance()->enableInternalPaths();
    break;
  default:
    break;
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
    std::unique_ptr<WText> t(new WText(text));
    text_ = t.get();
    addWidget(std::move(t));
  } else
    if (!text.empty())
      text_->setText(text);
    else
      text_->parent()->removeWidget(text_.get());
}

void WAnchor::setWordWrap(bool wordWrap)
{
  if (!text_)
    setText(WString());

  text_->setWordWrap(wordWrap);
}

bool WAnchor::wordWrap() const
{
  return text_ ? text_->wordWrap() : true;
}

void WAnchor::setTextFormat(TextFormat textFormat)
{
  if (!text_)
    setText(WString());

  text_->setTextFormat(textFormat);
}

TextFormat WAnchor::textFormat() const
{
  return text_ ? text_->textFormat() : TextFormat::XHTML;
}

void WAnchor::setImage(std::unique_ptr<WImage> image)
{
  image_ = image.get();

  if (image)
    addWidget(std::move(image));
}

void WAnchor::resourceChanged()
{
  flags_.set(BIT_LINK_CHANGED);
  repaint();
}

void WAnchor::enableAjax()
{
  if (linkState_.link.type() == LinkType::InternalPath) {
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

	/*
	 * TODO(Benoit)
	 * We do that here because of the static method, 
	 * We should maybe move the code to renderHTarget() 
	 * and make it non static ?
	 */
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
     * From 但浩亮: setRefInternalPath() and setTarget(LinkTarget::NewWindow)
     * does not work without the check below:
     */
    if (linkState.link.target() == LinkTarget::Self) {
      linkState.clickJS
	= linkState.link.manageInternalPathChange(app, widget,
						  linkState.clickJS);
    } else {
      delete linkState.clickJS;
      linkState.clickJS = nullptr;
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
  switch (linkState.link.target()) {
  case LinkTarget::Self:
    if (!all)
      element.setProperty(Property::Target, "_self");
    break;
  case LinkTarget::ThisWindow:
    element.setProperty(Property::Target, "_top");
    break;
  case LinkTarget::NewWindow:
    element.setProperty(Property::Target, "_blank");
    break;
  case LinkTarget::Download:
    element.setProperty(Property::Target, "wt_iframe_dl");
    element.setProperty(Property::Download, ""); // Only works on some browsers (FF, Chrome)
    break;
  }
}

void WAnchor::renderUrlResolution(WWidget *widget, DomElement& element,
				  bool all)
{
  if (all)
    element.setProperty(Property::Class,
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
  return DomElementType::A;
}

}
