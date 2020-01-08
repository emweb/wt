// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBORDER_LAYOUT_H_
#define WBORDER_LAYOUT_H_

#include <Wt/WGridLayout.h>

namespace Wt {

/*! \brief Enumeration of possible positions in the layout.
 */
enum class LayoutPosition {
    North,  //!< North (top)
    East,   //!< East (right)
    South,  //!< South (bottom)
    West,   //!< West (left)
    Center  //!< Center
  };


/*! \class WBorderLayout Wt/WBorderLayout.h Wt/WBorderLayout.h
 *  \brief A layout manager which divides the container region in five regions.
 *
 * The five regions are composed of:
 * <pre>
     ------------------------------------
     |              North               |
     ------------------------------------
     |      |                    |      |
     | West |       Center       | East |
     |      |                    |      |
     ------------------------------------
     |              South               |
     ------------------------------------
 * </pre>
 *
 * Each region may hold no more than one widget, and for all but the
 * Center region, the widget is optional.
 *
 * The North, West, East, and South widgets will take their preferred
 * sizes, while the Center widget takes all available remaining space.
 * 
 * \if cpp
 * Usage example:
 * \code
 * auto w = addWidget(std::make_unique<Wt::WContainerWidget>());
 * auto layout = std::make_unique<Wt::WBorderLayout>();
 * layout->addWidget(std::make_unique<Wt::WText>("West-side is best"), Wt::LayoutPosition::West);
 * layout->addWidget(std::make_unique<Wt::WText>("East-side is best"), Wt::LayoutPosition::East);
 * layout->addWidget(std::move(contents), Wt::LayoutPosition::Center);
 *
 * // use layout but do not justify vertically
 * w->setLayout(std::move(layout), Wt::AlignmentFlag::Top | Wt::AlignmentFlag::Justify);
 * \endcode
 * \endif
 */
class WT_API WBorderLayout : public WLayout
{
public:
  /*! \brief Typedef for enum Wt::LayoutPosition */
  typedef LayoutPosition Position;

  /*! \brief Creates a new border layout.
   */
  WBorderLayout();

  /*! \brief Destructor.
   */
  virtual ~WBorderLayout() override;

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

  virtual void addItem(std::unique_ptr<WLayoutItem> item) override;
  virtual std::unique_ptr<WLayoutItem> removeItem(WLayoutItem *item) override;
  virtual WLayoutItem *itemAt(int index) const override;
  virtual int count() const override;

  /*! \brief Adds a widget to the given position.
   *
   * Only one widget per position is supported.
   *
   * \sa add(WLayoutItem *, Position)
   */
  void addWidget(std::unique_ptr<WWidget> widget, LayoutPosition position);

  template <typename Widget>
    Widget *addWidget(std::unique_ptr<Widget> widget, LayoutPosition position)
#ifndef WT_TARGET_JAVA
  {
    Widget *result = widget.get();
    addWidget(std::unique_ptr<WWidget>(std::move(widget)), position);
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

  /*! \brief Adds a layout item to the given position.
   *
   * Only one widget per position is supported.
   */
  void add(std::unique_ptr<WLayoutItem> item, LayoutPosition position);

  /*! \brief Returns the widget at a position.
   *
   * Returns \c 0 if no widget was set for that position.
   */
  WWidget *widgetAt(LayoutPosition position) const;

  /*! \brief Returns the item at a position.
   *
   * Returns \c 0 if no item was set for that position.
   */
  WLayoutItem *itemAt(LayoutPosition position) const;

  /*! \brief Returns the position at which the given layout item is set.
   */
  LayoutPosition position(WLayoutItem *item) const;

  virtual void iterateWidgets(const HandleWidgetMethod& method) const override;

private:
  Impl::Grid grid_;

  const Impl::Grid::Item& itemAtPosition(LayoutPosition position) const;

  Impl::Grid::Item& itemAtPosition(LayoutPosition position);

  virtual void setParentWidget(WWidget *parent) override;
};

}

#endif // WBORDER_LAYOUT_H_
