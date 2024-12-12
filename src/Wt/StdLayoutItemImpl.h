// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef STD_LAYOUT_ITEM_IMPL_H_
#define STD_LAYOUT_ITEM_IMPL_H_

#include "Wt/WObject.h"
#include "Wt/WLayoutItemImpl.h"

namespace Wt {

  class DomElement;
  class StdLayoutImpl;
  class WApplication;
  class WContainerWidget;

/*! \class StdLayoutItemImpl Wt/StdLayoutItemImpl.h Wt/StdLayoutItemImpl.h
 *  \brief An abstract class for implementing layout managers.
 *
 * \sa WLayoutItem
 */
class WT_API StdLayoutItemImpl : public WObject, public WLayoutItemImpl
{
public:
  /*! \brief Constructor.
   */
  StdLayoutItemImpl();

  /*! \brief Destructor.
   */
  virtual ~StdLayoutItemImpl();

  /*! \brief Returns the container of the of the parent layout.
   */
  WContainerWidget *container() const;

  /*! \brief Returns the actual WLayoutItem.
   */
  virtual WLayoutItem *layoutItem() const = 0;

  /*! \brief Returns the minimum width of the item.
   */
  virtual int minimumWidth() const = 0;

  /*! \brief Returns the minimum height of the item.
   */
  virtual int minimumHeight() const = 0;

  /*! \brief Returns the maximum width of the item.
   */
  virtual int maximumWidth() const = 0;

  /*! \brief Returns the maximum height of the item.
   */
  virtual int maximumHeight() const = 0;

  /*! \brief Returns the parent layout of the item.
   * 
   * Returns the parent layout of the item as a StdLayoutImpl
   * if the layout is a subclass of StdLayoutImpl. Otherwise
   * returns nullptr;
   */
  StdLayoutImpl *parentLayoutImpl() const;

  virtual DomElement *createDomElement(DomElement *parent,
                                       bool fitWidth, bool fitHeight,
                                       WApplication *app) = 0;
};

}

#endif // STD_LAYOUT_ITEM_IMPL_H_
