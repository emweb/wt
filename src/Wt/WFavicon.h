// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFAVICON_H_
#define WFAVICON_H_

#include "Wt/WDllDefs.h"
#include <string>

namespace Wt {

/*! \class WFavicon Wt/WFavicon.h Wt/WFavicon.h
 *  \brief An abstract class representing a favicon.
 * 
 * The favicon of a page is an icon that is used by the browser to
 * represent the page in its user interface. In modern browsers, this
 * is usualy showed on the browser tab of the page.
 * 
 * Changing the favicon of a page can be used to notify the user that
 * something happend on the page without the having to use
 * notifications, which can feel invasive and requires the user's
 * approval to be used. This class is designed with this idea in mind.
 * 
 * A WFavicon can be in 2 states:
 * -# Updated: used to notify the user
 * -# Default: used otherwise
 * 
 * The state of the WFavicon can be changed using update() and reset().
 * 
 * \sa WApplication::setFavicon()
 */
class WT_API WFavicon
{
public:
  /*! \brief Returns the url to the favicon.
   * 
   * This should return the absolute url to the favicon.
   */
  virtual std::string url() const = 0;

  /*! \brief Sets the favicon in its update state.
   * 
   * This sets the favicon in its update state.
   * 
   * If you want the favicon to be different in its updated state, you
   * should override doUpdate(), which is called at the end of this
   * function.
   * 
   * \note Despite its name, this does not push any updates to the
   *       browser unless it changes the url of the favicon.
   * 
   * \sa reset(), url()
   */
  void update();

  /*! \brief Sets the favicon in its default state.
   * 
   * Sets the favicon in its default state.
   * 
   * If you did any changes in doUpdate(), you should clean up those
   * changes in doReset(), which is called at the end of this function.
   * 
   * \note This does not push any updates to the browser unless it
   *       changes the url of the favicon.
   * 
   * \sa update(), url()
   */
  void reset();

  /*! \brief Returns whether the favicon is in its update state or not.
   * 
   * \sa update(), reset()
   */
  bool isUpdated() const { return isUpdated_; }

protected:
  WFavicon();

  /*! \brief Updates the favicon.
   * 
   * This is called at the end of update() and does nothing by default.
   * You can override this function to make changes to the favicon when
   * update() is called.
   * 
   * If you do so, you will likely want to clean up those changes in
   * doReset().
   * 
   * \note If you want those changes to be pushed to the client, you
   *       have to make sure that this updates the url of the favicon.
   * 
   * \sa doReset(), update(), url()
   */
  virtual void doUpdate();

  /*! \brief Resets the favicon.
   * 
   * This is called at the end of reset() and should be overriden to
   * make it clean up what was done in doUpdate().
   * 
   * \note If you want those changes to be pushed to the client, you
   *       have to make sure that this updates the url of the favicon.
   * 
   * \sa doUpdate(), reset(), url()
   */
  virtual void doReset();

private:
  bool isUpdated_;
};

}
#endif //WFAVICON_H_