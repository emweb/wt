// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPOPUP_MENU_H_
#define WPOPUP_MENU_H_

#include <Wt/WMenu.h>
#include <Wt/WJavaScript.h>

namespace Wt {

/*! \class WPopupMenu Wt/WPopupMenu.h Wt/WPopupMenu.h
 *  \brief A menu presented in a popup window.
 *
 * The menu implements a typical context menu, with support for
 * submenu's. It is a specialized WMenu from which it inherits most of
 * the API.
 *
 * When initially created, the menu is invisible, until popup() or
 * exec() is called. Then, the menu will remain visible until an item
 * is selected, or the user cancels the menu (by hitting Escape or
 * clicking elsewhere).
 *
 * The implementation assumes availability of JavaScript to position
 * the menu at the current mouse position and provide feed-back of the
 * currently selected item.
 *
 * As with WDialog, there are two ways of using the menu. The simplest
 * way is to use one of the synchronous exec() methods, which starts a
 * reentrant event loop and waits until the user cancelled the popup
 * menu (by hitting Escape or clicking elsewhere), or selected an
 * item.
 *
 * Alternatively, you can use one of the popup() methods to show the
 * menu and listen to the \link WPopupMenu::triggered
 * triggered\endlink signal where you read the result(), or associate the
 * menu with a button using WPushButton::setMenu().
 *
 * You have several options to react to the selection of an item:
 * - Either you use the WMenuItem itself to identify the action,
 *   perhaps by specialization or simply by binding custom data using
 *   WMenuItem::setData().
 * - You can bind a separate method to each item's WMenuItem::triggered
 *   signal.
 *
 * Usage example:
 * \if cpp
 * \code
 * // Create a menu with some items
 * WPopupMenu popup;
 * popup.addItem("icons/item1.gif", "Item 1");
 * popup.addItem("Item 2")->setCheckable(true);
 * popup.addItem("Item 3");
 * popup.addSeparator();
 * popup.addItem("Item 4");
 * popup.addSeparator();
 * popup.addItem("Item 5");
 * popup.addItem("Item 6");
 * popup.addSeparator();
 *
 * auto subMenu = std::make_unique<Wt::WPopupMenu>();
 * subMenu->addItem("Sub Item 1");
 * subMenu->addItem("Sub Item 2");
 * popup.addMenu("Item 7", std::move(subMenu));
 *
 * WMenuItem *item = popup.exec(event);
 *
 * if (item) {
 *   // ... do associated action.
 * }
 * \endcode
 * \elseif java
 * \code
 * // Create a menu with some items
 * WPopupMenu popup = new WPopupMenu();
 * popup.addItem("icons/item1.gif", "Item 1");
 * popup.addItem("Item 2").setCheckable(true);
 * popup.addItem("Item 3");
 * popup.addSeparator();
 * popup.addItem("Item 4");
 * popup.addSeparator();
 * popup.addItem("Item 5");
 * popup.addItem("Item 6");
 * popup.addSeparator();
 *		 
 * WPopupMenu subMenu = new WPopupMenu();
 * subMenu.addItem("Sub Item 1");
 * subMenu.addItem("Sub Item 2");
 * popup.addMenu("Item 7", subMenu);
 *		 
 * WMenuItem item = popup.exec(event);
 *		 
 * if (item != null) {
 *  // ... do associated action.
 * }
 * \endcode
 * \endif
 *
 * A snapshot of the WPopupMenu: 
 * \image html WPopupMenu-default-1.png "WPopupMenu example (default)"
 * \image html WPopupMenu-polished-1.png "WPopupMenu example (polished)"
 *
 * \sa WMenuItem
 */
class WT_API WPopupMenu : public WMenu
{
public:
  /*! \brief Creates a new popup menu.
   *
   * The menu is hidden, by default, and must be shown using popup()
   * or exec().
   */
  WPopupMenu(WStackedWidget *contentsStack = nullptr);

  virtual ~WPopupMenu();

  /*! \brief Shows the the popup at a position.
   *
   * Displays the popup at a point with document coordinates
   * \p point. The positions intelligent, and will chose one of
   * the four menu corners to correspond to this point so that the
   * popup menu is completely visible within the window.
   *
   * \sa exec()
   */
  void popup(const WPoint& point);

  /*! \brief Shows the the popup at the location of a mouse event.
   *
   * This is a convenience method for popup(const WPoint&) that uses the
   * event's document coordinates.
   *
   * \sa popup(const WPoint& p), WMouseEvent::document()
   */
  void popup(const WMouseEvent& event);

  // Sets the button that triggers the popup
  void setButton(WInteractWidget *button);

  /*! \brief Shows the popup besides a widget.
   *
   * \sa positionAt(), popup(const WPointF&)
   */
  void popup(WWidget *location, 
	     Orientation orientation = Orientation::Vertical);

  /*! \brief Executes the the popup at a position.
   *
   * Displays the popup at a point with document coordinates \p p,
   * using popup(), and the waits until a menu item is selected, or
   * the menu is cancelled.
   *
   * Returns the selected menu (or sub-menu) item, or \c 0 if the user
   * cancelled the menu.
   *
   * \sa popup()
   */
  WMenuItem *exec(const WPoint& point);

  /*! \brief Executes the the popup at the location of a mouse event.
   *
   * This is a convenience method for exec(const WPoint& p) that uses the
   * event's document coordinates.
   *
   * \sa exec(const WPoint&)
   */
  WMenuItem *exec(const WMouseEvent& event);

  /*! \brief Executes the popup besides a widget.
   *
   * \sa positionAt(), popup(const WPointF&)
   */
  WMenuItem *exec(WWidget *location, 
		  Orientation orientation = Orientation::Vertical);

  /*! \brief Returns the last triggered menu item.
   *
   * The result is \c nullptr when the user cancelled the popup menu.
   */
  WMenuItem *result() const { return result_; }

  virtual void setHidden(bool hidden,
                         const WAnimation& animation = WAnimation()) override;

  virtual void setMaximumSize(const WLength& width, const WLength& height) override;
  virtual void setMinimumSize(const WLength& width, const WLength& height) override;

  /*! \brief %Signal emitted when the popup is hidden.
   *
   * Unlike the itemSelected() signal, aboutToHide() is only emitted
   * by the toplevel popup menu (and not by submenus), and is also
   * emitted when no item was selected.
   *
   * You can use result() to get the selected item, which may be \c nullptr.
   *
   * \sa triggered(), itemSelected()
   */
  Signal<>& aboutToHide() { return aboutToHide_; }

  /*! \brief %Signal emitted when an item is selected.
   *
   * Unlike the itemSelected() signal, triggered() is only emitted
   * by the toplevel popup menu (and not by submenus).
   *
   * \sa aboutToHide(), itemSelected()
   */
  Signal<WMenuItem *>& triggered() { return triggered_; }

  /*! \brief Configure auto-hide when the mouse leaves the menu.
   *
   * If \p enabled, The popup menu will be hidden when the mouse
   * leaves the menu for longer than \p autoHideDelay
   * (milliseconds). The popup menu result will be 0, as if the user
   * cancelled.
   *
   * By default, this option is disabled.
   */
  void setAutoHide(bool enabled, int autoHideDelay = 0);

  /*! \brief Set whether this popup menu should hide when an item is selected.
   *
   * Defaults to true.
   *
   * \sa hideOnSelect()
   */
  void setHideOnSelect(bool enabled = true);

  /*! \brief Returns whether this popup menu should hide when an item is selected.
   *
   * \sa setHideOnSelect()
   */
  bool hideOnSelect() const { return hideOnSelect_; }

protected:
  virtual void renderSelected(WMenuItem *item, bool selected) override;
  virtual void setCurrent(int index) override;
  virtual void getSDomChanges(std::vector<DomElement *>& result, WApplication *app) override;
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual std::string renderRemoveJs(bool recursive) override;

private:
  WPopupMenu *topLevel_;
  WMenuItem *result_;
  WWidget *location_;
  WInteractWidget *button_;

  Signal<> aboutToHide_;
  Signal<WMenuItem *> triggered_;
  JSignal<> cancel_;

  bool recursiveEventLoop_;
  bool willPopup_;
  bool hideOnSelect_;
  int autoHideDelay_;

  void exec();
  void cancel();
  void done(WMenuItem *result);
  void popupImpl();
  void prepareRender(WApplication *app);
  void adjustPadding();
  void popupAtButton();
  void connectSignals(WPopupMenu * const topLevel);
};

}

#endif // WPOPUP_MENU_H_
