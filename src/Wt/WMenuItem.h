// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMENU_ITEM_H_
#define WMENU_ITEM_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WString.h>

namespace Wt {

/*! \brief A single item in a menu.
 *
 * Since %Wt 3.3.0, this item is now a proper widget, which renders a
 * single item in a menu.
 *
 * An optional contents item can be associated with a menu item, which
 * is inserted and shown in the widget stack of the menu to which this
 * menu item belongs.
 *
 * <h3>CSS</h3>
 *
 * A menu item renders as a &gt;li&;lt with additional markup/style
 * classes provided by the theme. Unless you use the bootstrap theme,
 * you will need to provide appropriate CSS.
 */
class WT_API WMenuItem : public WContainerWidget
{
public:
  /*! \brief Creates a new item with given label.
   *
   * The optional contents is a widget that will be shown in the WMenu
   * contents stack when the item is selected. For this widget, a load
   * \p policy specifies whether the contents widgets is transmitted
   * only when it the item is activated for the first time
   * (LazyLoading) or transmitted prior to first rendering.
   *
   * If the menu supports internal path navigation, then a default
   * pathComponent() will be derived from the \p label, and can be
   * customized using setPathComponent().
   */
  WMenuItem(const WString& label, std::unique_ptr<WWidget> contents = nullptr,
	    ContentLoading policy = ContentLoading::Lazy);

  /*
   * The icon is displayed left to the text.
   *
   * \note The icon should have a width of 16 pixels.
   */
  WMenuItem(const std::string& iconPath, const WString& label,
	    std::unique_ptr<WWidget> contents = nullptr,
	    ContentLoading policy = ContentLoading::Lazy);

  /* !\brief Destructor.
   *
   * Removes the item from the menu (if it was added previously to one), and
   * also deletes the contents that was associated with the item.
   *
   * \sa WMenu::removeItem()
   */
  ~WMenuItem();

  /*! \brief Sets the text for this item.
   *
   * Unless a custom path component was defined, the pathComponent()
   * is also updated based on the new text.
   *
   * The item widget is updated using updateItemWidget().
   *
   * \sa setPathComponent();
   */
  void setText(const WString& text);

  /*! \brief Returns the text for this item.
   *
   * \sa setText();
   */
  WString text() const;

  /*! \brief Sets the item icon path.
   *
   * The icon should have a width of 16 pixels.
   *
   * \sa setText()
   */
  void setIcon(const std::string& path);

  /*! \brief Returns the item icon path.
   *
   * \sa setIcon()
   */
  std::string icon() const;

  /*! \brief Sets if the item is checkable.
   *
   * When an item is checkable, a checkbox is displayed to the left of the
   * item text (instead of an icon).
   *
   * \sa setChecked(), isChecked()
   */
  void setCheckable(bool checkable);

  /*! \brief Returns whether the item is checkable.
   *
   * \sa setCheckable()
   */
  bool isCheckable() const { return checkBox_ != nullptr; }

  /*! \brief Sets the path component for this item.
   *
   * The path component is used by the menu item in the application
   * internal path (see WApplication::setInternalPath()), when
   * internal paths are enabled (see WMenu::setInternalPathEnabled())
   * for the menu.
   *
   * You may specify an empty \p path to let a menu item be the
   * "default" menu option.
   *
   * For example, if WMenu::internalBasePath() is
   * <tt>"/examples/"</tt> and pathComponent() for is
   * <tt>"charts/"</tt>, then the internal path for the item will be
   * <tt>"/examples/charts/"</tt>.
   *
   * By default, the path is automatically derived from text(). If a
   * \link WString::literal() literal text\endlink is used, the path
   * is based on the text itself, otherwise on the \link
   * WString::key() key\endlink. It is converted to lower case, and
   * replacing whitespace and special characters with '_'.
   *
   * \sa setText(), WMenu::setInternalPathEnabled()
   *
   * \if cpp
   * \note the \p path should be CharEncoding::UTF8 encoded (we may fix the API
   *       to use WString in the future).
   * \endif
   */
  virtual void setPathComponent(const std::string& path);

  /*! \brief Returns the path component for this item.
   *
   * You may want to reimplement this to customize the path component
   * set by the item in the application internal path.
   *
   * \sa setPathComponent()
   *
   * \if cpp
   * \note the \p path component is CharEncoding::UTF8 encoded (we may fix the API
   *       to use WString in the future).
   * \endif
   */
  virtual std::string pathComponent() const;

  /*! \brief Configures internal path support for the item.
   *
   * This configures whether the item supports internal paths (in a menu
   * which supports internal paths).
   *
   * The default value is \c true for all items but section headers and
   * separators.
   *
   * \sa WMenu::setInternalPathEnabled()
   */
  virtual void setInternalPathEnabled(bool enabled);

  /*! \brief Returns whether an item participates in internal paths.
   *
   * \sa setInternalPathEnabled()
   */
  virtual bool internalPathEnabled() const;

  /*! \brief Sets the associated link.
   *
   * \sa Link()
   */
  void setLink(const WLink& link);

  /*! \brief Returns the associated link.
   *
   * \sa setLink()
   */
  WLink link() const;

  /*! \brief Sets a sub menu.
   *
   * In most cases, the sub menu would use the same contents stack as
   * the parent menu.
   *
   * Note that adding a submenu makes this item not \link isSelectable() selectable\endlink by default.
   *
   * \note If the \link parentMenu() parent menu\endlink is a WPopupMenu, the submenu should also be a
   *       WPopupMenu.
   *
   * \sa setSelectable()
   */
  void setMenu(std::unique_ptr<WMenu> menu);

  /*! \brief Returns the submenu.
   *
   * \sa setMenu()
   */
  WMenu *menu() const { return subMenu_; }

  /*! \brief Sets the checked state.
   *
   * This is only used when isCheckable() == \c true.
   *
   * \sa setCheckable(bool), isCheckable()
   */
  void setChecked(bool checked);

  /*! \brief Returns the checked state.
   *
   * This is only used when isCheckable() == \c true.
   *
   * \sa setChecked(bool), isCheckable()
   */
  bool isChecked() const;

  /*! \brief Sets whether the menu item can be selected.
   *
   * Only a menu item that can be selected can be the result of a
   * popup menu selection.
   *
   * The default value is \c true for a normal menu item, and \c false
   * for a menu item that has a submenu.
   *
   * An item that is selectable but is disabled can still not be
   * selected.
   */
  void setSelectable(bool selectable) override;

  /*! \brief Returns whether the menu item can be selected.
   *
   * \sa setSelectable()
   */
  bool isSelectable() const { return selectable_; }

  /*! \brief Sets associated additional data with the item.
   *
   * You can use this to associate model information with a menu item.
   */
  void setData(void *data) { data_ = data; }

  /*! \brief Returns additional data of the item.
   *
   * \sa setData()
   */
  void *data() const { return data_; }

  /*! \brief Returns the checkbox for a checkable item.
   *
   * \sa setCheckable()
   */
  WCheckBox *checkBox() const { return checkBox_; }

  /*! \brief Make it possible to close this item interactively or by close().
   *
   * \sa close(), isCloseable()
   */
  void setCloseable(bool closeable);

  /*! \brief Returns whether the item is closeable.
   *
   * \sa setCloseable()
   */
  bool isCloseable() const { return closeable_; }

  /*! \brief Closes this item.
   *
   * Hides the item widget and emits WMenu::itemClosed() signal. Only closeable
   * items can be closed.
   *
   * \sa setCloseable(), hide()
   */
  void close();

  /*! \brief Returns the menu that contains this item.
   */
  WMenu *parentMenu() const { return menu_; }

  /*! \brief Sets the contents widget for this item.
   *
   * The contents is a widget that will be shown in the WMenu contents
   * stack when the item is selected. For this widget, the load \p
   * policy specifies whether the contents widgets is transmitted only
   * when it the item is activated for the first time (LazyLoading) or
   * transmitted prior to first rendering.
   */
  void setContents(std::unique_ptr<WWidget> contents,
		   ContentLoading policy = ContentLoading::Lazy);

  /*! \brief Returns the contents widget for this item.
   *
   * \sa setContents()
   */
  WWidget *contents() const;

  /*! \brief Removes the contents widget from this item.
   */
  std::unique_ptr<WWidget> removeContents();

  /*! \brief Selects this item.
   *
   * If the item was previously closed it will be shown.
   *
   * \sa close()
   */
  void select();

  /*! \brief %Signal emitted when an item is activated.
   *
   * Returns this item as argument.
   *
   * \sa WMenu::triggered()
   */
  Signal<WMenuItem *>& triggered() { return triggered_; }

  /*! \brief Returns whether this item is a separator.
   *
   * \sa WMenu::addSeparator()
   */
  bool isSeparator() const { return separator_; }

  /*! \brief Returns whether this item is a section header.
   *
   * \sa WMenu::addSectionHeader()
   */
  bool isSectionHeader() const;

  /*! \brief Returns the anchor of this menu item.
   *
   *  Can be used to add widgets to the menu.
   */
  WAnchor *anchor() const;

  /*! \brief Renders the item as selected or unselected.
   *
   * The default implementation sets the styleclass for itemWidget()
   * to 'item' for an unselected not closeable, 'itemselected' for
   * selected not closeable, 'citem' for an unselected closeable and
   * 'citemselected' for selected closeable item.
   *
   * Note that this method is called from within a stateless slot
   * implementation, and thus should be stateless as well.
   */
  virtual void renderSelected(bool selected);

  virtual void setFromInternalPath(const std::string& path);

  virtual void enableAjax() override;

  virtual void setHidden(bool hidden,
                         const WAnimation& animation = WAnimation()) override;
  virtual void setDisabled(bool disabled) override;

protected:
  virtual void render(WFlags<RenderFlag> flags) override;

private:
  // Constructs a separator or header
  WMenuItem(bool separator, const WString& text);

  ContentLoading loadPolicy_;
  std::unique_ptr<WWidget> uContents_; // contents not added to the widget tree
  observing_ptr<WWidget> oContents_;   // always points to contents
  std::unique_ptr<WContainerWidget> uContentsContainer_;
  observing_ptr<WContainerWidget> oContentsContainer_;

  WMenu *menu_, *subMenu_;
  WText *icon_;
  WLabel *text_;
  WCheckBox *checkBox_;
  void *data_;
  bool separator_, selectable_, signalsConnected_, customLink_;

  Signal<WMenuItem *> triggered_;

  std::string pathComponent_;
  bool customPathComponent_, internalPathEnabled_;
  bool closeable_;

  void create(const std::string& iconPath, const WString& text,
	      std::unique_ptr<WWidget> contents, ContentLoading policy);
  void purgeContents();
  void updateInternalPath();
  bool contentsLoaded() const;
  void loadContents();
  void setParentMenu(WMenu *menu);
  void selectNotLoaded();
  void selectVisual();
  void undoSelectVisual();
  void connectClose();
  void connectSignals();
  void setItemPadding(bool padding);
  void contentsDestroyed();
  void setCheckBox();
  void setUnCheckBox();

  std::unique_ptr<WWidget> takeContentsForStack();
  WWidget *contentsInStack() const;
  void returnContentsInStack(std::unique_ptr<WWidget> widget);

  friend class WMenu;
  friend class WPopupMenu;
};

}

#endif // WMENU_ITEM_H_
