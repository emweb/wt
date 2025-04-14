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

  int minimumWidth() const override;
  int minimumHeight() const override;
  int maximumWidth() const override;
  int maximumHeight() const override;

  void itemAdded(WLayoutItem *) override;
  void itemRemoved(WLayoutItem *) override;

  void updateDom(DomElement& parent) override;

  DomElement *createDomElement(DomElement *parent, bool fitWidth,
                               bool fitHeight, WApplication *app) override;

  bool itemResized(WLayoutItem *item) override;
  bool parentResized() override;

  void setObjectName(const std::string& name) override;

private:
  static const int BIT_OBJECT_NAME_CHANGED = 0;

  std::bitset<1> flags_;

  Impl::Grid& grid_;
  std::vector<WLayoutItem *> addedItems_;
  std::vector<WLayoutItem *> resizedItems_;
  std::vector<std::string> removedItems_;
  std::vector<StdLayoutImpl *> childLayouts_;
  std::string elId_;
  bool canAdjustLayout_;

  int minimumHeightForRow(int row) const;
  int minimumWidthForColumn(int column) const;
  int maximumHeightForRow(int row) const;
  int maximumWidthForColumn(int column) const;
  DomElement *createElement(Orientation orientation, int index,
                            int totalStretch, WApplication *app);
  DomElement *wrap(DomElement *el, const std::string& flow);
  Orientation getOrientation() const;
  LayoutDirection getDirection() const;
  std::string styleDisplay() const;
  std::string styleFlex() const;
  std::string otherStyleFlex() const;

  int count(Orientation orientation) const;
  int indexOf(WLayoutItem *item, Orientation orientation);
  int getTotalStretch(Orientation orientation);

  static StdLayoutImpl *getStdLayoutImpl(WLayoutItem *item);

  Impl::Grid::Item& item(Orientation orientation, int i);
  Impl::Grid::Section& section(Orientation orientation, int i);
};

}

#endif // FLEX_LAYOUT_IMPL_H_
