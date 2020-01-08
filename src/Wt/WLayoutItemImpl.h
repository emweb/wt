// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLAYOUT_ITEM_IMPL_H_
#define WLAYOUT_ITEM_IMPL_H_

#include <Wt/WDllDefs.h>
#include <string>

namespace Wt {

class WLayoutItem;
class WWidget;

/*! \class WLayoutItemImpl Wt/WLayoutItemImpl.h Wt/WLayoutItemImpl.h
 *  \brief An abstract base class for implementing layout managers.
 *
 * \sa WLayoutItem
 */
class WT_API WLayoutItemImpl
{
public:
  /*! \brief Destructor.
   */
  virtual ~WLayoutItemImpl();
};

}

#endif // WLAYOUT_ITEM_IMPL_H_
