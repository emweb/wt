/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <cassert>

#include "DomElement.h"
#include "FlexItemImpl.h"
#include "ResizeSensor.h"

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WWidgetItem.h"

namespace Wt {

WT_MAYBE_UNUSED LOGGER("WWidgetItem");

FlexItemImpl::FlexItemImpl(WWidgetItem *item)
  : item_(item)
{ }

FlexItemImpl::~FlexItemImpl()
{ }

int FlexItemImpl::minimumWidth() const
{
  if (item_->widget()->isHidden())
    return 0;
  else
    return static_cast<int>(item_->widget()->minimumWidth().toPixels());
}

int FlexItemImpl::minimumHeight() const
{
  if (item_->widget()->isHidden())
    return 0;
  else
    return static_cast<int>(item_->widget()->minimumHeight().toPixels());
}

int FlexItemImpl::maximumWidth() const
{
  if (item_->widget()->isHidden())
    return 0;
  else
    return static_cast<int>(item_->widget()->maximumWidth().toPixels());
}

int FlexItemImpl::maximumHeight() const
{
  if (item_->widget()->isHidden())
    return 0;
  else
    return static_cast<int>(item_->widget()->maximumHeight().toPixels());
}

DomElement *FlexItemImpl::createDomElement(WT_MAYBE_UNUSED DomElement* parent, WT_MAYBE_UNUSED bool fitWidth,
                                           WT_MAYBE_UNUSED bool fitHeight, WApplication *app)
{
  WWidget *w = item_->widget();

  DomElement *result = w->createSDomElement(app);
  ResizeSensor::applyIfNeeded(w);
  return result;
}

WLayoutItem *FlexItemImpl::layoutItem() const
{
  return item_;
}

}
