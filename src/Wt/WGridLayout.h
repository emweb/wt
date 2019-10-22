// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WGRID_LAYOUT_H_
#define WGRID_LAYOUT_H_

#include <vector>
#include <Wt/WLayout.h>
#include <Wt/WLength.h>
#include <Wt/WWidget.h>

namespace Wt {

  namespace Impl {

struct Grid {
  int horizontalSpacing_, verticalSpacing_;

  struct Section {
    int stretch_;
    bool resizable_;
    WLength initialSize_;

    Section(int stretch = 0);
  };

  struct Item {
    std::unique_ptr<WLayoutItem> item_;
    int rowSpan_;
    int colSpan_;
    bool update_;
    WFlags<AlignmentFlag> alignment_;

    Item(std::unique_ptr<WLayoutItem> item = nullptr,
	 WFlags<AlignmentFlag> alignment = None);
    Item(Item&& other) = default;
    Item& operator=(Item&& other) = default;
    ~Item();
  };

  std::vector<Section> rows_;
  std::vector<Section> columns_;
  std::vector<std::vector<Item> > items_; // [row][column]

  Grid();
  ~Grid();

  void clear();
};

  }

/*! \class WGridLayout Wt/WGridLayout.h Wt/WGridLayout.h
 *  \brief A layout manager which arranges widgets in a grid
 *
 * This layout manager arranges widgets in a grid.
 *
 * Each grid cell (row, column) may contain one widget or nested
 * layout. Orientation::Horizontal and vertical space are divided so that each
 * non-stretchable column/row is given its preferred size (if
 * possible) and the remaining space is divided according to stretch
 * factors among the columns/rows. If not all columns/rows can be
 * given their preferred size (there is not enough room), then
 * columns/rows are given a smaller size (down to a minimum size based
 * on widget minimum sizes). If necessary, the container (or parent
 * layout) of this layout is resized to meet minimum size
 * requirements.
 *
 * The preferred width/height of a column/row is based on the natural
 * size of the widgets, where they present their contents without
 * overflowing. WWidget::resize() or (CSS <tt>width</tt>,
 * <tt>height</tt> properties) can be used to adjust the preferred
 * size of a widget.
 *
 * The minimum width/height of a column/row is based on the minimum
 * dimensions of contained widgets or nested layouts. The default
 * minimum height and width for a widget is 0. It can be specified
 * using WWidget::setMinimumSize() or using CSS <tt>min-width</tt> and
 * <tt>min-height</tt> properties.
 *
 * You should use \link WContainerWidget::setOverflow()
 * WContainerWidget::setOverflow(OverflowAuto) \endlink to automatically
 * show scrollbars for widgets inserted
 * in the layout to cope with a size set by the layout manager that is
 * smaller than the preferred size.
 *
 * When the container of a layout manager has a maximum size set using
 * WWidget::setMaximumSize(), then the size of the container will be
 * based on the preferred size of the contents, up to this maximum
 * size, instead of the default behaviour of constraining the size of
 * the children based on the size of the container.
 *
 * A layout manager may provide resize handles between columns or rows
 * which allow the user to change the automatic layout provided by the
 * layout manager (see setRowResizable() and
 * setColumnResizable()).
 *
 * Columns and rows are separated using a constant spacing, which
 * defaults to 6 pixels by default, and can be changed using
 * setHorizontalSpacing() and setVerticalSpacing(). In addition, when
 * this layout is a top-level layout (i.e. is not nested inside
 * another layout), a margin is set around the contents. This margin
 * defaults to 9 pixels, and can be changed using setContentsMargins().
 *
 * For each column or row, a stretch factor may be defined, which
 * controls how remaining horizontal or vertical space is used. Each
 * column and row is stretched using the stretch factor to fill the
 * remaining space. When the stretch factor is 0, the height of the
 * row and its contents is set to the preferred size (if
 * possible). When the stretch factor is 1 or higher, these widgets
 * will be given the remaining size, limited only by their minimum
 * size (their preferred size is ignored).
 *
 * Usage example:
 * \if cpp
 * \code
 * Wt::WContainerWidget *w = addWidget(std::make_unique<Wt::WContainerWidget>());
 * w->resize(WLength::Auto, 600);
 *
 * auto layout = std::make_unique<Wt::WGridLayout>();
 * layout->addWidget(std::make_unique<Wt::WText>("Item 0 0"), 0, 0);
 * layout->addWidget(std::make_unique<Wt::WText>("Item 0 1"), 0, 1);
 * layout->addWidget(std::make_unique<Wt::WText>("Item 1 0"), 1, 0);
 * layout->addWidget(std::make_unique<Wt::WText>("Item 1 1"), 1, 1);
 *
 * w->setLayout(std::move(layout));
 * \endcode
 * \elseif java
 * \code
 * WContainerWidget w = new WContainerWidget(this);
 * w.resize(WLength.Auto, new WLength(600));
 *		 
 * WGridLayout layout = new WGridLayout();
 * layout.addWidget(new WText("Item 0 0"), 0, 0);
 * layout.addWidget(new WText("Item 0 1"), 0, 1);
 * layout.addWidget(new WText("Item 1 0"), 1, 0);
 * layout.addWidget(new WText("Item 1 1"), 1, 1);
 *		 
 * w.setLayout(layout);
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
class WT_API WGridLayout : public WLayout
{
public:
  /*! \brief Create a new grid layout.
   *
   * The grid will grow dynamically as items are added.
   *
   * Use \p parent = \c 0 to create a layout manager that can be
   * nested inside other layout managers or if you use
   * WContainerWidget::setLayout() to add specify the container later.
   */
  WGridLayout();

  virtual ~WGridLayout() override;

  virtual void addItem(std::unique_ptr<WLayoutItem> item) override;
  virtual std::unique_ptr<WLayoutItem> removeItem(WLayoutItem *item) override;
  virtual WLayoutItem *itemAt(int index) const override;
  virtual int count() const override;

  /*! \brief Adds a layout item to the grid.
   *
   * Adds the <i>item</i> at (<i>row</i>, \p column). If an item
   * was already added to that location, it is replaced.
   *
   * An item may span several more rows or columns, which is
   * controlled by <i>rowSpan</i> and \p columnSpan.
   *
   * The \p alignment specifies the vertical and horizontal
   * alignment of the item. The default value 0 indicates that the
   * item is stretched to fill the entire grid cell. The alignment can
   * be specified as a logical combination of a horizontal alignment
   * (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or Wt::AlignmentFlag::Right) and a
   * vertical alignment (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or
   * Wt::AlignmentFlag::Bottom).
   *
   * \sa addLayout(), addWidget() 
   */
  void addItem(std::unique_ptr<WLayoutItem> item, int row, int column,
	       int rowSpan = 1, int columnSpan = 1,
	       WFlags<AlignmentFlag> alignment = None);

  /*! \brief Adds a nested layout item to the grid.
   *
   * Adds the <i>layout</i> at (<i>row</i>, \p column). If an item
   * was already added to that location, it is replaced (but not
   * deleted).
   *
   * The \p alignment specifies the vertical and horizontal
   * alignment of the item. The default value 0 indicates that the
   * item is stretched to fill the entire grid cell. The alignment can
   * be specified as a logical combination of a horizontal alignment
   * (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or Wt::AlignmentFlag::Right) and a
   * vertical alignment (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or
   * Wt::AlignmentFlag::Bottom).
   *
   * \sa addLayout(WLayout *, int, int, int, int, WFlags<AlignmentFlag>) 
   */
  void addLayout(std::unique_ptr<WLayout> layout, int row, int column,
		 WFlags<AlignmentFlag> alignment = None);

  /*! \brief Adds a nested layout item to the grid.
   *
   * Adds the <i>layout</i> at (<i>row</i>, \p column). If an item
   * was already added to that location, it is replaced (but not
   * deleted).
   *
   * An item may span several more rows or columns, which is
   * controlled by <i>rowSpan</i> and \p columnSpan.
   *
   * The \p alignment specifies the vertical and horizontal
   * alignment of the item. The default value 0 indicates that the
   * item is stretched to fill the entire grid cell. The alignment can
   * be specified as a logical combination of a horizontal alignment
   * (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or Wt::AlignmentFlag::Right) and a
   * vertical alignment (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or
   * Wt::AlignmentFlag::Bottom).
   *
   * \sa addLayout(WLayout *, int, int, WFlags<AlignmentFlag>) 
   */
  void addLayout(std::unique_ptr<WLayout> layout, int row, int column,
		 int rowSpan, int columnSpan,
		 WFlags<AlignmentFlag> alignment = None);

  /*! \brief Adds a widget to the grid.
   *
   * Adds the <i>widget</i> at (<i>row</i>, \p column). If an item
   * was already added to that location, it is replaced (but not
   * deleted).
   *
   * The \p alignment specifies the vertical and horizontal
   * alignment of the item. The default value 0 indicates that the
   * item is stretched to fill the entire grid cell. The alignment can
   * be specified as a logical combination of a horizontal alignment
   * (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or Wt::AlignmentFlag::Right) and a
   * vertical alignment (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or
   * Wt::AlignmentFlag::Bottom).
   *
   * \sa addWidget(WWidget *, int, int, int, int, WFlags<AlignmentFlag>) 
   */
#ifndef WT_TARGET_JAVA
  void addWidget(std::unique_ptr<WWidget> widget, int row, int column,
                 WFlags<AlignmentFlag> alignment);
#else // WT_TARGET_JAVA
  void addWidget(std::unique_ptr<WWidget> widget, int row, int column,
                 WFlags<AlignmentFlag> alignment = None);
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \brief Adds a widget to the grid, returning a raw pointer to the widget.
   *
   * This is implemented as:
   * 
   * \code
   * Widget *result = widget.get();
   * addWidget(std::unique_ptr<WWidget>(std::move(widget)), row, column, alignment);
   * return result;
   * \endcode
   */
  template <typename Widget>
    Widget *addWidget(std::unique_ptr<Widget> widget, int row, int column,
                      WFlags<AlignmentFlag> alignment = None)
  {
    Widget *result = widget.get();
    addWidget(std::unique_ptr<WWidget>(std::move(widget)), row, column, alignment);
    return result;
  }
#endif // WT_TARGET_JAVA

  /*! \brief Adds a widget to the grid.
   *
   * Adds the <i>widget</i> at (<i>row</i>, \p column). If an item
   * was already added to that location, it is replaced (but not
   * deleted).
   *
   * The widget may span several more rows or columns, which is
   * controlled by <i>rowSpan</i> and \p columnSpan.
   *
   * The \p alignment specifies the vertical and horizontal
   * alignment of the item. The default value 0 indicates that the
   * item is stretched to fill the entire grid cell. The alignment can
   * be specified as a logical combination of a horizontal alignment
   * (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or Wt::AlignmentFlag::Right) and a
   * vertical alignment (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or
   * Wt::AlignmentFlag::Bottom).
   *
   * \sa addWidget(WWidget *, int, int, WFlags<AlignmentFlag>)
   */
#ifndef WT_TARGET_JAVA
  void addWidget(std::unique_ptr<WWidget> widget, int row, int column,
		 int rowSpan, int columnSpan,
		 WFlags<AlignmentFlag> alignment);
#else // WT_TARGET_JAVA
  void addWidget(std::unique_ptr<WWidget> widget, int row, int column,
		 int rowSpan, int columnSpan,
		 WFlags<AlignmentFlag> alignment = None);
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \brief Adds a widget to the grid, returning a raw pointer to the widget.
   *
   * This is implemented as:
   * 
   * \code
   * Widget *result = widget.get();
   * addWidget(std::unique_ptr<WWidget>(std::move(widget)), row, column,
   *           rowSpan, columnSpan, alignment);
   * return result;
   * \endcode
   */
  template <typename Widget>
    Widget *addWidget(std::unique_ptr<Widget> widget, int row, int column,
                      int rowSpan, int columnSpan,
                      WFlags<AlignmentFlag> alignment = None)
  {
    Widget *result = widget.get();
    addWidget(std::unique_ptr<WWidget>(std::move(widget)), row, column,
              rowSpan, columnSpan, alignment);
    return result;
  }
#endif // WT_TARGET_JAVA

  /*! \brief Sets the horizontal spacing.
   *
   * The default horizontal spacing is 9 pixels.
   *
   * \sa setVerticalSpacing(int) 
   */
  void setHorizontalSpacing(int size);

  /*! \brief Returns the horizontal spacing.
   *
   * \sa setHorizontalSpacing(int) 
   */
  int horizontalSpacing() const { return grid_.horizontalSpacing_; }

  /*! \brief Sets the vertical spacing.
   *
   * The default vertical spacing is 9 pixels.
   *
   * \sa setHorizontalSpacing(int) 
   */
  void setVerticalSpacing(int size);

  /*! \brief Returns the vertical spacing.
   *
   * \sa setVerticalSpacing(int) 
   */
  int verticalSpacing() const { return grid_.verticalSpacing_; }

  /*! \brief Returns the column count.
   *
   * The grid dimensions change dynamically when adding contents to
   * the grid.
   *
   * \sa rowCount()
   */
  int columnCount() const;

  /*! \brief Returns the row count.
   *
   * The grid dimensions change dynamically when adding contents to
   * the grid.
   *
   * \sa columnCount()
   */
  int rowCount() const;

  /*! \brief Sets the column stretch.
   *
   * Sets the <i>stretch</i> factor for column \p column.
   *
   * \sa columnStretch()
   */
  void setColumnStretch(int column, int stretch);

  /*! \brief Returns the column stretch.
   *
   * \sa setColumnStretch(int, int)
   */
  int columnStretch(int column) const;

  /*! \brief Sets the row stretch.
   *
   * Sets the <i>stretch</i> factor for row \p row.
   *
   * \sa rowStretch()
   */
  void setRowStretch(int row, int stretch);

  /*! \brief Returns the column stretch.
   *
   * \sa setRowStretch(int, int)
   */
  int rowStretch(int row) const;

  /*! \brief Sets whether the user may drag a particular column border.
   *
   * This method sets whether the border that separates column
   * <i>column</i> from the next column may be resized by the user,
   * depending on the value of <i>enabled</i>.
   *
   * The default value is <i>false</i>.
   *
   * If an \p initialSize is given (that is not WLength::Auto), then
   * this size is used for the width of the column, overriding the width
   * it would be given by the layout manager.
   */
  void setColumnResizable(int column, bool enabled = true,
			  const WLength& initialSize = WLength::Auto);

  /*! \brief Returns whether the user may drag a particular column border.
   *
   * This method returns whether the border that separates column
   * <i>column</i> from the next column may be resized by the user.
   *
   * \sa setColumnResizable()
   */
  bool columnIsResizable(int column) const;

  /*! \brief Sets whether the user may drag a particular row border.
   *
   * This method sets whether the border that separates row <i>row</i> from
   * the next row may be resized by the user, depending on the value of
   * <i>enabled</i>.
   *
   * The default value is <i>false</i>.
   *
   * If an \p initialSize is given (that is not WLength::Auto), then
   * this size is used for the height of the row, overriding the height
   * it would be given by the layout manager.
   */
  void setRowResizable(int row, bool enabled = true,
		       const WLength& initialSize = WLength::Auto);

  /*! \brief Returns whether the user may drag a particular row border.
   *
   * This method returns whether the border that separates row
   * <i>row</i> from the next row may be resized by the user.
   *
   * \sa setRowResizable()
   */
  bool rowIsResizable(int row) const;

  virtual void iterateWidgets(const HandleWidgetMethod& method) const override;

private:
  Impl::Grid grid_;

  void expand(int row, int column, int rowSpan, int columnSpan);

  virtual void setParentWidget(WWidget *parent) override;
};

}

#endif // WGRID_LAYOUT_H_
