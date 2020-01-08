// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FLEX_LAYOUT_IMPL2_H_
#define FLEX_LAYOUT_IMPL2_H_

#include <Wt/WGridLayout.h>
#include <Wt/WBoxLayout.h>

#include "StdLayoutImpl.h"

namespace Wt {

  class WApplication;
  class WLayout;

class FlexLayoutImpl : public StdLayoutImpl
{
public:
  FlexLayoutImpl(WLayout *layout, Impl::Grid& grid);
  virtual ~FlexLayoutImpl();

  virtual int minimumWidth() const override;
  virtual int minimumHeight() const override;

  virtual void itemAdded(WLayoutItem *) override;
  virtual void itemRemoved(WLayoutItem *) override;

  virtual void updateDom(DomElement& parent) override;

  virtual void update() override;

  virtual DomElement *createDomElement(DomElement *parent,
				       bool fitWidth, bool fitHeight,
				       WApplication *app) override;

  virtual bool itemResized(WLayoutItem *item) override;
  virtual bool parentResized() override;

private:
  Impl::Grid& grid_;
  std::vector<WLayoutItem *> addedItems_;
  std::vector<std::string> removedItems_;
  std::string elId_;

  int minimumHeightForRow(int row) const;
  int minimumWidthForColumn(int column) const;
  DomElement *createElement(Orientation orientation, unsigned index,
			    int totalStretch, WApplication *app);
  Orientation getOrientation() const;
  LayoutDirection getDirection() const;
  std::string styleDisplay() const;
  std::string styleFlex() const;

  int count(Orientation orientation) const;
  int indexOf(WLayoutItem *item, Orientation orientation);
  int getTotalStretch(Orientation orientation);

  Impl::Grid::Item& item(Orientation orientation, int i);
  Impl::Grid::Section& section(Orientation orientation, int i);
};

}

#endif // FLEX_LAYOUT_IMPL_H_
