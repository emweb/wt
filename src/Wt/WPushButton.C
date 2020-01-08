/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WPopupMenu.h"
#include "Wt/WPushButton.h"
#include "Wt/WResource.h"
#include "Wt/WTheme.h"

#include "DomElement.h"

namespace Wt {

const char *WPushButton::CHECKED_SIGNAL = "M_checked";
const char *WPushButton::UNCHECKED_SIGNAL = "M_unchecked";

WPushButton::WPushButton()
{ 
  text_.format = TextFormat::Plain;
}

WPushButton::WPushButton(const WString& text)
{ 
  text_.format = TextFormat::Plain;
  text_.text = text;
}

WPushButton::WPushButton(const WString& text, TextFormat format)
{ 
  text_.format = TextFormat::Plain;
  text_.text = text;
  setTextFormat(format);
}

WPushButton::~WPushButton()
{
  if (popupMenu_)
    popupMenu_->setButton(nullptr);
}

bool WPushButton::setText(const WString& text)
{
  if (canOptimizeUpdates() && (text == text_.text))
    return true;

  bool ok = text_.setText(text);

  flags_.set(BIT_TEXT_CHANGED);
  repaint(RepaintFlag::SizeAffected);

  return ok;
}

void WPushButton::setDefault(bool enabled)
{
  flags_.set(BIT_DEFAULT, enabled);
}

bool WPushButton::isDefault() const
{
  return flags_.test(BIT_DEFAULT);
}

void WPushButton::setCheckable(bool checkable)
{
  flags_.set(BIT_IS_CHECKABLE, checkable);

  if (checkable) {
    clicked().connect("function(o,e) { $(o).toggleClass('active'); }");
    clicked().connect(this, &WPushButton::toggled);
  }
}

void WPushButton::toggled()
{
  // FIXME: later, make it a true EventSignal

  flags_.set(BIT_IS_CHECKED, !isChecked());

  if (isChecked())
    checked().emit();
  else
    unChecked().emit();
}

EventSignal<>& WPushButton::checked()
{
  return *voidEventSignal(CHECKED_SIGNAL, true);
}

EventSignal<>& WPushButton::unChecked()
{
  return *voidEventSignal(UNCHECKED_SIGNAL, true);
}

bool WPushButton::isCheckable() const
{
  return flags_.test(BIT_IS_CHECKABLE);
}

void WPushButton::setChecked(bool checked)
{
  if (isCheckable()) {
    flags_.set(BIT_IS_CHECKED, checked);

    flags_.set(BIT_CHECKED_CHANGED, true);
    repaint();
  }
}

void WPushButton::setChecked()
{
  setChecked(true);
}

void WPushButton::setUnChecked()
{
  setChecked(false);
}

bool WPushButton::isChecked() const
{
  return flags_.test(BIT_IS_CHECKED);
}

bool WPushButton::setTextFormat(TextFormat textFormat)
{
  return text_.setFormat(textFormat);
}

bool WPushButton::setFirstFocus()
{
  return false;
}

void WPushButton::setIcon(const WLink& link)
{
  if (canOptimizeUpdates() && (link == icon_))
    return;

  icon_ = link;
  flags_.set(BIT_ICON_CHANGED);

  repaint(RepaintFlag::SizeAffected);
}

void WPushButton::setLink(const WLink& link)
{
  if (link == linkState_.link)
    return;

  linkState_.link = link;
  flags_.set(BIT_LINK_CHANGED);

  if (linkState_.link.type() == LinkType::Resource)
    linkState_.link.resource()->dataChanged()
      .connect(this, &WPushButton::resourceChanged);

  repaint();
}

void WPushButton::resourceChanged()
{
  flags_.set(BIT_LINK_CHANGED);
  repaint();
}

void WPushButton::setMenu(std::unique_ptr<WPopupMenu> popupMenu)
{
  popupMenu_ = std::move(popupMenu);

  if (popupMenu_)
    popupMenu_->setButton(this);
}

void WPushButton::doRedirect()
{
  WApplication *app = WApplication::instance();

  if (!app->environment().ajax()) {
    if (linkState_.link.type() == LinkType::InternalPath)
      app->setInternalPath(linkState_.link.internalPath().toUTF8(), true);
    else
      app->redirect(linkState_.link.url());
  }
}

DomElementType WPushButton::domElementType() const
{
  if (!linkState_.link.isNull()) {
    WApplication *app = WApplication::instance();
    if (app->theme()->canStyleAnchorAsButton())
      return DomElementType::A;
  }

  return DomElementType::BUTTON;
}

void WPushButton::updateDom(DomElement& element, bool all)
{
  if (all && element.type() == DomElementType::BUTTON)
    element.setAttribute("type", "button");

  bool updateInnerHtml = !icon_.isNull() && flags_.test(BIT_TEXT_CHANGED);

  if (updateInnerHtml || flags_.test(BIT_ICON_CHANGED)
      || (all && !icon_.isNull())) {
    DomElement *image = DomElement::createNew(DomElementType::IMG);
    image->setProperty(Property::Src, 
		       icon_.resolveUrl(WApplication::instance()));
    image->setId("im" + formName());
    element.insertChildAt(image, 0);
    flags_.set(BIT_ICON_RENDERED);
    flags_.reset(BIT_ICON_CHANGED);
  }

  if (flags_.test(BIT_TEXT_CHANGED) || all) {
    element.setProperty(Wt::Property::InnerHTML, text_.formattedText());

    flags_.reset(BIT_TEXT_CHANGED);
  }

  // bool needsUrlResolution = false;

  if (flags_.test(BIT_LINK_CHANGED) || all) {
    if (element.type() == DomElementType::A) {
      /* needsUrlResolution = */ WAnchor::renderHRef(this, linkState_, element);
      WAnchor::renderHTarget(linkState_, element, all);
    } else
      renderHRef(element);

    flags_.reset(BIT_LINK_CHANGED);
  }

  if (isCheckable()) {
    if (flags_.test(BIT_CHECKED_CHANGED) || all) {
      if (!all || flags_.test(BIT_IS_CHECKED)) {
	toggleStyleClass("active", flags_.test(BIT_IS_CHECKED), true);
      }

      flags_.reset(BIT_CHECKED_CHANGED);
    }
  }

  if (!all)
    WApplication::instance()->theme()->apply(this, element, MainElement);

  WFormWidget::updateDom(element, all);
}

void WPushButton::renderHRef(DomElement& element)
{
  if (!linkState_.link.isNull() && !isDisabled()) {
    WApplication *app = WApplication::instance();

    if (!linkState_.clickJS) {
      linkState_.clickJS = new JSlot();
      clicked().connect(*linkState_.clickJS);

      if (!app->environment().ajax())
	clicked().connect(this, &WPushButton::doRedirect);
    }

    if (linkState_.link.type() == LinkType::InternalPath)
      linkState_.clickJS->setJavaScript
	("function(){" +
	 app->javaScriptClass() + "._p_.setHash("
	 + jsStringLiteral(linkState_.link.internalPath()) + ",true);"
	 "}");
    else {
      std::string url = linkState_.link.resolveUrl(app);

      if (linkState_.link.target() == LinkTarget::NewWindow)
	linkState_.clickJS->setJavaScript
	  ("function(){"
	   "window.open(" + jsStringLiteral(url) + ");"
	   "}");
      else if (linkState_.link.target() == LinkTarget::Download)
	linkState_.clickJS->setJavaScript
	  ("function(){"
	   "var ifr = document.getElementById('wt_iframe_dl_id');"
	   "ifr.src = "  + jsStringLiteral(url) + ";"
	   "}");
      else
	linkState_.clickJS->setJavaScript
	  ("function(){"
	   "window.location=" + jsStringLiteral(url) + ";"
	   "}");
    }

    clicked().ownerRepaint(); // XXX only for Java port necessary
  } else {
    delete linkState_.clickJS;
    linkState_.clickJS = nullptr;
  }
}

void WPushButton::getDomChanges(std::vector<DomElement *>& result,
				WApplication *app)
{
  if (flags_.test(BIT_ICON_CHANGED) && flags_.test(BIT_ICON_RENDERED)) {
    DomElement *image
      = DomElement::getForUpdate("im" + formName(), DomElementType::IMG);
    if (icon_.isNull()) {
      image->removeFromParent();
      flags_.reset(BIT_ICON_RENDERED);
    } else
      image->setProperty(Property::Src, icon_.resolveUrl(app));

    result.push_back(image);

    flags_.reset(BIT_ICON_CHANGED);
  }

  WFormWidget::getDomChanges(result, app);
}

void WPushButton::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_TEXT_CHANGED);
  flags_.reset(BIT_ICON_CHANGED);
  flags_.reset(BIT_LINK_CHANGED);
  flags_.reset(BIT_CHECKED_CHANGED);

  WFormWidget::propagateRenderOk(deep);
}

void WPushButton::propagateSetEnabled(bool enabled)
{
  WFormWidget::propagateSetEnabled(enabled);
  flags_.set(BIT_LINK_CHANGED);
  repaint();
}

WT_USTRING WPushButton::valueText() const
{
  return WT_USTRING();
}

void WPushButton::setValueText(const WT_USTRING& value)
{ }

void WPushButton::refresh()
{
  if (text_.text.refresh()) {
    flags_.set(BIT_TEXT_CHANGED);
    repaint(RepaintFlag::SizeAffected);
  }

  WFormWidget::refresh();
}

void WPushButton::enableAjax()
{
  if (!linkState_.link.isNull()) {
    WApplication *app = WApplication::instance();
    if (app->theme()->canStyleAnchorAsButton()) {
      flags_.set(BIT_LINK_CHANGED);
      repaint();
    }
  }

  WFormWidget::enableAjax();
}

}
