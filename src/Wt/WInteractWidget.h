// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WINTERACT_WIDGET_H_
#define WINTERACT_WIDGET_H_

#include <Wt/WWebWidget.h>
#include <Wt/WEvent.h>

namespace Wt {

class JSlot;

/*! \class WInteractWidget Wt/WInteractWidget.h Wt/WInteractWidget.h
 *  \brief An abstract widget that can receive user-interface interaction
 *
 * This abstract widget provides access to event signals that
 * correspond to user-interface interaction through mouse or keyboard.
 *
 * When JavaScript is disabled, only the clicked() event will
 * propagate (but without event details information).
 *
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 */
class WT_API WInteractWidget : public WWebWidget
{
public:
  /*! \brief Create an InteractWidget.
   */
  WInteractWidget();

  ~WInteractWidget();

  /*! \brief Event signal emitted when a keyboard key is pushed down.
   *
   * The keyWentDown signal is the first signal emitted when a key is
   * pressed (before the keyPressed signal). Unlike keyPressed()
   * however it is also emitted for modifier keys (such as "shift",
   * "control", ...) or keyboard navigation keys that do not have a
   * corresponding character.
   *
   * Form widgets (like WLineEdit) will receive key events when
   * focussed. Other widgets will receive key events when they contain
   * (directly or indirectly) a form widget that has focus.
   *
   * To capture a key down event when no element has focus, see
   * WApplication::globalKeyWentDown()
   *
   * \sa keyPressed(), keyWentUp()
   */
  EventSignal<WKeyEvent>& keyWentDown();

  /*! \brief Event signal emitted when a "character" was entered.
   *
   * The keyPressed signal is emitted when a key is pressed, and a
   * character is entered. Unlike keyWentDown(), it is emitted only
   * for key presses that result in a character being entered, and
   * thus not for modifier keys or keyboard navigation keys.
   *
   * Form widgets (like WLineEdit) will receive key events when
   * focussed. Other widgets will receive key events when they contain
   * (directly or indirectly) a form widget that has focus.
   *
   * To capture a key press when no element has focus, see
   * WApplication::globalKeyPressed()
   *
   * \sa keyWentDown()
   */
  EventSignal<WKeyEvent>& keyPressed();
    
  /*! \brief Event signal emitted when a keyboard key is released.
   *
   * This is the counter-part of the keyWentDown() event. Every
   * key-down has its corresponding key-up.
   *
   * Form widgets (like WLineEdit) will receive key events when
   * focussed. Other widgets will receive key events when they contain
   * (directly or indirectly) a form widget that has focus.
   *
   * To capture a key up event when no element has focus, see
   * WApplication::globalKeyWentUp()
   *
   * \sa keyWentDown()
   */
  EventSignal<WKeyEvent>& keyWentUp();

  /*! \brief Event signal emitted when enter was pressed.
   *
   * This signal is emitted when the Enter or Return key was pressed.
   *
   * Form widgets (like WLineEdit) will receive key events when
   * focussed. Other widgets will receive key events when they contain
   * (directly or indirectly) a form widget that has focus.
   *
   * To capture an enter press when no element has focus, see
   * WApplication::globalEnterPressed()
   *
   * \sa keyPressed(), Key::Enter
   */
  EventSignal<>& enterPressed();

  /*! \brief Event signal emitted when escape was pressed.
   *
   * This signal is emitted when the Escape key was pressed.
   *
   * Form widgets (like WLineEdit) will receive key events when
   * focussed. Other widgets will receive key events when they contain
   * (directly or indirectly) a form widget that has focus.
   *
   * To capture an escape press when no element has focus, see
   * WApplication::globalEscapePressed()
   *
   * \sa keyPressed(), Key::Escape
   */
  EventSignal<>& escapePressed();

  /*! \brief Event signal emitted when a mouse key was clicked on this
   *         widget.
   *
   * The event details contains information such as the \link
   * WMouseEvent::button button\endlink, optional \link
   * WMouseEvent::modifiers() keyboard modifiers\endlink, and mouse
   * coordinates relative to the \link WMouseEvent::widget()
   * widget\endlink, the window \link WMouseEvent::window()
   * window\endlink, or the \link WMouseEvent::document()
   * document\endlink.
   *
   * \note When JavaScript is disabled, the event details contain
   * invalid information.
   */
  EventSignal<WMouseEvent>& clicked();

  /*! \brief Event signal emitted when a mouse key was double clicked
   *         on this widget.
   *
   * The event details contains information such as the \link
   * WMouseEvent::button button\endlink, optional \link
   * WMouseEvent::modifiers() keyboard modifiers\endlink, and mouse
   * coordinates relative to the \link WMouseEvent::widget()
   * widget\endlink, the window \link WMouseEvent::window()
   * window\endlink, or the \link WMouseEvent::document()
   * document\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& doubleClicked();

  /*! \brief Event signal emitted when a mouse key was pushed down on this
   *         widget.
   *
   * The event details contains information such as the \link
   * WMouseEvent::button button\endlink, optional \link
   * WMouseEvent::modifiers() keyboard modifiers\endlink, and mouse
   * coordinates relative to the \link WMouseEvent::widget()
   * widget\endlink, the window \link WMouseEvent::window()
   * window\endlink, or the \link WMouseEvent::document()
   * document\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& mouseWentDown();

  /*! \brief Event signal emitted when a mouse key was released on this
   *         widget.
   *
   * The event details contains information such as the \link
   * WMouseEvent::button button\endlink, optional \link
   * WMouseEvent::modifiers() keyboard modifiers\endlink, and mouse
   * coordinates relative to the \link WMouseEvent::widget()
   * widget\endlink, the window \link WMouseEvent::window()
   * window\endlink, or the \link WMouseEvent::document()
   * document\endlink.
   *
   * If you connect also the mouseWentDown() signal, then a subsequent
   * mouseWentUp() will be received by the same widget, even if mouse is
   * no longer over the original widget.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& mouseWentUp();

  /*! \brief Event signal emitted when the mouse went out of this widget.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& mouseWentOut();
    
  /*! \brief Event signal emitted when the mouse entered this widget.
   *
   * The signal is emitted as soon as the mouse enters the widget, or
   * after some delay as configured by setMouseOverDelay()
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& mouseWentOver();

  /*! \brief Event signal emitted when the mouse moved over this widget.
   *
   * The mouse event contains information on the button(s) currently
   * pressed. If multiple buttons are currently pressed, only the
   * button with smallest enum value is returned.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& mouseMoved();

  /*! \brief Event signal emitted when the mouse is dragged over this widget.
   *
   * The mouse event contains information on the button(s) currently
   * pressed. If multiple buttons are currently pressed, only the
   * button with smallest enum value is returned.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& mouseDragged();

  /*! \brief Event signal emitted when the mouse scroll wheel was used.
   *
   * The event details contains information such as the \link
   * WMouseEvent::wheelDelta() wheel delta\endlink, optional \link
   * WMouseEvent::modifiers() keyboard modifiers\endlink, and mouse
   * coordinates relative to the \link WMouseEvent::widget()
   * widget\endlink, the window \link WMouseEvent::window()
   * window\endlink, or the \link WMouseEvent::document()
   * document\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& mouseWheel();

  /*! \brief Event signal emitted when a finger is placed on the screen.
   *
   * The event details contains information such as the \link
   * WTouchEvent::touches() touches\endlink, \link WTouchEvent::targetTouches()
   * target touches\endlink and \link WTouchEvent::changedTouches() changed
   * touches\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WTouchEvent>& touchStarted();
  
  /*! \brief Event signal emitted when a finger is removed from the screen.
   *
   * The event details contains information such as the \link
   * WTouchEvent::touches() touches\endlink, \link WTouchEvent::targetTouches()
   * target touches\endlink and \link WTouchEvent::changedTouches() changed
   * touches\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WTouchEvent>& touchEnded();

  /*! \brief Event signal emitted when a finger, which is already placed on the
   *         screen, is moved across the screen.
   *
   * The event details contains information such as the \link
   * WTouchEvent::touches() touches\endlink, \link WTouchEvent::targetTouches()
   * target touches\endlink and \link WTouchEvent::changedTouches() changed
   * touches\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WTouchEvent>& touchMoved();
  
  /*! \brief Event signal emitted when a gesture is started.
   *
   * The event details contains information about the \link
   * WGestureEvent::scale() scale\endlink and the \link 
   * WGestureEvent::rotation() rotation\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WGestureEvent>& gestureStarted();

  /*! \brief Event signal emitted when a gesture is changed.
   *
   * The event details contains information about the \link
   * WGestureEvent::scale() scale\endlink and the \link 
   * WGestureEvent::rotation() rotation\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WGestureEvent>& gestureChanged();

  /*! \brief Event signal emitted when a gesture is ended.
   *
   * The event details contains information about the \link
   * WGestureEvent::scale() scale\endlink and the \link 
   * WGestureEvent::rotation() rotation\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WGestureEvent>& gestureEnded();

  /*! \brief Configure dragging for drag and drop.
   *
   * Enable drag&drop for this widget. The mimeType is used to find a
   * suitable drop target, which must accept dropping of this mimetype.
   *
   * By default, the entire widget is dragged. One may specify another
   * widget to be dragged (for example the parent as \p dragWidget) or
   * a \p dragWidget whose function is only to represent the drag
   * visually (when \p isDragWidgetOnly = \c true).
   *
   * The widget to be identified as source in the dropEvent may be given
   * explicitly, and will default to this widget otherwise.
   *
   * When using a touch interface, the widget can also be dragged after
   * a long press.
   *
   * \note When JavaScript is disabled, drag&drop does not work.
   *
   * \sa WWidget::dropEvent(), WWidget::acceptDrops(), WDropEvent
   */
  void setDraggable(const std::string& mimeType,
		    WWidget *dragWidget = nullptr,
		    bool isDragWidgetOnly = false,
		    WObject *sourceWidget = nullptr);

  /*! \brief Disable drag & drop for this widget.
   *
   * \sa setDraggable()
   */
  void unsetDraggable();

  /*! \brief Sets a delay for the mouse over event.
   *
   * This sets a delay (in milliseconds) before the mouse over event
   * is emitted.
   *
   * The default value is 0.
   *
   * \sa mouseWentOver()
   */
  void setMouseOverDelay(int delay);

  /*! \brief Returns the mouse over signal delay.
   *
   * \sa setMouseOverDelay()
   */
  int mouseOverDelay() const;

  virtual void setPopup(bool popup) override;
  virtual void load() override;
  virtual bool isEnabled() const override;

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void propagateRenderOk(bool deep) override;
  virtual void propagateSetEnabled(bool enabled) override;

  void updateEventSignals(DomElement& element, bool all);

  std::unique_ptr<JSlot> dragSlot_;
  std::unique_ptr<JSlot> dragTouchSlot_;
  std::unique_ptr<JSlot> dragTouchEndSlot_;

protected:
  // also used in WAbstractToggleButton
  static const char *M_CLICK_SIGNAL;

private:
  static const char *CLICK_SIGNAL;
  static const char *KEYDOWN_SIGNAL;
  static const char *KEYPRESS_SIGNAL;
  static const char *KEYUP_SIGNAL;
  static const char *ENTER_PRESS_SIGNAL;
  static const char *ESCAPE_PRESS_SIGNAL;
  static const char *DBL_CLICK_SIGNAL;
  static const char *MOUSE_DOWN_SIGNAL;
  static const char *MOUSE_UP_SIGNAL;
  static const char *MOUSE_OUT_SIGNAL;
  static const char *MOUSE_OVER_SIGNAL;
  static const char *MOUSE_MOVE_SIGNAL;
  static const char *MOUSE_DRAG_SIGNAL;
  static const char *MOUSE_WHEEL_SIGNAL;
  static const char *WHEEL_SIGNAL;
  static const char *TOUCH_START_SIGNAL;
  static const char *TOUCH_MOVE_SIGNAL;
  static const char *TOUCH_END_SIGNAL;
  static const char *GESTURE_START_SIGNAL;
  static const char *GESTURE_CHANGE_SIGNAL;
  static const char *GESTURE_END_SIGNAL;
  static const char *DRAGSTART_SIGNAL;

  friend class DomElement;
  friend class WAbstractToggleButton;
  friend class WWebWidget;

  int mouseOverDelay_;
};

}

#endif // WINTERACT_WIDGET_H_
