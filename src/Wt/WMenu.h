// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMENU_H_
#define WMENU_H_

#include <vector>

#include <Wt/WCompositeWidget.h>
#include <Wt/WMenuItem.h>

namespace Wt {

/*! \class WMenu Wt/WMenu.h Wt/WMenu.h
 *  \brief A widget that shows a menu of options.
 *
 * The %WMenu widget offers menu navigation.
 *
 * Typically, a menu is used in conjunction with a WStackedWidget (but
 * can be used without too), where different 'contents' are stacked
 * upon each other. Each choice in the menu (which is implemented as a
 * WMenuItem) corresponds to a tab in the contents stack. The contents
 * stack may contain other items, and could be shared with other WMenu
 * instances.
 *
 * When used without a contents stack, you can react to menu item
 * selection using the itemSelected() signal, to implement some custom
 * handling of item selection.
 *
 * Usage example:
 * \if cpp
 * \code
 * // create the stack where the contents will be located
 * auto contents = contentsParent->addWidget(std::make_unique<Wt::WStackedWidget>());
 *
 * // create a menu
 * auto menu = std::make_unique<Wt::WMenu>(contents);
 *
 * // add four items using the default lazy loading policy.
 * menu->addItem("Introduction", std::make_unique<Wt::WText>("intro"));
 * menu->addItem("Download", std::make_unique<Wt::WText>("Not yet available"));
 * menu->addItem("Demo", std::make_unique<DemoWidget>());
 * menu->addItem(std::make_unique<Wt::WMenuItem>("Demo2", std::make_unique<DemoWidget>()));
 * \endcode
 * \elseif java
 * \code
 * // create the stack where the contents will be located
 * WStackedWidget contents = new WStackedWidget(contentsParent);
 *		 
 * // create a menu
 * WMenu menu = new WMenu(contents);
 *		 
 * // add four items using the default lazy loading policy.
 * menu.addItem("Introduction", new WText("intro"));
 * menu.addItem("Download", new WText("Not yet available"));
 * menu.addItem("Demo", new DemoWidget());
 * menu.addItem(new WMenuItem("Demo2", new DemoWidget()));
 * \endcode
 * \endif
 *
 * After construction, the first entry will be selected. At any time,
 * it is possible to select a particular item using select().
 *
 * Each item of %WMenu may be closeable (see WMenuItem::setCloseable(bool).
 * Like selection, at any time, it is possible to close a particular item
 * using close(). You can react to close of item by using the itemClosed()
 * signal.
 *
 * The %WMenu implementation offers fine-grained control on how
 * contents should be preloaded. By default, all contents is
 * lazy-loaded, only when needed. To improve response time, an item
 * may also be preloaded (using addItem()). In that case, the item
 * will be loaded in the background, before its first use. In any
 * case, once the contents corresponding to a menu item is loaded,
 * subsequent navigation to it is handled entirely client-side.
 *
 * The %WMenu may participate in the application's internal path,
 * which lets menu items correspond to internal URLs, see
 * setInternalPathEnabled().
 *
 * The look of the items may be defined through style sheets. The default
 * WMenuItem implementation uses four style classes to distinguish
 * between inactivated, activated, closeable inactivated and closeable
 * activated menu items: <tt>"item"</tt>, <tt>"itemselected"</tt>,
 * <tt>"citem"</tt>, <tt>"citemselected"</tt>. By using CSS nested selectors,
 * a different style may be defined for items in a different menu.
 *
 * You may customize the rendering and behaviour of menu entries by
 * specializing WMenuItem.
 *
 * <h3>CSS</h3>
 *
 * The menu is rendered as a &lt;ul&gt;. Unless you use the bootstrap
 * theme, you will want to customize the menu using inline or external
 * styles to hide the bullets and provide the appropriate horizontal
 * or vertical layout.
 *
 * \sa WMenuItem
 */
class WT_API WMenu : public WCompositeWidget
{
public:
  /*! \brief Creates a new menu.
   *
   * The menu is not associated with a contents stack, and thus you
   * will want to react to the itemSelected() signal to react to menu
   * changes.
   */
  WMenu();

  /*! \brief Creates a new menu.
   *
   * Construct a menu to manage the widgets in \p contentsStack.
   *
   * Each menu item will manage a single widget in the
   * \p contentsStack, making it the current widget when the menu
   * item is activated.
   */
  WMenu(WStackedWidget *contentsStack);

  /*! \brief Destructor.
   */
  ~WMenu();

  /*! \brief Adds an item.
   *
   * Use this version of addItem() if you do not want to specify an icon
   * for this menu item.
   *
   * Returns the corresponding WMenuItem.
   *
   * \sa addItem(const std::string&, const WString&, WWidget *, ContentLoading)
   * \sa addItem(WMenuItem *)
   */
  WMenuItem *addItem(const WString& label,
		     std::unique_ptr<WWidget> contents = nullptr,
		     ContentLoading policy 
		       = ContentLoading::Lazy);

  /*! \brief Adds an item.
   *
   * Adds a menu item with given \p contents, which is added to the
   * menu's associated contents stack.
   *
   * \p contents may be \c 0 for two reasons:
   * - if the menu is not associated with a contents stack, then you cannot
   *   associate a menu item with a contents widget
   * - or, you may have one or more items which which are not associated with
   *   a contents widget in the contents stack.
   *
   * Returns the corresponding WMenuItem.
   *
   * \sa addItem(WMenuItem *)
   */
  WMenuItem *addItem(const std::string& iconPath, const WString& label,
		     std::unique_ptr<WWidget> contents = nullptr,
		     ContentLoading policy 
		       = ContentLoading::Lazy);

    /*! \brief Adds an item with given text, and specify a slot method to be
   *         called when the item is triggered.
   *
   * The <i>target</i> and \p method are connected to the
   * WMenuItem::triggered signal.
   *
   * \sa add(WMenuItem *)
   */
  template<class T, class V>
    WMenuItem *addItem(const WString& text, T *target, void (V::*method)());
    
  /*! \brief Adds an item with given text and icon, and specify a slot
   *         method to be called when activated.
   *
   * The <i>target</i> and \p method are connected to the
   * WMenuItem::triggered signal.
   *
   * \note The icon should have a width of 16 pixels.
   *
   * \sa add(WMenuItem *)
   */
  template<class T, class V>
    WMenuItem *addItem(const std::string& iconPath, const WString& text,
		       T *target, void (V::*method)());

  /*! \brief Adds a submenu, with given text.
   *
   * Adds an item with text \p text, that leads to a submenu
   * \p menu.
   *
   * \sa add(WMenuItem *)
   */
  WMenuItem *addMenu(const WString& text, std::unique_ptr<WMenu> menu);

  /*! \brief Adds a submenu, with given icon and text.
   *
   * Adds an item with given text and icon, that leads to a submenu
   * \p menu.
   *
   * \sa add(WMenuItem *)
   */
  WMenuItem *addMenu(const std::string& iconPath, const WString& text,
		     std::unique_ptr<WMenu> menu);

  /*! \brief Adds an item.
   *
   * Adds a menu item. Use this form to add specialized WMenuItem
   * implementations.
   *
   * \sa addItem(const WString&, WWidget *, ContentLoading)
   */
  virtual WMenuItem *addItem(std::unique_ptr<WMenuItem> item);

  /*! \brief inserts an item.
   *
   * Use this version of insertItem() if you do not want to specify an icon
   * for this menu item.
   *
   * Returns the corresponding WMenuItem.
   *
   * \sa insertItem(int index, const std::string&, const WString&, WWidget *, ContentLoading)
   * \sa insertItem(int index, WMenuItem *)
   */
  WMenuItem *insertItem(int index, const WString& label,
			std::unique_ptr<WWidget> contents = nullptr,
			ContentLoading policy
			  = ContentLoading::Lazy);

  /*! \brief inserts an item.
   *
   * inserts a menu item with given \p contents, which is inserted to the
   * menu's associated contents stack.
   *
   * \p contents may be \c 0 for two reasons:
   * - if the menu is not associated with a contents stack, then you cannot
   *   associate a menu item with a contents widget
   * - or, you may have one or more items which which are not associated with
   *   a contents widget in the contents stack.
   *
   * Returns the corresponding WMenuItem.
   *
   * \sa insertItem(int index, WMenuItem *)
   */
  WMenuItem *insertItem(int index, const std::string& iconPath,
                        const WString& label,
			std::unique_ptr<WWidget> contents = nullptr,
			ContentLoading policy
			  = ContentLoading::Lazy);

    /*! \brief inserts an item with given text, and specify a slot method to be
   *         called when the item is triggered.
   *
   * The <i>target</i> and \p method are connected to the
   * WMenuItem::triggered signal.
   *
   * \sa insert(int index,WMenuItem *)
   */
  template<class T, class V>
    WMenuItem *insertItem(int index, const WString& text,
                          T *target, void (V::*method)());

  /*! \brief inserts an item with given text and icon, and specify a slot
   *         method to be called when activated.
   *
   * The <i>target</i> and \p method are connected to the
   * WMenuItem::triggered signal.
   *
   * \note The icon should have a width of 16 pixels.
   *
   * \sa insert(int index, WMenuItem *)
   */
  template<class T, class V>
    WMenuItem *insertItem(int index, const std::string& iconPath,
                          const WString& text, T *target, void (V::*method)());

  /*! \brief inserts a submenu, with given text.
   *
   * inserts an item with text \p text, that leads to a submenu
   * \p menu.
   *
   * \sa insert(int index, WMenuItem *)
   */
  WMenuItem *insertMenu(int index, const WString& text,
			std::unique_ptr<WMenu> menu);

  /*! \brief inserts a submenu, with given icon and text.
   *
   * inserts an item with given text and icon, that leads to a submenu
   * \p menu.
   *
   * \sa insert(int index, WMenuItem *)
   */
  WMenuItem *insertMenu(int index, const std::string& iconPath,
                        const WString& text,
			std::unique_ptr<WMenu> menu);

  /*! \brief Inserts an item.
   *
   * Inserts a menu item. Use this form to insert specialized WMenuItem
   * implementations.
   *
   * \sa insertItem(WMenuItem *item)
   */
  virtual WMenuItem *insertItem(int index, std::unique_ptr<WMenuItem> item);

  /*! \brief Adds a separator to the menu.
   *
   * Adds a separator the menu.
   */
  WMenuItem *addSeparator();

  /*! \brief Adds a section header to the menu.
   */
  WMenuItem *addSectionHeader(const WString& text);

  /*! \brief Removes an item.
   *
   * Removes the given item.
   *
   * \sa addItem()
   */
  virtual std::unique_ptr<WMenuItem> removeItem(WMenuItem *item);

  /*! \brief Selects an item.
   *
   * Select the menu item \p item.
   *
   * When \p item is \c 0, the current selection is removed.
   *
   * \sa select(int), currentItem(), WMenuItem::select()
   */
  void select(WMenuItem *item);

  /*! \brief Selects an item.
   *
   * Menu items in a menu with \p N items are numbered from 0 to
   * \p N - 1.
   *
   * Using a value of -1 removes the current selection.
   *
   * \sa select(WMenuItem *), currentIndex()
   */
  void select(int index);

  /*! \brief %Signal which indicates that a new item was selected.
   *
   * This signal is emitted when an item was selected. It is emitted
   * both when the user activated an item, or when select() was
   * invoked.
   *
   * \sa itemSelectRendered()
   */
  Signal<WMenuItem *>& itemSelected() { return itemSelected_; }

  /*! \brief %Signal which indicates that a new selected item is rendered.
   *
   * This signal is similar to \link WMenu::itemSelected
   * itemSelected\endlink, but is emitted from within a stateless
   * slot. Therefore, any slot connected to this signal will be
   * optimized to client-side JavaScript, and must support the
   * contract of a stateless slot (i.e., be idempotent).
   *
   * If you are unsure what is the difference with the \link
   * WMenu::itemSelected itemSelected\endlink signal, you'll probably
   * need the latter instead.
   *
   * \sa itemSelected()
   */
  Signal<WMenuItem *>& itemSelectRendered() { return itemSelectRendered_; }

  /*! \brief Closes an item.
   *
   * Close the menu item \p item. Only \link WMenuItem::setCloseable(bool)
   * closeable\endlink items can be closed.
   *
   * \sa close(int), WMenuItem::close()
   */
  void close(WMenuItem *item);

  /*! \brief Closes an item.
   *
   * Menu items in a menu with \p N items are numbered from 0 to
   * \p N - 1.
   *
   * \sa close(WMenuItem *)
   */
  void close(int index);

  /*! \brief Returns the items.
   *
   * Returns the list of menu items in this menu.
   *
   * \sa itemAt()
   */
  std::vector<WMenuItem *> items() const;

  /*! \brief %Signal which indicates that an item was closed.
   *
   * This signal is emitted when an item was closed. It is emitted
   * both when the user closes an item, or when close() was
   * invoked.
   */
  Signal<WMenuItem *>& itemClosed() { return itemClosed_; }

  /*! \brief Hides an item.
   *
   * Hides the menu item \p item. By default, all menu items are
   * visible.
   *
   * If the item was currently selected, then the next item to be selected
   * is determined by nextAfterHide().
   *
   * \sa setItemHidden(int, bool), WMenuItem::hide()
   */
  void setItemHidden(WMenuItem *item, bool hidden);

  /*! \brief Hides an item.
   *
   * Menu items in a menu with \p N items are numbered from 0 to \p N - 1.
   *
   * \sa setItemHidden(WMenuItem *, bool)
   */
  void setItemHidden(int index, bool hidden);

  /*! \brief Returns whether the item widget of the given item is hidden.
   *
   * \sa setItemHidden()
   */
  bool isItemHidden(WMenuItem *item) const;

  /*! \brief Returns whether the item widget of the given index is hidden.
   *
   * Menu items in a menu with \p N items are numbered from 0 to \p N - 1.
   *
   * \sa setItemHidden()
   */
  bool isItemHidden(int index) const;

  /*! \brief Disables an item.
   *
   * Disables the menu item \p item. Only an item that is enabled can
   * be selected. By default, all menu items are enabled.
   *
   * \sa setItemDisabled(int, bool), WMenuItem::setDisabled()
   */
  void setItemDisabled(WMenuItem *item, bool disabled);

  /*! \brief Disables an item.
   *
   * Menu items in a menu with \p N items are numbered from 0 to
   * \p N - 1.
   *
   * \sa setItemDisabled(WMenuItem *, bool)
   */
  void setItemDisabled(int index, bool disabled);

  /*! \brief Returns whether the item widget of the given item is disabled.
   *
   * \sa setItemDisabled()
   */
  bool isItemDisabled(WMenuItem *item) const;

  /*! \brief Returns whether the item widget of the given index is disabled.
   *
   * Menu items in a menu with \p N items are numbered from 0 to
   * \p N - 1.
   *
   * \sa setItemDisabled()
   */
  bool isItemDisabled(int index) const;

  /*! \brief Returns the currently selected item.
   *
   * \sa currentIndex(), select(WMenuItem *)
   */
  WMenuItem *currentItem() const;

  /*! \brief Returns the index of the currently selected item.
   *
   * \sa currentItem(), select(int)
   */
  int currentIndex() const { return current_; }

  /*! \brief Enables internal paths for items.
   *
   * The menu participates in the internal path by changing the
   * internal path when an item has been selected, and listening for
   * path changes to react to path selections. As a consequence this
   * allows the user to bookmark the current menu selection and
   * revisit it later, use back/forward buttons to navigate through
   * history of visited menu items, and allows indexing of pages.
   *
   * For each menu item, WMenuItem::pathComponent() is appended to the
   * \p basePath, which defaults to the internal path
   * (WApplication::internalPath()). A '/' is appended to the base
   * path, to turn it into a folder, if needed.
   *
   * By default, menu interaction does not change the application
   * internal path.
   *
   * \sa WMenuItem::setPathComponent().
   */
  void setInternalPathEnabled(const std::string& basePath = "");

  /*! \brief Returns whether the menu generates internal paths entries.
   *
   * \sa setInternalPathEnabled()
   */
  bool internalPathEnabled() const { return internalPathEnabled_; }

  /*! \brief Sets the internal base path.
   *
   * A '/' is appended to turn it into a folder, if needed.
   *
   * \sa setInternalPathEnabled(), internalBasePath()
   */
  void setInternalBasePath(const std::string& basePath);

  /*! \brief Returns the internal base path.
   *
   * The default value is the application's internalPath
   * (WApplication::internalPath()) that was recorded when
   * setInternalPathEnabled() was called, and together with each
   * WMenuItem::pathComponent() determines the paths for each item.
   *
   * For example, if internalBasePath() is <tt>"/examples/"</tt> and
   * pathComponent() for a particular item is <tt>"charts/"</tt>, then
   * the internal path for that item will be
   * <tt>"/examples/charts/"</tt>.
   *
   * \sa setInternalPathEnabled()
   */
  const std::string& internalBasePath() const { return basePath_; }

  /*! \brief Returns the contents stack associated with the menu.
   */
  WStackedWidget *contentsStack() const { return contentsStack_; }

  /*! \brief Returns the item count.
   */
  int count() const;

  /*! \brief Returns the item by index.
   *
   * \sa indexOf()
   */
  WMenuItem *itemAt(int index) const;

  /*! \brief Returns the index of an item.
   *
   * \sa itemAt()
   */
  int indexOf(WMenuItem *item) const;

  /*! \brief Returns the parent item (for a submenu)
   *
   * This is the item with which this menu is associated as a submenu
   * (if any).
   */
  WMenuItem *parentItem() const { return parentItem_; }

  virtual void load() override;

protected:
  virtual void render(WFlags<RenderFlag> flags) override;

  /*! \brief Handling of internal path changes.
   *
   * This methods makes the menu react to internal path changes (and also
   * the initial internal path).
   *
   * You may want to reimplement this if you want to customize the internal
   * path handling.
   */
  virtual void internalPathChanged(const std::string& path);

  /*! \brief Returns the index of the item to be selected after hides.
   *
   * Returns the index of the item to be selected after the item with given
   * index will be hidden.
   *
   * By default, if the given index is an index of currently selected item,
   * returns an index of the first visible item to the right of it. If it is not
   * found, returns the index of the first visible item to the left of it. If
   * there are no visible items around the currently selected item, returns the
   * index of currently selected item.
   *
   * You may want to reimplement this if you want to customize the algorithm
   * of determining the index of the item to be selected after hiding the item
   * with given index.
   */
  virtual int nextAfterHide(int index);

protected:
  WContainerWidget *ul() const { return ul_; }

  virtual void renderSelected(WMenuItem *item, bool selected);
  virtual void setCurrent(int index);

  /*! \brief Selects an item.
   *
   * This is the internal function that implements the selection
   * logic, including optional internal path change (if \p changePath
   * is \c true). The latter may be \c false in case an internal path
   * change itself is the reason for selection.
   */
  virtual void select(int index, bool changePath);

private:
  WContainerWidget *ul_;
  WStackedWidget *contentsStack_;
  bool internalPathEnabled_, emitPathChange_;
  std::string basePath_, previousInternalPath_;
  WMenuItem *parentItem_;

  Signal<WMenuItem *> itemSelected_, itemSelectRendered_;
  Signal<WMenuItem *> itemClosed_;

  void handleInternalPathChange(const std::string& path);

  int current_;
  int previousStackIndex_;
  bool needSelectionEventUpdate_;

  void updateItemsInternalPath();
  void itemPathChanged(WMenuItem *item);
  void selectVisual(WMenuItem *item);
  void undoSelectVisual();
  void selectVisual(int item, bool changePath, bool showContents);
  void onItemHidden(int index, bool hidden);

  friend class WMenuItem;

  void updateSelectionEvent();
};

template<class T, class V>
WMenuItem *WMenu::addItem(const WString& text, T *target, void (V::*method)())
{
  return addItem(std::string(), text, target, method);
}

template<class T, class V>
WMenuItem *WMenu::addItem(const std::string& iconPath, const WString& text,
			  T *target, void (V::*method)())
{
  WMenuItem *item = addItem(iconPath, text);
  item->triggered().connect(target, method);
  return item;
}

template<class T, class V>
WMenuItem *WMenu::insertItem(int index, const WString& text, T *target,
                             void (V::*method)())
{
  return insertItem( index, std::string(), text, target, method);
}

template<class T, class V>
WMenuItem *WMenu::insertItem(int index, const std::string& iconPath,
                             const WString& text, T *target,
                             void (V::*method)())
{
  WMenuItem *item = insertItem(index, iconPath, text);
  item->triggered().connect(target, method);
  return item;
}
}

#endif // WMENU_H_
