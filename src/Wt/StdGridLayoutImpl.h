// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_GRID_LAYOUT_IMPL_H_
#define STD_GRID_LAYOUT_IMPL_H_

#include "StdLayoutImpl.h"

namespace Wt {

  class WApplication;
  class WLayout;

  namespace Impl {
    struct Grid;
  }

class StdGridLayoutImpl : public StdLayoutImpl
{
public:
  StdGridLayoutImpl(WLayout *layout, Impl::Grid& grid);
  virtual ~StdGridLayoutImpl();

  virtual int minimumHeight() const;

  virtual DomElement *createDomElement(bool fitWidth, bool fitHeight,
				       WApplication *app);
  virtual void updateDom();

  static bool useJavaScriptHeights(WApplication *app);

  virtual void setHint(const std::string& name, const std::string& value);

  // Does not really belong here, but who cares ?
  static const char* childrenResizeJS();

  virtual bool itemResized(WLayoutItem *item);

protected:
  virtual void containerAddWidgets(WContainerWidget *container);

private:
  Impl::Grid& grid_;
  bool        useFixedLayout_;
  bool        forceUpdate_;
};

}

#endif // STD_GRID_LAYOUT_IMPL_H_
