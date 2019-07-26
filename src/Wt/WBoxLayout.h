// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBOX_LAYOUT_H_
#define WBOX_LAYOUT_H_

#include <Wt/WGridLayout.h>

namespace Wt {

/*! \class WBoxLayout Wt/WBoxLayout.h Wt/WBoxLayout.h
 *  \brief A layout manager which arranges widgets horizontally or vertically
 *
 * This layout manager arranges widgets horizontally or vertically
 * inside the parent container.
 *
 * The space is divided so that each widget is given its preferred
 * size, and remaining space is divided according to stretch factors
 * among widgets. If not all widgets can be given their preferred size
 * (there is not enough room), then widgets are given a smaller size
 * (down to their minimum size). If necessary, the container (or
 * parent layout) of this layout is resized to meet minimum size
 * requirements.
 *
 * The preferred width or height of a widget is based on its natural
 * size, where it presents its contents without
 * overflowing. WWidget::resize() or (CSS <tt>width</tt>,
 * <tt>height</tt> properties) can be used to adjust the preferred
 * size of a widget.
 *
 * The minimum width or height of a widget is based on the minimum
 * dimensions of the widget or the nested layout. The default minimum
 * height or width for a widget is 0. It can be specified using
 * WWidget::setMinimumSize() or using CSS <tt>min-width</tt> or
 * <tt>min-height</tt> properties.
 *
 * You should use \link WContainerWidget::setOverflow()
 * WContainerWidget::setOverflow(OverflowAuto) \endlink to automatically
 * show scrollbars for widgets inserted in the layout to cope with a size
 * set by the layout manager that is smaller than the preferred size.
 *
 * When the container of a layout manager does not have a defined size
 * (by having an explicit size, or by being inside a layout manager),
 * or has has only a maximum size set using
 * WWidget::setMaximumSize(), then the size of the container will be
 * based on the preferred size of the contents, up to this maximum
 * size, instead of the default behaviour of constraining the size of
 * the children based on the size of the container. Note that because
 * of the CSS defaults, a WContainer has by default no height, but
 * inherits the width of its parent widget. The width is thus by default
 * defined.
 *
 * A layout manager may provide resize handles between items which
 * allow the user to change the automatic layout provided by the
 * layout manager (see setResizable()).
 *
 * Each item is separated using a constant spacing, which defaults to
 * 6 pixels, and can be changed using setSpacing(). In addition, when
 * this layout is a top-level layout (i.e. is not nested inside
 * another layout), a margin is set around the contents. This margin
 * defaults to 9 pixels, and can be changed using
 * setContentsMargins(). You can add more space between two widgets
 * using addSpacing().
 *
 * For each item a stretch factor may be defined, which controls how
 * remaining space is used. Each item is stretched using the stretch
 * factor to fill the remaining space.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WContainerWidget *w = addWidget(std::make_unique<Wt::WContainerWidget>());
 * w->resize(Wt::WLength(), 600);
 *
 * auto layout = std::make_unique<Wt::WBoxLayout>(Wt::LayoutDirection::TopToBottom);
 * layout->addWidget(std::make_unique<Wt::WText>("One"));
 * layout->addWidget(std::make_unique<Wt::WText>("Two"));
 * layout->addWidget(std::make_unique<Wt::WText>("Three"));
 * layout->addWidget(std::make_unique<Wt::WText>("Four"));
 *
 * w->setLayout(std::move(layout));
 * \endcode
 * \endif
 *
 * \note When JavaScript support is not available, not all functionality
 * of the layout is available. In particular, vertical size management is
 * not available.
 *
 * \note When a layout is used on a first page with progressive
 * bootstrap, then the layout will progress only in a limited way to a
 * full JavaScript-based layout. You can thus not rely on it to behave
 * properly for example when dynamically adding or removing widgets.
 */
class WT_API WBoxLayout : public WLayout
{
public:
  /*! \brief Creates a new box layout.
   *
   * This constructor is rarely used. Instead, use the convenient
   * constructors of the specialized WHBoxLayout or WVBoxLayout classes.
   *
   * Use \p parent = \c 0 to created a layout manager that can be
   * nested inside other layout managers.
   */
  WBoxLayout(LayoutDirection dir);

  virtual void addItem(std::unique_ptr<WLayoutItem> item) override;
  virtual std::unique_ptr<WLayoutItem> removeItem(WLayoutItem *item) override;
  virtual WLayoutItem *itemAt(int index) const override;
  virtual int count() const override;

  /*! \brief Sets the layout direction.
   *
   * \note Changing the layout direction after something (a widget or nested layout)
   *       has been added is not supported.
   *
   * \sa direction()
   */
  void setDirection(LayoutDirection direction);

  /*! \brief Returns the layout direction.
   *
   * \sa setDirection()
   */
  LayoutDirection direction() const { return direction_; }

  /*! \brief Sets spacing between each item.
   *
   * The default spacing is 6 pixels.
   */
  void setSpacing(int size);

  /*! \brief Returns the spacing between each item.
   *
   * \sa setSpacing()
   */
  int spacing() const { return grid_.horizontalSpacing_; }

  /*! \brief Adds a widget to the layout.
   *
   * Adds a widget to the layout, with given \p stretch factor. When
   * the stretch factor is 0, the widget will not be resized by the
   * layout manager (stretched to take excess space).
   *
   * The \p alignment parameter is a combination of a horizontal
   * and/or a vertical AlignmentFlag OR'ed together.
   *
   * The \p alignment specifies the vertical and horizontal
   * alignment of the item. The default value 0 indicates that the
   * item is stretched to fill the entire column or row. The alignment can
   * be specified as a logical combination of a horizontal alignment
   * (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or Wt::AlignmentFlag::Right) and a
   * vertical alignment (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or
   * Wt::AlignmentFlag::Bottom).
   *
   * \sa addLayout(), insertWidget()
   */
#ifndef WT_TARGET_JAVA
  void addWidget(std::unique_ptr<WWidget> widget, int stretch,
                 WFlags<AlignmentFlag> alignment);
#else // WT_TARGET_JAVA
  void addWidget(std::unique_ptr<WWidget> widget, int stretch = 0,
                 WFlags<AlignmentFlag> alignment = None);
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \brief Adds a widget to the layout, returning a raw pointer.
   *
   * This is implemented as:
   *
   * \code
   * Widget *result = widget.get();
   * addWidget(std::unique_ptr<WWidget>(std::move(widget)), stretch, alignment);
   * return result;
   * \endcode
   */
  template <typename Widget>
    Widget *addWidget(std::unique_ptr<Widget> widget, int stretch = 0,
                      WFlags<AlignmentFlag> alignment = None)
  {
    Widget *result = widget.get();
    addWidget(std::unique_ptr<WWidget>(std::move(widget)), stretch, alignment);
    return result;
  }
#endif // WT_TARGET_JAVA

  /*! \brief Adds a nested layout to the layout.
   *
   * Adds a nested layout, with given \p stretch factor.
   *
   * \sa addWidget(WWidget *, int, WFlags<AlignmentFlag>), insertLayout()
   */
#ifndef WT_TARGET_JAVA
  void addLayout(std::unique_ptr<WLayout> layout, int stretch,
                 WFlags<AlignmentFlag> alignment);
#else // WT_TARGET_JAVA
  void addLayout(std::unique_ptr<WLayout> layout, int stretch = 0,
                 WFlags<AlignmentFlag> alignment = None);
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \briefs Adds a nested layout to the layout, returning a raw pointer.
   *
   * This is implemented as:
   * 
   * \code
   * Layout *result = layout.get();
   * addLayout(std::unique_ptr<WLayout>(std::move(layout)), stretch, alignment);
   * return result;
   * \endcode
   */
  template <typename Layout>
    Layout *addLayout(std::unique_ptr<Layout> layout, int stretch = 0,
                      WFlags<AlignmentFlag> alignment = None)
  {
    Layout *result = layout.get();
    addLayout(std::unique_ptr<WLayout>(std::move(layout)), stretch, alignment);
    return result;
  }
#endif // WT_TARGET_JAVA

  /*! \brief Adds extra spacing.
   *
   * Adds extra spacing to the layout.
   *
   * \sa addStretch(), insertStretch()
   */
  void addSpacing(const WLength& size);

  /*! \brief Adds a stretch element.
   *
   * Adds a stretch element to the layout. This adds an empty space
   * that stretches as needed.
   *
   * \sa addSpacing(), insertStretch()
   */
  void addStretch(int stretch = 0);

  /*! \brief Inserts a widget in the layout.
   *
   * Inserts a widget in the layout at position \p index, with given
   * \p stretch factor. When the stretch factor is 0, the widget will
   * not be resized by the layout manager (stretched to take excess
   * space).
   *
   * The \p alignment specifies the vertical and horizontal alignment
   * of the item. The default value None indicates that the item is
   * stretched to fill the entire column or row. The alignment can be
   * specified as a logical combination of a horizontal alignment
   * (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or
   * Wt::AlignmentFlag::Right) and a vertical alignment
   * (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or
   * Wt::AlignmentFlag::AlignBottom).
   *
   * \sa insertLayout(), addWidget(WWidget *, int, WFlags<AlignmentFlag>)
   */
  void insertWidget(int index, std::unique_ptr<WWidget> widget, int stretch = 0,
		    WFlags<AlignmentFlag> alignment = None);

  /*! \brief Inserts a widget in the layout, returning a raw pointer.
   *
   * This is implemented as:
   * \code
   * Widget *result = widget.get();
   * insertWidget(index, std::unique_ptr<WWidget>(std::move(widget)), stretch, alignment);
   * return result;
   * \endcode
   */
#ifndef WT_TARGET_JAVA
  template <typename Widget>
    Widget *insertWidget(int index, std::unique_ptr<Widget> widget, int stretch = 0,
			 WFlags<AlignmentFlag> alignment = None)
  {
    Widget *result = widget.get();
    insertWidget(index, std::unique_ptr<WWidget>(std::move(widget)), stretch, alignment);
    return result;
  }
#else // WT_TARGET_JAVA
  template <typename Widget>
    Widget *insertWidget(int index, std::unique_ptr<Widget> widget, int stretch);
#endif // WT_TARGET_JAVA

  /*! \brief Inserts a nested layout in the layout.
   *
   * Inserts a nested layout in the layout at position\p index,
   * with given \p stretch factor.
   *
   * \sa insertWidget(), addLayout()
   */
  void insertLayout(int index, std::unique_ptr<WLayout> layout, int stretch = 0,
		    WFlags<AlignmentFlag> alignment = None);

  /*! \briefs Inserts a nested layout in the layout, returning a raw pointer.
   *
   * This is implemented as:
   * \code
   * Layout *result = layout.get();
   * addLayout(index, std::unique_ptr<WLayout>(std::move(layout)), stretch, alignment);
   * return result;
   * \endcode
   */
#ifndef WT_TARGET_JAVA
  template <typename Layout>
    Layout *insertLayout(int index, std::unique_ptr<Layout> layout, int stretch = 0,
	                 WFlags<AlignmentFlag> alignment = None)
  {
    Layout *result = layout.get();
    insertLayout(index, std::unique_ptr<WLayout>(std::move(layout)), stretch, alignment);
    return result;
  }
#else // WT_TARGET_JAVA
  template <typename Layout>
    Layout *insertLayout(int index, std::unique_ptr<Layout> layout, int stretch);
#endif

  /*! \brief Inserts extra spacing in the layout.
   *
   * Inserts extra spacing in the layout at position \p index.
   *
   * \sa insertStretch(), addSpacing()
   */
  void insertSpacing(int index, const WLength& size);

  /*! \brief Inserts a stretch element in the layout.
   *
   * Inserts a stretch element in the layout at position
   * \p index. This adds an empty space that stretches as needed.
   *
   * \sa insertSpacing(), addStretch()
   */
  void insertStretch(int index, int stretch = 0);

  /*! \brief Sets the stretch factor for a nested layout.
   *
   * The \p layout must have previously been added to this layout
   * using insertLayout() or addLayout().
   *
   * Returns whether the \p stretch could be set.
   */
  bool setStretchFactor(WLayout *layout, int stretch);

  /*! \brief Sets the stretch factor for a widget.
   *
   * The \p widget must have previously been added to this layout
   * using insertWidget() or addWidget().
   *
   * Returns whether the \p stretch could be set.
   */
  bool setStretchFactor(WWidget *widget, int stretch);

  /*! \brief Sets whether the use may drag a particular border.
   *
   * This method sets whether the border that separates item
   * <i>index</i> from the next item may be resized by the user,
   * depending on the value of <i>enabled</i>.
   *
   * The default value is <i>false</i>.
   *
   * If an \p initialSize is given (that is not WLength::Auto), then
   * this size is used for the size of the item, overriding the size
   * it would be given by the layout manager.
   */
  void setResizable(int index, bool enabled = true,
		    const WLength& initialSize = WLength::Auto);

  /*! \brief Returns whether the user may drag a particular border.
   *
   * This method returns whether the border that separates item
   * <i>index</i> from the next item may be resized by the user.
   *
   * \sa setResizable()
   */
  bool isResizable(int index) const;

  virtual void iterateWidgets(const HandleWidgetMethod& method) const override;

protected:
  void insertItem(int index, std::unique_ptr<WLayoutItem> item, int stretch,
		  WFlags<AlignmentFlag> alignment);

  virtual void updateImplementation() override;

private:
  LayoutDirection direction_;
  Impl::Grid grid_;

  void setStretchFactor(int index, int stretch);
  std::unique_ptr<WWidget> createSpacer(const WLength& size);

  virtual void setParentWidget(WWidget *parent) override;

  void setImplementation();
  bool implementationIsFlexLayout() const;
};

}

#endif // WBOX_LAYOUT_H_
