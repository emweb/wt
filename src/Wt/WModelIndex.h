// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMODEL_INDEX_H_
#define WMODEL_INDEX_H_

#include <vector>
#include <set>

#include <Wt/WDllDefs.h>
#include <Wt/WFlags.h>
#include <Wt/WAny.h>

namespace Wt {

class WAbstractItemModel;
class WModelIndex;

  namespace Chart {
    class WCartesianChart;
    class WPieChart;
  }

#ifndef WT_TARGET_JAVA
 /*! \brief A set of WModelIndexes
 */
typedef std::set<WModelIndex> WModelIndexSet;

extern std::size_t hash_value(const Wt::WModelIndex& index);
#else
typedef std::treeset<WModelIndex> WModelIndexSet;
#endif

#ifdef WT_TARGET_JAVA
#define constexpr
#endif // WT_TARGET_JAVA

/*! \defgroup modelview Model/view system
 *  \brief Classes that implement %Wt's model/view system.
 *
 * <h2>1. Models</h2>
 *
 * The library provides support for tabular, tree-like and tree-table like
 * models. All of these implement WAbstractItemModel.
 *
 * <h3>Concrete model implementations</h3>
 *
 * The following concrete model implementations are provided:
 * - Wt::WStandardItemModel: a full general purpose model, which stores data
 *   in memory.
 * - Wt::WStringListModel: a low-height single column model, useful for simple
 *   list views
 * - Wt::Dbo::QueryModel: a database query backed model
 *
 * <h3>Proxy model implementations</h3>
 *
 * Proxy models are helper models which wrap around another model, to
 * provide additional functionality on top of the existing model. The
 * following propy models:
 *
 * - Wt::WSortFilterProxyModel: provides sorting and filtering support
 * - Wt::WAggregateProxyModel: provides column aggregation (useful only for
 *   views that implement column aggregation such as WAbstractItemView's).
 *
 * <h3>Abstract models</h3>
 *
 * Abstract models cannot be instantiated as such, but are the base point
 * for custom model implementations:
 *
 * - Wt::WAbstractItemModel: abstract base class of all models
 * - Wt::WAbstractTableModel: abstract base class for tabular models
 * - Wt::WAbstractProxyModel: abstract base class for proxy models.
 *
 * <h2>2. Views</h2>
 *
 * <h3>Item-based views</h3>
 *
 * - WComboBox: a combo box
 * - WSelectionBox: a selection box
 * - WTableView: a table view (with editing support)
 * - WTreeView: a tree(-table) view (with editing support)
 * - WSuggestionPopup: an intelligent input-driven combo box
 *
 * <h3>Graphical views</h3>
 *
 * - Chart::WCartesianChart: 2D cartesian chart
 * - Chart::WPieChart: pie charts
 *
 * <h2>3. Helper classes</h2>
 *
 * <h3>Model indexes</h3>
 *
 * WModelIndex represents an index to an item of a WAbstractItemModel,
 * identified by a row, column and parent node.
 *
 * <h3>Item delegates</h3>
 *
 * Item delegates are used by WTableView and WTreeView to render a
 * single item and to provide editing support.
 *
 * The abstract base class is WAbstractItemDelegate, and a default
 * implementation is provided by WItemDelegate.
 */

/*! \brief Enumeration that indicates a role for a data item.
 *
 * A single data item can have data associated with it corresponding
 * to different roles. Each role may be used by the corresponding view
 * class in a different way.
 *
 * \sa WModelIndex::data()
 *
 * \ingroup modelview
 */
class WT_API ItemDataRole final {
public:
  /*! \brief Create a new role with a certain int value.
   */
  constexpr ItemDataRole(int role) noexcept
    : role_(role)
  { }

  /*! \brief Returns the underlying int of this role.
   */
  constexpr int value() const noexcept
  {
    return role_;
  }

  constexpr bool operator== (const ItemDataRole &rhs) const noexcept
  {
    return role_ == rhs.role_;
  }

  constexpr bool operator!= (const ItemDataRole &rhs) const noexcept
  {
    return role_ != rhs.role_;
  }

  constexpr bool operator< (const ItemDataRole &rhs) const noexcept
  {
    return role_ < rhs.role_;
  }

#if !defined(WT_TARGET_JAVA) || defined(DOXYGEN_ONLY)
  static constexpr const int Display = 0;       //!< Role for textual representation
  static constexpr const int Decoration = 1;    //!< Role for the url of an icon
  static constexpr const int Edit = 2;          //!< Role for the edited value
  static constexpr const int StyleClass = 3;    //!< Role for the style class

  /*! Role that indicates the check state.
   *
   * Data for this role should be a <tt>bool</tt>. When the
   * Wt::ItemFlag::Tristate flag is set for the item, data for this role
   * should be of type Wt::CheckState.
   */
  static constexpr const int Checked = 4;
  static constexpr const int ToolTip = 5;         //!< Role for a (plain) tooltip
  static constexpr const int Link = 6;            //!< Role for a link
  static constexpr const int MimeType = 7;        //!< Role for mime type information
  static constexpr const int Level = 8;           //!< Level in aggregation, for header data.

  static constexpr const int MarkerPenColor = 16;    //!< Marker pen color (for Chart::WCartesianChart)
  static constexpr const int MarkerBrushColor = 17;  //!< Marker brush color (for Chart::WCartesianChart)
  static constexpr const int MarkerScaleFactor = 20; //!< Marker size (for Chart::WCartesianChart)
  static constexpr const int MarkerType = 21; //!< Marker type (for Chart::WCartesianChart)
  static constexpr const int BarPenColor = 18;   //!< Bar pen color (for Chart::WCartesianChart)
  static constexpr const int BarBrushColor = 19; //!< Bar brush color (for Chart::WCartesianChart)

  static constexpr const int User = 32;           //!< First role reserved for user purposes
#else
  static const ItemDataRole Display;
  static const ItemDataRole Decoration;
  static const ItemDataRole Edit;
  static const ItemDataRole StyleClass;
  static const ItemDataRole Checked;
  static const ItemDataRole ToolTip;
  static const ItemDataRole Link;
  static const ItemDataRole MimeType;
  static const ItemDataRole Level;
  static const ItemDataRole MarkerPenColor;
  static const ItemDataRole MarkerBrushColor;
  static const ItemDataRole MarkerScaleFactor;
  static const ItemDataRole MarkerType;
  static const ItemDataRole BarPenColor;
  static const ItemDataRole BarBrushColor;
  static const ItemDataRole User;
#endif

private:
  int role_;
};

/*! \brief Flags that indicate data item options
 *
 * \sa WModelIndex::flags()
 *
 * \ingroup modelview
 */
enum class ItemFlag {
  Selectable = 0x1,       //!< Item can be selected
  Editable = 0x2,         //!< Item can be edited
  UserCheckable = 0x4,    //!< Item can be checked (checkbox is enabled)
  DragEnabled = 0x8,      //!< Item can be dragged
  DropEnabled = 0x10,     //!< Item can be a drop target
  /*! Item has tree states.
   *
   * When set, Wt::ItemDataRole::Checked data is of type
   * Wt::CheckState
   */
  Tristate = 0x20,
  XHTMLText = 0x40,        //!< Item's text (ItemDataRole::Display, ItemDataRole::ToolTip) is HTML
  Dirty = 0x80,            //!< Item's value has been modified
  DeferredToolTip = 0x100 //!< Item's tooltip is deferred
};

W_DECLARE_OPERATORS_FOR_FLAGS(ItemFlag)

/*! \brief Enumeration that indicates a sort order.
 *
 * \ingroup modelview
 */
enum class SortOrder {
  Ascending,  //!< Ascending sort order
  Descending  //!< Descending sort order
};

/*! \brief Enumeration that indicates a drop action.
 *
 * \sa WAbstractItemModel::dropEvent()
 *
 * \ingroup modelview
 */
enum class DropAction {
  Copy = 0x1, //!< Copy the selection
  Move = 0x2  //!< Move the selection (deleting originals)
};

/*! \class WModelIndex Wt/WModelIndex.h Wt/WModelIndex.h
 *  \brief A value class that describes an index to an item in a data model.
 *
 * Indexes are used to indicate a particular item in a
 * WAbstractItemModel. An index points to the item by identifying its
 * row and column location within a parent model index.
 *
 * An index is immutable.
 *
 * The default constructor creates an <i>invalid index</i>, which by
 * convention indicates the parent of top level indexes. Thus, a model
 * that specifies only a list or table of data (but no hierarchical
 * data) would have as valid indexes only indexes that specify the
 * <i>invalid</i> model index as parent.
 *
 * Upon the model's choice, model indexes for hierarchical models may
 * have an internal Id represented by a int64_t (internalId()), a
 * pointer (internalPointer()).
 *
 * Indexes are created by the model, within the protected
 * WAbstractItemModel::createIndex() methods. In this way, models can
 * define an internal pointer or id suitable for identifying parent
 * items in the model.
 *
 * When a model's geometry changes due to row or column insertions or
 * removals, you may need to update your indexes, as otherwise they
 * may no longer point to the same item (but instead still to the same
 * row/column). Thus, if you store indexes and want to support model
 * changes such as row or columns insertions/removals, then you need
 * to react to the corresponding signals such as
 * WAbstractItemModel::rowsInserted() to update these indexes
 * (i.e. shift them), or even remove them when the corresponding
 * row/column has been removed.
 *
 * When a model's layout changes (it is rearranging its contents for
 * example in response to a sort operation), a similar problem
 * arises. Some models support tracking of indexes over layout
 * changes, using <i>raw</i> indexes. In reaction to
 * WAbstractItemModel::layoutAboutToBeChanged(), you should encode any
 * index which you wish to recover after the layout change using
 * encodeAsRawIndex(), and in WAbstractItemModel::layoutChanged() you
 * can obtain an index that points to the same item using
 * decodeFromRawIndex().
 *
 * \sa WAbstractItemModel
 *
 * \ingroup modelview
 */
class WT_API WModelIndex
{
public:
  /*! \brief Create an invalid WModelIndex.
   *
   * Returns a model index for which isValid() return \c false.
   */
  WModelIndex();

  /*! \brief Returns the column for this model index.
   *
   * \sa row()
   */
  int column() const { return column_; }

  /*! \brief Returns the row for this model index.
   *
   * \sa column()
   */
  int row() const { return row_; }

  /*! \brief Returns the internal pointer.
   *
   * The internal pointer is used by the model to retrieve the corresponding
   * data.
   *
   * This is only defined when the model created the index using
   * WAbstractItemModel::createIndex(int, int, void *) const.
   *
   * \sa internalId(),
   * \sa WAbstractItemModel::createIndex(int, int, void *) const
   */
  void *internalPointer() const { return reinterpret_cast<void*>(internalId_); }

  /*! \brief Returns the internal id.
   *
   * The internal id is used by the model to retrieve the
   * corresponding data.
   *
   * This is only defined when the model created the index using
   * WAbstractItemModel::createIndex(int, int, uint64_t) const.
   *
   * \sa internalPointer()
   * \sa WAbstractItemModel::createIndex(int, int, uint64_t) const
   */
  ::uint64_t internalId() const { return internalId_; }

  /*! \brief Returns a model index for a child item.
   *
   * This is a convenience method, and is only defined for indexes
   * that are valid().
   *
   * It has the same function as WAbstractItemModel::index() but is
   * less general because the latter expression may also be used to
   * retrieve top level children, i.e. when \p index is invalid.
   *
   * \sa WAbstractItemModel::index(), isValid()
   */
  WModelIndex child(int row, int column) const;

  /*! \brief Returns an index to the parent.
   *
   * This is a convenience method for WAbstractItemModel::parent().
   *
   * For a top level data item, the parent() is an invalid index (see
   * WModelIndex()).
   *
   * \sa WAbstractItemModel::parent()
   */
  WModelIndex parent() const;

  /*! \brief Returns data in the model at this index.
   *
   * This is a convenience method for WAbstractItemModel::data().
   *
   * \sa WAbstractItemModel::data()
   * \sa ItemDataRole
   */
  cpp17::any data(ItemDataRole role = ItemDataRole::Display) const;

  /*! \brief Returns the flags for this item.
   *
   * This is a convenience method for WAbstractItemModel::flags().
   *
   * \sa WAbstractItemModel::flags()
   * \sa ItemFlag
   */
  WFlags<ItemFlag> flags() const;

  /*! \brief Returns whether the index is a real valid index.
   *
   * Returns \c true when the index points to a valid data item,
   * i.e. at a valid row() and column().
   *
   * An index may be invalid for two reasons:
   *  - an operation requested an index that was out of model bounds,
   *  - or, the index corresponds to the model's top level root item, and is
   *    thus the parent index for top level items.
   */
  bool isValid() const { return model_ != nullptr; }

  /*! \brief Returns the model to which this (valid) index is bound.
   *
   * This returns the model that created the model index.
   */
  const WAbstractItemModel *model() const { return model_; }

  /*! \brief Comparison operator.
   *
   * Returns \c true only if the indexes point at the same data, in the
   * same model.
   */
  bool operator== (const WModelIndex& other) const;

  /*! \brief Comparison operator.
   *
   * \sa operator==()
   */
  bool operator!= (const WModelIndex& other) const;

  /*! \brief Comparison operator.
   *
   * Returns \c true if the index comes topologically before \p other.
   *
   * Topological order follows the order in which the indexes would be
   * displayed in a tree table view, from top to bottom followed by
   * left to right.
   */
  bool operator< (const WModelIndex& other) const;

  /*! \brief Encode to raw index (before a layout change).
   *
   * Use this method to encode an index for which you want to recover
   * an index after the layout change to the same item (which may
   * still be in the model, but at a different location).
   *
   * An index that has been encoded as a raw index cannot be used for
   * anything but decodeFromRawIndex() at a later point.
   *
   * \sa WAbstractItemModel::toRawIndex(), WAbstractItemModel::layoutAboutToBeChanged()
   * \sa decodeFromRawIndex()
   */
  void encodeAsRawIndex();

  /*! \brief Decodes a raw index (after a layout change).
   *
   * A raw index can be decoded, within the context of a model that has been
   * re-layed out.
   *
   * This method returns a new index that points to the same item, or,
   * WModelIndex() if the underlying model did not support encoding to
   * raw indexes, or, if the item to which the index previously
   * pointed, is no longer part of the model.
   *
   * \sa WAbstractItemModel::fromRawIndex(), WAbstractItemModel::layoutChanged()
   * \sa encodeAsRawIndex()
   */
  WModelIndex decodeFromRawIndex() const;

  /*! \brief Returns the depth (in a hierarchical model).
   *
   * A top level index has depth 0.
   */
  int depth() const;

  /*! \brief Utility method for converting an entire set of indexes to raw.
   *
   * \sa encodeAsRawIndex()
   */
  static void encodeAsRawIndexes(WModelIndexSet& indexes);

  /*! \brief Utility method to decode an entire set of raw indexes.
   *
   * \sa decodeFromRawIndex()
   */
  static
    WModelIndexSet decodeFromRawIndexes(const WModelIndexSet& encodedIndexes);

  struct UnorderedLess {
    bool operator()(const WModelIndex& i1, const WModelIndex& i2) const;
  };

  /*! \brief Returns whether i2 is an ancestor of i1
   */
  static bool isAncestor(const Wt::WModelIndex& i1, const Wt::WModelIndex& i2);

private:
  const WAbstractItemModel *model_;
  int row_, column_;
  ::uint64_t internalId_;

  WModelIndex(int row, int column, const WAbstractItemModel *model, void *ptr);
  WModelIndex(int row, int column, const WAbstractItemModel *model,
	      ::uint64_t id);

  friend class WAbstractItemModel;

  WModelIndex ancestor(int depth) const;

  bool isRawIndex() const;
};

/*! \brief List of indexes
 *
 * The list is defined as std::vector<WModelIndex>.
 */
typedef std::vector<WModelIndex> WModelIndexList;

}

#ifndef WT_TARGET_JAVA
namespace std {
  template<>
  struct hash<Wt::WModelIndex>
  {
    std::size_t operator()(const Wt::WModelIndex& index) const
    {
      return Wt::hash_value(index);
    }
  };

  template<>
  struct equal_to<Wt::WModelIndex>
  {
    bool operator()(const Wt::WModelIndex& index1, const Wt::WModelIndex& index2) const
    {
      return index1 == index2;
    }
  };
}
#endif // WT_TARGET_JAVA

/*! @} */

#endif // WMODEL_INDEX_H_
