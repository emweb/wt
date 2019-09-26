// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLAYOUT_H_
#define WLAYOUT_H_

#include <string>
#include <vector>

#include <Wt/WGlobal.h>
#include <Wt/WLayoutImpl.h>
#include <Wt/WLayoutItem.h>
#include <Wt/WObject.h>

namespace Wt {

class WWidgetItem;
class WLayoutImpl;

/*! \brief Enumeration to indicate which layout implementation to use.
 *
 * \sa WLayout::setPreferredImplementation()
 * \sa WLayout::setDefaultImplementation()
 */
enum class LayoutImplementation {
  Flex, //!< Use CSS flex layout (if supported by the browser)
  JavaScript //!< Uses the classic JavaScript-based method. In some cases flex layout may fail when nesting multiple layouts, so this method can be used instead
};

/*! \class WLayout Wt/WLayout.h Wt/WLayout.h
 *  \brief An abstract base class for layout managers.
 *
 * This class is the abstract base class for any layout manager. A layout
 * manager is associated with a container widget, and manages the layout of
 * children inside the whole space available to the container widget.
 *
 * The implementation of the layout manager depends on the container widget
 * to which it is set, and is therefore deferred to WLayoutImpl.
 *
 * \note When applying a layout manager to a WContainerWidget, you may
 * not define any padding for the container widget. Instead, use
 * setContentsMargins().
 */
class WT_API WLayout : public WLayoutItem, public WObject
{
public:
  /*! \brief Destructor.
   *
   * This will delete the layout (and nested layouts), but not the
   * contained widgets.
   */
  virtual ~WLayout();

  /*! \brief Set the preferred layout implementation.
   *
   * The default implementation for box layouts and fit layouts is
   * \link LayoutImplementation::Flex Flex\endlink
   * (if supported by the browser). Otherwise a fallback to
   * \link LayoutImplementation::JavaScript JavaScript\endlink
   * is used.
   *
   * \sa setDefaultImplementation()
   */
  void setPreferredImplementation(LayoutImplementation implementation);

  /*! \brief Sets the preferred layout implementation globally.
   *
   * The default implementation for box layouts and fit layouts is
   * \link LayoutImplementation::Flex Flex\endlink
   * (if supported by the browser). Otherwise a fallback to
   * \link LayoutImplementation::JavaScript JavaScript\endlink
   * is used.
   *
   * Because there are cases where \link LayoutImplementation::Flex Flex\endlink
   * does not work properly, this method can be used to set the global
   * preferred implementation to \link LayoutImplementation::JavaScript\endlink
   * instead.
   *
   * Since this is a system-wide setting, and not a per-session setting,
   * you should call this function before any session is created, e.g. in
   * main() before calling WRun().
   *
   * \sa setPreferredImplementation()
   */
  static void setDefaultImplementation(LayoutImplementation implementation);

  /*! \brief Adds a layout <i>item</i>.
   *
   * The item may be a widget or nested layout.
   *
   * How the item is layed out with respect to siblings is
   * implementation specific to the layout manager. In some cases, a
   * layout manager will overload this method with extra arguments
   * that specify layout options.
   *
   * \sa removeItem(WLayoutItem *), addWidget(WWidget *)
   */
  virtual void addItem(std::unique_ptr<WLayoutItem> item) = 0;

  /*! \brief Adds the given <i>widget</i> to the layout.
   *
   * This method wraps the widget in a WWidgetItem and calls
   * addItem(WLayoutItem *).
   *
   * How the widget is layed out with respect to siblings is
   * implementation specific to the layout manager. In some cases, a
   * layout manager will overload this method with extra arguments
   * that specify layout options.
   *
   * \sa removeWidget(WWidget *), addItem(WLayoutItem *)
   */
  void addWidget(std::unique_ptr<WWidget> widget);

  /*! \brief Removes a layout <i>item</i> (widget or nested layout).
   *
   * \sa addItem(WLayoutItem *), removeWidget(WWidget *)
   */
  virtual std::unique_ptr<WLayoutItem> removeItem(WLayoutItem *item) = 0;

  /*! \brief Removes the given <i>widget</i> from the layout.
   *
   * This method finds the corresponding WWidgetItem and calls
   * removeItem(WLayoutItem *), and returns the widget.
   *
   * \sa addWidget(WWidget *), removeItem(WLayoutItem *)
   */
  std::unique_ptr<WWidget> removeWidget(WWidget *widget);

  /*! \brief Returns the number of items in this layout.
   *
   * This may be a theoretical number, which is greater than the
   * actual number of items. It can be used to iterate over the items
   * in the layout, in conjunction with itemAt().
   */
  virtual int count() const = 0;

  /*! \brief Returns the layout item at a specific <i>index</i>.
   *
   * If there is no item at the \p index, \c 0 is returned.
   *
   * \sa indexOf(WLayoutItem *) const, count()
   */
  virtual WLayoutItem *itemAt(int index) const = 0;

  /*! \brief Returns the index of a given <i>item</i>.
   *
   * The default implementation loops over all items, and returns the
   * index for which itemAt(index) equals \p item.
   *
   * \sa itemAt(int) const
   */
  virtual int indexOf(WLayoutItem *item) const;

  /*! \brief Finds the widget item associated with the given <i>widget</i>.
   */
  virtual WWidgetItem *findWidgetItem(WWidget *widget) override;

  virtual WWidget *widget() override { return nullptr; }
  virtual WLayout *layout() override { return this; }
  virtual WLayout *parentLayout() const override;
  virtual WWidget *parentWidget() const override;

  virtual WLayoutImpl *impl() const override { return impl_.get(); }

  /*! \brief Set contents margins (in pixels).
   *
   * The default contents margins are 9 pixels in all directions.
   *
   * \note Only used when the layout manager is applied to a WContainerWidget.
   *
   * \sa setContentsMargins()
   */
  void setContentsMargins(int left, int top, int right, int bottom);

#ifndef WT_TARGET_JAVA
  /*! \brief Returns the contents margins.
   *
   * \sa setContentsMargins()
   */
  void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
#else // WT_TARGET_JAVA
  /*! \brief Returns a contents margin.
   *
   * \sa setContentsMargins()
   */
  int getContentsMargin(Side side) const;
#endif // WT_TARGET_JAVA

  // true if flex is preferred and supported (by the layout and the browser)
  virtual bool implementationIsFlexLayout() const;

protected:
  /*! \brief Create a layout.
   */
  WLayout();

  /*! \brief Update the layout.
   *
   * Must be called whenever some properties of the layout have changed.
   */
  void update(WLayoutItem *item = nullptr);

  void itemAdded(WLayoutItem *item);
  void itemRemoved(WLayoutItem *item);

  virtual void setParentWidget(WWidget *parent) override;
  void setImpl(std::unique_ptr<WLayoutImpl> impl);

  LayoutImplementation implementation() const;
  LayoutImplementation preferredImplementation() const;
  virtual void updateImplementation();

private:
  WLayout *parentLayout_;
  WWidget *parentWidget_;
  int margins_[4];
  std::unique_ptr<WLayoutImpl> impl_;
  LayoutImplementation preferredImplementation_;
  static LayoutImplementation defaultImplementation_;

  virtual void setParentLayout(WLayout *parentLayout) override;

  friend class WWidget;
  friend class WContainerWidget;
};

}

#endif // WLAYOUT_H_
