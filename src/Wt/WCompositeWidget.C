/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Integrated memory management to XLObject
 * abj <xynopsis@yahoo.com> 2006.4.20
 *
 */

#include "Wt/WCompositeWidget"
#include "Wt/WContainerWidget"
#include "Wt/WApplication"
#include "Wt/WLogger"

#include "WtException.h"

namespace Wt {

WCompositeWidget::WCompositeWidget(WContainerWidget *parent)
  : WWidget(parent),
    impl_(0)
{
  if (parent)
    parent->addWidget(this);
}

WCompositeWidget::WCompositeWidget(WWidget *implementation,
				   WContainerWidget *parent)
  : WWidget(parent),
    impl_(0)
{
  if (parent)
    parent->addWidget(this);

  setImplementation(implementation);
}

WCompositeWidget::~WCompositeWidget()
{
  setParentWidget(0);

  delete impl_;
}

const std::string WCompositeWidget::id() const
{
  return impl_->id();
}

void WCompositeWidget::setId(const std::string& id)
{
  impl_->setId(id);
}

WWidget *WCompositeWidget::find(const std::string& name)
{
  return impl_->find(name);
}

void WCompositeWidget::setSelectable(bool selectable)
{
  impl_->setSelectable(selectable);
}

void WCompositeWidget::setPositionScheme(PositionScheme scheme)
{
  impl_->setPositionScheme(scheme);
}

PositionScheme WCompositeWidget::positionScheme() const
{
  return impl_->positionScheme();
}

void WCompositeWidget::setOffsets(const WLength& offset, WFlags<Side> sides)
{
  impl_->setOffsets(offset, sides);
}

WLength WCompositeWidget::offset(Side s) const
{
  return impl_->offset(s);
}

void WCompositeWidget::resize(const WLength& width, const WLength& height)
{
  impl_->resize(width, height);

  WWidget::resize(width, height);
}

WLength WCompositeWidget::width() const
{
  return impl_->width();
}

WLength WCompositeWidget::height() const
{
  return impl_->height();
}

void WCompositeWidget::setMinimumSize(const WLength& width, const WLength& height)
{
  impl_->setMinimumSize(width, height);
}

WLength WCompositeWidget::minimumWidth() const
{
  return impl_->minimumWidth();
}

WLength WCompositeWidget::minimumHeight() const
{
  return impl_->minimumHeight();
}

void WCompositeWidget::setMaximumSize(const WLength& width, const WLength& height)
{
  impl_->setMaximumSize(width, height);
}

WLength WCompositeWidget::maximumWidth() const
{
  return impl_->maximumWidth();
}

WLength WCompositeWidget::maximumHeight() const
{
  return impl_->maximumHeight();
}

void WCompositeWidget::setLineHeight(const WLength& height)
{
  impl_->setLineHeight(height);
}

WLength WCompositeWidget::lineHeight() const
{
  return impl_->lineHeight();
}

void WCompositeWidget::setFloatSide(Side s)
{
  impl_->setFloatSide(s);
}

Side WCompositeWidget::floatSide() const
{
  return impl_->floatSide();
}

void WCompositeWidget::setClearSides(WFlags<Side> sides)
{
  impl_->setClearSides(sides);
}

WFlags<Side> WCompositeWidget::clearSides() const
{
  return impl_->clearSides();
}

void WCompositeWidget::setMargin(const WLength& margin, WFlags<Side> sides)
{
  impl_->setMargin(margin, sides);
}

WLength WCompositeWidget::margin(Side side) const
{
  return impl_->margin(side);
}

void WCompositeWidget::setHiddenKeepsGeometry(bool enabled)
{
  impl_->setHiddenKeepsGeometry(enabled);
}

bool WCompositeWidget::hiddenKeepsGeometry() const
{
  return impl_->hiddenKeepsGeometry();
}

void WCompositeWidget::setHidden(bool how)
{
  impl_->setHidden(how);
}

bool WCompositeWidget::isHidden() const
{
  return impl_->isHidden();
}

bool WCompositeWidget::isVisible() const
{
  if (isHidden())
    return false;
  else if (parent())
    return parent()->isVisible();
  else
    return true;
}

void WCompositeWidget::setDisabled(bool disabled)
{
  impl_->setDisabled(disabled);
}

bool WCompositeWidget::isDisabled() const
{
  return impl_->isDisabled();
}

bool WCompositeWidget::isEnabled() const
{
  if (isDisabled())
    return false;
  else if (parent())
    return parent()->isEnabled();
  else
    return true;
}

void WCompositeWidget::setPopup(bool how)
{
  impl_->setPopup(how);
}

bool WCompositeWidget::isPopup() const
{
  return impl_->isPopup();
}

void WCompositeWidget::setInline(bool how)
{
  resetLearnedSlot(&WWidget::show);

  impl_->setInline(how);
}

bool WCompositeWidget::isInline() const
{
  return impl_->isInline();
}

void WCompositeWidget::setDecorationStyle(const WCssDecorationStyle& style)
{
  impl_->setDecorationStyle(style);
}

WCssDecorationStyle& WCompositeWidget::decorationStyle()
{
  return impl_->decorationStyle();
}

void WCompositeWidget::setStyleClass(const WT_USTRING& styleClass)
{
  impl_->setStyleClass(styleClass);
}

void WCompositeWidget::setStyleClass(const char *styleClass)
{
  impl_->setStyleClass(WT_USTRING::fromUTF8(styleClass));
}

WT_USTRING WCompositeWidget::styleClass() const
{
  return impl_->styleClass();
}

void WCompositeWidget::addStyleClass(const WT_USTRING& styleClass,
				     bool force)
{
  impl_->addStyleClass(styleClass, force);
}

void WCompositeWidget::addStyleClass(const char *styleClass, bool force)
{
  impl_->addStyleClass(WT_USTRING::fromUTF8(styleClass), force);
}

void WCompositeWidget::removeStyleClass(const WT_USTRING& styleClass,
					bool force)
{
  impl_->removeStyleClass(styleClass, force);
}

void WCompositeWidget::removeStyleClass(const char *styleClass, bool force)
{
  impl_->removeStyleClass(WT_USTRING::fromUTF8(styleClass), force);
}

void WCompositeWidget::setVerticalAlignment(AlignmentFlag alignment,
					    const WLength& length)
{
  if (AlignHorizontalMask & alignment) {
    wApp->log("warning") << "WCompositeWidget::setVerticalAlignment: "
      "alignment " << alignment << "is horizontal, expected vertical";
  }
  impl_->setVerticalAlignment(alignment, length);
}

AlignmentFlag WCompositeWidget::verticalAlignment() const
{
  return impl_->verticalAlignment();
}

WLength WCompositeWidget::verticalAlignmentLength() const
{
  return impl_->verticalAlignmentLength();
}

WWebWidget *WCompositeWidget::webWidget()
{
  return impl_ ? impl_->webWidget() : 0;
}

void WCompositeWidget::setToolTip(const WString& text)
{
  impl_->setToolTip(text);
}

WString WCompositeWidget::toolTip() const
{
  return impl_->toolTip();
}

void WCompositeWidget::refresh()
{
  impl_->refresh();

  WWidget::refresh();
}

void WCompositeWidget::enableAjax()
{
  impl_->enableAjax();
}

void WCompositeWidget::addChild(WWidget *child)
{
  if (child != impl_)
    impl_->addChild(child);
  else
    impl_->setParent(this);
}

void WCompositeWidget::removeChild(WWidget *child)
{
  if (child != impl_)
    impl_->removeChild(child);
  else
    impl_->setParent(0);
}

void WCompositeWidget::setHideWithOffsets(bool how)
{
  impl_->setHideWithOffsets(how);
}

bool WCompositeWidget::isStubbed() const
{
  if (parent())
    return parent()->isStubbed();
  else
    return false;
}

bool WCompositeWidget::needsToBeRendered() const
{
  return impl_->needsToBeRendered();
}

void WCompositeWidget::setAttributeValue(const std::string& name,
					 const WT_USTRING& value)
{
  impl_->setAttributeValue(name, value);
}

WT_USTRING WCompositeWidget::attributeValue(const std::string& name) const
{
  return impl_->attributeValue(name);
}

void WCompositeWidget::setJavaScriptMember(const std::string& name,
					   const std::string& value)
{
  impl_->setJavaScriptMember(name, value);
}

std::string WCompositeWidget::javaScriptMember(const std::string& name) const
{
  return impl_->javaScriptMember(name);
}

void WCompositeWidget::callJavaScriptMember(const std::string& name,
					    const std::string& args)
{
  impl_->callJavaScriptMember(name, args);
}

void WCompositeWidget::load()
{
  if (impl_)
    impl_->load();
}

bool WCompositeWidget::loaded() const
{
  return impl_ ? impl_->loaded() : true;
}

void WCompositeWidget::setTabIndex(int index)
{
  impl_->setTabIndex(index);
}

int WCompositeWidget::tabIndex() const
{
  return impl_->tabIndex();
}

void WCompositeWidget::setImplementation(WWidget *widget)
{
  if (widget->parent())
    throw WtException("WCompositeWidget implemnation widget "
		      "cannot have a parent");

  delete impl_;

  impl_ = widget;
  if (parent()) {
    WWebWidget *ww = impl_->webWidget();
    if (ww)
      ww->gotParent();

    if (parent()->loaded())
      impl_->load();
  }

  widget->setParentWidget(this);
}

void WCompositeWidget::setLayout(WLayout *layout)
{
  impl_->setLayout(layout);
}

WLayout *WCompositeWidget::layout()
{
  return impl_->layout();
}

WLayoutItemImpl *WCompositeWidget::createLayoutItemImpl(WLayoutItem *item)
{
  return impl_->createLayoutItemImpl(item);
}

void WCompositeWidget::getSDomChanges(std::vector<DomElement *>& result,
				      WApplication *app)
{
  if (needsToBeRendered())
    render(impl_->isRendered() ? RenderUpdate : RenderFull);

  impl_->getSDomChanges(result, app);
}

void WCompositeWidget::render(WFlags<RenderFlag> flags)
{
  impl_->render(flags);

  renderOk();
}

int WCompositeWidget::boxPadding(Orientation orientation) const
{
  return impl_->boxPadding(orientation);
}

int WCompositeWidget::boxBorder(Orientation orientation) const
{
  return impl_->boxBorder(orientation);
}

void WCompositeWidget::propagateSetEnabled(bool enabled)
{
  impl_->webWidget()->propagateSetEnabled(enabled);
}

}
