/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WPopupMenu"
#include "Wt/WPushButton"
#include "Wt/WResource"
#include "Wt/WTheme"

#include "DomElement.h"

namespace Wt {

const char *WPushButton::CHECKED_SIGNAL = "M_checked";
const char *WPushButton::UNCHECKED_SIGNAL = "M_unchecked";

WPushButton::WPushButton(WContainerWidget *parent)
  : WFormWidget(parent),
    popupMenu_(0)
{ 
  text_.format = PlainText;
}

WPushButton::WPushButton(const WString& text, WContainerWidget *parent)
  : WFormWidget(parent),
    popupMenu_(0)
{ 
  text_.format = PlainText;
  text_.text = text;
}

WPushButton::~WPushButton()
{
  if (popupMenu_)
    popupMenu_->setButton(0);
}

bool WPushButton::setText(const WString& text)
{
  if (canOptimizeUpdates() && (text == text_.text))
    return true;

  bool ok = text_.setText(text);

  flags_.set(BIT_TEXT_CHANGED);
  repaint(RepaintSizeAffected);

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

  repaint(RepaintSizeAffected);
}

void WPushButton::setLink(const WLink& link)
{
  if (link == linkState_.link)
    return;

  linkState_.link = link;
  flags_.set(BIT_LINK_CHANGED);

  if (linkState_.link.type() == WLink::Resource)
    linkState_.link.resource()->dataChanged()
      .connect(this, &WPushButton::resourceChanged);

  repaint();
}

void WPushButton::setLinkTarget(AnchorTarget target)
{
  linkState_.target = target;
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
  repaint();
}

void WPushButton::setMenu(WPopupMenu *popupMenu)
{
  popupMenu_ = popupMenu;

  if (popupMenu_)
    popupMenu_->setButton(this);
}

void WPushButton::doRedirect()
{
  WApplication *app = WApplication::instance();

  if (!app->environment().ajax()) {
    if (linkState_.link.type() == WLink::InternalPath)
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
      return DomElement_A;
  }

  return DomElement_BUTTON;
}

void WPushButton::updateDom(DomElement& element, bool all)
{
  if (all && element.type() == DomElement_BUTTON)
    element.setAttribute("type", "button");

  bool updateInnerHtml = !icon_.isNull() && flags_.test(BIT_TEXT_CHANGED);

  if (updateInnerHtml || flags_.test(BIT_ICON_CHANGED)
      || (all && !icon_.isNull())) {
    DomElement *image = DomElement::createNew(DomElement_IMG);
    image->setProperty(PropertySrc, icon_.resolveUrl(WApplication::instance()));
    image->setId("im" + formName());
    element.insertChildAt(image, 0);
    flags_.set(BIT_ICON_RENDERED);
    flags_.reset(BIT_ICON_CHANGED);
  }

  if (flags_.test(BIT_TEXT_CHANGED) || all) {
    element.setProperty(Wt::PropertyInnerHTML, text_.formattedText());

    flags_.reset(BIT_TEXT_CHANGED);
  }

  // bool needsUrlResolution = false;

  if (flags_.test(BIT_LINK_CHANGED) || all) {
    if (element.type() == DomElement_A) {
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
    WApplication::instance()->theme()->apply(this, element,
					     MainElementThemeRole);

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

    if (linkState_.link.type() == WLink::InternalPath)
      linkState_.clickJS->setJavaScript
	("function(){" +
	 app->javaScriptClass() + "._p_.setHash("
	 + jsStringLiteral(linkState_.link.internalPath()) + ",true);"
	 "}");
    else {
      std::string url = linkState_.link.resolveUrl(app);

      if (linkState_.target == TargetNewWindow)
	linkState_.clickJS->setJavaScript
	  ("function(){"
	   "window.open(" + jsStringLiteral(url) + ");"
	   "}");
      else
	linkState_.clickJS->setJavaScript
	  ("function(){"
	   "window.location=" + jsStringLiteral(url) + ";"
	   "}");
    }

    clicked().senderRepaint(); // XXX only for Java port necessary
  } else {
    delete linkState_.clickJS;
    linkState_.clickJS = 0;
  }
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
      image->setProperty(PropertySrc, icon_.resolveUrl(app));

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
    repaint(RepaintSizeAffected);
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
