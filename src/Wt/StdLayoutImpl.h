// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_LAYOUT_IMPL_H_
#define STD_LAYOUT_IMPL_H_

#include "StdLayoutItemImpl.h"
#include "Wt/WLayoutImpl.h"

namespace Wt {

class WLayout;

class StdLayoutImpl : public StdLayoutItemImpl, public WLayoutImpl
{
public:
  StdLayoutImpl(WLayout *layout);
  virtual ~StdLayoutImpl();

  virtual void updateDom(DomElement& parent) = 0;

  // Returns whether updateDom() is needed
  virtual bool itemResized(WLayoutItem *item) = 0;
  virtual bool parentResized() = 0;

  virtual WLayoutItem *layoutItem() const override;

protected:
  WLayout *layout() const { return layout_; }

  static StdLayoutItemImpl *getImpl(WLayoutItem *item);

private:
  WLayout *layout_;
};

}

#endif // STD_LAYOUT_IMPL_H_
