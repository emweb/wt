/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <stdio.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Wt/WModelIndex"
#include "Wt/WAbstractItemModel"
#include "Wt/WItemSelectionModel"
#include "Wt/WDate"
#include "Wt/WDateTime"
#include "Wt/WTime"
#include "Wt/WWebWidget"

#include "WtException.h"
#include "Utils.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace {
  const char *DRAG_DROP_MIME_TYPE = "application/x-wabstractitemmodelselection";
}

namespace Wt {

#ifdef WT_TARGET_JAVA
extern bool matchValue(const boost::any& value, const boost::any& query,
		       WFlags<MatchFlag> flags);

#else

namespace {

bool matchValue(const boost::any& value,
		const boost::any& query,
		WFlags<MatchFlag> flags)
{
  WFlags<MatchFlag> f = flags & MatchTypeMask;

  if ((f & MatchTypeMask) == MatchExactly)
    return (query.type() == value.type()) && asString(query) == asString(value);
  else {
    std::string query_str = asString(query).toUTF8();
    std::string value_str = asString(value).toUTF8();

    switch (f) {
    case MatchStringExactly:
      return boost::iequals(value_str, query_str);
    case MatchStringExactly | (int)MatchCaseSensitive:
      return boost::equals(value_str, query_str);

    case MatchStartsWith:
      return boost::istarts_with(value_str, query_str);
    case MatchStartsWith | (int)MatchCaseSensitive:
      return boost::starts_with(value_str, query_str);

    case MatchEndsWith:
      return boost::iends_with(value_str, query_str);
    case MatchEndsWith | (int)MatchCaseSensitive:
      return boost::ends_with(value_str, query_str);

    default:
      throw WtException("Not yet implemented: WAbstractItemModel::match with "
			"MatchFlags = "
			+ boost::lexical_cast<std::string>(flags));
    }
  }
}

}

std::string asJSLiteral(const boost::any& v)
{
  if (v.empty())
    return std::string("''");
  else if (v.type() == typeid(WString))
    return boost::any_cast<WString>(v).jsStringLiteral();
  else if (v.type() == typeid(std::string))
    return
      WWebWidget::jsStringLiteral(boost::any_cast<std::string>(v),
				  '\'');
  else if (v.type() == typeid(const char *))
    return
      WWebWidget::jsStringLiteral(std::string(boost::any_cast<const char *>(v)),
				  '\'');
  else if (v.type() == typeid(WDate)) {
    const WDate& d = boost::any_cast<WDate>(v);

    return "new Date(" + boost::lexical_cast<std::string>(d.year())
      + ',' + boost::lexical_cast<std::string>(d.month())
      + "," + boost::lexical_cast<std::string>(d.day())
      + ")";
  } else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = boost::any_cast<WDateTime>(v);
    const WDate& d = dt.date();
    const WTime& t = dt.time();

    return "new Date(" + boost::lexical_cast<std::string>(d.year())
      + ',' + boost::lexical_cast<std::string>(d.month())
      + "," + boost::lexical_cast<std::string>(d.day())
      + ',' + boost::lexical_cast<std::string>(t.hour())
      + "," + boost::lexical_cast<std::string>(t.minute())
      + ',' + boost::lexical_cast<std::string>(t.second())
      + "," + boost::lexical_cast<std::string>(t.msec())
      + ")";
  }

#define ELSE_LEXICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return boost::lexical_cast<std::string>(boost::any_cast<TYPE>(v))

  ELSE_LEXICAL_ANY(short);
  ELSE_LEXICAL_ANY(unsigned short);
  ELSE_LEXICAL_ANY(int);
  ELSE_LEXICAL_ANY(unsigned int);
  ELSE_LEXICAL_ANY(long);
  ELSE_LEXICAL_ANY(unsigned long);
  ELSE_LEXICAL_ANY(int64_t);
  ELSE_LEXICAL_ANY(uint64_t);
  ELSE_LEXICAL_ANY(float);
  ELSE_LEXICAL_ANY(double);

#undef ELSE_LEXICAL_ANY

  else
    throw WtException(std::string("WAbstractItemModel: unsupported type ")
		      + v.type().name());
}

WString asString(const boost::any& v, const WT_USTRING& format)
{
  if (v.empty())
    return WString();
  else if (v.type() == typeid(WString))
    return boost::any_cast<WString>(v);
  else if (v.type() == typeid(std::string))
    return WString::fromUTF8(boost::any_cast<std::string>(v));
  else if (v.type() == typeid(const char *))
    return WString::fromUTF8(boost::any_cast<const char *>(v));
  else if (v.type() == typeid(WDate)) {
    const WDate& d = boost::any_cast<WDate>(v);
    return d.toString(format.empty() ? "dd/MM/yy" : format);
  } else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = boost::any_cast<WDateTime>(v);
    return dt.toString(format.empty() ? "dd/MM/yy HH:mm:ss" : format);
  } else if (v.type() == typeid(WTime)) {
    const WTime& t = boost::any_cast<WTime>(v);
    return t.toString(format.empty() ? "HH:mm:ss" : format);
  }

#define ELSE_LEXICAL_ANY(TYPE)						\
  else if (v.type() == typeid(TYPE)) {					\
    if (format.empty())							\
      return WString(boost::lexical_cast<std::string>			\
		     (boost::any_cast<TYPE>(v)));			\
    else {								\
      char buf[100];							\
      snprintf(buf, 100, format.toUTF8().c_str(), boost::any_cast<TYPE>(v)); \
      return WString::fromUTF8(buf);					\
    }									\
  }

  ELSE_LEXICAL_ANY(short)
  ELSE_LEXICAL_ANY(unsigned short)
  ELSE_LEXICAL_ANY(int)
  ELSE_LEXICAL_ANY(unsigned int)
  ELSE_LEXICAL_ANY(long)
  ELSE_LEXICAL_ANY(unsigned long)
  ELSE_LEXICAL_ANY(int64_t)
  ELSE_LEXICAL_ANY(uint64_t)
  ELSE_LEXICAL_ANY(float)
  ELSE_LEXICAL_ANY(double)

#undef ELSE_LEXICAL_ANY

  else
    throw WtException(std::string("WAbstractItemModel: unsupported type ")
		      + v.type().name());
}

double asNumber(const boost::any& v)
{
  if (v.empty())
    return std::numeric_limits<double>::signaling_NaN();
  else if (v.type() == typeid(WString))
    try {
      return boost::lexical_cast<double>(boost::any_cast<WString>(v).toUTF8());
    } catch (boost::bad_lexical_cast& e) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(std::string))
    try {
      return boost::lexical_cast<double>(boost::any_cast<std::string>(v));
    } catch (boost::bad_lexical_cast& e) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(const char *))
    try {
      return boost::lexical_cast<double>(boost::any_cast<const char *>(v));
    } catch (boost::bad_lexical_cast&) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(WDate))
    return static_cast<double>(boost::any_cast<WDate>(v).toJulianDay());
  else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = boost::any_cast<WDateTime>(v);
    return static_cast<double>(dt.toTime_t());
  } else if (v.type() == typeid(WTime)) {
    const WTime& t = boost::any_cast<WTime>(v);
    return static_cast<double>(WTime(0, 0).msecsTo(t));
  }

#define ELSE_NUMERICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return static_cast<double>(boost::any_cast<TYPE>(v))

  ELSE_NUMERICAL_ANY(short);
  ELSE_NUMERICAL_ANY(unsigned short);
  ELSE_NUMERICAL_ANY(int);
  ELSE_NUMERICAL_ANY(unsigned int);
  ELSE_NUMERICAL_ANY(long);
  ELSE_NUMERICAL_ANY(unsigned long);
  ELSE_NUMERICAL_ANY(int64_t);
  ELSE_NUMERICAL_ANY(uint64_t);
  ELSE_NUMERICAL_ANY(float);
  ELSE_NUMERICAL_ANY(double);

#undef ELSE_NUMERICAL_ANY

  else
    throw WtException(std::string("WAbstractItemModel: unsupported type ")
		      + v.type().name());
}

boost::any updateFromJS(const boost::any& v, std::string s)
{
  if (v.empty())
    return boost::any(s);
  else if (v.type() == typeid(WString))
    return boost::any(WString::fromUTF8(s));
  else if (v.type() == typeid(std::string))
    return boost::any(s);
  else if (v.type() == typeid(const char *))
    return boost::any(s);
  else if (v.type() == typeid(WDate))
    return boost::any(WDate::fromString(WString::fromUTF8(s),
					"ddd MMM d yyyy"));
  else if (v.type() == typeid(WDateTime))
    return boost::any(WDateTime::fromString(WString::fromUTF8(s),
					    "ddd MMM d yyyy HH:mm:ss"));
#define ELSE_LEXICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return boost::any(boost::lexical_cast<TYPE>(s))

  ELSE_LEXICAL_ANY(short);
  ELSE_LEXICAL_ANY(unsigned short);
  ELSE_LEXICAL_ANY(int);
  ELSE_LEXICAL_ANY(unsigned int);
  ELSE_LEXICAL_ANY(long);
  ELSE_LEXICAL_ANY(unsigned long);
  ELSE_LEXICAL_ANY(int64_t);
  ELSE_LEXICAL_ANY(uint64_t);
  ELSE_LEXICAL_ANY(float);
  ELSE_LEXICAL_ANY(double);

#undef ELSE_LEXICAL_ANY

  else
    throw WtException(std::string("WAbstractItemModel: unsupported type ")
		      + v.type().name());
}

int compare(const boost::any& d1, const boost::any& d2)
{
  const int UNSPECIFIED_RESULT = -1;

  /*
   * If the types are the same then we use std::operator< on that type
   * otherwise we compare lexicographically
   */
  if (!d1.empty())
    if (!d2.empty()) {
      if (d1.type() == d2.type()) {
	if (d1.type() == typeid(bool))
	  return static_cast<int>(boost::any_cast<bool>(d1))
	    - static_cast<int>(boost::any_cast<bool>(d2));

#define ELSE_COMPARE_ANY(TYPE)				\
	else if (d1.type() == typeid(TYPE)) {		\
	  TYPE v1 = boost::any_cast<TYPE>(d1);		\
	  TYPE v2 = boost::any_cast<TYPE>(d2);		\
	  return v1 == v2 ? 0 : (v1 < v2 ? -1 : 1);	\
        }

	ELSE_COMPARE_ANY(WString)
	ELSE_COMPARE_ANY(std::string)
	ELSE_COMPARE_ANY(WDate)
	ELSE_COMPARE_ANY(WDateTime)
	ELSE_COMPARE_ANY(WTime)
	ELSE_COMPARE_ANY(short)
	ELSE_COMPARE_ANY(unsigned short)
	ELSE_COMPARE_ANY(int)
	ELSE_COMPARE_ANY(unsigned int)
	ELSE_COMPARE_ANY(long)
	ELSE_COMPARE_ANY(unsigned long)
	ELSE_COMPARE_ANY(int64_t)
	ELSE_COMPARE_ANY(uint64_t)
	ELSE_COMPARE_ANY(float)
	ELSE_COMPARE_ANY(double)

#undef ELSE_COMPARE_ANY
	else
	  throw WtException(std::string("WAbstractItemModel: unsupported type ")
			    + d1.type().name());
      } else {
	WString s1 = asString(d1);
	WString s2 = asString(d2);

	return s1 == s2 ? 0 : (s1 < s2 ? -1 : 1);
      }
    } else
      return -UNSPECIFIED_RESULT;
  else
    if (!d2.empty())
      return UNSPECIFIED_RESULT;
    else
      return 0;
}

#endif // WT_TARGET_JAVA

WAbstractItemModel::WAbstractItemModel(WObject *parent)
  : WObject(parent),
    columnsAboutToBeInserted_(this),
    columnsAboutToBeRemoved_(this),
    columnsInserted_(this),
    columnsRemoved_(this),
    rowsAboutToBeInserted_(this),
    rowsAboutToBeRemoved_(this),
    rowsInserted_(this),
    rowsRemoved_(this),
    dataChanged_(this),
    headerDataChanged_(this),
    layoutAboutToBeChanged_(this),
    layoutChanged_(this),
    modelReset_(this)
{ }

WAbstractItemModel::~WAbstractItemModel()
{ }

bool WAbstractItemModel::canFetchMore(const WModelIndex& parent) const
{
  return false;
}

void WAbstractItemModel::fetchMore(const WModelIndex& parent)
{ }

WFlags<ItemFlag> WAbstractItemModel::flags(const WModelIndex& index) const
{
  return ItemIsSelectable;
}

WFlags<HeaderFlag> WAbstractItemModel::headerFlags(int section,
						   Orientation orientation)
  const
{
  return 0;
}

bool WAbstractItemModel::hasChildren(const WModelIndex& index) const
{
  return rowCount(index) > 0 && columnCount(index) > 0;
}

bool WAbstractItemModel::hasIndex(int row, int column,
				  const WModelIndex& parent) const
{
  return (row >= 0
	  && column >= 0
	  && row < rowCount(parent)
	  && column < columnCount(parent));
}

WAbstractItemModel::DataMap
WAbstractItemModel::itemData(const WModelIndex& index) const
{
  DataMap result;

  if (index.isValid()) {
    for (int i = 0; i <= UrlRole; ++i)
      result[i] = data(index, i);
    result[UserRole] = data(index, UserRole);
  }

  return result;
}

boost::any WAbstractItemModel::data(int row, int column, int role,
				    const WModelIndex& parent) const
{
  return data(index(row, column, parent), role);
}

boost::any WAbstractItemModel::headerData(int section,
					  Orientation orientation,
					  int role) const
{
  if (role == LevelRole)
    return 0;
  else
    return boost::any();
}

void WAbstractItemModel::sort(int column, SortOrder order)
{ }

void WAbstractItemModel::expandColumn(int column)
{ }

void WAbstractItemModel::collapseColumn(int column)
{ }

bool WAbstractItemModel::insertColumns(int column, int count,
				       const WModelIndex& parent)
{
  return false;
}

bool WAbstractItemModel::insertRows(int row, int count,
				    const WModelIndex& parent)
{
  return false;
}

bool WAbstractItemModel::removeColumns(int column, int count,
				       const WModelIndex& parent)
{
  return false;
}

bool WAbstractItemModel::removeRows(int row, int count,
				    const WModelIndex& parent)
{
  return false;
}

bool WAbstractItemModel::setData(const WModelIndex& index,
				 const boost::any& value, int role)
{
  return false;
}

bool WAbstractItemModel::setHeaderData(int section, Orientation orientation,
				       const boost::any& value, int role)
{
  return false;
}

bool WAbstractItemModel::setHeaderData(int section, const boost::any& value)
{
  return setHeaderData(section, Horizontal, value);
}

bool WAbstractItemModel::setItemData(const WModelIndex& index,
				     const DataMap& values)
{
  bool result = true;

  bool wasBlocked = dataChanged().isBlocked();
  dataChanged().setBlocked(true);

  for (DataMap::const_iterator i = values.begin(); i != values.end(); ++i)
    if (i->first != EditRole)
      if (!setData(index, i->second, i->first))
	result = false;

  dataChanged().setBlocked(wasBlocked);
  dataChanged().emit(index, index);

  return result;
}

bool WAbstractItemModel::insertColumn(int column, const WModelIndex& parent)
{
  return insertColumns(column, 1, parent);
}

bool WAbstractItemModel::insertRow(int row, const WModelIndex& parent)
{
  return insertRows(row, 1, parent);
}

bool WAbstractItemModel::removeColumn(int column, const WModelIndex& parent)
{
  return removeColumns(column, 1, parent);
}

bool WAbstractItemModel::removeRow(int row, const WModelIndex& parent)
{
  return removeRows(row, 1, parent);
}

bool WAbstractItemModel::setData(int row, int column, const boost::any& value,
				 int role, const WModelIndex& parent)
{
  WModelIndex i = index(row, column, parent);

  if (i.isValid())
    return setData(i, value, role);
  else
    return false;
}

void WAbstractItemModel::reset()
{
  modelReset_.emit();
}

WModelIndex WAbstractItemModel::createIndex(int row, int column, void *ptr)
  const
{
  return WModelIndex(row, column, this, ptr);
}

#ifndef WT_TARGET_JAVA
WModelIndex WAbstractItemModel::createIndex(int row, int column, uint64_t id)
  const
{
  return WModelIndex(row, column, this, id);
}
#endif // WT_TARGET_JAVA

void *WAbstractItemModel::toRawIndex(const WModelIndex& index) const
{
  return 0;
}

WModelIndex WAbstractItemModel::fromRawIndex(void *rawIndex) const
{
  return WModelIndex();
}

std::string WAbstractItemModel::mimeType() const
{
  return DRAG_DROP_MIME_TYPE;
}

std::vector<std::string> WAbstractItemModel::acceptDropMimeTypes() const
{
  std::vector<std::string> result;

  result.push_back(DRAG_DROP_MIME_TYPE);

  return result;
}

void WAbstractItemModel::copyData(const WAbstractItemModel *source,
				  const WModelIndex& sIndex,
				  WAbstractItemModel *destination,
				  const WModelIndex& dIndex)
{
  destination->setItemData(dIndex, source->itemData(sIndex));
}

void WAbstractItemModel::dropEvent(const WDropEvent& e, DropAction action,
				   int row, int column,
				   const WModelIndex& parent)
{
  // TODO: For now, we assumes selectionBehavior() == RowSelection !

  WItemSelectionModel *selectionModel
    = dynamic_cast<WItemSelectionModel *>(e.source());
  if (selectionModel) {
    WAbstractItemModel *sourceModel = selectionModel->model();

    /*
     * (1) Insert new rows (or later: cells ?)
     */
    if (action == MoveAction || row == -1) {
      if (row == -1)
	row = rowCount(parent);

      insertRows(row, selectionModel->selectedIndexes().size(), parent);
    }

    /*
     * (2) Copy data
     */
    WModelIndexSet selection = selectionModel->selectedIndexes();

    int r = row;
    for (WModelIndexSet::const_iterator i = selection.begin();
	 i != selection.end(); ++i) {
      WModelIndex sourceIndex = *i;
      if (selectionModel->selectionBehavior() == SelectRows) {
	WModelIndex sourceParent = sourceIndex.parent();

	for (int col = 0; col < sourceModel->columnCount(sourceParent); ++col) {
	  WModelIndex s = sourceModel->index(sourceIndex.row(), col,
					     sourceParent);
	  WModelIndex d = index(r, col, parent);
	  copyData(sourceModel, s, this, d);
	}

	++r;
      } else {
	  
      }
    }

    /*
     * (3) Remove original data
     */
    if (action == MoveAction) {
      while (!selectionModel->selectedIndexes().empty()) {
	WModelIndex i = Utils::last(selectionModel->selectedIndexes());

	sourceModel->removeRow(i.row(), i.parent());
      }
    }
  }
}

void WAbstractItemModel::beginInsertColumns(const WModelIndex& parent, 
					    int first, int last)
{
  first_ = first;
  last_ = last;
  parent_ = parent;

  columnsAboutToBeInserted().emit(parent_, first, last);
}

void WAbstractItemModel::endInsertColumns()
{
  columnsInserted().emit(parent_, first_, last_);
}

void WAbstractItemModel::beginInsertRows(const WModelIndex& parent,
					 int first, int last)
{
  first_ = first;
  last_ = last;
  parent_ = parent;

  rowsAboutToBeInserted().emit(parent, first, last);
}

void WAbstractItemModel::endInsertRows()
{
  rowsInserted().emit(parent_, first_, last_);
}

void WAbstractItemModel::beginRemoveColumns(const WModelIndex& parent,
					    int first, int last)
{
  first_ = first;
  last_ = last;
  parent_ = parent;

  columnsAboutToBeRemoved().emit(parent, first, last);
}

void WAbstractItemModel::endRemoveColumns()
{
  columnsRemoved().emit(parent_, first_, last_);
}

void WAbstractItemModel::beginRemoveRows(const WModelIndex& parent,
					 int first, int last)
{
  first_ = first;
  last_ = last;
  parent_ = parent;

  rowsAboutToBeRemoved().emit(parent, first, last);
}

void WAbstractItemModel::endRemoveRows()
{
  rowsRemoved().emit(parent_, first_, last_);
}

WModelIndexList WAbstractItemModel::match(const WModelIndex& start,
					  int role,
					  const boost::any& value,
					  int hits,
					  WFlags<MatchFlag> flags)
  const
{
  WModelIndexList result;

  const int rc = rowCount(start.parent());

  for (int i = 0; i < rc; ++i) {
    int row = start.row() + i;

    if (row >= rc)
      if (!(flags & MatchWrap))
	break;
      else
	row -= rc;

    WModelIndex idx = index(row, start.column(), start.parent());
    boost::any v = data(idx, role);

    if (matchValue(v, value, flags))
      result.push_back(idx);
  }

  return result;
}

}
