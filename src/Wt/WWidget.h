// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WWIDGET_H_
#define WWIDGET_H_

#include <Wt/WObject.h>
#include <Wt/WGlobal.h>
#include <Wt/WAnimation.h>
#include <Wt/WLength.h>
#include <Wt/WSignal.h>
#include <Wt/WJavaScript.h>

#include <list>
#include <vector>
#include <sstream>

namespace Wt {

enum class RepaintFlag {
  SizeAffected = 0x1,
  ToAjax = 0x2
};

W_DECLARE_OPERATORS_FOR_FLAGS(RepaintFlag)

class WContainerWidget;
class WCssDecorationStyle;
class WDropEvent;
class WMouseEvent;
class WString;
class WWebWidget;
class DomElement;
class WCssTextRule;

/*! \class WWidget Wt/WWidget.h Wt/WWidget.h
 *  \brief The abstract base class for a user-interface component.
 *
 * The user-interface is organized in a tree structure, in which each
 * node is a widget. All widgets, except for the application's root
 * widget and dialogs, have a parent which is usually a
 * WContainerWidget.
 *
 * \if cpp
 *
 * When a widget is deleted, it is also visually removed from the
 * user-interface and all children are deleted recursively.
 *
 * \endif
 *
 * This is an abstract base class. Implementations derive either from
 * the abstract WWebWidget (for basic widgets with a direct HTML
 * counter-part) or from the abstract WCompositeWidget (for anything
 * else). To add a %WWebWidget directly to a parent container, either
 * specify the parent in the constructor (which is conventionally the
 * last constructor argument), or add the widget to the parent using
 * WContainerWidget::addWidget(). Alternatively, you may add the widget
 * to a layout manager set for a %WContainerWidget.
 *
 * A widget provides methods to manage its decorative style base on
 * CSS. It also provides access to CSS-based layout, which you may not
 * use when the widget is not inserted into a layout manager.
 */
class WT_API WWidget : public WObject
{
public:
  /*! \brief Destructor.
   *
   * Deletes a widget and all contained contents.
   *
   * \sa WWidget::removeWidget()
   */
  virtual ~WWidget();

  /*! \brief Returns the parent widget.
   */
  WWidget *parent() const { return parent_; }

  /*! \brief Returns child widgets.
   *
   * This returns widgets for which widget->parent() == this.
   */
  virtual std::vector<WWidget *> children() const = 0;

  /*! \brief Removes a child widget.
   */
  virtual std::unique_ptr<WWidget> removeWidget(WWidget *widget);

  /*! \brief Removes the widget from its parent.
   *
   * This is equivalent to parent()->removeWidget(this);
   */
  std::unique_ptr<WWidget> removeFromParent();

  /*! \brief Sets the CSS position scheme.
   *
   * Establishes how the widget must be layed-out relative to its
   * siblings. The default position scheme is PositionScheme::Static.
   *
   * This applies to CSS-based layout.
   *
   * \sa Wt::PositionScheme, positionScheme()
   */
  virtual void setPositionScheme(PositionScheme scheme) = 0;

  /*! \brief Returns the CSS position scheme.
   *
   * This applies to CSS-based layout.
   *
   * \sa Wt::PositionScheme, setPositionScheme(PositionScheme)
   */
  virtual PositionScheme positionScheme() const = 0;

  /*! \brief Sets CSS offsets for a non-statically positioned widget.
   *
   * The argument \p sides may be a combination of Wt::Side::Left,
   * Wt::Side::Right, Wt::Side::Top, and Wt::Side::Bottom.
   *
   * This applies only to widgets that have a position scheme that is
   * Wt::PositionScheme::Relative, Wt::PositionScheme::Absolute, or Wt::PositionScheme::Fixed, and has a slightly
   * different meaning for these three cases.
   *
   * For a \link Wt::PositionScheme::Relative relatively positioned\endlink widget, an
   * offset applies relative to the position the widget would have
   * when layed-out using a \link Wt::PositionScheme::Static static\endlink position
   * scheme. The widget may be shifted to the left or right by
   * specifying an offset for the \link Wt::Side::Left left\endlink or \link
   * Wt::Side::Right right\endlink) side. The widget may be shifted
   * vertically, by specifying an offset for the \link Wt::AlignmentFlag::Top
   * top\endlink or \link Wt::Side::Bottom bottom\endlink side.
   *
   * For an \link Wt::PositionScheme::Absolute absolutely positioned\endlink widget,
   * an offset specifies a distance of the corresponding side of the
   * widget with respect to the corresponding side of the reference
   * parent widget. Thus, setting all offsets to 0 result in a widget
   * that spans the entire reference widget. The reference parent
   * widget is the first ancestor widget that is a table cell, or a
   * widget with a relative, absolute or fixed position scheme.
   *
   * For an \link Wt::PositionScheme::Fixed fixed positioned\endlink widget, an offset
   * specifies a distance of the corresponding side of the widget with
   * respect to the browser window, regardless of scrolling. Thus,
   * setting all offsets to 0 result in a widget that spans the entire
   * browser window.
   *
   * This applies to CSS-based layout.
   *
   * \sa offset(Side) const
   */
  virtual void setOffsets(const WLength& offset,
			  WFlags<Side> sides = AllSides) = 0;

#ifdef WT_TARGET_JAVA
  /*! \brief Sets CSS offsets for a non-statically positioned widget.
   *
   * This is a convenience method for applying offsets in pixel units.
   *
   * \sa setOffsets(const WLength&, WFlags<Side>)
   */
  void setOffsets(int pixels, WFlags<Side> sides = AllSides);
#endif // WT_TARGET_JAVA

  /*! \brief Returns a CSS offset.
   *
   * This applies to CSS-based layout.
   *
   * \sa setOffsets(const WLength&, WFlags<Side>)
   */
  virtual WLength offset(Side side) const = 0;

  /*! \brief Resizes the widget.
   *
   * Specifies a fixed size for this widget, setting CSS
   * <tt>width</tt> and <tt>height</tt> properties. By default a
   * widget has automatic width and height, which sets a size for the
   * widget following CSS rules.
   *
   * When the widget is not managed by a layout manager, the automatic
   * (natural) size of a widget depends on whether they widget is a
   * <i>block</i> or <i>inline</i> widget:
   * - a <i>block</i> widget takes by default the width of the parent, and the height
   *   that it needs based on its contents
   * - an <i>inline</i> widget takes the width and height that it needs based on its
   *   contents (possibly wrapping over multiple lines). The width and height of
   *   an inline widget cannot be changed (by the letter of CSS, although most
   *   browsers will react to it in varying ways).
   *
   * When inserted in a layout manager, the size set will be used as a
   * widget's preferred size, but the widget may be given a different
   * size by the layout manager based on available space and stretch
   * factors. The actual size given by a layout manager may be
   * retrieved by making the widget "layout size aware", using
   * setLayoutSizeAware(). If you have defined a <tt>"wtResize()"</tt>
   * JavaScript method for the widget, then this method will also be
   * called.
   *
   * The default width and height of a widget is WLength::Auto.
   *
   * \sa width(), height()
   */
  virtual void resize(const WLength& width, const WLength& height);

#ifdef WT_TARGET_JAVA
  /*! \brief Resizes the widget.
   *
   * This is a convenience method for resizing a widget using pixel units.
   *
   * \sa resize(const WLength&, const WLength&)
   */
  void resize(int widthPixels, int heightPixels);
#endif // WT_TARGET_JAVA

  /*! \brief Returns the width.
   *
   * Returns the width set for this widget. This is not a calculated
   * width, based on layout, but the width as specified with
   * resize(const WLength&, const WLength&).
   *
   * This applies to CSS-based layout.
   *
   * \sa resize(const WLength&, const WLength&), height()
   */
  virtual WLength width() const = 0;

  /*! \brief Sets the width.
   *
   * This is a convenience method to change only the width of a widget, and
   * is implemented as:
   * \code
   * resize(width, height())
   * \endcode
   *
   * \sa resize(), setHeight()
   */
  void setWidth(const WLength& width);

  /*! \brief Returns the height.
   *
   * Returns the height set for this widget. This is not a calculated
   * height, based on layout, but the height as specified previously
   * with resize(const WLength& width, const WLength& height).
   *
   * This applies to CSS-based layout.
   *
   * \sa resize(const WLength&, const WLength&), width()
   */
  virtual WLength height() const = 0;

  /*! \brief Sets the height.
   *
   * This is a convenience method to change only the height of a widget, and
   * is implemented as:
   * \code
   * resize(width(), height)
   * \endcode
   *
   * This applies to CSS-based layout.
   *
   * \sa resize(), setWidth()
   */
  void setHeight(const WLength& height);

  /*! \brief Sets a minimum size.
   *
   * Specifies a minimum size for this widget, setting CSS <tt>min-width</tt>
   * and <tt>min-height</tt> properties.
   *
   * The default minimum width and height is 0. The special value
   * WLength::Auto indicates that the initial width is used as minimum
   * size. A LengthUnit::Percentage size should not be used, as this is
   * (in virtually all cases) undefined behaviour.
   *
   * When the widget is inserted in a layout manager, then the minimum size
   * will be taken into account.
   *
   * \sa resize(), minimumWidth(), minimumHeight()
   */
  virtual void setMinimumSize(const WLength& width, const WLength& height) = 0;

  /*! \brief Returns the minimum width.
   *
   * Returns the minimum width set for this widget with setMinimumSize().
   *
   * \sa setMinimumSize(), minimumHeight()
   */
  virtual WLength minimumWidth() const = 0;

  /*! \brief Returns the minimum height.
   *
   * Returns the minmum height set for this widget with setMinimumSize().
   *
   * \sa setMinimumSize(), minimumWidth()
   */
  virtual WLength minimumHeight() const = 0;

  /*! \brief Sets a maximum size.
   *
   * Specifies a maximum size for this widget, setting CSS <tt>max-width</tt>
   * and <tt>max-height</tt> properties.
   *
   * The default the maximum width and height are WLength::Auto,
   * indicating no maximum size. A LengthUnit::Percentage size should not
   * be used, as this is (in virtually all cases) undefined behaviour.
   *
   * When the widget is a container widget that contains a layout
   * manager, then setting a maximum size will have the effect of
   * letting the size of the container to reflect the preferred size
   * of the contents (rather than constraining the size of the
   * children based on the size of the container), up to the specified
   * maximum size.
   *
   * \sa resize(), setMinimumSize()
   */
  virtual void setMaximumSize(const WLength& width, const WLength& height) = 0;

  /*! \brief Returns the maximum width.
   *
   * Returns the maximum width set for this widget with setMaximumSize().
   *
   * \sa setMaximumSize(), maximumHeight()
   */
  virtual WLength maximumWidth() const = 0;

  /*! \brief Returns the maximum height.
   *
   * Returns the minmum height set for this widget with setMaximumSize().
   *
   * \sa setMaximumSize(), maximumWidth()
   */
  virtual WLength maximumHeight() const = 0;

  /*! \brief Positions a widget next to another widget.
   *
   * Positions this absolutely positioned widget next to another \p
   * widget. Both widgets must be visible (including all their
   * ancestors). The current widget is shown automatically if needed.
   *
   * When \p orientation = Wt::Orientation::Vertical, the widget is displayed below the
   * other widget (or above in case there is not enough room
   * below). It is aligned so that the left edges align (or the right
   * edges if there is not enough room to the right).
   *
   * Conversely, when \p orientation = Wt::Orientation::Horizontal, the widget is
   * displayed to the right of the other widget (or to the left in
   * case there is not enough room to the right). It is aligned so
   * that the top edges align (or the bottom edges if there is not
   * enough room below).
   *
   * \note This only works if JavaScript is available.
   */
  virtual void positionAt(const WWidget *widget,
			  Orientation orientation = Orientation::Vertical);

  /*! \brief Sets the CSS line height for contained text.
   */
  virtual void setLineHeight(const WLength& height) = 0;

  /*! \brief Returns the CSS line height for contained text.
   *
   * sa setLineHeight()
   */
  virtual WLength lineHeight() const = 0;

  /*! \brief Specifies a CSS float side.
   *
   * This only applies to widgets with a Wt::PositionScheme::Static positionScheme().
   *
   * This lets the widget float to one of the sides of the parent
   * widget, at the current line. A typical use is to position images
   * within text. Valid values for Side or \if cpp Wt::None \elseif
   * java {javadoclink Side#None None} \endif, Wt::Side::Left or Wt::Side::Right.
   *
   * This applies to CSS-based layout.
   */
  virtual void setFloatSide(Side s) = 0;

  /*! \brief Returns the CSS float side.
   *
   * \sa setFloatSide(Side)
   */
  virtual Side floatSide() const = 0;

  /*! \brief Sets the sides that should be cleared of floats.
   *
   * This pushes the widget down until it is not surrounded by floats
   * at the \p sides (which may be a combination of Wt::Side::Left and
   * Wt::Side::Right).
   * 
   * This applies to CSS-based layout.
   *
   * \sa setFloatSide()
   */
  virtual void setClearSides(WFlags<Side> sides) = 0;

  /*! \brief Returns the sides that should remain empty.
   *
   * \sa setClearSides(WFlags<Side>)
   */
  virtual WFlags<Side> clearSides() const = 0;

  /*! \brief Sets CSS margins around the widget.
   *
   * Setting margin has the effect of adding a distance between the widget
   * and surrounding widgets. The default margin (with an automatic length)
   * is zero.
   *
   * Use any combination of Wt::Side::Left, Wt::Side::Right, Wt::Side::Bottom, or Wt::Side::Top.
   *
   * This applies to CSS-based layout.
   *
   * \sa margin()
   */
  virtual void setMargin(const WLength& margin,
			 WFlags<Side> sides = AllSides) = 0;

#ifdef WT_TARGET_JAVA
  /*! \brief Sets CSS margins around the widget.
   *
   * This is a convenience method for setting margins in pixel units.
   *
   * \sa setMargin(const WLength&, WFlags<Side>)
   */
  virtual void setMargin(int pixels,
			 WFlags<Side> sides = AllSides);
#endif // WT_TARGET_JAVA

  /*! \brief Returns a CSS margin set.
   *
   * This applies to CSS-based layout.
   *
   * \sa setMargin()
   */
  virtual WLength margin(Side side) const = 0;

  /*! \brief Sets whether the widget keeps its geometry when hidden.
   *
   * Normally, a widget that is hidden will no longer occupy space,
   * causing a reflow of sibling widgets. Using this method you may
   * change this behavior to keep an (open) space when hidden.
   *
   * \note Currently you can only set this before initial rendering.
   *
   * \sa setHidden()
   */
  virtual void setHiddenKeepsGeometry(bool enabled) = 0;

  /*! \brief Returns whether the widget keeps its geometry when hidden.
   *
   * \sa setHiddenKeepsGeometry()
   */
  virtual bool hiddenKeepsGeometry() const = 0;

  /*! \brief Hides or shows the widget.
   *
   * Hides or show the widget (including all its descendant widgets).
   * When setting \p hidden = \c false, this widget and all descendant
   * widgets that are not hidden will be shown. A widget is only
   * visible if it and all its ancestors in the widget tree are
   * visible, which may be checked using isVisible().
   *
   * \note hide() and show() are considered to be stateless slots by default.
   * If you override setHidden() and need to modify server state whenever it is called,
   * you'll need to call WObject::isNotStateless().
   */
  virtual void setHidden(bool hidden,
			 const WAnimation& animation = WAnimation()) = 0;

  /*! \brief Returns whether the widget is set hidden.
   *
   * A widget that is not hidden may still be not visible when one of
   * its ancestor widgets is hidden. Use isVisible() to check the
   * visibility of a widget.
   *
   * \sa setHidden(), isVisible()
   */
  virtual bool isHidden() const = 0;

  /*! \brief Returns whether the widget is visible.
   *
   * A widget is visible if it is not hidden, and none of its
   * ancestors are hidden. This method returns the true visibility,
   * while isHidden() returns whether a widget has been explicitly
   * hidden.
   *
   * Note that a widget may be at the same time not hidden, and not
   * visible, in case one of its ancestors was hidden.
   *
   * \sa isHidden()
   */
  virtual bool isVisible() const = 0;

  /*! \brief Sets whether the widget is disabled.
   *
   * Enables or disables the widget (including all its descendant
   * widgets). setDisabled(false) will enable this widget and all
   * descendant widgets that are not disabled. A widget is only
   * enabled if it and all its ancestors in the widget tree are
   * disabled.
   *
   * Typically, a disabled form widget will not allow changing the
   * value, and disabled widgets will not react to mouse click events.
   *
   * \note enable() and disable() are considered to be stateless slots by default.
   * If you override setDisabled() and need to modify server state whenever it is called,
   * you'll need to call WObject::isNotStateless().
   *
   * \sa disable(), enable()
   */
  virtual void setDisabled(bool disabled) = 0;

  /*! \brief Returns whether the widget is set disabled.
   *
   * A widget that is not set disabled may still be disabled when one
   * of its ancestor widgets is set disabled. Use isEnabled() to find
   * out whether a widget is enabled.
   *
   * \sa setDisabled(), isEnabled()
   */
  virtual bool isDisabled() const = 0;

  /*! \brief Returns whether the widget is enabled.
   *
   * A widget is enabled if it is not disabled, and none of its
   * ancestors are disabled. This method returns whether the widget is
   * rendered as enabled, while isDisabled() returns whether a widget
   * has been explicitly disabled.
   *
   * Note that a widget may be at the same time not enabled, and not
   * disabled, in case one of its ancestors was disabled.
   *
   * \sa isDisabled()
   */
  virtual bool isEnabled() const = 0;

  /*! \brief Lets the widget overlay over other sibling widgets.
   *
   * A widget that isPopup() will be rendered on top of any other
   * sibling widget contained within the same parent (including other
   * popup widgets previously added to the container).
   *
   * This will only have an effect when the widgetis either
   * Wt::PositionScheme::Absolute or Wt::PositionScheme::Fixed positionScheme().
   *
   * This applies to CSS-based layout, and configures the z-index
   * property.
   */
  virtual void setPopup(bool popup) = 0;

  /*! \brief Returns whether the widget is overlayed.
   *
   * This applies to CSS-based layout.
   *
   * \sa setPopup(bool)
   */
  virtual bool isPopup() const = 0;

  /*! \brief Sets whether the widget is displayed inline or as a block.
   *
   * This option changes whether this widget must be rendered in line
   * with sibling widgets wrapping at the right edge of the parent
   * container (like text), or whether this widget must be rendered as
   * a rectangular block that stacks vertically with sibling widgets
   * (unless a CSS float property is applied). Depending on the widget
   * type, the default value is inline (such as for example for WText,
   * or WPushButton), or block (such as for example for a
   * WContainerWidget).
   *
   * This applies to CSS-based layout.
   */
  virtual void setInline(bool inlined) = 0;

  /*! \brief Returns whether the widget is displayed inline or as block.
   *
   * \sa setInline(bool)
   */
  virtual bool isInline() const = 0;

  /*! \brief Sets a CSS decoration style.
   *
   * This copies the style over its current decorationStyle()
   */
  virtual void setDecorationStyle(const WCssDecorationStyle& style) = 0;

  /*! \brief Returns the decoration style of this widget.
   *
   * This groups all decorative aspects of the widget, which do not
   * affect the widget layout (except for the border properties which
   * may behave like extra margin around the widget).
   *
   * When a decoration style has not been previously set, it returns a
   * default decoration style object.
   *
   * \sa setDecorationStyle()
   */
  virtual WCssDecorationStyle& decorationStyle() = 0;
  virtual const WCssDecorationStyle& decorationStyle() const = 0;

  /*! \brief Sets (one or more) CSS style classes.
   *
   * You may set one or more space separated style classes. CSS style
   * class works in conjunction with style sheet, and provides a
   * flexible way to provide many widgets the same markup.
   *
   * Setting an empty string removes the style class(es).
   * 
   * \sa WApplication::styleSheet()
   */
  virtual void setStyleClass(const WT_USTRING& styleClass) = 0;

  void setStyleClass(const char *styleClass);

  /*! \brief Returns the CSS style class.
   *
   * \sa setStyleClass()
   */
  virtual WT_USTRING styleClass() const = 0;

  /*! \brief Adds a CSS style class.
   *
   * When \p force = \c true, a JavaScript call will be used to add
   * the style class to the DOM element (if JavaScript is
   * available). This may be necessary when client-side JavaScript
   * manipulates the same style class.
   *
   * The \p styleClass should be a single class (although multiple
   * classes will work for the common case that the additional style classes
   * are all not yet present on the element.
   */
  virtual void addStyleClass(const WT_USTRING& styleClass,
			     bool force = false) = 0;

  void addStyleClass(const char *styleClass, bool force = false);

  /*! \brief Removes a CSS style class.
   *
   * When \p force = \c true, a JavaScript call will be used to remove
   * the style class from the DOM element (if JavaScript is
   * available). This may be necessary when client-side JavaScript
   * manipulates the same style class.
   *
   * The \p styleClass should be a single class
   */
  virtual void removeStyleClass(const WT_USTRING& styleClass,
				bool force = false) = 0;

  void removeStyleClass(const char *styleClass, bool force = false);

  /*! \brief Toggles a CSS style class.
   */
  virtual void toggleStyleClass(const WT_USTRING& styleClass, bool add,
				bool force = false);

  void toggleStyleClass(const char *styleClass, bool add, bool force = false);

  /*! \brief Returns whether the widget has a style class.
   */
  virtual bool hasStyleClass(const WT_USTRING& styleClass) const = 0;

  /*! \brief Sets the vertical alignment.
   *
   * This only applies to inline widgets, and determines how to position
   * itself on the current line, with respect to sibling inline widgets.
   *
   * This applies to CSS-based layout.
   */
  virtual void setVerticalAlignment(AlignmentFlag alignment,
				    const WLength& length = WLength::Auto) = 0;

  /*! \brief Returns the vertical alignment.
   *
   * This applies to CSS-based layout.
   *
   * \sa setVerticalAlignment()
   */
  virtual AlignmentFlag verticalAlignment() const = 0;

  /*! \brief Returns the fixed vertical alignment that was set.
   *
   * This applies to CSS-based layout.
   *
   * \sa setVerticalAlignment()
   */
  virtual WLength verticalAlignmentLength() const = 0;

  /*! \brief Sets a tooltip.
   *
   * The tooltip is displayed when the cursor hovers over the widget.
   *
   * When \p textFormat is TextFormat::XHTML, the tooltip may contain any valid
   * XHTML snippet. The tooltip will then be rendered using JavaScript.
   *
   * Note: This will set deferred tooltip to false.
   *
   * \sa setDeferredTooltip()
   */
  virtual void setToolTip(const WString& text,
			  TextFormat textFormat = TextFormat::Plain) = 0;

  /*! \brief Returns the tooltip.
   */
  virtual WString toolTip() const = 0;

  /*! \brief Enable deferred tooltip.
   *
   *  You may override toolTip() to read data only when the user hovers over
   *  the widget.
   *
   *  When \p textFormat is TextFormat::XHTML, the tooltip may contain any valid
   *  XHTML snippet. The tooltip will then be rendered using JavaScript.
   *
   *  Note: To change existing toolTip call setDeferredToolTip() again.
   *
   *  \sa toolTip
   */
  virtual void setDeferredToolTip(bool enable,
                                  TextFormat textFormat = TextFormat::Plain)
    = 0;

  /*! \brief Refresh the widget.
   *
   * The refresh method is invoked when the locale is changed using
   * WApplication::setLocale() or when the user hit the refresh button.
   *
   * The widget must actualize its contents in response.
   *
   * \note This does *not* rerender the widget! Calling refresh() usually
   *       does not have any effect (unless you've reimplemented refresh()
   *       to attach to it an effect).
   */
  virtual void refresh();

  /*! \brief Returns a JavaScript expression to the corresponding DOM node.
   *
   * You may want to use this in conjunction with JSlot or
   * doJavaScript() in custom JavaScript code.
   *
   * \sa isRendered()
   */
  std::string jsRef() const;

  /*! \brief Sets an attribute value.
   *
   * Sets the value for an HTML attribute.
   *
   * This is only useful for HTML features that are not supported
   * directly in %Wt (and beware that browsers have non-consistent
   * support for many more exotic HTML features).
   *
   * \sa JSlot, doJavaScript()
   */
  virtual void setAttributeValue(const std::string& name,
				 const WT_USTRING& value) = 0;

  /*! \brief Returns an attribute value.
   *
   * \sa setAttributeValue()
   */
  virtual WT_USTRING attributeValue(const std::string& name) const = 0;

  /*! \brief Sets a JavaScript member.
   *
   * This binds a JavaScript member, which is set as a JavaScript property
   * to the DOM object that implements this widget. The value may be any
   * JavaScript expression, including a function.
   *
   * Members that start with <tt>"wt"</tt> are reserved for internal
   * use. You may define a member <tt>"wtResize(self, width,
   * height)"</tt> method if your widget needs active layout
   * management. If defined, this method will be used by layout
   * managers and when doing resize() to set the size of the widget,
   * instead of setting the CSS width and height properties.
   */
  virtual void setJavaScriptMember(const std::string& name,
				   const std::string& value) = 0;

  /*! \brief Returns the value of a JavaScript member.
   *
   * \sa setJavaScriptMember()
   */
  virtual std::string javaScriptMember(const std::string& name) const = 0;

  /*! \brief Calls a JavaScript member.
   *
   * This calls a JavaScript member.
   *
   * \sa setJavaScriptMember()
   */
  virtual void callJavaScriptMember(const std::string& name,
				    const std::string& args) = 0;

  /*! \brief Short hand for WString::tr()
   *
   * Creates a localized string with the given key.
   */
  static WString tr(const char *key);
  static WString tr(const std::string& key);

  /*! \brief Loads content just before the widget is used.
   *
   * This method is called after a widget is inserted in the widget
   * hierarchy and fully constructed, but before the widget is rendered.
   * Widgets that get inserted in the widget hierarchy will
   * be rendered. Visible widgets are rendered immediately, and
   * invisible widgets in the back-ground (or not for a plain HTML
   * session). This method is called when the widget is directly or
   * indirectly inserted into the widget tree.
   *
   * The default implementation simply propagates the load signal to
   * its children. You may want to override this method to delay loading
   * of resource-intensive contents.
   *
   * During the life-time of a widget, this method may be called
   * multiple times, so you should make sure that you do a deferred
   * initializiation only once.
   */
  virtual void load() = 0;

  /*! \brief Returns whether this widget has been loaded.
   *
   * \sa load()
   */
  virtual bool loaded() const = 0;

  /*! \brief Sets whether the widget can receive focus.
   *
   * By default, only form widgets (descendants of WFormWidget),
   * anchors (WAnchor) and buttons (WPushButton) can receive focus.
   *
   * Any other widget can be configured to receive focus however.
   */
  virtual void setCanReceiveFocus(bool enabled) = 0;

  /*! \brief Returns whether the widget can receive focus.
   *
   * \sa setCanReceiveFocus()
   */
  virtual bool canReceiveFocus() const = 0;

  /*! \brief Sets focus.
   *
   * This only has an effect for a widget which can receive focus, and
   * is equivalent to setFocus(true).
   *
   * \sa setCanReceiveFocus()
   */
  void setFocus();

  /*! \brief Sets focus.
   *
   * When using \p focus = \c false, you can undo a previous setFocus() call.
   */
  virtual void setFocus(bool focus) = 0;

  /*! \brief Set focus on the widget's first descendant.
   *
   * Set focus on the widget itself, or on a first descendant which can
   * receive focus.
   *
   * Returns whether focus could be set.
   */
  virtual bool setFirstFocus() = 0;

  /*! \brief Returns whether the widget currently has the focus.
   */
  virtual bool hasFocus() const = 0;

  /*! \brief Sets the tab index.
   *
   * For widgets that receive focus (canReceiveFocus()), focus is
   * passed on to the next widget in the <i>tabbing chain</i> based on
   * their tab index. When the user navigates through form widgets
   * using the keyboard, widgets receive focus starting from the
   * element with the lowest tab index to elements with the highest
   * tab index.
   *
   * Widgets with a same tab index will receive focus in the same order
   * as they are inserted in the widget tree.
   *
   * The default tab index is 0 (for a widget that can receive focus).
   *
   * \sa setTabOrder()
   */
  virtual void setTabIndex(int index) = 0;

  /*! \brief Returns the tab index.
   *
   * \sa setTabIndex()
   */
  virtual int tabIndex() const = 0;

  virtual int zIndex() const = 0;

  /*! \brief Sets a mime type to be accepted for dropping.
   *
   * You may specify a style class that is applied to the widget when the
   * specified mimetype hovers on top of it.
   *
   * \sa dropEvent(), WInteractWidget::setDraggable(), stopAcceptDrops()
   */
  virtual void acceptDrops(const std::string& mimeType,
			   const WT_USTRING& hoverStyleClass = WT_USTRING());

  /*! \brief Indicates that a mime type is no longer accepted for dropping.
   *
   * \sa acceptDrops()
   */
  virtual void stopAcceptDrops(const std::string& mimeType);

  /*! \brief Sets the CSS Id.
   *
   * Sets a custom Id. Note that the Id must be unique across the whole
   * widget tree, can only be set right after construction and cannot
   * be changed. This is mostly useful for in tests using a test plan that
   * manipulates DOM elements by Id.
   *
   * By default, auto-generated id's are used.
   *
   * \note An id must start with a letter ([A-Za-z]), followed by one or more
   * letters, digits ([0-9]), hyphens ("-"), underscores ("_"), colons (":"),
   * and periods (".").
   *
   * \sa WObject::id()
   */
  virtual void setId(const std::string& id) = 0;

  /*! \brief Finds a descendent widget by name.
   *
   * \sa setObjectName()
   */
  virtual WWidget *find(const std::string& name) = 0;

  /*! \brief Finds a descendent widget by id.
   */
  virtual WWidget *findById(const std::string& id) = 0;

  /*! \brief Streams the (X)HTML representation.
   *
   * Streams the widget as CharEncoding::UTF8-encoded (HTML-compatible) XHTML.
   *
   * This may be useful as a debugging tool for the web-savvy, or in
   * other rare situations. Usually, you will not deal directly with
   * HTML, and calling this method on a widget that is rendered may
   * interfere with the library keeping track of changes to the
   * widget.
   */
  virtual void htmlText(std::ostream& out);

  /*! \brief Sets as selectable.
   *
   * When a widget is made unselectable, a selection of text (or images)
   * will not be visible (but may still be possible).
   *
   * By default, the widget inherits this property from its parent,
   * and this property propagates to all children. The top level
   * container (WApplication::root()) selectable by default.
   */
  virtual void setSelectable(bool selectable) = 0;

  /*! \brief Executes the given JavaScript statements when
   *         the widget is rendered or updated.
   *
   * Calling WApplication::doJavaScript() with JavaScript code that
   * refers to a widget using jsRef(), that is still to be rendered
   * may cause JavaScript errors because the corresponding DOM node
   * does not exist. This happens for example when a widget is
   * created, but not yet inserted in the widget tree.
   *
   * This method guarantees that the JavaScript code is only run when
   * the corresponding DOM node (using jsRef()) resolves to a valid
   * DOM object.
   *
   * \sa jsRef()
   */
  virtual void doJavaScript(const std::string& js) = 0;

  /*! \brief Returns whether the widget is rendered.
   *
   * \sa jsRef()
   */
  bool isRendered() const;

  std::string inlineCssStyle();

  std::string createJavaScript(WStringStream& js, std::string insertJS);

  /*! \brief Hides the widget.
   *
   * \sa setHidden()
   */
  void hide();

  /*! \brief Hides the widget using an animation.
   *
   * To hide the widget, the animation is replayed in reverse.
   *
   * \sa setHidden()
   */
  void animateHide(const WAnimation& animation);

  /*! \brief Shows the widget.
   *
   * \sa setHidden()
   */
  void show();

  /*! \brief Shows the widget using an animation.
   *
   * \sa setHidden()
   */
  void animateShow(const WAnimation& animation);

  /*! \brief Enables the widget.
   *
   * This calls \link setDisabled() setDisabled(false)\endlink.
   */
  void enable();

  /*! \brief Disable thes widget.
   *
   * This calls \link setDisabled() setDisabled(true)\endlink.
   */
  void disable();

   /*! \brief Returns whether the widget is layout size aware.
   *
   * \sa setLayoutSizeAware()
   */
  bool layoutSizeAware() const;

  /*! \brief Returns whether scroll visibility detection is enabled for this widget.
   *
   * \sa setScrollVisibilityEnabled()
   */
  virtual bool scrollVisibilityEnabled() const = 0;

  /*! \brief Sets whether scroll visibility detection is enabled for this widget.
   *
   * Disabled by default. When enabled, the client keeps track of whether this widget
   * is currently visible inside of the browser window. A widget is "scroll visible"
   * if it is currently visible according to isVisible(), and its position is
   * inside of the browser window, with an extra margin determined by scrollVisibilityMargin().
   *
   * If scroll visibility changes, the scrollVisibilityChanged() signal is fired,
   * and isScrollVisible() is updated.
   *
   * This feature can be useful to implement infinite scroll, where a sentinel widget
   * placed at the bottom of the page causes more content to be loaded when it
   * becomes visible, see the <tt>infinite-scroll</tt> example.
   *
   * This feature can also be used to lazy load widgets when they become visible.
   *
   * \note If the widget is "scroll visible" when scroll visibility detection is first enabled,
   *       the scrollVisibilityChanged() signal will be emitted. If it is outside of the
   *       browser's viewport when first enabled, the scrollVisibilityChanged() signal will
   *       not be emitted.
   * \note If scroll visibility is enabled, disabled, and then enabled again,
   *       isScrollVisible() may not be correctly updated, and scrollVisibilityChanged()
   *       may not be correctly emitted, because then Wt can't properly keep track of
   *       the state that the widget is in on the client side.
   *       This feature is not intended to be toggled on and off, but rather enabled
   * 	   once and disabled once after that.
   */
  virtual void setScrollVisibilityEnabled(bool enabled) = 0;

  /*! \brief Returns the margin around the viewport within which the widget is considered visible.
   *
   * \sa setScrollVisibilityMargin()
   */
  virtual int scrollVisibilityMargin() const = 0;

  /*! \brief Sets the margin around the viewport within which the widget is considered visible.
   *
   * This causes the widget to be considered "scroll visible" before it is within
   * the viewport. Setting this margin could be useful to trigger the loading
   * of content before it is in view.
   */
  virtual void setScrollVisibilityMargin(int margin) = 0;

  /*! \brief Signal triggered when the scroll visibility of this widget changes.
   *
   * The boolean parameter indicates whether the widget is currently scroll visible.
   *
   * \sa setScrollVisibilityEnabled()
   */
  virtual Signal<bool> &scrollVisibilityChanged() = 0;

  /*! \brief Returns whether this widget is currently considered scroll visible.
   *
   * isScrollVisible() is initially false.
   *
   * \sa setScrollVisibilityEnabled()
   */
  virtual bool isScrollVisible() const = 0;

  /*!
   * \brief Sets whether theme styling for a widget is enabled or disabled.
   *
   * By default all widgets are styled according to the chosen theme.
   * Disabling the theme style could be useful to completely customize the style of the widget
   * outside of the theme. 
   *
   * \note This should be changed after the construction but before the rendering 
   * of the widget.
   */
  virtual void setThemeStyleEnabled(bool enabled) = 0;

  /*!
   * \brief Returns whether this widget is currently styled by 
   * the chosen theme.
   *
   * isThemeEnabled() is initially true.
   *
   * \sa setThemeStyleEnabled()
   */
  virtual bool isThemeStyleEnabled() const = 0;

  DomElement *createSDomElement(WApplication *app);

  static void setTabOrder(WWidget *first, WWidget *second);

  virtual bool isExposed(WWidget *widget);

  void addJSignal(EventSignalBase * signal);

  virtual int baseZIndex() const = 0;

  static const char *WT_RESIZE_JS;

protected:
  static const char *WT_GETPS_JS;

  /*! \brief Sets the widget to be aware of its size set by a layout manager.
   *
   * When the widget is inserted in a layout manager, it will be
   * resized to fit within the constraints imposed by the layout
   * manager. By default, this done client-side only by setting the
   * CSS height (and if needed, width) properties of the DOM element
   * corresponding to the widget.
   *
   * A widget may define a JavaScript method, <tt>"wtResize(self,
   * width, height)"</tt>, to actively manage its client-side width
   * and height, if it wants to react to these client-side size hints
   * in a custom way (see setJavaScriptMember()).
   *
   * By setting \p sizeAware to true, the widget will propagate the
   * width and height provided by the layout manager to the virtual
   * layoutSizeChanged() method, so that you may for example change
   * the size of contained children in a particular way (doing a
   * custom, manual, layout).
   *
   * \sa layoutSizeChanged()
   */
  void setLayoutSizeAware(bool sizeAware);

  /*! \brief Virtual method that indicates a size change.
   *
   * This method propagates the client-side width and height of the
   * widget when the widget is contained by a layout manager and
   * setLayoutSizeAware(true) was called.
   *
   * \sa setLayoutSizeAware()
   */
  virtual void layoutSizeChanged(int width, int height);

  /*! \brief Creates a widget.
   */
  WWidget();

  /*! \brief Handles a drop event.
   *
   * Reimplement this method to handle a drop events for mime types you
   * declared to accept using acceptDrops.
   *
   * The default implementation simply completes the drag and drop operation
   * as if nothing happened.
   *
   * \sa acceptDrops(), WInteractWidget::setDraggable()
   */
  virtual void dropEvent(WDropEvent dropEvent);

  /*! \brief Progresses to an Ajax-enabled widget.
   *
   * This method is called when the progressive bootstrap method is
   * used, and support for AJAX has been detected. The default
   * behavior will upgrade the widget's event handling to use AJAX
   * instead of full page reloads, and propagate the call to its
   * children.
   *
   * You may want to reimplement this method if you want to make
   * changes to widget when AJAX is enabled. You should always call
   * the base implementation.
   *
   * \sa WApplication::enableAjax()
   */
  virtual void enableAjax() = 0;

  /*! \brief Returns the widget's built-in padding.
   *
   * This is used by the layout managers to correct for a built-in
   * padding which interferes with setting a widget's width (or
   * height) to 100%.
   *
   * A layout manager needs to set the width to 100% only for form
   * widgets (WTextArea, WLineEdit, WComboBox, etc...). Therefore,
   * only for those widgets this needs to return the padding (the
   * default implementation returns 0).
   *
   * For form widgets, the padding depends on the specific
   * browser/platform combination, unless an explicit padding is set
   * for the widget.
   *
   * When setting an explicit padding for the widget using a style
   * class, you will want to reimplement this method to return this
   * padding in case you want to set the widget inside a layout
   * manager.
   *
   * \sa boxBorder()
   */
  virtual int boxPadding(Orientation orientation) const;

  /*! \brief Returns the widget's built-in border width.
   *
   * This is used by the layout managers to correct for a built-in
   * border which interferes with setting a widget's width (or height)
   * to 100%.
   *
   * A layout manager needs to set the width to 100% only for form
   * widgets (WTextArea, WLineEdit, WComboBox, etc...). Therefore,
   * only for those widgets this needs to return the border width (the
   * default implementation returns 0).
   *
   * For form widgets, the border width depends on the specific
   * browser/platform combination, unless an explicit border is set
   * for the widget.
   *
   * When setting an explicit border for the widget using a style
   * class, you will want to reimplement this method to return this
   * border width, in case you want to set the widget inside a layout
   * manager.
   *
   * \sa boxPadding()
   */
  virtual int boxBorder(Orientation orientation) const;

  /*! \brief Propagates that a widget was enabled or disabled through children.
   *
   * When enabling or disabling a widget, you usually also want to disable
   * contained children. This method is called by setDisabled() to propagate
   * its state to all children.
   *
   * You may want to reimplement this method if they wish to render
   * differently when a widget is disabled. The default implementation will
   * propagate the signal to all children.
   */
  virtual void propagateSetEnabled(bool enabled) = 0;

  virtual void propagateSetVisible(bool visible) = 0;

  void getDrop(const std::string sourceId, const std::string mimeType,
	       WMouseEvent event);
  void getDropTouch(const std::string sourceId, const std::string mimeType,
	       WTouchEvent event);

  virtual void setHideWithOffsets(bool how = true) = 0;

  virtual bool isStubbed() const = 0;

  /*! \brief Schedules rerendering of the widget.
   *
   * This schedules a rendering phase after all events have been
   * processed. This method is used internally whenever a property of
   * a widget has been changed. But you may want to use this if you
   * are deferring actual changes to a widget in response to an event,
   * and instead postpone this until all events have been received.
   *
   * \sa render()
   */
  void scheduleRender(WFlags<RepaintFlag> flags = None);

  /*! \brief Renders the widget.
   *
   * This function renders the widget (or an update for the widget), after
   * this has been scheduled using scheduleRender().
   *
   * The default implementation will render the widget by serializing changes
   * to JavaScript and HTML. You may want to reimplement this widget if you
   * have been postponing some of the layout / rendering implementation
   * until the latest moment possible. In that case you should make sure you
   * call the base implementation however.
   */
  virtual void render(WFlags<RenderFlag> flags);

  virtual void childResized(WWidget *child, WFlags<Orientation> directions);

  WWidget *adam();

  typedef std::list<EventSignalBase *> EventSignalList;

  void addEventSignal(EventSignalBase& s);
  EventSignalBase *getEventSignal(const char *name);
  EventSignalList& eventSignals() { return eventSignals_; }

  virtual WStatelessSlot *getStateless(Method method) override;

  void renderOk();
  void scheduleRerender(bool laterOnly, WFlags<RepaintFlag> flags = None);
  bool needRerender() const { return flags_.test(BIT_NEED_RERENDER); }

  virtual void getSDomChanges(std::vector<DomElement *>& result,
			      WApplication *app) = 0;
  virtual bool needsToBeRendered() const = 0;
  bool isInLayout() const;

  virtual bool hasParent() const;

  WCssTextRule *addCssRule(const std::string& selector,
			   const std::string& declarations,
			   const std::string& ruleName = std::string());

private:
  /*
   * Booleans packed in a bitset.
   */
  static const int BIT_WAS_HIDDEN = 0;
  static const int BIT_WAS_DISABLED = 1;
  static const int BIT_NEED_RERENDER = 2;
  static const int BIT_NEED_RERENDER_SIZE_CHANGE = 3;
  static const int BIT_HAS_PARENT = 4;
  static const int BIT_RESIZE_AWARE = 5;
  static const int BIT_SCROLL_VISIBILITY_ENABLED = 6;
  std::bitset<7> flags_;

  EventSignalList eventSignals_;
  std::vector<EventSignalBase*> jsignals_;

  WWidget *parent_;
  void setJsSize();
  void undoHideShow();
  void undoDisableEnable();
  virtual void setParentWidget(WWidget *parent);

  virtual WWebWidget *webWidget() = 0;

  friend class WebRenderer;
  friend class WAbstractArea;
  friend class WAbstractItemView;
  friend class WApplication;
  friend class WCalendar;
  friend class WContainerWidget;
  friend class WCompositeWidget;
  friend class WFileUpload;
  friend class WGLWidget;
  friend class WMenuItem;
  friend class WPaintedWidget;
  friend class WPopupWidget;
  friend class WScrollArea;
  friend class WTemplate;
  friend class WViewWidget;
  friend class WWebWidget;
  friend class WWidgetItem;
  friend class StdWidgetItemImpl;
};

}

#endif // WWIDGET_H_
