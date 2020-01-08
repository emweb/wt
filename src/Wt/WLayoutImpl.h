// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLAYOUT_IMPL_H_
#define WLAYOUT_IMPL_H_

#include <Wt/WLayoutItemImpl.h>

namespace Wt {

class WLayoutItem;
class WWidget;

/*! \class WLayoutImpl Wt/WLayoutImpl.h Wt/WLayoutImpl.h
 *  \brief An abstract base class for implementing layout managers.
 *
 * \sa WLayoutItem, WLayout
 */
class WT_API WLayoutImpl : public WLayoutItemImpl
{
public:
  /*! \brief Destructor.
   */
  virtual ~WLayoutImpl();

  /*! \brief Adds a layout <i>item</i>.
   *
   * The \p item already has an implementation set.
   */
  virtual void itemAdded(WLayoutItem *item) = 0;

  /*! \brief Removes a layout <i>item</i>.
   */
  virtual void itemRemoved(WLayoutItem *item) = 0;

  /*! \brief Updates the layout.
   */
  virtual void update() = 0;
};

}

#endif // WLAYOUT_IMPL_H_
