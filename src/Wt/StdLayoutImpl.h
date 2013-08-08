// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_LAYOUT_IMPL_H_
#define STD_LAYOUT_IMPL_H_

#include "StdLayoutItemImpl.h"

namespace Wt {

class WLayout;

class StdLayoutImpl : public StdLayoutItemImpl
{
public:
  StdLayoutImpl(WLayout *layout);
  virtual ~StdLayoutImpl();

  virtual void updateAddItem(WLayoutItem *);
  virtual void updateRemoveItem(WLayoutItem *);
  virtual void update(WLayoutItem *) = 0;
  virtual void updateDom(DomElement& parent) = 0;

  // Returns whether updateDom() is needed
  virtual bool itemResized(WLayoutItem *item) = 0;
  virtual bool parentResized() = 0;

  virtual WContainerWidget *container() const;
  virtual WLayoutItem *layoutItem() const;

protected:
  virtual void containerAddWidgets(WContainerWidget *container);

  WLayout *layout() const { return layout_; }

  static StdLayoutItemImpl *getImpl(WLayoutItem *item);

private:
  WLayout          *layout_;
  WContainerWidget *container_;

  void setContainer(WContainerWidget *c);

  friend class WContainerWidget;
};

}

#endif // STD_LAYOUT_IMPL_H_
