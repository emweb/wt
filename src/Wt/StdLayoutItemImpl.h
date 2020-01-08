// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_LAYOUT_ITEM_IMPL_H_
#define STD_LAYOUT_ITEM_IMPL_H_

#include "Wt/WObject.h"
#include "Wt/WLayoutItemImpl.h"

namespace Wt {

  class DomElement;
  class StdLayoutImpl;
  class WApplication;
  class WContainerWidget;

class StdLayoutItemImpl : public WObject, public WLayoutItemImpl
{
public:
  StdLayoutItemImpl();
  virtual ~StdLayoutItemImpl();

  WContainerWidget *container() const;
  virtual WLayoutItem *layoutItem() const = 0;

  virtual int minimumWidth() const = 0;
  virtual int minimumHeight() const = 0;

  StdLayoutImpl *parentLayoutImpl() const;

  virtual DomElement *createDomElement(DomElement *parent,
				       bool fitWidth, bool fitHeight,
				       WApplication *app) = 0;
};

}

#endif // STD_LAYOUT_ITEM_IMPL_H_
