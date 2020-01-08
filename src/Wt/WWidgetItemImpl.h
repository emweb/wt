// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WWIDGET_ITEM_IMPL_H_
#define WWIDGET_ITEM_IMPL_H_

#include <Wt/WLayoutItemImpl.h>

namespace Wt {

class WLayoutItem;
class WWidget;

/*! \class WWidgetItemImpl Wt/WWidgetItemImpl.h Wt/WWidgetItemImpl.h
 *  \brief An abstract base class for implementing layout managers.
 *
 * \sa WLayoutItem, WLayout
 */
class WT_API WWidgetItemImpl : public WLayoutItemImpl
{
public:
  /*! \brief Destructor.
   */
  virtual ~WWidgetItemImpl();
};

}

#endif // WWIDGET_ITEM_IMPL_H_
