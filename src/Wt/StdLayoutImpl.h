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

/*! \class StdLayoutImpl Wt/StdLayoutImpl.h Wt/StdLayoutImpl.h
 *  \brief An abstract base class for implementing layout managers.
 *  
 * \sa StdLayoutItemImpl, WLayout
 */
class WT_API StdLayoutImpl : public StdLayoutItemImpl, public WLayoutImpl
{
public:
  /*! \brief Constructor.
   * 
   * Creates a new StdLayoutImpl for the given WLayout.
   */
  StdLayoutImpl(WLayout *layout);

  /*! \brief Destructor.
   */
  virtual ~StdLayoutImpl();

  /*! \brief Updates the DomElements in the WLayout.
   * 
   * This function should update the DomElements in the WLayout. This
   * means creating DomElements for newly added StdLayoutItemImpl,
   * deleting DomElements from deleted StdLayoutItemImpl and updating
   * the placement and size of the DomElements.
   */
  virtual void updateDom(DomElement& parent) = 0;

  /*! \brief Called when a WLayoutItem in the WLayout is resized.
   * 
   * When a WLayoutItem is resized, it may be necessary to update the
   * whole layout. Items may have moved in such a way, that it would
   * push other items of screen, requiring other items to adapt to it.
   * 
   * If it returns \p true, a subsequent updateDom() may be necessary.
   * 
   * \sa WContainerWidget::updateDomChildren()
   */
  virtual bool itemResized(WLayoutItem *item) = 0;

  /*! \brief Called when the parent is resized.
   * 
   * When the parent is resized, it may be necessary to update the whole
   * layout. More or less items could not potentially fit in the layout, 
   * or the layout's boundaries may have changed.
   * 
   * If it returns \p true, a subsequent updateDom() may be necessary.
   * 
   * \sa updateDom(), WContainerWidget::parentResized()
   */
  virtual bool parentResized() = 0;

  //! Returns the WLayout as a WLayoutItem.
  WLayoutItem *layoutItem() const override;

  /*! \brief Updates the layout.
   * 
   * By default, this will trigger a call to updateDom().
   * 
   * \note Several calls to update() may happens before updateDom() is
   * called.
   * 
   * \sa updateDom()
   */
  void update() override;

protected:

  //! Returns the WLayout.
  WLayout *layout() const { return layout_; }

  /*! \brief Returns a WLayoutItem implementation.
   * 
   * Returns a WLayoutItem implementation if the implementation is a
   * subclass of StdLayoutItemImpl. Otherwise returns \p nullptr.
   */
  static StdLayoutItemImpl *getImpl(WLayoutItem *item);

private:
  WLayout *layout_;
};

}

#endif // STD_LAYOUT_IMPL_H_
