// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEVENT_H_
#define WEVENT_H_

#include <Wt/WGlobal.h>
#include <string>
#include <vector>

namespace Wt {

  /*! \brief A coordinate.
   */
  struct Coordinates {
    int x; //!< X coordinate
    int y; //!< Y coordinate

    operator WPointF() const;

    /*! \brief Constructor.
     */
    Coordinates(int X, int Y) : x(X), y(Y) { }
  };

  /*! \brief A single finger touch of a touch event.
   *
   * \sa WTouchEvent
   */
  class WT_API Touch {
  public:
    /*! \brief Constructor.
     */
    Touch(long long identifier,
	  int clientX, int clientY,
	  int documentX, int documentY,
	  int screenX, int screenY,
	  int widgetX, int widgetY);

    /*! \brief Returns the touch position relative to the document.
     */
    Coordinates document() const { return Coordinates(documentX_, documentY_); }
    
    /*! \brief Returns the touch position relative to the window.
     *
     * This differs from document() only when scrolling through
     * the document.
     */
    Coordinates window() const { return Coordinates(clientX_, clientY_); }

    /*! \brief Returns the touch position relative to the screen.
     */
    Coordinates screen() const { return Coordinates(screenX_, screenY_); }

    /*! \brief Returns the touch position relative to the widget.
     */
    Coordinates widget() const { return Coordinates(widgetX_, widgetY_); }

    /*! \brief Returns the identifier for this touch.
     */
    long long identifier() const { return identifier_; }

  private:
    int clientX_, clientY_;
    int documentX_, documentY_;
    int screenX_, screenY_;
    int widgetX_, widgetY_;
    long long identifier_;
  };

class WebRequest;
class WObject;
class WString;

template<class E> class EventSignal;

class WT_API JavaScriptEvent {
public:
  // for mouse events:
  int clientX, clientY;
  int documentX, documentY;
  int screenX, screenY;
  int widgetX, widgetY;
  int dragDX, dragDY;
  int wheelDelta; 

  // for key events or mouse event modifiers
  int button;
  int keyCode, charCode;
  WFlags<KeyboardModifier> modifiers;

  // for touch events
  std::vector<Touch> touches, targetTouches, changedTouches;

  // for gesture events
  double scale, rotation;

  // for scroll events
  int  scrollX, scrollY, viewportWidth, viewportHeight;

  // event type
  std::string type;

  // target id
  std::string tid;

  std::string response;

  std::vector<std::string> userEventArgs;

  void get(const WebRequest& request, const std::string& se);

  JavaScriptEvent();
};

/*! \class WEvent Wt/WEvent.h Wt/WEvent.h
 *  \brief An application event.
 *
 * The application is notified of an event (like a user interaction, a
 * sesion timeout, an internal keep-alive event, or other event) using
 * WApplication::notify().
 *
 * You can check for a particular event type using eventType().
 */
class WT_API WEvent {
public:
  struct Impl;

  /*! \brief Returns the event type.
   */
  EventType eventType() const; 

private:
  WEvent(const Impl& impl)
    : impl_(impl)
  { }

  const Impl& impl_;

  friend class WebSession;
  friend class WebController;
};

#ifdef WT_TARGET_JAVA
/*! \class WAbstractEvent Wt/WEvent.h Wt/WEvent.h
 *  \brief Internal class WAbstractEvent.
 */
class WAbstractEvent
{
public:
  virtual WAbstractEvent *createFromJSEvent(const JavaScriptEvent& jsEvent) = 0;
};
#endif // WT_TARGET_JAVA


/*! \brief Enumeration for the mouse button.
 */
enum class MouseButton {
  None = 0,      //!< No button
  Left = 1,    //!< Left button
  Middle = 2,  //!< Middle button
  Right = 4    //!< Right button
};

/*! \class WMouseEvent Wt/WEvent.h Wt/WEvent.h
 *  \brief A class providing details for a mouse event.
 *
 * \sa WInteractWidget::clicked(), WInteractWidget::doubleClicked(),
 *     WInteractWidget::mouseWentDown(), WInteractWidget::mouseWentUp(),
 *     WInteractWidget::mouseWentOver(), WInteractWidget::mouseMoved()
 *
 * \ingroup signalslot
 */
class WT_API WMouseEvent
#ifdef WT_TARGET_JAVA
                         : public WAbstractEvent
#endif // WT_TARGET_JAVA
{
public:
  /*! \brief Typedef for enum Wt::MouseButton */
  typedef MouseButton Button;

  /*! \brief Default constructor
   */
  WMouseEvent();

  /*! \brief Returns the button.
   *
   * If multiple buttons are currently pressed for a mouse moved or
   * mouse dragged event, then the one with the smallest numerical value
   * is returned.
   */
  MouseButton button() const;

  /*! \brief Returns keyboard modifiers.
   *
   * The result is a logical OR of \link Wt::KeyboardModifier
   * KeyboardModifier\endlink flags.
   */
  WFlags<KeyboardModifier> modifiers() const { return jsEvent_.modifiers; }

  /*! \brief Returns the mouse position relative to the document.
   */
  Coordinates document() const
  { return Coordinates(jsEvent_.documentX, jsEvent_.documentY); }

  /*! \brief Returns the mouse position relative to the window.
   *
   * This differs from documentX() only through scrolling
   * through the document.
   */
  Coordinates window() const
  { return Coordinates(jsEvent_.clientX, jsEvent_.clientY); }

  /*! \brief Returns the mouse position relative to the screen.
   */
  Coordinates screen() const
  { return Coordinates(jsEvent_.screenX, jsEvent_.screenY); }

  /*! \brief Returns the mouse position relative to the widget.
   */
  Coordinates widget() const
  { return Coordinates(jsEvent_.widgetX, jsEvent_.widgetY); }

  /*! \brief Returns the distance over which the mouse has been dragged.
   *
   * This is only defined for a WInteractWidget::mouseWentUp() event.
   */
  Coordinates dragDelta() const
  { return Coordinates(jsEvent_.dragDX, jsEvent_.dragDY); }

  /*! \brief Returns the scroll wheel delta.
   *
   * This is 1 when wheel was scrolled up or -1 when wheel was scrolled down.
   *
   * This is only defined for a WInteractWidget::mouseWheel() event.
   */
  int wheelDelta() const { return jsEvent_.wheelDelta; }

#ifdef WT_TARGET_JAVA
  virtual WAbstractEvent *createFromJSEvent(const JavaScriptEvent& jsEvent)
  {
    return new WMouseEvent(jsEvent);
  }

  static WMouseEvent templateEvent;
#endif // WT_TARGET_JAVA

  WMouseEvent(const JavaScriptEvent& jsEvent);

protected:
  JavaScriptEvent jsEvent_;
};

/*! \class WKeyEvent Wt/WEvent.h Wt/WEvent.h
 *  \brief A class providing details for a keyboard event.
 *
 * A key event is associated with the WInteractWidget::keyWentDown(),
 * WInteractWidget::keyWentUp() and WInteractWidget::keyPressed()
 * signals.
 *
 * \ingroup signalslot
 */
class WT_API WKeyEvent
#ifdef WT_TARGET_JAVA
                         : public WAbstractEvent
#endif // WT_TARGET_JAVA
{
public:
  /*! \brief Default constructor
   */
  WKeyEvent();

  /*! \brief Returns the key code key that was pressed or released.
   *
   * The key code corresponds to the actual key on the keyboard,
   * rather than the generated character.
   *
   * All three types of key events provide this information.
   *
   * \sa modifiers(), charCode()
   */
  Key key() const;

  /*! \brief Returns keyboard modifiers.
   *
   * The result is a logical OR of \link Wt::KeyboardModifier
   * KeyboardModifier\endlink flags.
   *
   * All three types of key events provide this information.
   *
   * \sa key(), charCode()
   */
  WFlags<KeyboardModifier> modifiers() const { return jsEvent_.modifiers; }

  /*! \brief Returns the unicode character code.
   *
   * This is only defined for a \link WInteractWidget::keyPressed
   * keyPressed \endlink event, and returns the unicode character code point
   * of a character that is entered.
   *
   * For the \link WInteractWidget::keyWentDown keyWentDown \endlink
   * and \link WInteractWidget::keyWentUp keyWentUp \endlink events,
   * '0' is returned.
   *
   * The charCode() may be different from key(). For example, a \link
   * Wt::Key::M Key::M\endlink key may correspond to 'm' or 'M'
   * character, depending on whether the shift key is pressed
   * simultaneously.
   *
   * \sa key(), text()
   */
  int charCode() const;

  /*! \brief The (unicode) text that this key generated.
   *
   * This is only defined for a \link WInteractWidget::keyPressed
   * keyPressed \endlink event, and returns a string that holds
   * exactly one unicode character, which corresponds to charCode().
   *
   * For the \link WInteractWidget::keyWentDown keyWentDown \endlink
   * and \link WInteractWidget::keyWentUp keyWentUp \endlink events,
   * an empty string is returned.
   *
   * \sa charCode()
   */
  WT_USTRING text() const;

#ifdef WT_TARGET_JAVA
  virtual WAbstractEvent *createFromJSEvent(const JavaScriptEvent& jsEvent)
  {
    return new WKeyEvent(jsEvent);
  }

  static WKeyEvent templateEvent;
#endif // WT_TARGET_JAVA

  WKeyEvent(const JavaScriptEvent& jsEvent);

private:
  JavaScriptEvent jsEvent_;
};

/*! \class WDropEvent Wt/WEvent.h Wt/WEvent.h
 *  \brief A class providing details for a drop event.
 *
 * \sa WWidget::dropEvent(WDropEvent)
 *
 * \ingroup signalslot
 */
class WT_API WDropEvent
{
public:
  /*! \brief The type of the original event.
   */
  enum class OriginalEventType {
    Mouse, //!< The original event was a WMouseEvent
    Touch //!< The original event was a WTouchEvent
  };

  /*! \brief Constructor.
   */
  WDropEvent(WObject *source, const std::string& mimeType,
	     const WMouseEvent& mouseEvent);

  /*! \brief Constructor.
   */
  WDropEvent(WObject *source, const std::string& mimeType,
	     const WTouchEvent& touchEvent);

#ifndef WT_TARGET_JAVA
  WDropEvent(const WDropEvent &other);
  WDropEvent &operator=(const WDropEvent &other);
#endif

  /*! \brief Returns the source of the drag&drop operation.
   *
   * The source is the widget that was set draggable using
   * WInteractWidget::setDraggable().
   */
  WObject *source() const { return dropSource_; }

  /*! \brief Returns the mime type of this drop event.
   */
  const std::string& mimeType() const { return dropMimeType_; }

  /*! \brief Returns the original mouse event.
   *
   * If eventType() == OriginalEventType::Mouse, this returns the original mouse event,
   * otherwise this returns null.
   */
  const WMouseEvent *mouseEvent() const { return mouseEvent_.get(); }

  /*! \brief Returns the original touch event.
   *
   * If eventType() == OriginalEventType::Touch, this returns the original touch event,
   * otherwise this returns null.
   */
  const WTouchEvent *touchEvent() const { return touchEvent_.get(); }

  /*! \brief Returns the type of the original event.
   */
  OriginalEventType originalEventType() const { return mouseEvent_ ? OriginalEventType::Mouse : OriginalEventType::Touch; }

private:
  WObject            *dropSource_;
  std::string         dropMimeType_;
  std::unique_ptr<const WMouseEvent> mouseEvent_;
  std::unique_ptr<const WTouchEvent> touchEvent_;
};

/*! \class WScrollEvent Wt/WEvent.h Wt/WEvent.h
 *  \brief A class providing details for a scroll event.
 *
 * \sa WContainerWidget::scrolled()
 *
 * \ingroup signalslot
 */
class WT_API WScrollEvent
#ifdef WT_TARGET_JAVA
                         : public WAbstractEvent
#endif // WT_TARGET_JAVA
{
public:
  /*! \brief Default constructor
   */
  WScrollEvent();

  /*! \brief Returns the current horizontal scroll position.
   *
   * \sa scrollY(), viewportWidth()
   */
  int scrollX() const { return jsEvent_.scrollX; }

  /*! \brief Returns the current vertical scroll position.
   *
   * \sa scrollX(), viewportHeight()
   */
  int scrollY() const { return jsEvent_.scrollY; }

  /*! \brief Returns the current horizontal viewport width.
   *
   * Returns the current viewport width.
   *
   * \sa viewportHeight(), scrollX()
   */
  int viewportWidth() const { return jsEvent_.viewportWidth; }

  /*! \brief Returns the current horizontal viewport height.
   *
   * Returns the current viewport height.
   *
   * \sa viewportWidth(), scrollY()
   */
  int viewportHeight() const { return jsEvent_.viewportHeight; }

#ifdef WT_TARGET_JAVA
  virtual WAbstractEvent *createFromJSEvent(const JavaScriptEvent& jsEvent)
  {
    return new WScrollEvent(jsEvent);
  }

  static WScrollEvent templateEvent;
#endif // WT_TARGET_JAVA

private:
  JavaScriptEvent jsEvent_;

  WScrollEvent(const JavaScriptEvent& jsEvent);

  friend class EventSignal<WScrollEvent>;
};

/*! \class WTouchEvent Wt/WEvent.h Wt/WEvent.h
 *  \brief A class providing details for a touch event.
 *
 * \sa WInteractWidget::touchStarted(), WInteractWidget::touchMoved(),
 *     WInteractWidget::touchEnded()
 *
 * \ingroup signalslot
 */
class WT_API WTouchEvent
#ifdef WT_TARGET_JAVA
                         : public WAbstractEvent
#endif // WT_TARGET_JAVA
{
public:
  /*! \brief Default constructor
   */
  WTouchEvent();

  /*! \brief Returns a list of \link Touch\endlink objects for every finger 
   *         currently touching the screen.
   */
  const std::vector<Touch>& touches() const
    { return jsEvent_.touches; }

  /*! \brief Returns a list of \link Touch\endlink objects for finger touches 
   *         that started out within the same widget.
   */
  const std::vector<Touch>& targetTouches() const
    { return jsEvent_.targetTouches; }

  /*! \brief Returns a list of \link Touch\endlink objects for every finger 
   *         involved in the event.
   */
  const std::vector<Touch>& changedTouches() const
    { return jsEvent_.changedTouches; }

#ifdef WT_TARGET_JAVA
  virtual WAbstractEvent *createFromJSEvent(const JavaScriptEvent& jsEvent)
  {
    return new WTouchEvent(jsEvent);
  }

  static WTouchEvent templateEvent;
#endif // WT_TARGET_JAVA

  WTouchEvent(const JavaScriptEvent& jsEvent);

protected:
  JavaScriptEvent jsEvent_;
};

/*! \class WGestureEvent Wt/WEvent.h Wt/WEvent.h
 *  \brief A class providing details for a gesture event.
 *
 * \sa WInteractWidget::gestureStarted(), WInteractWidget::gestureChanged(),
 *     WInteractWidget::gestureEnded()
 *
 * \ingroup signalslot
 */
class WT_API WGestureEvent
#ifdef WT_TARGET_JAVA
                         : public WAbstractEvent
#endif // WT_TARGET_JAVA
{
public:
  /*! \brief Default constructor
   */
  WGestureEvent();

  /*! \brief Returns the multiplier which the user has pinched or pushed 
             (relative to 1).
   */
  double scale() const { return jsEvent_.scale; }

  /*! \brief Returns the number of degrees the user has rotated his/her fingers.
   */
  double rotation() const { return jsEvent_.rotation; }

#ifdef WT_TARGET_JAVA
  virtual WAbstractEvent *createFromJSEvent(const JavaScriptEvent& jsEvent)
  {
    return new WGestureEvent(jsEvent);
  }

  static WGestureEvent templateEvent;
#endif // WT_TARGET_JAVA

  WGestureEvent(const JavaScriptEvent& jsEvent);

protected:
  JavaScriptEvent jsEvent_;
};

}

#endif // WEVENT_H_
