// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_TREEVIEW_H_
#define WT_TREEVIEW_H_

#include <limits>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Wt/WAbstractItemView.h>
#include <Wt/WJavaScript.h>
#include <Wt/WModelIndex.h>

namespace Wt {

  class WCheckBox;
  class WCssRule;
  class WItemSelectionModel;
  class WText;
  class WTreeViewNode;
  class ToggleButtonConfig;

/*! \class WTreeView Wt/WTreeView.h Wt/WTreeView.h
 *  \brief A view class that displays a model as a tree or tree table.
 *
 * The view displays data from a WAbstractItemModel in a tree or tree
 * table. It provides incremental rendering, allowing the display of
 * data models of any size efficiently, without excessive use of
 * client- or serverside resources.
 *
 * The rendering (and editing) of items is handled by a
 * WAbstractItemDelegate, by default it uses WItemDelegate which
 * renders data of all predefined roles (see also Wt::ItemDataRole),
 * including text, icons, checkboxes, and tooltips.
 *
 * The view may support editing of items, if the model indicates
 * support (see the Wt::ItemFlag::Editable flag). You can define triggers
 * that initiate editing of an item using setEditTriggers(). The
 * actual editing is provided by the item delegate (you can set an
 * appropriate delegate for one column using
 * setItemDelegateForColumn()). Using setEditOptions() you can
 * customize if and how the view deals with multiple editors.
 *
 * By default, all but the first columns are given a width of 150px,
 * and the first column takes the remaining size. <b>Note that this
 * may have as consequence that the first column's size is reduced to
 * 0.</b> Column widths of all columns, including the first column,
 * can be set through the API method setColumnWidth(), and also by the
 * user using handles provided in the header.
 *
 * Optionally, the treeview may be configured so that the first column
 * is always visible while scrolling through the other columns, which
 * may be convenient if you wish to display a model with many
 * columns. Use setColumn1Fixed() to enable this behaviour.
 *
 * If the model supports sorting (WAbstractItemModel::sort()), such as
 * the WStandardItemModel, then you can enable sorting buttons in the
 * header, using setSortingEnabled().
 *
 * You can allow selection on row or item level (using
 * setSelectionBehavior()), and selection of single or multiple items
 * (using setSelectionMode()), and listen for changes in the selection
 * using the selectionChanged() signal.
 *
 * You may enable drag & drop support for this view, with awareness
 * of the items in the model. When enabling dragging (see
 * setDragEnabled()), the current selection may be dragged, but only
 * when all items in the selection indicate support for dragging
 * (controlled by the \link Wt::ItemFlag::DragEnabled
 * ItemFlag::DragEnabled\endlink flag), and if the model indicates a
 * mime-type (controlled by WAbstractItemModel::mimeType()). Likewise,
 * by enabling support for dropping (see setDropsEnabled()), the
 * treeview may receive a drop event on a particular item, at least if
 * the item indicates support for drops (controlled by the \link
 * Wt::ItemFlag::DropEnabled ItemFlag::DropEnabled\endlink flag).
 *
 * You may also react to mouse click events on any item, by connecting
 * to one of the clicked() or doubleClicked() signals.
 *
 * \if cpp
 * Usage example:
 * \code
 * // WTreeView will display the data of a model
 * std::shared_ptr<Wt::WAbstractItemModel> model = ...
 *
 * // Create the WTreeView
 * auto gitView = std::make_unique<Wt::WTreeView>();
 * gitView->resize(300, Wt::WLength::Auto);
 * gitView->setSortingEnabled(false);
 * gitView->setModel(model);
 * gitView->setSelectionMode(SelectionMode::Single);
 * \endcode
 * \endif
 *
 * <h3>Graceful degradation</h3>
 *
 * The view provides a virtual scrolling behavior which relies on Ajax
 * availability. When Ajax is not available, a page navigation bar is
 * used instead, see createPageNavigationBar(). In that case, the widget
 * needs to be given an explicit height using resize() which determines the
 * number of rows that are displayed at a time.
 *
 * A snapshot of the WTreeView: 
 * \image html WTreeView-default-1.png "WTreeView example (default)"
 * \image html WTreeView-polished-1.png "WTreeView example (polished)"
 *
 * \ingroup modelview
 */
class WT_API WTreeView : public WAbstractItemView
{
public:
  /*! \brief Creates a new tree view.
   */
  WTreeView();

  ~WTreeView();

  /*! \brief Expands or collapses a node.
   *
   * \sa expand(), collapse()
   */
  void setExpanded(const WModelIndex&, bool expanded);

  /*! \brief Returns whether a node is expanded.
   *
   * \sa setExpanded()
   */
  bool isExpanded(const WModelIndex& index) const;

  /*! \brief Collapses a node.
   *
   * \sa setExpanded(), expand()
   *
   * \note until 3.3.4, selection was removed from within nodes that
   *       were collapsed. This (inconsistent) behavior has been
   *       removed in 3.3.4.
   */
  void collapse(const WModelIndex& index);

  /*! \brief Collapse all expanded nodes.
   *
   * \sa collapse(), expand()
   */
  void collapseAll();

  /*! \brief Expands a node.
   *
   * \sa setExpanded(), collapse()
   */
  void expand(const WModelIndex& index);

  /*! \brief Expands all nodes to a depth.
   *
   * Expands all nodes to the given \p depth. A depth of 1 corresponds
   * to the top level nodes.
   *
   * \sa expand()
   */
  void expandToDepth(int depth);

  /*! \brief Sets whether toplevel items are decorated.
   *
   * By default, top level nodes have expand/collapse and other lines
   * to display their linkage and offspring, like any node.
   *
   * By setting \p show to \c false, you can hide these decorations
   * for root nodes, and in this way mimic a plain list. You could also
   * consider using a WTableView instead.
   */
  void setRootIsDecorated(bool show);

  /*! \brief Returns whether toplevel items are decorated.
   *
   * \sa setRootIsDecorated()
   */
  bool rootIsDecorated() const { return rootIsDecorated_; }

  virtual void resize(const WLength& width, const WLength& height) override;

  /*! \brief %Signal emitted when a node is collapsed.
   *
   * \sa setExpanded(), expanded()
   */
  Signal<WModelIndex>& collapsed() { return collapsed_; }

  /*! \brief %Signal emitted when a node is expanded.
   *
   * \sa setExpanded(), collapsed()
   */
  Signal<WModelIndex>& expanded() { return expanded_; }

  virtual WWidget *itemWidget(const WModelIndex& index) const override;
  virtual void setModel(const std::shared_ptr<WAbstractItemModel>& model)
    override;

  /*! \brief Sets the column width.
   *
   * For a model with \link WAbstractItemModel::columnCount()
   * columnCount()\endlink == \p N, the initial width of columns 1..\p
   * N is set to 150 pixels, and column 0 will take all remaining
   * space.
   *
   * \note The actual space occupied by each column is the column width
   *       augmented by 7 pixels for internal padding and a border.
   *
   * \sa setRowHeight()
   */
  virtual void setColumnWidth(int column, const WLength& width) override;
  virtual void setAlternatingRowColors(bool enable) override;
  virtual void setRowHeight(const WLength& rowHeight) override;
  virtual void setHeaderHeight(const WLength& height) override;
#ifndef WT_CNOR
  using WAbstractItemView::setHeaderHeight;
#endif
  virtual void setColumnHidden(int column, bool hidden) override;
  virtual void setRowHeaderCount(int count) override;

  virtual int pageCount() const override;
  virtual int pageSize() const override;
  virtual int currentPage() const override;
  virtual void setCurrentPage(int page) override;

  virtual void scrollTo(const WModelIndex& index,
			ScrollHint hint = ScrollHint::EnsureVisible) override;
  virtual EventSignal<WScrollEvent>& scrolled() override;

  virtual void setId(const std::string &id) override;

protected:
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void enableAjax() override;

private:
  typedef std::unordered_map<WModelIndex, WTreeViewNode *> NodeMap;

  bool skipNextMouseEvent_;

  std::unordered_set<WModelIndex> expandedSet_;
  NodeMap renderedNodes_;
  bool renderedNodesAdded_;
  WTreeViewNode *rootNode_;
  WCssTemplateRule *rowHeightRule_, *rowWidthRule_, *rowContentsWidthRule_;
  WCssRule *c0StyleRule_;

  bool rootIsDecorated_, column1Fixed_;

  Signal<WModelIndex>  collapsed_, expanded_;

  // in rows, as indicated by the current position of the viewport:
  int viewportTop_, viewportHeight_;

  // the firstRenderedRow may differ from viewportTop_, because the user
  // adjusted the view port slightly, but not enough to trigger a correction
  //
  // the validRowCount may differ from viewportHeight_ as a result of
  // expanding or collapsing nodes, or inserting and deleting rows.
  // it takes into account that an expanded node may be incomplete, and
  // thus everything beyond is irrelevant
  int firstRenderedRow_, validRowCount_;

  // rendered nodes in memory (including those collapsed and not included in
  // actualRenderedRowCount_), but excluding nodes that are simply there since
  // some of its children are rendered
  int nodeLoad_;

  WContainerWidget *headers_, *headerContainer_;
  WContainerWidget *contents_, *contentsContainer_;
  WContainerWidget *scrollBarC_;

  int firstRemovedRow_, removedHeight_;

  JSignal<std::string, std::string, std::string,
          std::string, WMouseEvent> itemEvent_;
  JSignal<std::string, std::string, WTouchEvent> itemTouchEvent_;

  std::unique_ptr<ToggleButtonConfig> expandConfig_;

  JSlot tieRowsScrollJS_,
        itemClickedJS_, rootClickedJS_,
        itemDoubleClickedJS_, rootDoubleClickedJS_,
        itemMouseDownJS_, rootMouseDownJS_,
        itemMouseUpJS_, rootMouseUpJS_,
        touchStartedJS_, touchMovedJS_, touchEndedJS_;
  
  virtual ColumnInfo createColumnInfo(int column) const override;

  void defineJavaScript();
  void rerenderHeader();
  void rerenderTree();
  void setup();

  virtual void scheduleRerender(RenderState what) override;

  void modelColumnsInserted(const WModelIndex& parent, int start, int end);
  void modelColumnsAboutToBeRemoved(const WModelIndex& parent,
				    int start, int end);
  void modelColumnsRemoved(const WModelIndex& parent, int start, int end);
  void modelRowsInserted(const WModelIndex& parent, int start, int end);
  void modelRowsAboutToBeRemoved(const WModelIndex& parent, int start, int end);
  void modelRowsRemoved(const WModelIndex& parent, int start, int end);
  virtual void modelDataChanged(const WModelIndex& topLeft,
				const WModelIndex& bottomRight) override;
  virtual void modelLayoutAboutToBeChanged() override;
  virtual void modelLayoutChanged() override;

  void onViewportChange(WScrollEvent event);
  void contentsSizeChanged(int width, int height);
  void onItemEvent(std::string nodeAndColumnId, std::string type,
		   std::string extra1, std::string extra2, WMouseEvent event);
  void onItemTouchEvent(std::string nodeAndColumnId, std::string type, WTouchEvent event);
  WModelIndex calculateModelIndex(std::string nodeAndColumnId);
  void setRootNodeStyle();
  void setCollapsed(const WModelIndex& index);

  int calcOptimalFirstRenderedRow() const;
  int calcOptimalRenderedRowCount() const;

  void shiftModelIndexes(const WModelIndex& parent, int start, int count);
  static int shiftModelIndexes(const WModelIndex& parent, int start, int count,
			       const std::shared_ptr<WAbstractItemModel>& model,
			       WModelIndexSet& set);
  static int shiftModelIndexes(const WModelIndex& parent, int start, int count,
                               const std::shared_ptr<WAbstractItemModel>& model,
                               std::unordered_set<WModelIndex>& set);

  void addRenderedNode(WTreeViewNode *node);
  void removeRenderedNode(WTreeViewNode *node);

  void adjustToViewport(WTreeViewNode *changed = nullptr);

  int pruneNodes(WTreeViewNode *node, int theNodeRow);
  int adjustRenderedNode(WTreeViewNode *node, int theNodeRow);

  WWidget *widgetForIndex(const WModelIndex& index) const;
  WTreeViewNode *nodeForIndex(const WModelIndex& index) const;

  int subTreeHeight(const WModelIndex& index,
		    int lowerBound = 0,
		    int upperBound = std::numeric_limits<int>::max()) const;
  int renderedRow(const WModelIndex& index,
		  WWidget *w,
		  int lowerBound = 0,
		  int upperBound = std::numeric_limits<int>::max());
  int getIndexRow(const WModelIndex& index,
		  const WModelIndex& ancestor,
		  int lowerBound = 0,
		  int upperBound = std::numeric_limits<int>::max()) const;

  std::string columnStyleClass(int column) const;

  int renderLowerBound() const;
  int renderUpperBound() const;

  void renderedRowsChanged(int row, int count);

  WContainerWidget *headerRow();

  virtual bool internalSelect(const WModelIndex& index, SelectionFlag option)
    override;
  virtual void selectRange(const WModelIndex& first, const WModelIndex& last)
    override;

  bool isExpandedRecursive(const WModelIndex& index) const;

  void expandChildrenToDepth(const WModelIndex& index, int depth);

  void updateColumnWidth(int columnId, int width);

  virtual WContainerWidget* headerContainer() override {
    return headerContainer_;
  }

  virtual WWidget *headerWidget(int column, bool contentsOnly = true) override;

  friend class WTreeViewNode;
  friend class ContentsContainer;
};

}

#endif // WT_TREEVIEW_H_
