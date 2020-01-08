// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WSPLITBUTTON_H_
#define WT_WSPLITBUTTON_H_

#include <vector>

#include <Wt/WCompositeWidget.h>
#include <Wt/WMenuItem.h>

namespace Wt {

class WStackedWidget;
class WTable;

/*! \class WSplitButton Wt/WSplitButton.h Wt/WSplitButton.h
 *  \brief A split button
 *
 * A split button combines a button and a drop down menu. Typically,
 * the button represents an action, with related alternative actions
 * accessible from the drop down menu.
 */
class WT_API WSplitButton : public WCompositeWidget
{
public:
  /*! \brief Constructor.
   */
  WSplitButton();

  /*! \brief Constructor passing the label.
   */
  WSplitButton(const WString& label);

  /*! \brief Returns the action button.
   *
   * This is the button that represents the main action.
   */
  WPushButton *actionButton() const;

  /*! \brief Returns the drop down button.
   *
   * This represents the button that represents the drop-down action.
   */
  WPushButton *dropDownButton() const;

  /*! \brief Sets the menu for the drop-down button.
   */
  void setMenu(std::unique_ptr<WPopupMenu> menu);

  /*! \brief Returns the menu for the drop-down button.
   */
  WPopupMenu *menu() const;

private:
  WToolBar *impl_;

  void init(const WString& label);
};

}

#endif // WT_WSPLITBUTTON_H_
