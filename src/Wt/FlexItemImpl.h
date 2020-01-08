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

  virtual WLayoutItem *layoutItem() const override;
  virtual int minimumHeight() const override;
  virtual int minimumWidth() const override;

  DomElement *createDomElement(DomElement *parent,
			       bool fitWidth, bool fitHeight,
			       WApplication *app) override;

private:
  WWidgetItem *item_;
};

}

#endif // FLEX_ITEM_IMPL_H_
