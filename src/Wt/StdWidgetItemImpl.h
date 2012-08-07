// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_WIDGET_ITEM_IMPL_H_
#define STD_WIDGET_ITEM_IMPL_H_

#include "StdLayoutItemImpl.h"

namespace Wt {

  class WWidgetItem;

class StdWidgetItemImpl : public StdLayoutItemImpl
{
public:
  StdWidgetItemImpl(WWidgetItem *item);
  virtual ~StdWidgetItemImpl();

  static const char *childrenResizeJS();
  static const char *childrenGetPSJS();
  static const char *secondResizeJS();
  static const char *secondGetPSJS();

  virtual const std::string id() const;
  virtual int minimumHeight() const;
  virtual int minimumWidth() const;

  virtual void updateAddItem(WLayoutItem *);
  virtual void updateRemoveItem(WLayoutItem *);
  virtual void update(WLayoutItem *);

  virtual WLayoutItem *layoutItem() const;
  virtual void containerAddWidgets(WContainerWidget *container);
  virtual DomElement *createDomElement(bool fitWidth, bool fitHeight,
				       WApplication *app);

  virtual void setHint(const std::string& name, const std::string& value);

private:
  WWidgetItem *item_;
};

}

#endif // STD_WIDGET_ITEM_IMPL_H_
