// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCONTAINER_WIDGET_H_
#define WCONTAINER_WIDGET_H_

#include <Wt/WInteractWidget.h>

namespace Wt {

/*! \brief How to handle overflow of inner content
 */
enum class Overflow {
  Visible=0x0,//!< Show content that overflows.
  Auto=0x1,   //!< Show scrollbars when needed.
  Hidden=0x2, //!< Hide content that overflows.
  Scroll=0x3  //!< Always show scroll bars.
};

class WApplication;
class StdLayoutImpl;

/*! \class WContainerWidget Wt/WContainerWidget.h Wt/WContainerWidget.h
 *  \brief A widget that holds and manages child widgets.
 *
 * A %WContainerWidget acts as a container for child widgets. Child
 * widgets may be added directly to the container or using a layout
 * manager.
 *
 * Use addWidget() or pass the container as constructor argument to a
 * widget to directly add children to the container, without using a
 * layout manager. In that case, CSS-based layout is used, and the
 * resulting display is determined by properties of the children and
 * the container. By default, a %WContainerWidget is displayed as a
 * \link WWidget::setInline() block \endlink and manages its children
 * within a rectangle. Inline child widgets are layed out in lines,
 * wrapping around as needed, while block child widgets are stacked
 * vertically. The container may add padding at the container edges
 * using setPadding(), and provide alignment of contents using
 * setContentAlignment(). A container is rendered by default using a
 * HTML <tt>div</tt> tag, but this may be changed to an HTML
 * <tt>ul</tt> or <tt>ol</tt> tag to make use of other CSS layout
 * techniques, using setList(). In addition, specializations of this
 * class as implemented by WAnchor, WGroupBox, WStackedWidget and
 * WTableCell provide other alternative rendering of the container.
 *
 * When setting the %WContainerWidget \link WWidget::setInline()
 * inline \endlink the container only acts as a conceptual container,
 * offering a common style to its children. Inline children are still
 * layed out inline within the flow of the parent container of this
 * container, as if they were inserted directly into that parent
 * container. Block children are then not allowed (according to the
 * HTML specification).
 *
 * To use a layout manager instead of CSS-based layout, use
 * setLayout() or pass the container as constructor argument to a
 * layout manager. In that case you should not define any padding for
 * the container, and widgets and nested layout managers must be added
 * to the layout manager, instead of to the container directly.
 *
 * Usage example:
 * \if cpp
 * \code
 *
 * // Example 1:
 * // Instantiate a container widget and add some children whose layout 
 * // is governed based on HTML/CSS rules.
 * auto container1 = std::make_unique<Wt::WContainerWidget>();
 * container1->addWidget(std::make_unique<Wt::WText>("Some text"));
 * container1->addWidget(std::make_unique<Wt::WImage>("images/img.png"));
 * Wt::WContainerWidget *child3 = container1->addWidget(std::make_unique<Wt::WContainerWidget>());
 *
 * // Example 2:
 * // Instantiate a container widget which uses a layout manager
 * auto container2 = std::make_unique<Wt::WContainerWidget>();
 * container2->resize(Wt::WLength::Auto, 600); // give the container a fixed height.
 *
 * auto layout = std::make_unique<Wt::WVBoxLayout>();
 * layout->addWidget(std::make_unique<Wt::WText>("Some text"));
 * layout->addWidget(std::make_unique<Wt::WImage>("images/img.png"));
 *
 * container2->setLayout(std::move(layout));      // set the layout to the container.
 * \endcode
 * \elseif java
 * \code
 * // Example 1:
 * // Instantiate a container widget and add some children whose layout 
 * // is governed based on HTML/CSS rules.
 * WContainerWidget container1 = new WContainerWidget();
 * container1.addWidget(new WText("Some text"));
 * container1.addWidget(new WImage("images/img.png"));
 * WContainerWidget child3 = new WContainerWidget(container1);
		 
 * // Example 2:
 * // Instantiate a container widget which uses a layout manager
 * WContainerWidget container2 = new WContainerWidget();
 * // give the container a fixed height
 * container2.resize(WLength.Auto, new WLength(600)); 
 *
 * WVBoxLayout layout = new WVBoxLayout();
 * layout.addWidget(new WText("Some text"));
 * layout.addWidget(new WImage("images/img.png"));
		 
 * container2.setLayout(layout);      // set the layout to the container.
 * \endcode
 * \endif
 *
 * When using a layout manager, you need to carefully consider the
 * alignment of the layout manager with respect to the container: when
 * the container's height is unconstrained (not specified explicitly
 * using resize() or a style class, and the container is not included
 * in a layout manager), you should pass AlignmentFlag::Top to setLayout().
 *
 * <h3>CSS</h3>
 *
 * Depending on its configuration and usage, the widget corresponds to the
 * following HTML tags:
 *  - By default, the widget corresponds to a <tt>&lt;div&gt;</tt> tag.
 *  - When configured with setInline(true), the widget corresponds to a
 *    <tt>&lt;span&gt;</tt>.
 *  - When configured with setList(true), the widget corresponds to a
 *    <tt>&lt;ul&gt;</tt>.
 *  - When configured with setList(true, true), the widget corresponds to a
 *    <tt>&lt;ol&gt;</tt>.
 *  - When inserted into a container widget that isList(), the widget
 *    corresponds to a <tt>&lt;li&gt;</tt>.
 *
 * This widget does not provide styling, and can be styled using
 * inline or external CSS as appropriate.
 */
class WT_API WContainerWidget : public WInteractWidget
{
public:
  /*! \brief Creates a container.
   */
  WContainerWidget();

  /*! \brief Destructor.
   */
  ~WContainerWidget();

  /*! \brief Sets a layout manager for the container.
   *
   * Note that you can nest layout managers inside each other, to
   * create a complex layout hierarchy.
   *
   * If a previous layout manager was already set, it is first deleted.
   * In that case, you will need to make sure that you either readd all
   * widgets that were part of the previous layout to the new layout, or
   * delete them, to avoid memory leaks.
   *
   * \sa layout()
   */
  void setLayout(std::unique_ptr<WLayout> layout);

  /*! \brief Sets a layout manager for the container, returning a raw pointer.
   *
   * This is implemented as:
   *
   * \code
   * Layout *result = layout.get();
   * setLayout(std::unique_ptr<WLayout>(std::move(layout)));
   * return result;
   * \endcode
   *
   * This is a useful shorthand that allows to create a layout and get a raw
   * pointer to it in one line.
   */
  template <typename Layout>
    Layout *setLayout(std::unique_ptr<Layout> layout)
#ifndef WT_TARGET_JAVA
  {
    Layout *result = layout.get();
    setLayout(std::unique_ptr<WLayout>(std::move(layout)));
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

  /*! \brief Returns the layout manager that was set for the container.
   *
   * If no layout manager was previously set using setLayout(WLayout
   * *), 0 is returned.
   *
   * \sa setLayout(WLayout *)
   */
  WLayout *layout() const { return layout_.get(); }

  /*! \brief Adds a child widget to this container.
   *
   * The widget is appended to the list of
   * children, and thus also layed-out at the end.
   *
   * If, for some reason, you want to be in control of the lifetime
   * of the widget, you can retrieve a unique_ptr with WObject::removeChild()
   */
  virtual void addWidget(std::unique_ptr<WWidget> widget);

  /*! \brief Adds a child widget to this container, returning a raw pointer.
   *
   * This is implemented as:
   *
   * \code
   * Widget *result = widget.get();
   * addWidget(std::unique_ptr<WWidget>(std::move(widget)));
   * return result;
   * \endcode
   *
   * This is a useful shorthand to add a widget and get a raw pointer to
   * it, e.g.:
   *
   * \code
   * Wt::WPushButton *button = container->addWidget(std::make_unique<Wt::WPushButton>("Click me!"));
   * // do something with button, including safely using it in a lambda function
   * \endcode
   */
  template <typename Widget>
    Widget *addWidget(std::unique_ptr<Widget> widget)
#ifndef WT_TARGET_JAVA
  {
    Widget *result = widget.get();
    addWidget(std::unique_ptr<WWidget>(std::move(widget)));
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA

  /*! \brief Creates a widget and adds it, returning a reference to it.
   *
   * This is a useful shorthand for adding a new widget to the container, and
   * getting a reference to it, e.g.:
   *
   * \code
   * Wt::WPushButton *button = container->addNew<Wt::WPushButton>("Click me!");
   * // do something with button, including safely using it in a lambda function
   * \endcode
   *
   * The implementation is equivalent to the following code:
   *
   * \code
   * std::unique_ptr<Widget> w{new Widget(std::forward<Args>(args)...)};
   * Widget *result = w.get();
   * addWidget(std::unique_ptr<WWidget>{std::move(w)});
   * return result;
   * \endcode
   *
   * There's an exception for global widgets, like WPopupWidget. In this case
   * the implementation is equivalent to:
   *
   * \code
   * std::unique_ptr<Widget> w{new Widget(std::forward<Args>(args)...)};
   * Widget *result = w.get();
   * addChild(std::unique_ptr<WObject>(std::move(w)));
   * return result;
   * \endcode
   *
   * Since popup widgets (and similar widgets) are always and automatically placed
   * in a global location in the widget tree, only their ownership is transferred.
   */
  template <typename Widget, typename ...Args>
  Widget *addNew( Args&& ...args )
  {
    std::unique_ptr<Widget> w{new Widget(std::forward<Args>(args)...)};
    Widget *result = w.get();
    if (w->isGlobalWidget()) {
      addChild(std::unique_ptr<WObject>{std::move(w)});
    } else {
      addWidget(std::unique_ptr<WWidget>{std::move(w)});
    }
    return result;
  }
#else // WT_TARGET_JAVA
  template <typename Widget>
    Widget *addNew();
  template <typename Widget, typename Arg1>
    Widget *addNew(Arg1 arg1);
  template <typename Widget, typename Arg1, typename Arg2>
    Widget *addNew(Arg1 arg1, Arg2 arg2);
  template <typename Widget, typename Arg1, typename Arg2, typename Arg3>
    Widget *addNew(Arg1 arg1, Arg2 arg2, Arg3 arg3);
  template <typename Widget, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    Widget *addNew(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4);
  template <typename Widget, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    Widget *addNew(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5);
#endif // WT_TARGET_JAVA

  /*! \brief Inserts a child widget in this container, before another
   *         widget.
   *
   * The <i>widget</i> is inserted at the place of the \p before
   * widget, and subsequent widgets are shifted.
   *
   * If, for some reason, you want to be in control of the lifetime of
   * the widget, you can regain ownership of the widget (without any
   * functional implication) using WObject::removeChild()
   *
   * \sa insertWidget(int index, WWidget *widget);
   */
  virtual void insertBefore(std::unique_ptr<WWidget> widget, WWidget *before);

  /*! \brief Inserts a child widget to this container, before another widget, returning a raw pointer.
   *
   * This is implemented as:
   *
   * \code
   * Widget *result = widget.get();
   * insertBefore(std::unique_ptr<WWidget>(std::move(widget)), before);
   * return result;
   * \endcode
   */
  template <typename Widget>
    Widget *insertBefore(std::unique_ptr<Widget> widget, WWidget *before)
#ifndef WT_TARGET_JAVA
  {
    Widget *result = widget.get();
    insertBefore(std::unique_ptr<WWidget>(std::move(widget)), before);
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

  /*! \brief Inserts a child widget in this container at given index.
   *
   * The <i>widget</i> is inserted at the given \p index, and
   * subsequent widgets are shifted.
   *
   * If, for some reason, you want to be in control of the lifetime of
   * the widget, you can regain ownership of the widget (without any
   * functional implication) using WObject::removeChild()
   *
   * \sa insertBefore(WWidget *widget, WWidget *before);
   */
  virtual void insertWidget(int index, std::unique_ptr<WWidget> widget);

  /*! \brief Inserts a child widget to this container at given index, returning a raw pointer.
   *
   * This is implemented as:
   *
   * \code
   * Widget *result = widget.get();
   * insertWidget(index, std::unique_ptr<WWidget>(std::move(widget)));
   * return result;
   * \endcode
   */
  template <typename Widget>
    Widget *insertWidget(int index, std::unique_ptr<Widget> widget)
#ifndef WT_TARGET_JAVA
  {
    Widget *result = widget.get();
    insertWidget(index, std::unique_ptr<WWidget>(std::move(widget)));
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \brief Creates a widget and inserts it, returning a reference to it.
   *
   * This is implemented as:
   *
   * \code
   * std::unique_ptr<Widget> w{new Widget(std::forward<Args>(args)...)};
   * Widget *result = w.get();
   * insertWidget(index, std::unique_ptr<WWidget>{std::move(w)});
   * return result;
   * \endcode
   */
  template <typename Widget, typename ...Args>
    Widget *insertNew(int index, Args&& ...args)
  {
    std::unique_ptr<Widget> w{new Widget(std::forward<Args>(args)...)};
    Widget *result = w.get();
    insertWidget(index, std::unique_ptr<WWidget>{std::move(w)});
    return result;
  }
#endif // WT_TARGET_JAVA

  using WWidget::removeWidget;

  /*! \brief Removes a child widget from this container.
   *
   * If the WContainerWidget owns the given widget (i.e. if it was added
   * with addWidget() or insertWidget() and not removed with WObject::removeChild()),
   * a unique_ptr to this widget is returned. Otherwise, this returns nullptr.
   */
  virtual std::unique_ptr<WWidget> removeWidget(WWidget *widget) override;

  /*! \brief Removes all widgets.
   *
   * This removes all children that have been added to this container.
   * If a layout was set, also the layout manager is cleared.
   */
  virtual void clear();

  /*! \brief Returns the index of a widget.
   */
  virtual int indexOf(WWidget *widget) const;

  /*! \brief Returns the widget at <i>index</i>
   */
  virtual WWidget *widget(int index) const;

  /*! \brief Returns the number of widgets in this container.
   */
  virtual int count() const;

  /*! \brief Specifies how child widgets must be aligned within the container
   *
   * For a WContainerWidget, only specifes the horizontal alignment of
   * child widgets. Note that there is no way to specify vertical
   * alignment: children are always pushed to the top of the
   * container.
   *
   * For a WTableCell, this may also specify the vertical alignment.
   * The default alignment is (Wt::AlignmentFlag::Top | Wt::AlignmentFlag::Left).
   */
  void setContentAlignment(WFlags<AlignmentFlag> contentAlignment);

  /*! \brief Sets padding inside the widget
   *
   * Setting padding has the effect of adding distance between the
   * widget children and the border.
   */
  void setPadding(const WLength& padding, WFlags<Side> sides = AllSides);

  /*! \brief Returns the padding set for the widget.
   *
   * \sa setPadding(const WLength&, WFlags<Side>)
   */
  WLength padding(Side side) const;

  /*! \brief Returns the alignment of children
   *
   * \sa setContentAlignment(WFlags<AlignmentFlag>)
   */
  WFlags<AlignmentFlag> contentAlignment() const { return contentAlignment_; }

  /*! \brief Sets how overflow of contained children must be handled.
   */
  void setOverflow(Overflow overflow, WFlags<Orientation> orientation = 
		   (Orientation::Horizontal | Orientation::Vertical));

  /*! \brief Renders the container as an HTML list.
   *
   * Setting \p renderList to \c true will cause the container to be
   * using an HTML <tt>&lt;ul&gt;</tt> or <tt>&lt;ol&gt;</tt> type,
   * depending on the value of \p orderedList. This must be set
   * before the initial render of the container. When set, any
   * contained WContainerWidget will be rendered as an HTML
   * <tt>&lt;li&gt;</tt>. Adding non-WContainerWidget children results
   * in unspecified behaviour.
   *
   * Note that CSS default layout rules for <tt>&lt;ul&gt;</tt> and
   * <tt>&lt;ol&gt;</tt> add margin and padding to the container,
   * which may look odd if you do not use bullets.
   *
   * By default, a container is rendered using a <tt>&lt;div&gt;</tt>
   * element.
   *
   * \sa isList(), isOrderedList(), isUnorderedList()
   */
  void setList(bool list, bool ordered = false);

  /*! \brief Returns if this container is rendered as a List
   *
   * \sa setList(), isOrderedList(), isUnorderedList()
   */  
  bool isList() const;

  /*! \brief Returns if this container is rendered as an Unordered List
   *
   * \sa setList(), isList(), isOrderedList()
   */  
  bool isUnorderedList() const;

  /*! \brief Returns if this container is rendered as an Ordered List
   *
   * \sa setList(), isList(), isUnorderedList()
   */  
  bool isOrderedList() const;

  /*! \brief Event signal emitted when scrolling in the widget.
   *
   * This event is emitted when the user scrolls in the widget (for setting
   * the scroll bar policy, see setOverflow()). The event conveys details
   * such as the new scroll bar position, the total contents height and the
   * current widget height.
   *
   * \sa setOverflow()
   */
  EventSignal<WScrollEvent>& scrolled();

  /*! \brief return the number of pixels the container is scrolled horizontally
   *
   * This value is only set if setOverflow() has been called 
   *
   * \sa setOverflow()
   * \sa scrollLeft();
   */
  int scrollTop() const { return scrollTop_;}

  /*! \brief return the number of pixels the container is scrolled vertically
   *
   * This value is only set if setOverflow() has been called 
   *
   * \sa setOverflow()
   * \sa scrollTop();
   */
  int scrollLeft() const { return scrollLeft_;}

  void setGlobalUnfocused(bool b);

  bool isGlobalUnfocussed() const;

private:
  static const char *SCROLL_SIGNAL;

  static const int BIT_CONTENT_ALIGNMENT_CHANGED = 0;
  static const int BIT_PADDINGS_CHANGED = 1;
  static const int BIT_OVERFLOW_CHANGED = 2;
  static const int BIT_ADJUST_CHILDREN_ALIGN = 3;
  static const int BIT_LIST = 4;
  static const int BIT_ORDERED_LIST = 5;
  static const int BIT_LAYOUT_NEEDS_RERENDER = 6;
  static const int BIT_LAYOUT_NEEDS_UPDATE = 7;


  /*
   * Frequently used attributes.
   */
  std::bitset<8> flags_;
  WFlags<AlignmentFlag> contentAlignment_;
  Overflow *WT_ARRAY overflow_;
  WLength *WT_ARRAY padding_;
  std::vector<WWidget *> children_;
  std::unique_ptr<WLayout> layout_;

  std::unique_ptr<std::vector<WWidget *> > addedChildren_;

  bool globalUnfocused_;
  int scrollTop_, scrollLeft_;

  bool wasEmpty() const;

  void rootAsJavaScript(WApplication *app, WStringStream& out, bool all);

  friend class WebRenderer;

protected:
  virtual int firstChildIndex() const;

  virtual void childResized(WWidget *child, WFlags<Orientation> directions)
    override;
  virtual void parentResized(WWidget *parent, WFlags<Orientation> directions)
    override;
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app) override;
  virtual void iterateChildren(const HandleWidgetMethod& method) const override;

  DomElement *createDomElement(WApplication *app, bool addChildren);

  void createDomChildren(DomElement& parent, WApplication *app);
  void updateDomChildren(DomElement& parent, WApplication *app);

  virtual DomElementType domElementType() const override;
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void propagateRenderOk(bool deep) override;
  virtual DomElement *createDomElement(WApplication *app) override;

  StdLayoutImpl *layoutImpl() const;
  virtual void setFormData(const FormData& formData) override;

  friend class StdLayoutImpl;
  friend class StdGridLayoutImpl2;
  friend class FlexLayoutImpl;

private:
  void propagateLayoutItemsOk(WLayoutItem *item);
  void layoutChanged(bool rerender);

  friend class WImage;
  friend class WTableCell;
};

}

#endif // WCONTAINER_WIDGET_H_
