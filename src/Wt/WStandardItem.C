/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include "Wt/WDate"
#include "Wt/WStandardItem"
#include "Wt/WStandardItemModel"

#include "WtException.h"

namespace {

  const bool UNSPECIFIED_IS_SMALLER = true;

  using namespace Wt;

  struct WStandardItemCompare
  {
    WStandardItemCompare(WStandardItem *anItem, int aColumn, SortOrder anOrder)
      : item(anItem),
	column(aColumn),
	order(anOrder)
    { }

    bool operator()(int r1, int r2) const {
      if (order == AscendingOrder)
	return lessThan(r1, r2);
      else
	return lessThan(r2, r1);
    }

    bool lessThan(int r1, int r2) const {
      WStandardItem *item1 = item->child(r1, column);
      WStandardItem *item2 = item->child(r2, column);

      if (item1)
	if (item2)
	  return (*item1) < (*item2);
	else
	  return !UNSPECIFIED_IS_SMALLER;
      else
	if (item2)
	  return UNSPECIFIED_IS_SMALLER;
	else
	  return false; // equal
    }

    WStandardItem *item;
    int            column;
    SortOrder      order;
  };

}

namespace Wt {

/*
 * For a standard item:
 *  rowCount() > 0 => columnCount() > 0
 * but it is possible to have:
 *  rowCount() = 0 && columnCount() > 0
 */
WStandardItem::WStandardItem()
  : model_(0),
    parent_(0),
    row_(-1), column_(-1),
    flags_(ItemIsSelectable),
    columns_(0)
{ }

WStandardItem::WStandardItem(const WString& text)
  : model_(0),
    parent_(0),
    row_(-1), column_(-1),
    flags_(ItemIsSelectable),
    columns_(0)
{
  setText(text);
}

WStandardItem::WStandardItem(const std::string& iconUri, const WString& text)
  : model_(0),
    parent_(0),
    row_(-1), column_(-1),
    flags_(ItemIsSelectable),
    columns_(0)
{
  setText(text);
  setIcon(iconUri);
}

WStandardItem::WStandardItem(int rows, int columns)
  : model_(0),
    parent_(0),
    row_(-1), column_(-1),
    flags_(ItemIsSelectable),
    columns_(0)
{
  // create at least one column if we have at least one row
  if (rows > 0)
    columns = std::max(columns, 1);

  if (columns > 0) {
    columns_ = new ColumnList;
    columns_->insert(columns_->end(), columns, Column(rows));
  }
}

WStandardItem::~WStandardItem()
{
  if (columns_) {
    for (unsigned i = 0; i < columns_->size(); ++i)
      for (unsigned j = 0; j < (*columns_)[i].size(); ++j)
	delete (*columns_)[i][j];

    delete columns_;
  }
}

void WStandardItem::setData(const boost::any& d, int role)
{
  if (model_)
    model_->setData(index(), d, role);
  else
    setDataImpl(d, role);
}

void WStandardItem::setDataImpl(const boost::any& d, int role)
{
  if (role == EditRole)
    role = DisplayRole;

  data_[role] = d;
}

boost::any WStandardItem::data(int role) const
{
  DataMap::const_iterator i = data_.find(role);

  if (i != data_.end())
    return i->second;
  else
    return boost::any();
}

void WStandardItem::setText(const WString& text)
{
  setData(boost::any(text), DisplayRole);
}

WString WStandardItem::text() const
{
  boost::any d = data(DisplayRole);

  if (!d.empty() && d.type() == typeid(WString))
    return boost::any_cast<WString>(d);
  else
    return WString();
}

void WStandardItem::setIcon(const std::string& uri)
{
  setData(uri, DecorationRole);
}

std::string WStandardItem::icon() const
{
  boost::any d = data(DisplayRole);

  if (!d.empty() && d.type() == typeid(std::string))
    return boost::any_cast<std::string>(d);
  else
    return std::string();
}

void WStandardItem::setInternalPath(const std::string& internalpath)
{
  setData(internalpath, InternalPathRole);
}

std::string WStandardItem::internalPath() const
{
  boost::any d = data(InternalPathRole);

  if (!d.empty() && d.type() == typeid(std::string))
    return boost::any_cast<std::string>(d);
  else
    return std::string();
}

void WStandardItem::setUrl(const std::string& url)
{
  setData(url, UrlRole);
}

std::string WStandardItem::url() const
{
  boost::any d = data(UrlRole);

  if (!d.empty() && d.type() == typeid(std::string))
    return boost::any_cast<std::string>(d);
  else
    return std::string();
}

void WStandardItem::setFlags(int flags)
{
  if (flags_ != flags) {
    flags_ = flags;
    signalModelDataChange();
  }
}

int WStandardItem::flags() const
{
  return flags_;
}

void WStandardItem::setStyleClass(const WString& styleClass)
{
  setData(styleClass, StyleClassRole);
}

WString WStandardItem::styleClass() const
{
  boost::any d = data(StyleClassRole);

  if (!d.empty() && d.type() == typeid(WString))
    return boost::any_cast<WString>(d);
  else
    return WString();
}

void WStandardItem::setToolTip(const WString& toolTip)
{
  setData(toolTip, ToolTipRole);
}

WString WStandardItem::toolTip() const
{
  boost::any d = data(ToolTipRole);

  if (!d.empty() && d.type() == typeid(WString))
    return boost::any_cast<WString>(d);
  else
    return WString();
}

void WStandardItem::setCheckable(bool checkable)
{
  if (!isCheckable() & checkable) {
    flags_ |= ItemIsUserCheckable;
    signalModelDataChange();
  } if (isCheckable() & !checkable) {
    flags_ &= ~ItemIsUserCheckable;
    signalModelDataChange();
  }
}

bool WStandardItem::isCheckable() const
{
  return flags_ & ItemIsUserCheckable;
}

void WStandardItem::setChecked(bool checked)
{
  setData(boost::any(checked), CheckStateRole);
}

bool WStandardItem::isChecked() const
{
  boost::any d = data(CheckStateRole);

  if (!d.empty() && d.type() == typeid(bool))
    return boost::any_cast<bool>(d);
  else
    return false;
}

void WStandardItem::setEditable(bool editable)
{
  if (!isEditable()) {
    flags_ |= ItemIsEditable;
    signalModelDataChange();
  }
}

bool WStandardItem::isEditable() const
{
  return flags_ & ItemIsEditable;
}

bool WStandardItem::hasChildren() const
{
  return columns_;
}

void WStandardItem::setRowCount(int rows)
{
  if (rows > rowCount())
    insertRows(rowCount(), rows - rowCount());
  else if (rows < rowCount())
    removeRows(rows, rowCount() - rows);
}

int WStandardItem::rowCount() const
{
  return columns_ ? (*columns_)[0].size() : 0;
}

void WStandardItem::setColumnCount(int columns)
{
  if (columns > columnCount())
    insertColumns(columnCount(), columns - columnCount());
  else
    if (columns < columnCount())
      removeColumns(columns, columnCount() - columns);
}

int WStandardItem::columnCount() const
{
  return columns_ ? columns_->size() : 0;
}

void WStandardItem::appendColumn(const std::vector<WStandardItem *>& items)
{
  insertColumn(columnCount(), items);
}

void WStandardItem::insertColumn(int column,
				 const std::vector<WStandardItem *>& items)
{
  int rc = rowCount();

  if (!columns_)
    columns_ = new ColumnList();
  else
    if ((unsigned)column < items.size())
      setRowCount(items.size());

  if (model_)
    model_->beginInsertColumns(index(), column, column);

  columns_->insert(columns_->begin() + column, items);
  for (unsigned i = 0; i < items.size(); ++i)
    if (items[i])
      adoptChild(i, column, items[i]);

  if (items.size() < (unsigned)column) {
    std::vector<WStandardItem *>& inserted = (*columns_)[column];
    inserted.insert(inserted.end(), rc - items.size(), 0);
  }

  renumberColumns(column + 1);

  if (model_)
    model_->endInsertColumns();
}

void WStandardItem::appendRow(const std::vector<WStandardItem *>& items)
{
  insertRow(rowCount(), items);
}

void WStandardItem::insertRow(int row,
			      const std::vector<WStandardItem *>& items)
{
  if (!columns_)
    setColumnCount(1);

  unsigned cc = columnCount();

  if (cc < items.size()) {
    setColumnCount(items.size());
    cc = items.size();
  }

  if (model_)
    model_->beginInsertRows(index(), row, row);

  for (unsigned i = 0; i < cc; ++i) {
    Column& c = (*columns_)[i];

    WStandardItem *item = i < items.size() ? items[i] : 0;
    c.insert(c.begin() + row, item);
    adoptChild(row, i, item);
  }

  renumberRows(row + 1);

  if (model_)
    model_->endInsertRows();
}

void WStandardItem::insertColumns(int column, int count)
{
  if (count > 0) {
    if (model_)
      model_->beginInsertColumns(index(), column, column + count - 1);    

    int rc = rowCount();

    if (!columns_)
      columns_ = new ColumnList;

    columns_->insert(columns_->begin() + column, count, Column(rc));

    renumberColumns(column + count);

    if (model_)
      model_->endInsertColumns();
  }
}

void WStandardItem::insertRows(int row, int count)
{
  if (count > 0) {
    if (model_)
      model_->beginInsertRows(index(), row, row + count - 1);

    if (!columns_)
      setColumnCount(1);

    unsigned cc = columnCount();

    for (unsigned i = 0; i < cc; ++i) {
      Column& c = (*columns_)[i];

      c.insert(c.begin() + row, count, static_cast<WStandardItem *>(0));
    }

    renumberRows(row + count);

    if (model_)
      model_->endInsertRows();
  }
}

void WStandardItem::appendRow(WStandardItem *item)
{
  insertRow(rowCount(), item);
}

void WStandardItem::insertRow(int row, WStandardItem *item)
{
  std::vector<WStandardItem *> r;
  r.push_back(item);

  insertRow(row, r);
}

void WStandardItem::appendRows(const std::vector<WStandardItem *>& items)
{
  insertRows(rowCount(), items);
}

void WStandardItem::insertRows(int row,
			       const std::vector<WStandardItem *>& items)
{
  // FIXME, could be done smarter and more efficient
  std::vector<WStandardItem *> r(1);

  for (unsigned i = 0; i < items.size(); ++i) {
    r[0] = items[i];

    insertRow(row + i, r);
  }
}

void WStandardItem::setChild(int row, int column, WStandardItem *item)
{
  if (column >= columnCount())
    setColumnCount(column + 1);

  if (row >= rowCount())
    setRowCount(row + 1);

  delete (*columns_)[column][row];
  (*columns_)[column][row] = item;

  adoptChild(row, column, item);
}

void WStandardItem::adoptChild(int row, int column, WStandardItem *item)
{
  if (item) {
    item->parent_ = this;
    item->row_ = row;
    item->column_ = column;

    item->setModel(model_);
  }
}

void WStandardItem::orphanChild(WStandardItem *item)
{
  if (item) {
    item->parent_ = 0;
    item->row_ = -1;
    item->column_ = -1;

    item->setModel(0);
  }
}

void WStandardItem::setModel(WStandardItemModel *model)
{
  model_ = model;

  for (int i = 0; i < columnCount(); ++i)
    for (int j = 0; j < rowCount(); ++j) {
      WStandardItem *c = (*columns_)[i][j];

      if (c)
	c->setModel(model);
    }
}

void WStandardItem::setChild(int row, WStandardItem *item)
{
  setChild(row, 0, item);
}

WStandardItem *WStandardItem::child(int row, int column) const
{
  if (row < rowCount() && column < columnCount())
    return (*columns_)[column][row];
  else
    return 0;
}

WStandardItem *WStandardItem::takeChild(int row, int column)
{
  WStandardItem *result = child(row, column);
  if (result) {
    orphanChild(result);
    (*columns_)[column][row] = 0;
  }

  return result;
}

std::vector<WStandardItem *> WStandardItem::takeColumn(int column)
{
  if (model_)
    model_->beginRemoveColumns(index(), column, column);

  std::vector<WStandardItem *> result = (*columns_)[column];

  columns_->erase(columns_->begin() + column);

  if (columns_->empty()) {
    delete columns_;
    columns_ = 0;
  }

  for (unsigned i = 0; i < result.size(); ++i)
    orphanChild(result[i]);

  renumberColumns(column);

  if (model_)
    model_->endRemoveColumns();

  return result;
}

std::vector<WStandardItem *> WStandardItem::takeRow(int row)
{
  if (model_)
    model_->beginRemoveRows(index(), row, row);

  std::vector<WStandardItem *> result(columnCount());

  for (unsigned i = 0; i < result.size(); ++i) {
    Column& c = (*columns_)[i];
    result[i] = c[row];
    orphanChild(result[i]);
    c.erase(c.begin() + row);
  }

  renumberRows(row);

  if (model_)
    model_->endRemoveRows();

  return result;
}

void WStandardItem::removeColumn(int column)
{
  removeColumns(column, 1);
}

void WStandardItem::removeColumns(int column, int count)
{
  if (model_)
    model_->beginRemoveColumns(index(), column, column + count - 1);

  for (int i = 0; i < count; ++i)
    for (int j = 0; j < rowCount(); ++j)
      delete (*columns_)[column + i][j];

  columns_->erase(columns_->begin() + column,
		  columns_->begin() + column + count);

  if (columns_->empty()) {
    delete columns_;
    columns_ = 0;
  }

  renumberColumns(column);

  if (model_)
    model_->endRemoveColumns();
}

void WStandardItem::removeRow(int row)
{
  removeRows(row, 1);
}

void WStandardItem::removeRows(int row, int count)
{
  if (model_)
    model_->beginRemoveRows(index(), row, row + count - 1);

  for (int i = 0; i < columnCount(); ++i) {
    Column& c = (*columns_)[i];

    for (int j = 0; j < count; ++j)
      delete c[row + j];

    c.erase(c.begin() + row, c.begin() + row + count);
  }

  renumberRows(row);

  if (model_)
    model_->endRemoveRows();
}

void WStandardItem::renumberColumns(int column)
{
  for (int c = column; c < columnCount(); ++c)
    for (int r = 0; r < rowCount(); ++r) {
      WStandardItem *item = child(r, c);
      if (item)
	item->column_ = c;
    }
}

void WStandardItem::renumberRows(int row)
{
  for (int c = 0; c < columnCount(); ++c)
    for (int r = row; r < rowCount(); ++r) {
      WStandardItem *item = child(r, c);
      if (item)
	item->row_ = r;
    }
}

WModelIndex WStandardItem::index() const
{
  if (model_)
    return model_->indexFromItem(this);
  else
    return WModelIndex();
}

WStandardItem *WStandardItem::clone() const
{
  WStandardItem *result = new WStandardItem();

  result->data_ = data_;
  result->flags_ = flags_;

  return result;
}

void WStandardItem::sortChildren(int column, SortOrder order)
{
  if (model_)
    model_->layoutAboutToBeChanged();

  recursiveSortChildren(column, order);

  if (model_)
    model_->layoutChanged();
}

bool WStandardItem::operator< (const WStandardItem& other) const
{
  int role = model_ ? model_->sortRole() : DisplayRole;

  boost::any d1 = data(role);
  boost::any d2 = other.data(role);

  return lessThan(d1, d2);
}

void WStandardItem::recursiveSortChildren(int column, SortOrder order)
{
  if (column < columnCount()) {
    std::vector<int> permutation(rowCount());

    for (unsigned i = 0; i < permutation.size(); ++i)
      permutation[i] = i;

    std::stable_sort(permutation.begin(), permutation.end(),
		     WStandardItemCompare(this, column, order));

    Column temp(rowCount());
    for (int c = 0; c < columnCount(); ++c) {
      Column& cc = (*columns_)[c];
      for (int r = 0; r < rowCount(); ++r) {
	temp[r] = cc[permutation[r]];
	if (temp[r])
	  temp[r]->row_ = r;
      }
      cc = temp;
    }
  }

  for (int c = 0; c < columnCount(); ++c)
    for (int r = 0; r < rowCount(); ++r) {
      WStandardItem *ch = child(r, c);
      if (ch)
	ch->recursiveSortChildren(column, order);
    }
}

void WStandardItem::signalModelDataChange()
{
  if (model_) {
    WModelIndex self = index();
    model_->dataChanged.emit(self, self);
  }
}

}
