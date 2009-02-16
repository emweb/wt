// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXT_WWIDGET_ITEM_IMPL_H_
#define EXT_WWIDGET_ITEM_IMPL_H_

#include "Wt/Ext/LayoutItemImpl.h"

namespace Wt {
  class WWidgetItem;

  namespace Ext {

class WWidgetItemImpl : public LayoutItemImpl
{
public:
  WWidgetItemImpl(WWidgetItem *item);
  virtual ~WWidgetItemImpl();

  void updateAddItem(WLayoutItem *);
  void updateRemoveItem(WLayoutItem *);

  virtual WLayoutItem *layoutItem() const;

protected:
  virtual void createComponent(DomElement *parentContainer);
  virtual void containerAddWidgets(Container *container);
  virtual std::string componentVar() const;
  virtual std::string componentId() const;

private:
  WWidgetItem *item_;
  std::string  var_, id_;
};

  }
}

#endif // EXT_WWIDGET_ITEM_IMPL_H_
