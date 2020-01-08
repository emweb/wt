// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLAYOUT_ITEM_H_
#define WLAYOUT_ITEM_H_

#include <Wt/WDllDefs.h>
#include <Wt/WWebWidget.h>

#include <functional>

namespace Wt {

  class WLayout;
  class WWidget;
  class WWidgetItem;
  class WLayoutItemImpl;

/*! \class WLayoutItem Wt/WLayoutItem.h Wt/WLayoutItem.h
 *  \brief An abstract base class for items that can participate in a layout.
 *
 * \sa WLayout
 */
class WT_API WLayoutItem
{
public:
  /*! \brief Destructor.
   */
  virtual ~WLayoutItem();

  /*! \brief Finds the widget item corresponding to the given <i>widget</i>
   *
   * The widget is searched for recursively inside nested layouts.
   */
  virtual WWidgetItem *findWidgetItem(WWidget *widget) = 0;

  /*! \brief Returns the layout that implements this WLayoutItem.
   *
   * This implements a type-safe upcasting mechanism to a WLayout.
   */
  virtual WLayout *layout() = 0;

  /*! \brief Returns the widget that is held by this WLayoutItem.
   *
   * This implements a type-safe upcasting mechanism to a WWidgetItem.
   */
  virtual WWidget *widget() = 0;

  /*! \brief Returns the layout in which this item is contained.
   */
  virtual WLayout *parentLayout() const = 0;

  virtual WWidget *parentWidget() const = 0;

  virtual WLayoutItemImpl *impl() const = 0;

#ifndef WT_TARGET_JAVA
  typedef std::function<void (WWidget *)> HandleWidgetMethod;
#endif
  virtual void iterateWidgets(const HandleWidgetMethod& method) const = 0;

private:
  /*! \brief Internal method.
   */
  virtual void setParentWidget(WWidget *parent) = 0;
  
  /*! \brief Internal method.
   */
  virtual void setParentLayout(WLayout *parentLayout) = 0;

  friend class WLayout;
};

}

#endif // WLAYOUT_ITEM_H_
