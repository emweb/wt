// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FLEX_ITEM_IMPL_H_
#define FLEX_ITEM_IMPL_H_

#include "Wt/WWidgetItemImpl.h"
#include "StdLayoutItemImpl.h"

namespace Wt {

class FlexItemImpl : public StdLayoutItemImpl, public WWidgetItemImpl
{
public:
  FlexItemImpl(WWidgetItem *item);
  virtual ~FlexItemImpl();

  WLayoutItem *layoutItem() const override;
  int minimumHeight() const override;
  int minimumWidth() const override;
  int maximumHeight() const override;
  int maximumWidth() const override;

  DomElement *createDomElement(DomElement *parent,
                               bool fitWidth, bool fitHeight,
                               WApplication *app) override;

private:
  WWidgetItem *item_;
};

}

#endif // FLEX_ITEM_IMPL_H_
