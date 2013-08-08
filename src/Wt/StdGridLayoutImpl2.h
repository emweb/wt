// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_GRID_LAYOUT_IMPL2_H_
#define STD_GRID_LAYOUT_IMPL2_H_

#include <Wt/WGridLayout>
#include "StdLayoutImpl.h"

namespace Wt {

  class WApplication;
  class WLayout;
  class WStringStream;

class StdGridLayoutImpl2 : public StdLayoutImpl
{
public:
  StdGridLayoutImpl2(WLayout *layout, Impl::Grid& grid);
  virtual ~StdGridLayoutImpl2();

  virtual int minimumWidth() const;
  virtual int minimumHeight() const;

  virtual void updateAddItem(WLayoutItem *);
  virtual void updateRemoveItem(WLayoutItem *);
  virtual void update(WLayoutItem *);
  virtual DomElement *createDomElement(bool fitWidth, bool fitHeight,
				       WApplication *app);
  virtual void updateDom(DomElement& parent);

  static bool useJavaScriptHeights(WApplication *app);

  virtual void setHint(const std::string& name, const std::string& value);

  // Does not really belong here, but who cares ?
  static const char* childrenResizeJS();

  virtual bool itemResized(WLayoutItem *item);
  virtual bool parentResized();

protected:
  virtual void containerAddWidgets(WContainerWidget *container);

private:
  Impl::Grid& grid_;
  bool needAdjust_, needRemeasure_, needConfigUpdate_;
  std::vector<WLayoutItem *> addedItems_;
  std::vector<std::string> removedItems_;

  int nextRowWithItem(int row, int c) const;
  int nextColumnWithItem(int row, int col) const;
  bool hasItem(int row, int col) const;
  int minimumHeightForRow(int row) const;
  int minimumWidthForColumn(int column) const;
  static int pixelSize(const WLength& size);

  void streamConfig(WStringStream& js,
		    const std::vector<Impl::Grid::Section>& sections,
		    bool rows, WApplication *app);
  void streamConfig(WStringStream& js, WApplication *app);
  DomElement *createElement(WLayoutItem *item, WApplication *app);
};

}

#endif // STD_GRID_LAYOUT_IMPL2_H_
