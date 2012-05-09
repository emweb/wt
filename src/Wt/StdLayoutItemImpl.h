// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_LAYOUT_ITEM_IMPL_H_
#define STD_LAYOUT_ITEM_IMPL_H_

#include "Wt/WLayoutItemImpl"
#include "Wt/WObject"

namespace Wt {

  class DomElement;
  class StdLayoutImpl;
  class WApplication;
  class WContainerWidget;

class StdLayoutItemImpl : public WLayoutItemImpl, public WObject
{
public:
  StdLayoutItemImpl();
  virtual ~StdLayoutItemImpl();

  virtual WContainerWidget *container() const;
  virtual WLayoutItem      *layoutItem() const = 0;
  virtual WWidget          *parentWidget() const;

  virtual int minimumWidth() const = 0;
  virtual int minimumHeight() const = 0;

  StdLayoutImpl *parentLayoutImpl() const;

  virtual void containerAddWidgets(WContainerWidget *container) = 0;
  virtual DomElement *createDomElement(bool fitWidth, bool fitHeight,
				       WApplication *app) = 0;
};

}

#endif // STD_LAYOUT_ITEM_IMPL_H_
