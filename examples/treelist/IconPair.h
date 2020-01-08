// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ICONPAIR_H_
#define ICONPAIR_H_

#include <Wt/WCompositeWidget.h>

using namespace Wt;

namespace Wt {
  class WImage;
}

/**
 * @addtogroup treelist
 */
/*@{*/

/*! \brief An icon pair (identical to WIconPair)
 *
 * This widget manages two images, only one of which is shown at a single
 * time.
 *
 * The widget may also react to click events, by changing state.
 *
 * This widget is part of the %Wt treelist example, where it is used
 * to represent the expand/collapse icons, and the corresponding
 * map open/close icon.
 *
 * \sa TreeNode
 */
class IconPair : public WCompositeWidget
{
public:
  /*! \brief Construct a two-state icon widget.
   *
   * The constructor takes the URI of the two icons. When clickIsSwitch
   * is set true, clicking on the icon will switch state.
   */
  IconPair(const std::string icon1URI, const std::string icon2URI,
           bool clickIsSwitch = true);

  /*! \brief Set which icon should be visible.
   *
   * The first icon has number 0, and the second icon has number 1.
   *
   * \sa state()
   */
  void setState(int num);

  /*! \brief Get the current state.
   *
   * \sa setState()
   */
  int state() const;

  /*! \brief Get the first icon image
   */
  WImage *icon1() const { return icon1_; }
  
  /*! \brief Get the second icon image
   */
  WImage *icon2() const { return icon2_; }

  /*! \brief Set state to 0 (show icon 1).
   */
  void showIcon1();

  /*! \brief Set state to 1 (show icon 2).
   */ 
  void showIcon2();

private:
  WContainerWidget *impl_;

  //! First icon.
  WImage *icon1_;

  //! Second icon.
  WImage *icon2_;

public:
  /*! \brief Signal emitted when clicked while in state 0 (icon 1 is
   *         shown).
   */
  EventSignal<WMouseEvent> *icon1Clicked;

  /*! \brief Signal emitted when clicked while in state 1 (icon 2 is
   *         shown).
   */
  EventSignal<WMouseEvent> *icon2Clicked;

private:
  //! Undo state for prelearning stateless showIcon1() and showIcon2() slots
  int previousState_;

  //! Undo function for prelearning showIcon1()
  void undoShowIcon1();

  //! Undo function for prelearning showIcon2()
  void undoShowIcon2();
};

/*@}*/

#endif // ICONPAIR_H_
