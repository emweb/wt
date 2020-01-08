// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACT_ITEM_MODEL_H_
#define WABSTRACT_ITEM_MODEL_H_

#include <Wt/WObject.h>
#include <Wt/WModelIndex.h>
#include <Wt/WSignal.h>
#include <Wt/WGlobal.h>
#include <Wt/WAny.h>

namespace Wt {

  class WDropEvent;

/*! \class WAbstractItemModel Wt/WAbstractItemModel.h Wt/WAbstractItemModel.h
 *  \brief An abstract model for use with %Wt's view classes.
 *
 * This abstract model is used by several %Wt view widgets as data models.
 *
 * It may model data for both tree-like and table-like view
 * widgets. Data is therefore organized in a hierarchical structure of
 * tables, where every item stores data and items in column 0 can be
 * the parent of a nested table of data. Every data item is uniquely
 * identified by their row, column and parent index, and items may be
 * referenced using the helper class WModelIndex.
 *
 * Each item may provide data for one or more \link Wt::ItemDataRole
 * roles\endlink, and indicate options using \link Wt::ItemFlag
 * flags\endlink. The different roles can be used to model different
 * aspects of an item (its text value, an icon, style class), or to
 * hold auxiliary custom information. The flags provide information to
 * the View on possible interactivity.
 *
 * \if cpp
 * Side::Top level data have an \link WModelIndex::isValid() invalid\endlink
 * parent WModelIndex.
 * \endif
 * \if java
 * Side::Top level data have a \c null parent WModelIndex.
 * \endif
 *
 * \if cpp
 * The data itself is of type <b>Wt::any</b>, which can either be
 * empty, or hold any type of data. Depending on the role however,
 * view classes may expect certain types of data (e.g. a string for
 * Wt::ItemDataRole::StyleClass).
 *
 * Wt's standard view classes can display (Wt::ItemDataRole::Display)
 * the following data:
 *
 *  - strings of type WString or std::string
 *  - WDate, WTime, WDateTime, WLocalDateTime
 *  - standard C++ numeric types (int, double, etc...)
 *  - bool
 *
 * The view classes know how to interpret data of these types \link
 * Wt::asString() as a string\endlink or \link Wt::asNumber() as a
 * number\endlink.
 *
 * \elseif java
 *
 * The data itself is of type Object, which can either be \c null, or be
 * any type of data. Depending on the role however, view classes may
 * expect certain types of data (e.g. numerical types for charts) or
 * will convert the data to a string (e.g. for Wt::ItemDataRole::Display).
 *
 * \endif
 * 
 * \if cpp
 * Conversion between native types and Wt::any is done like this:
 * <ul>
 *  <li>Conversion from <i>v</i> (of type <i>Type</i>) to Wt::any <i>a</i>
 *   (for setData() and setHeaderData())
 *    <pre>
 * Wt::any <i>a</i> = Wt::any(<i>v</i>);
 *    </pre>
 *   For example:
 *    <pre>
 * WDate d(1976,6,14);
 * model->setData(row, column, Wt::any(d));
 *    </pre>
 * 
 *  </li>
 *  <li>Conversion from Wt::any <i>a</i> to <i>v</i> (of type <i>Type</i>)
     (for data() and headerData()):
 *    <pre>
 * <i>Type v</i> = Wt::any_cast<<i>Type</i>>(<i>a</i>);
 *    </pre>
 *   For example:
 *    <pre>
 * WDate d = Wt::any_cast<WDate>(model->data(row, column));
 *    </pre>
 *  </li>
 *  <li>Checking if a Wt::any <i>a</i> holds a value:</li>
 *    <pre>
 * if (!<i>a</i>.empty()) {
 *   ...
 * }
 *    </pre>
 *  </li>
 *  <li>Determining the value type of a Wt::any <i>a</i>, for example:</li>
 *    <pre>
 * if (<i>a</i>.type() == typeid(double)) {
 *   ...
 * }
 *    </pre>
 *  </li>
 * </ul>
 *
 * \endif 
 *
 * To implement a custom model, you need to reimplement the following methods:
 *  - index() and parent() methods that allow one to navigate the model
 *  - columnCount() and rowCount() to specify the top level geometry and the
 *    nested geometry at every item
 *  - data() to return the data for an item
 *  - optionally, headerData() to return row and column header data
 *  - optionally, flags() to indicate data options
 *
 * \if cpp
 * A crucial point in implementing a hierarchical model is to decide
 * how to reference an index in terms of an internal pointer
 * (WModelIndex::internalPointer()) or internal id
 * (WModelIndex::internalId()). Other than the top-level index, which
 * is special since it is referenced using an \link
 * WModelIndex::isValid() invalid\endlink index, every index with
 * children must be identifiable using this number or pointer. For
 * example, in the WStandardItemModel, the internal pointer points to
 * the parent WStandardItem. For table models, the internal pointer
 * plays no role, since only the toplevel index has children.
 * \elseif java
 * A crucial point in implementing a hierarchical model is to decide
 * how to reference an index in terms of an internal pointer
 * (WModelIndex::internalPointer()).
 * Other than the top-level index, which is special since it is 
 * referenced using an invalid index, every index with
 * children must be identifiable using this object. For
 * example, in the WStandardItemModel, the internal pointer points to
 * the parent WStandardItem. For table models, the internal pointer
 * plays no role, since only the toplevel index has children.
 * \endif
 *
 * If you want to support editing of the model, then you need to
 * indicate this support using a Wt::ItemFlag::Editable flag, and
 * reimplement setData(). View classes will use the
 * \link Wt::ItemDataRole::Edit ItemDataRole::Edit\endlink to read and update
 * the data for the editor.
 *
 * When the model's data has been changed, the model must emit the
 * dataChanged() signal.
 *
 * Finally, there is a generic interface for insertion of new data or
 * removal of data (changing the geometry), although this interface is
 * not yet used by any View class:
 *
 * - insertRows()
 * - insertColumns()
 * - removeRows()
 * - removeColumns()
 *
 * Alternatively, you can provide your own API for changing the
 * model. In either case it is important that you call the
 * corresponding protected member functions which will emit the
 * relevant signals so that views can adapt themselves to the new
 * geometry.
 *
 * \ingroup modelview
 */
class WT_API WAbstractItemModel : public WObject
{
public:
  /*! \brief Data map.
   *
   * A map of data, indexed by a role.
   */
#ifndef WT_TARGET_JAVA
  typedef std::map<ItemDataRole, cpp17::any> DataMap;
#else
  typedef std::treemap<ItemDataRole, cpp17::any> DataMap;
#endif

  /*! \brief Creates a new data model.
   */
  WAbstractItemModel();

  virtual ~WAbstractItemModel();

  /*! \brief Returns the number of columns.
   *
   * This returns the number of columns at index \p parent.
   *
   * \sa rowCount()
   */
  virtual int columnCount(const WModelIndex& parent = WModelIndex()) const = 0;

  /*! \brief Returns the number of rows.
   *
   * This returns the number of rows at index \p parent.
   *
   * \sa columnCount()
   */
  virtual int rowCount(const WModelIndex& parent = WModelIndex()) const = 0;

  // not yet used by views
  virtual bool canFetchMore(const WModelIndex& parent) const;

  // not yet used by views
  virtual void fetchMore(const WModelIndex& parent);

  /*! \brief Returns the flags for an item.
   *
   * The default implementation returns \link Wt::ItemFlag::Selectable
   * ItemFlag::Selectable\endlink.
   *
   * \sa Wt::ItemFlag
   */
  virtual WFlags<ItemFlag> flags(const WModelIndex& index) const;

  /*! \brief Returns the flags for a header.
   *
   * The default implementation returns no flags set.
   *
   * \sa Wt::HeaderFlag
   */
  virtual WFlags<HeaderFlag> headerFlags
    (int section, Orientation orientation = Orientation::Horizontal) const;

  /*! \brief Returns if there are children at an index.
   *
   * Returns \c true when rowCount(index) > 0 and columnCount(index) > 0.
   *
   * \sa rowCount(), columnCount()
   */
  virtual bool hasChildren(const WModelIndex& index) const;

  /*! \brief Returns the parent for a model index.
   *
   * An implementation should use createIndex() to create a model
   * index that corresponds to the parent of a given index.
   *
   * Note that the index itself may be stale (referencing a row/column
   * within the parent that is outside the model geometry), but its
   * parent (identified by the WModelIndex::internalPointer()) is
   * referencing an existing parent. A stale index can only be used
   * while the model geometry is being updated, i.e. during the
   * emission of the corresponding
   * [rows/columns](Being)[Removed/Inserted]() signals.
   *
   * \sa index()
   */
  virtual WModelIndex parent(const WModelIndex& index) const = 0;

  /*! \brief Returns data at a specified model index for the given role.
   *
   * You should check the \p role to decide what data to
   * return. Usually a View class will ask for data for several roles
   * which affect not only the contents
   * (Wt::ItemDataRole::Display) but also icons
   * (Wt::ItemDataRole::Decoration), URLs
   * (Wt::ItemDataRole::Link), and other visual aspects. If your
   * item does not specify data for a particular role, it should
   * simply return a Wt::cpp17::any().
   *
   * \sa flags(), headerData(), setData()
   */
  virtual cpp17::any data(const WModelIndex& index, 
                       ItemDataRole role = ItemDataRole::Display)
    const = 0;

  /*! \brief Returns all data at a specific index.
   *
   * This is a convenience function that returns a map with data
   * corresponding to all standard roles.
   *
   * \sa data()
   */
  virtual DataMap itemData(const WModelIndex& index) const;

  /*! \brief Returns the row or column header data.
   *
   * When \p orientation is \link Wt::Orientation::Horizontal
   * Orientation::Horizontal\endlink, \p section is a column number, when
   * \p orientation is \link Wt::Orientation::Vertical Orientation::Vertical\endlink,
   * \p section is a row number.
   *
   * \sa data(), setHeaderData()
   */
  virtual cpp17::any headerData(int section,
			     Orientation orientation = Orientation::Horizontal,
                             ItemDataRole role = ItemDataRole::Display) const;

  /*! \brief Returns the child index for the given row and column.
   *
   * When implementing this method, you can use createIndex() to
   * create an index that corresponds to the item at \p row and
   * \p column within \p parent.
   *
   * If the location is invalid (out of bounds at the parent), then an
   * invalid index must be returned.
   *
   * \sa parent()
   */
  virtual WModelIndex index(int row, int column,
			    const WModelIndex& parent = WModelIndex())
    const = 0;

  /*! \brief Returns an index list for data items that match.
   *
   * Returns an index list of data items that match, starting at
   * start, and searching further in that column. If flags specifies
   * \link Wt::MatchFlag::Wrap MatchFlag::Wrap \endlink then the search wraps around 
   * from the start. If hits is not -1, then at most that number of 
   * hits are returned.
   */
  virtual WModelIndexList match(const WModelIndex& start,
                                ItemDataRole role,
				const cpp17::any& value,
				int hits = -1,
				WFlags<MatchFlag> flags
				= WFlags<MatchFlag>(MatchFlag::StartsWith
						    | MatchFlag::Wrap))
    const;

  /*! \brief Returns the data item at the given column and row.
   *
   * This is a convenience method, and is equivalent to:
   * \code
   * index(row, column, parent).data(role)
   * \endcode
   *
   * \sa index(), data()
   */
  cpp17::any data(int row, int column, 
               ItemDataRole role = ItemDataRole::Display,
	       const WModelIndex& parent = WModelIndex()) const;

  /*! \brief Returns if an index at the given position is valid
   *         (i.e. falls within the column-row bounds).
   *
   * Equivalent to:
   * \code
   * return row >= 0 && column >= 0
   *        && row < rowCount(parent) && column < columnCount(parent);
   * \endcode
   *
   * \sa rowCount(), columnCount()
   */
  virtual bool hasIndex(int row, int column,
			const WModelIndex& parent = WModelIndex()) const;

  /*! \brief Inserts one or more columns.
   *
   * In models that support column insertion, this inserts \c count
   * columns, starting at \c column, and returns \c true if the
   * operation was successful. The new columns are inserted under \p
   * parent.
   *
   * The default implementation returns \c false.
   *
   * The model implementation must call beginInsertColumns() and
   * endInsertColumns() before and after the operation whenever its
   * geometry is changed by inserting columns. This emits signals for
   * views to properly react to these changes.
   *
   * \sa insertRows(), removeColumns(), beginInsertColumns(), endInsertColumns()
   */
  virtual bool insertColumns(int column, int count,
			     const WModelIndex& parent = WModelIndex());

  /*! \brief Inserts one or more rows.
   *
   * In models that support row insertion, this inserts \c count rows,
   * starting at \c row, and returns \c true if the operation was
   * successful. The new rows are inserted under \p parent.
   *
   * If parent had no children, then a single column is added with \c
   * count rows.
   *
   * The default implementation returns \c false.
   *
   * The model implementation must call beginInsertRows() and
   * endInsertRows() before and after the operation whenever its
   * geometry is changed by inserting rows. This emits signals for
   * views to properly react to these changes.
   *
   * \sa insertColumns(), removeRows(), beginInsertRows(), endInsertRows()
   */
  virtual bool insertRows(int row, int count,
			  const WModelIndex& parent = WModelIndex());

  /*! \brief Removes columns.
   *
   * Returns \c true if the operation was successful.
   *
   * The default implementation returns \c false.
   *
   * The model implementation must call beginRemoveColumns() and
   * endRemoveColumns() before and after the operation whenever its
   * geometry is changed by removing columns. This emits signals for
   * views to properly react to these changes.
   *
   * \sa removeRows(), insertColumns(), beginRemoveColumns(), endRemoveColumns()
   */
  virtual bool removeColumns(int column, int count,
			     const WModelIndex& parent = WModelIndex());

  /*! \brief Removes rows.
   *
   * Returns \c true if the operation was successful.
   *
   * The default implementation returns \c false.
   *
   * The model implementation must call beginRemoveRows() and
   * endRemoveRows() before and after the operation whenever its
   * geometry is changed by removing rows. This emits signals for
   * views to properly react to these changes.
   *
   * \sa removeColumns(), insertRows(), beginRemoveRows(), endRemoveRows()
   */
  virtual bool removeRows(int row, int count,
			  const WModelIndex& parent = WModelIndex());

  /*! \brief Sets data at the given model index.
   *
   * Returns \c true if the operation was successful.
   *
   * The default implementation returns \c false.
   *
   * The model implementation must emit the dataChanged() signal after
   * data was changed.
   *
   * \sa data()
   */
  virtual bool setData(const WModelIndex& index, const cpp17::any& value,
                       ItemDataRole role = ItemDataRole::Edit);

  /*! \brief Sets data at the given model index.
   *
   * This is a convenience function that sets data for all roles at once.
   *
   * \sa setData()
   */
  virtual bool setItemData(const WModelIndex& index, const DataMap& values);

  /*! \brief Sets header data for a column or row.
   *
   * Returns \c true if the operation was successful.
   *
   * \sa headerData()
   */
  virtual bool setHeaderData(int section, Orientation orientation,
			     const cpp17::any& value,
                             ItemDataRole role = ItemDataRole::Edit);

  /*! \brief Sets column header data.
   *
   * Returns \c true if the operation was successful.
   *
   * \sa setHeaderData(int, Orientation, const cpp17::any&, int)
   */
  bool setHeaderData(int section, const cpp17::any& value);

  /*! \brief Sorts the model according to a particular column.
   *
   * If the model supports sorting, then it should emit the
   * layoutAboutToBeChanged() signal, rearrange its items, and
   * afterwards emit the layoutChanged() signal.
   *
   * \sa layoutAboutToBeChanged(), layoutChanged()
   */
  virtual void sort(int column, SortOrder order = SortOrder::Ascending);

  /*! \brief Expands a column.
   *
   * Expands a column. This may only be called by a view when the
   * Wt::HeaderFlag::ColumnIsCollapsed flag is set.
   *
   * The default implementation does nothing.
   *
   * \sa WAggregateProxyModel
   */
  virtual void expandColumn(int column);

  /*! \brief Collapses a column.
   *
   * Collapses a column. This may only be called by a view when the
   * Wt::HeaderFlag::ColumnIsExpandedLeft or Wt::HeaderFlag::ColumnIsExpandedRight flag is set.
   *
   * The default implementation does nothing.
   *
   * \sa WAggregateProxyModel
   */
  virtual void collapseColumn(int column);

  /*! \brief Converts a model index to a raw pointer that remains valid
   *         while the model's layout is changed. 
   *
   * Use this method to temporarily save model indexes while the model's
   * layout is changed by for example a sorting operation.
   *
   * The default implementation returns \c 0, which indicates that the
   * index cannot be converted to a raw pointer. If you reimplement
   * this method, you also need to reimplemnt fromRawIndex().
   *
   * \sa layoutAboutToBeChanged, sort(), fromRawIndex()
   */
  virtual void *toRawIndex(const WModelIndex& index) const;

  /*! \brief Converts a raw pointer to a model index. 
   *
   * Use this method to create model index from temporary raw
   * pointers. It is the reciproce method of toRawIndex().
   *
   * You can return an invalid modelindex if the rawIndex no longer points
   * to a valid item because of the layout change.
   *
   * \sa toRawIndex()
   */
  virtual WModelIndex fromRawIndex(void *rawIndex) const;

  /*! \brief Returns a mime-type for dragging a set of indexes.
   *
   * This method returns a mime-type that describes dragging of a selection
   * of items.
   *
   * The drop event will indicate a \link WItemSelectionModel
   * selection model\endlink for this abstract item model as \link
   * WDropEvent::source() source\endlink.
   *
   * The default implementation returns a mime-type for generic
   * drag&drop support between abstract item models.
   *
   * \sa acceptDropMimeTypes()
   */
  virtual std::string mimeType() const;

  /*! \brief Returns a list of mime-types that could be accepted for a
   *         drop event.
   *
   * The default implementation only accepts drag&drop support between
   * abstract item models.
   *
   * \sa mimeType()
   */
  virtual std::vector<std::string> acceptDropMimeTypes() const;

  /*! \brief Handles a drop event.
   *
   * The default implementation only handles generic drag&drop between
   * abstract item models. Source item data is copied (but not the
   * source item's flags).
   *
   * The location in the model is indicated by the \p row and
   * \p column within the \p parent index. If \p row is
   * -1, then the item is appended to the \p parent. Otherwise,
   * the item is inserted at or copied over the indicated item (and
   * subsequent rows). When \p action is a \link Wt::DropAction::Move
   * DropAction::Move\endlink, the original items are deleted from the
   * source model.
   *
   * You may want to reimplement this method if you want to handle
   * other mime-type data, or if you want to refine how the drop event
   * of an item selection must be interpreted.
   *
   * \note Currently, only row selections are handled by the default
   *       implementation.
   *
   * \sa mimeType(), WItemSelectionModel
   */
  virtual void dropEvent(const WDropEvent& e, DropAction action,
			 int row, int column, const WModelIndex& parent);

  /*! \brief Inserts one column.
   *
   * This is a convenience method that adds a single column, and is
   * equivalent to:
   * \code
   * insertColumns(column, 1, parent);
   * \endcode
   *
   * Returns \c true if the operation was successful.
   *
   * \sa insertColumns()
   */
  bool insertColumn(int column, const WModelIndex& parent = WModelIndex());

  /*! \brief Inserts one row.
   *
   * This is a convenience method that adds a single row, and is
   * equivalent to:
   * \code
   * insertRows(row, 1, parent);
   * \endcode
   *
   * Returns \c true if the operation was successful.
   *
   * \sa insertRows()
   */
  bool insertRow(int row, const WModelIndex& parent = WModelIndex());

  /*! \brief Removes one column.
   *
   * This is a convenience method that removes a single column, and is
   * equivalent to:
   * \code
   * removeColumns(column, 1, parent);
   * \endcode
   *
   * Returns \c true if the operation was successful.
   *
   * \sa removeColumns()
   */
  bool removeColumn(int column, const WModelIndex& parent = WModelIndex());

  /*! \brief Removes one row.
   *
   * This is a convenience method that removes a single row, and is
   * equivalent to:
   * \code
   * removeRows(row, 1, parent);
   * \endcode
   *
   * Returns \c true if the operation was successful.
   *
   * \sa removeRows()
   */
  bool removeRow(int row, const WModelIndex& parent = WModelIndex());

  /*! \brief Sets data at the given row and column.
   *
   * This is a convience method, and is equivalent to:
   * \code
   * setData(index(row, column, parent), value, role);
   * \endcode
   *
   * Returns \c true if the operation was successful.
   *
   * \sa setData(), index()
   */
  bool setData(int row, int column, const cpp17::any& value,
               ItemDataRole role = ItemDataRole::Edit,
	       const WModelIndex& parent = WModelIndex());

  /*! \brief %Signal emitted before a number of columns will be inserted.
   *
   * The first argument is the parent index. The two integer arguments
   * are the column numbers that the first and last column will have when
   * inserted.
   *
   * \sa columnsInserted(), beginInsertColumns()
   */
  virtual Signal<WModelIndex, int, int>& columnsAboutToBeInserted()
    { return columnsAboutToBeInserted_; }

  /*! \brief %Signal emitted before a number of columns will be removed.
   *
   * The first argument is the parent index. The two integer arguments
   * are the column numbers of the first and last column that will be
   * removed.
   *
   * \sa columnsRemoved(), beginRemoveColumns()
   */
  virtual Signal<WModelIndex, int, int>& columnsAboutToBeRemoved()
    { return columnsAboutToBeRemoved_; }
 
  /*! \brief %Signal emitted after a number of columns were inserted.
   *
   * The first argument is the parent index. The two integer arguments
   * are the column numbers of the first and last column that were
   * inserted.
   *
   * \sa columnsAboutToBeInserted(), endInsertColumns()
   */
  virtual Signal<WModelIndex, int, int>& columnsInserted()
    { return columnsInserted_; }

  /*! \brief %Signal emitted after a number of columns were removed.
   *
   * The first argument is the parent index. The two integer arguments
   * are the column numbers of the first and last column that were removed.
   *
   * \sa columnsAboutToBeRemoved(), endRemoveColumns()
   */
  virtual Signal<WModelIndex, int, int>& columnsRemoved()
    { return columnsRemoved_; }

  /*! \brief %Signal emitted before a number of rows will be inserted.
   *
   * The first argument is the parent index. The two integer arguments
   * are the row numbers that the first and last row will have when
   * inserted.
   *
   * \sa rowsInserted(), beginInsertRows()
   */
  virtual Signal<WModelIndex, int, int>& rowsAboutToBeInserted()
    { return rowsAboutToBeInserted_; }

  /*! \brief %Signal emitted before a number of rows will be removed.
   *
   * The first argument is the parent index. The two integer arguments
   * are the row numbers of the first and last row that will be
   * removed.
   *
   * \sa rowsRemoved(), beginRemoveRows()
   */
  virtual Signal<WModelIndex, int, int>& rowsAboutToBeRemoved()
    { return rowsAboutToBeRemoved_; }
 
  /*! \brief %Signal emitted after a number of rows were inserted.
   *
   * The first argument is the parent index. The two integer arguments
   * are the row numbers of the first and last row that were inserted.
   *
   * \sa rowsAboutToBeInserted(), endInsertRows()
   */
  virtual Signal<WModelIndex, int, int>& rowsInserted()
    { return rowsInserted_; }

  /*! \brief %Signal emitted after a number of rows were removed.
   *
   * The first argument is the parent index. The two integer arguments
   * are the row numbers of the first and last row that were removed.
   *
   * \sa rowsAboutToBeRemoved(), endRemoveRows()
   */
  virtual Signal<WModelIndex, int, int>& rowsRemoved()
    { return rowsRemoved_; }

  /*! \brief %Signal emitted when some data was changed.
   *
   * The two arguments are the model indexes of the top-left and bottom-right
   * data items that span the rectangle of changed data items.
   *
   * \sa setData()
   */
  virtual Signal<WModelIndex, WModelIndex>& dataChanged()
    { return dataChanged_; }

  /*! \brief %Signal emitted when some header data was changed.
   *
   * The first argument indicates the orientation of the header, and
   * the two integer arguments are the row or column numbers of the
   * first and last header item of which the value was changed.
   *
   * \sa setHeaderData()
   */
  virtual Signal<Orientation, int, int>& headerDataChanged()
    { return headerDataChanged_; }

  /*! \brief %Signal emitted when the layout is about to be changed.
   *
   * A layout change may reorder or add/remove rows in the model, but
   * columns are preserved. Model indexes are invalidated by a layout
   * change, but indexes may be ported across a layout change by using
   * the toRawIndex() and fromRawIndex() methods.
   *
   * \sa layoutChanged(), toRawIndex(), fromRawIndex()
   */
  virtual Signal<>& layoutAboutToBeChanged() { return layoutAboutToBeChanged_; }

  /*! \brief %Signal emitted when the layout is changed.
   *
   * \sa layoutAboutToBeChanged()
   */
  virtual Signal<>& layoutChanged() { return layoutChanged_; }

  /*! \brief %Signal emitted when the model was reset.
   *
   * A model reset invalidates all existing data, and the model may change
   * its entire geometry (column count, row count).
   *
   * \sa reset()
   */
  virtual Signal<>& modelReset() { return modelReset_; }

protected:
  /*! \brief Resets the model and invalidate any data.
   *
   * Informs any attached view that all data in the model was invalidated,
   * and the model's data should be reread.
   *
   * This causes the modelReset() signal to be emitted.
   */
  void reset();

  /*! \brief Creates a model index for the given row and column.
   *
   * Use this method to create a model index. \p ptr is an internal
   * pointer that may be used to identify the <b>parent</b> of the
   * corresponding item. For a flat table model, \p ptr can thus
   * always be 0.
   *
   * \sa WModelIndex::internalPointer()
   */
  WModelIndex createIndex(int row, int column, void *ptr) const;

  /*! \brief Creates a model index for the given row and column.
   *
   * Use this method to create a model index. \p id is an internal id
   * that may be used to identify the <b>parent</b> of the
   * corresponding item. For a flat table model, \p ptr can thus
   * always be 0.
   *
   * \sa WModelIndex::internalId()
   */
  WModelIndex createIndex(int row, int column, ::uint64_t id) const;

  /*! \brief Method to be called before inserting columns.
   *
   * If your model supports insertion of columns, then you should call
   * this method before inserting one or more columns, and
   * endInsertColumns() afterwards. These methods emit the necessary
   * signals to allow view classes to update themselves.
   *
   * \sa endInsertColumns(), insertColumns(), columnsAboutToBeInserted
   */  
  void beginInsertColumns(const WModelIndex& parent, int first, int last);

  /*! \brief Method to be called before inserting rows.
   *
   * If your model supports insertion of rows, then you should call
   * this method before inserting one or more rows, and
   * endInsertRows() afterwards. These methods emit the necessary
   * signals to allow view classes to update themselves.
   *
   * \sa endInsertRows(), insertRows(), rowsAboutToBeInserted
   */  
  void beginInsertRows(const WModelIndex& parent, int first, int last);

  /*! \brief Method to be called before removing columns.
   *
   * If your model supports removal of columns, then you should call
   * this method before removing one or more columns, and
   * endRemoveColumns() afterwards. These methods emit the necessary
   * signals to allow view classes to update themselves.
   *
   * \sa endRemoveColumns(), removeColumns(), columnsAboutToBeRemoved
   */
  void beginRemoveColumns(const WModelIndex& parent, int first, int last);

  /*! \brief Method to be called before removing rows.
   *
   * If your model supports removal of rows, then you should call this
   * method before removing one or more rows, and endRemoveRows()
   * afterwards. These methods emit the necessary signals to allow
   * view classes to update themselves.
   *
   * \sa endRemoveRows(), removeRows(), rowsAboutToBeRemoved
   */  
  void beginRemoveRows(const WModelIndex& parent, int first, int last);

  /*! \brief Method to be called after inserting columns.
   *
   * \sa beginInsertColumns()
   */
  void endInsertColumns();

  /*! \brief Method to be called after inserting rows.
   *
   * \sa beginInsertRows()
   */  
  void endInsertRows();

  /*! \brief Method to be called after removing columns.
   *
   * \sa beginRemoveColumns()
   */  
  void endRemoveColumns();

  /*! \brief Method to be called after removing rows.
   *
   * \sa beginRemoveRows()
   */
  void endRemoveRows();

private:
  int first_, last_;
  WModelIndex parent_;

  Signal<WModelIndex, int, int> columnsAboutToBeInserted_;
  Signal<WModelIndex, int, int> columnsAboutToBeRemoved_;
  Signal<WModelIndex, int, int> columnsInserted_;
  Signal<WModelIndex, int, int> columnsRemoved_;
  Signal<WModelIndex, int, int> rowsAboutToBeInserted_;
  Signal<WModelIndex, int, int> rowsAboutToBeRemoved_;
  Signal<WModelIndex, int, int> rowsInserted_;
  Signal<WModelIndex, int, int> rowsRemoved_;
  Signal<WModelIndex, WModelIndex> dataChanged_;
  Signal<Orientation, int, int> headerDataChanged_;
  Signal<> layoutAboutToBeChanged_;
  Signal<> layoutChanged_;
  Signal<> modelReset_;

  static void copyData(const WAbstractItemModel *source,
		       const WModelIndex& sIndex,
		       WAbstractItemModel *destination,
		       const WModelIndex& dIndex);

  friend class WAbstractProxyModel;
};

}

#endif // WABSTRACT_ITEM_MODEL_H_
