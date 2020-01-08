// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLOADING_INDICATOR_H_
#define WLOADING_INDICATOR_H_

#include <Wt/WCompositeWidget.h>

namespace Wt {

/*! \class WLoadingIndicator Wt/WLoadingIndicator.h Wt/WLoadingIndicator.h
 *  \brief An abstract interface for a loading indicator.
 *
 * The loading indicator displays a message while a response from the
 * server is pending.
 *
 * The widget will be shown and hidden using WWidget::show() and
 * WWidget::hide(). If you want to customize this behaviour, you
 * should reimplement the WWidget::setHidden() method.
 *
 * \if cpp
 * Note that show() and hide() are stateless slots, and thus you need to make
 * sure that your implementation comforms to that contract, so that it
 * may be optimized to JavaScript (the server-side implementation will
 * only be called during stateless slot prelearning).
 * \endif
 *
 * \sa WApplication::setLoadingIndicator()
 */
class WT_API WLoadingIndicator : public WCompositeWidget
{
public:
  /*! \brief Constructor
   */
  WLoadingIndicator();

  /*! \brief Destructor.
   *
   * The destructor must delete the widget().
   */
  virtual ~WLoadingIndicator();

  /*! \brief Sets the message that you want to be displayed.
   *
   * If the indicator is capable of displaying a text message, then
   * you should reimplement this method to allow this message to be
   * modified.
   */
  virtual void setMessage(const WString& text) = 0;
};

}

#endif // WLOADING_INDICATOR_H_
