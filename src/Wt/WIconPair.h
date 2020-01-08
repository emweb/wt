// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WICONPAIR_H_
#define WICONPAIR_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WEvent.h>

namespace Wt {

class WImage;

/*! \class WIconPair Wt/WIconPair.h Wt/WIconPair.h
 *  \brief A widget that shows one of two icons depending on its state.
 *
 * This is a utility class that simply manages two images, only one of
 * which is shown at a single time, which reflects the current
 * 'state'.
 *
 * The widget may react to click events, by changing state.
 *
 * <h3>CSS</h3>
 * 
 * This widget does not provide styling, 
 * and can be styled using inline or external CSS as appropriate.
 * The image may be styled via the <tt>&lt;img&gt;</tt> elements.
 */
class WT_API WIconPair : public WCompositeWidget
{
public:
  /*! \brief Construct an icon pair from the two icons.
   *
   * The constructor takes the URL of the two icons. When
   * \p clickIsSwitch is set \c true, clicking on the icon will
   * switch state.
   */
  WIconPair(const std::string& icon1URL, const std::string& icon2URL,
	    bool clickIsSwitch = true);

  /*! \brief Sets the state, which determines the visible icon.
   *
   * The first icon has number 0, and the second icon has number 1.
   *
   * The default state is 0.
   *
   * \sa state()
   */
  void setState(int num);

  /*! \brief Returns the current state.
   *
   * \sa setState()
   */
  int state() const;

  /*! \brief Returns the first icon image
   */
  WImage *icon1() const { return icon1_; }
  
  /*! \brief Returns the second icon image
   */
  WImage *icon2() const { return icon2_; }

  /*! \brief Sets the state to 0 (show icon 1).
   *
   * \sa setState(int)
   */
  void showIcon1();

  /*! \brief Sets the state to 1 (show icon 2).
   *
   * \sa setState(int)
   */ 
  void showIcon2();

  /*! \brief %Signal emitted when clicked while in state 0 (icon 1 is
   *         shown).
   *
   * Equivalent to:
   * \code
   * icon1()->clicked()
   * \endcode
   */
  EventSignal<WMouseEvent>& icon1Clicked();

  /*! \brief %Signal emitted when clicked while in state 1 (icon 2 is
   *         shown).
   *
   * Equivalent to:
   * \code
   * icon2()->clicked()
   * \endcode
   */
  EventSignal<WMouseEvent>& icon2Clicked();

private:
  WContainerWidget *impl_;
  WImage *icon1_;
  WImage *icon2_;
};

}

#endif // WICONPAIR_H_
