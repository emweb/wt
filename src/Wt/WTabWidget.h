// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTABWIDGET_H_
#define WTABWIDGET_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

namespace Wt {

  class WMenu;
  class WMenuItem;
  class WStackedWidget;

/*! \class WTabWidget Wt/WTabWidget.h Wt/WTabWidget.h
 *  \brief A widget that organizes contents in tab panes.
 *
 * This widget combines a horizontal WMenu with a WStackedWidget, and a
 * tab-like look.
 *
 * A tab widget will place the tab bar on top of the contents, and fit the
 * contents below it.
 *
 * Usage example:
 * \if cpp
 * \code
 * Wt::WTabWidget *examples = addWidget(std::make_unique<Wt::WTabWidget>());
 *
 * examples->addTab(helloWorldExample(), "Hello World");
 * examples->addTab(chartExample(), "Charts");
 * examples->addTab(std::make_unique<Wt::WText>("A WText"), "WText");
 *
 * examples->currentChanged().connect(this, &MyClass::logInternalPath);
 * examples->setInternalPathEnabled();
 * examples->setInternalBasePath("/examples");
 * \endcode
 * \elseif java
 * \code
 * WTabWidget examples = new WTabWidget(this);
 *	 
 * examples.addTab(helloWorldExample(), "Hello World");
 * examples.addTab(chartExample(), "Charts");
 * examples.addTab(new WText("A WText"), "WText");
 *	 
 * examples.currentChanged().addListener(this, new Signal.Listener(){
 *	public void trigger() {
 *		//custom code
 *	}
 *  });
 * examples.setInternalPathEnabled();
 * examples.setInternalBasePath("/examples");		
 * \endcode
 * \endif
 *
 * <h3>CSS</h3>
 *
 * The tab widget is styled by the current CSS theme.
 * 
 * <TABLE border="0" align="center"> <TR> <TD> 
 * \image html WTabWidget-default-1.png "An example WTabWidget (default)"
 * </TD> <TD>
 * \image html WTabWidget-polished-1.png "An example WTabWidget (polished)"
 * </TD> </TR> </TABLE>
 */
class WT_API WTabWidget : public WCompositeWidget
{
public:
  /*! \brief Creates a new tab widget
   */
  WTabWidget();

  /*! \brief Adds a new tab, with <i>child</i> as content, and the given label.
   *
   * Returns the menu item that implements the tab item.
   */
  WMenuItem *addTab(std::unique_ptr<WWidget> child,
		    const WString& label,
		    ContentLoading loadPolicy = ContentLoading::Lazy);

  /*! \brief Inserts a new tab, with <i>child</i> as content, and the given label.
   *
   * Returns the menu item that implements the tab item.
   */
  WMenuItem *insertTab(int index, std::unique_ptr<WWidget> child, const WString& label,
                       ContentLoading loadPolicy = ContentLoading::Lazy);

  /*! \brief Removes a tab item.
   *
   * \sa WMenu::removeItem()
   */
  std::unique_ptr<WWidget> removeTab(WWidget *widget);

  /*! \brief Returns the number of tabs.
   */
  int count() const;

  /*! \brief Returns the content widget at the given tab <i>index</i>.
   */
  WWidget *widget(int index) const;

  /*! \brief Returns the item at the given tab <i>index</i>.
   */
  WMenuItem *itemAt(int index) const;

  /*! \brief Returns the index of the tab of the given content widget.
   *
   * If the widget is not in this tab widget, then -1 is returned.
   */
  int indexOf(WWidget *widget) const;

  /*! \brief Activates the tab at <i>index</i>.
   */
  void setCurrentIndex(int index);

  /*! \brief Returns the index of the activated tab.
   */
  int currentIndex() const;

  /*! \brief Activates the tab showing the given <i>widget</i>
   */
  void setCurrentWidget(WWidget *widget);

  /*! \brief Returns the widget of the activated tab.
   */
  WWidget *currentWidget() const;

  /*! \brief Returns the item of the activated tab.
   */
  WMenuItem *currentItem() const;

  /*! \brief Enables or disables a tab.
   *
   * Enables or disables the tab at \p index. A disabled tab cannot be
   * activated.
   */
  void setTabEnabled(int index, bool enable);

  /*! \brief Returns whether a tab is enabled.
   *
   * \sa WMenu::enableItem(), WMenu::disableItem()
   */
  bool isTabEnabled(int index) const;

  /*! \brief Hides or shows a tab.
   *
   * Hides or shows the tab at \p index.
   */
  void setTabHidden(int index, bool hidden);

  /*! \brief Returns whether a tab is hidden.
   */
  bool isTabHidden(int index) const;

  /*! \brief Make it possible to close a tab interactively or by
   * \link WTabWidget::closeTab() closeTab\endlink.
   *
   * A tab that has been closed is marked as hidden, but not removed
   * from the menu.
   *
   * \sa removeTab()
   */
  void setTabCloseable(int index, bool closeable);

  /*! \brief Returns whether a tab is closeable.
   *
   * \sa setTabCloseable()
   */
  bool isTabCloseable(int index);

  /*! \brief Changes the label for a tab.
   */
  void setTabText(int index, const WString& label);

  /*! \brief Returns the label for a tab.
   *
   * \sa setTabText()
   */
  WString tabText(int index) const;

  /*! \brief Sets the tooltip for a tab.
   *
   * The tooltip is shown when the user hovers over the label.
   */
  void setTabToolTip(int index, const WString& tip);

  /*! \brief Returns the tooltip for a tab.
   *
   * \sa setTabToolTip()
   */
  WString tabToolTip(int index) const;

  /*! \brief Enables internal paths for items.
   *
   * \copydetails WMenu::setInternalPathEnabled
   */
  void setInternalPathEnabled(const std::string& basePath = "");

  /*! \brief Returns whether internal paths are enabled.
   *
   * \copydetails WMenu::internalPathEnabled
   */
  bool internalPathEnabled() const;

  /*! \brief Sets the internal base path.
   *
   * \copydetails WMenu::setInternalBasePath
   */
  void setInternalBasePath(const std::string& path);

  /*! \brief Returns the internal base path.
   *
   * \copydetails WMenu::internalBasePath
   */
  const std::string& internalBasePath() const;

  /*! \brief %Signal emitted when the user activates a tab.
   *
   * The index of the newly activated tab is passed as an argument.
   */
  Signal<int>& currentChanged() { return currentChanged_; }

  /*! \brief Closes a tab at \p index.
   *
   * A tab that has been closed is marked as hidden, but not removed
   * from the menu.
   *
   * \sa removeTab(), setTabHidden()
   */
  void closeTab(int index);

  /*! \brief %Signal emitted when the user closes a tab.
   *
   * The index of the closed tab is passed as an argument.
   *
   * \sa closeTab(), setTabCloseable()
   */
  Signal<int>& tabClosed() { return tabClosed_; }

  /*! \brief Returns the contents stack.
   *
   * The tab widget is implemented as a WMenu + WStackedWidget which
   * displays the contents. This method returns a reference to this
   * contents stack.
   */
  WStackedWidget *contentsStack() const;
  
  /*! \brief Sets how overflow of contained children must be handled.
   */
  void setOverflow(Overflow overflow,
		   WFlags<Orientation> orientation 
		   = (Orientation::Horizontal | Orientation::Vertical));


private:
  Signal<int> currentChanged_;
  Signal<int> tabClosed_;
  WContainerWidget *layout_;
  WMenu            *menu_;

  std::vector<WWidget *> contentsWidgets_;

  void create();
  void onItemSelected(WMenuItem *item);
  void onItemClosed(WMenuItem *item);

  void setJsSize();
};

}

#endif // WTABWIDGET_H_
