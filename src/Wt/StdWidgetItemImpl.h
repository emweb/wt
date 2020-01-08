// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_WIDGET_ITEM_IMPL_H_
#define STD_WIDGET_ITEM_IMPL_H_

#include "StdLayoutItemImpl.h"
#include "Wt/WWidgetItemImpl.h"

namespace Wt {

class StdWidgetItemImpl : public StdLayoutItemImpl, public WWidgetItemImpl
{
public:
  StdWidgetItemImpl(WWidgetItem *item);
  virtual ~StdWidgetItemImpl();

  static const char *childrenResizeJS();
  static const char *childrenGetPSJS();
  static const char *secondResizeJS();
  static const char *secondGetPSJS();

  virtual const std::string id() const override;
  virtual int minimumHeight() const override;
  virtual int minimumWidth() const override;

  virtual WLayoutItem *layoutItem() const override;

  virtual DomElement *createDomElement(DomElement *parent,
				       bool fitWidth, bool fitHeight,
				       WApplication *app) override;

private:
  WWidgetItem *item_;
};

}

#endif // STD_WIDGET_ITEM_IMPL_H_
