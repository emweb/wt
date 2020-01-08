// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACT_AREA_H_
#define WABSTRACT_AREA_H_

#include <Wt/WAnchor.h> // for AnchorTarget

namespace Wt {

  namespace Impl {
    class AreaWidget;
    class MapWidget;
  }

/*! \class WAbstractArea Wt/WAbstractArea.h Wt/WAbstractArea.h
 *  \brief Abstract base class for interactive areas in a widget.
 *
 * Use an %WAbstractArea (or rather, one of its concrete
 * implementations), to define interactivity that applies on a part of
 * a WImage or WPaintedWidget. The area may be defined using different
 * shapes through WRectArea, WCircleArea or WPolygonArea.
 *
 * \sa WImage::addArea(), WPaintedWidget::addArea()
 */
class WT_API WAbstractArea : public WObject
{
public:
  /*! \brief Destructor.
   *
   * The area is automatically removed from the WImage or
   * WPaintedWidget to which it was added.
   *
   * \sa WImage::removeArea(), WPaintedWidget::removeArea()
   */
  virtual ~WAbstractArea();

  /*! \brief Specifies that this area specifies a hole for another area.
   *
   * When set to \c true, this area will define an area that does not
   * provide interactivity. When it preceeds other, overlapping,
   * areas, it acts as if it cuts a hole in those areas.
   *
   * The default value is \c false.
   *
   * \sa isHole()
   */
  void setHole(bool hole);

  /*! \brief Returns whether this area specifies a hole.
   *
   * \sa setHole()
   */
  bool isHole() const { return hole_; }

  void setTransformable(bool transformable);
  bool isTransformable() const { return transformable_; }

  /*! \brief Sets a link.
   *
   * By setting a link, the area behaves like a WAnchor.
   *
   * By default, no destination link is set.
   *
   * \note Even when no destination link is set, in some circumstances,
   *       an identity URL ('#') will be linked to on the underlying HTML
   *       &lt;area&gt; element (see also setCursor()).
   */
  void setLink(const WLink& link);

  /*! \brief Returns the link.
   *
   * \sa setLink()
   */
  WLink link() const;

  /*! \brief Sets an alternate text.
   *
   * The alternate text should provide a fallback for browsers that do
   * not display an image. If no sensible fallback text can be
   * provided, an empty text is preferred over nonsense.
   *
   * This should not be confused with toolTip() text, which provides
   * additional information that is displayed when the mouse hovers
   * over the area.
   *
   * The default alternate text is an empty text ("").
   *
   * \sa alternateText()
   */
  void setAlternateText(const WString& text);

  /*! \brief Returns the alternate text.
   *
   * \sa setAlternateText()
   */
  const WString alternateText() const;

  /*! \brief Sets the tooltip.
   *
   * The tooltip is displayed when the cursor hovers over the area.
   */
  void setToolTip(const WString& text);

  /*! \brief Returns the tooltip text.
   *
   * \sa setToolTip()
   */
  WString toolTip() const;

  /*! \brief Defines a style class.
   *
   * \note Only few CSS declarations are known to affect the look of a
   *       image area, the most notable one being the 'cursor'. Other things
   *       will simply be ignored.
   */
  void setStyleClass(const WT_USTRING& styleClass);
  void setStyleClass(const char *styleClass);

  /*! \brief Returns the style class.
   *
   * \sa setStyleClass()
   */
  WT_USTRING styleClass() const;

  /*! \brief Adds a style class.
   *
   * \note Only few CSS declarations are known to affect the look of a
   *       image area, the most notable one being the 'cursor'. Other things
   *       will simply be ignored.
   */
  void addStyleClass(const WT_USTRING& styleClass, bool force = false);

  /*! \brief Removes a style class.
   */
  void removeStyleClass(const WT_USTRING& styleClass, bool force = false);

  /*! \brief Sets the cursor.
   *
   * This sets the mouse cursor that is shown when the mouse pointer
   * is over the area. Most browsers only support Cursor::PointingHand,
   * which is activated by a non-empty ref.
   *
   * \sa setRef()
   */
  void setCursor(Cursor cursor);

  /*! \brief Sets a custom cursor image URL.
   *
   * The URL should point to a .cur file. For optimal portability, make
   * sure that the .cur file is proparly constructed. A renamed .ico file
   * will not work on Internet Explorer.
   */
  void setCursor(std::string cursorImage,
		 Cursor fallback = Cursor::Arrow);

  /*! \brief Returns the cursor.
   *
   * \sa setCursor()
   */
  Cursor cursor() const;

  WImage *image() const;

public:
  /*! \brief Event signal emitted when a keyboard key is pushed down.
   *
   * The keyWentDown signal is the first signal emitted when a key is
   * pressed (before the keyPressed() signal). Unlike keyPressed()
   * however it is also emitted for modifier keys (such as "shift",
   * "control", ...) or keyboard navigation keys that do not have a
   * corresponding character.
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
   * \sa keyWentDown()
   */
  EventSignal<WKeyEvent>& keyPressed();
    
  /*! \brief Event signal emitted when a keyboard key is released.
   *
   * This is the counter-part of the keyWentDown() event. Every
   * key-down has its corresponding key-up.
   *
   * \sa keyWentDown()
   */
  EventSignal<WKeyEvent>& keyWentUp();

  /*! \brief Event signal emitted when enter was pressed.
   *
   * This signal is emitted when the Enter or Return key was pressed.
   *
   * \sa keyPressed(), Key::Enter
   */
  EventSignal<>& enterPressed();

  /*! \brief Event signal emitted when escape was pressed.
   *
   * This signal is emitted when the Escape key was pressed.
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
   * \note When JavaScript is disabled, the signal will never fire.
   */
  EventSignal<WMouseEvent>& mouseWentOver();

  /*! \brief Event signal emitted when the mouse moved over this widget.
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

protected:
  WAbstractArea();

  virtual bool updateDom(DomElement& element, bool all);
  virtual std::string updateAreaCoordsJS() = 0;
  void repaint();
  std::string jsRef() const;

private:
  struct AnchorImpl {
    AnchorImpl(){}
    WAnchor::LinkState linkState;
    WString altText;
  };

  std::unique_ptr<Impl::AreaWidget> uWidget_;
  Impl::AreaWidget *widget_;

  bool hole_;
  bool transformable_;
  std::unique_ptr<AnchorImpl> anchor_;
  observing_ptr<WImage> image_;

  void createAnchorImpl();
  void resourceChanged();

  WInteractWidget *widget();

  void setImage(WImage *image);
  std::unique_ptr<WWidget> takeWidget();
  void returnWidget(std::unique_ptr<WWidget> w);

  friend class WImage;
  friend class Impl::AreaWidget;
  friend class Impl::MapWidget;
  friend class WPaintedWidget;
};

}

#endif // WABSTRACT_AREA_H_
