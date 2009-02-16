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

#include "WtException.h"

namespace Wt {

WCompositeWidget::WCompositeWidget(WContainerWidget *parent)
  : WWidget(parent),
    impl_(0)
{
  if (parent)
    parent->addWidget(this);
}

WCompositeWidget::~WCompositeWidget()
{
  setParent(0);

  delete impl_;
}

const std::string WCompositeWidget::formName() const
{
  return impl_->formName();
}

void WCompositeWidget::setId(const std::string& id)
{
  impl_->setId(id);
}

void WCompositeWidget::setPositionScheme(PositionScheme scheme)
{
  impl_->setPositionScheme(scheme);
}

WWidget::PositionScheme WCompositeWidget::positionScheme() const
{
  return impl_->positionScheme();
}

void WCompositeWidget::setOffsets(const WLength& offset, int sides)
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

WWidget::Side WCompositeWidget::floatSide() const
{
  return impl_->floatSide();
}

void WCompositeWidget::setClearSides(int sides)
{
  impl_->setClearSides(sides);
}

int WCompositeWidget::clearSides() const
{
  return impl_->clearSides();
}

void WCompositeWidget::setMargin(const WLength& margin, int sides)
{
  impl_->setMargin(margin, sides);
}

WLength WCompositeWidget::margin(Side side) const
{
  return impl_->margin(side);
}

void WCompositeWidget::setHidden(bool how)
{
  impl_->setHidden(how);
}

bool WCompositeWidget::isHidden() const
{
  return impl_->isHidden();
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

  return impl_->setInline(how);
}

bool WCompositeWidget::isInline() const
{
  return impl_->isInline();
}

WCssDecorationStyle& WCompositeWidget::decorationStyle()
{
  return impl_->decorationStyle();
}

void WCompositeWidget::setStyleClass(const WString& styleClass)
{
  impl_->setStyleClass(styleClass);
}

void WCompositeWidget::setStyleClass(const char *value)
{
  impl_->setStyleClass(WString(value, UTF8));
}

WString WCompositeWidget::styleClass() const
{
  return impl_->styleClass();
}

void WCompositeWidget::setVerticalAlignment(VerticalAlignment alignment,
					    const WLength& length)
{
  impl_->setVerticalAlignment(alignment, length);
}

WWidget::VerticalAlignment WCompositeWidget::verticalAlignment() const
{
  return impl_->verticalAlignment();
}

WLength WCompositeWidget::verticalAlignmentLength() const
{
  return impl_->verticalAlignmentLength();
}

WWebWidget *WCompositeWidget::webWidget()
{
  return impl_->webWidget();
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
}

void WCompositeWidget::addChild(WWidget *child)
{
  if (child != impl_)
    impl_->addChild(child);
  else
    impl_->WObject::setParent(this);
}

void WCompositeWidget::removeChild(WWidget *child)
{
  if (child != impl_)
    impl_->removeChild(child);
  else
    impl_->WObject::setParent(0);
}

void WCompositeWidget::setHideWithOffsets(bool how)
{
  impl_->setHideWithOffsets(how);
}

bool WCompositeWidget::isVisible() const
{
  if (parent())
    return parent()->isVisible();
  else
    return true;
}

bool WCompositeWidget::isStubbed() const
{
  if (parent())
    return parent()->isStubbed();
  else
    return false;
}

void WCompositeWidget::setAttributeValue(const std::string& attribute,
					 const WString& value)
{
  impl_->setAttributeValue(attribute, value);
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

void WCompositeWidget::setImplementation(WWidget *widget)
{
  if (widget->parent())
    throw WtException("WCompositeWidget implemnation widget "
		      "cannot have a parent");

  if (impl_)
    delete impl_;

  impl_ = widget;
  if (parent() && parent()->loaded())
    impl_->load();

  widget->setParent(this);
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

}
