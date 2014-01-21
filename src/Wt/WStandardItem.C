/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLink"
#include "Wt/WStandardItem"
#include "Wt/WStandardItemModel"

#include "WebUtils.h"

#define UNSPECIFIED_RESULT -1

namespace {

  using namespace Wt;

  struct WStandardItemCompare W_JAVA_COMPARATOR(int)
  {
    WStandardItemCompare(WStandardItem *anItem, int aColumn, SortOrder anOrder)
      : item(anItem),
	column(aColumn),
	order(anOrder)
    { }

#ifndef WT_TARGET_JAVA
    bool operator()(int r1, int r2) const {

      if (order == AscendingOrder)
	return compare(r1, r2);
      else
	return compare(r2, r1);
    }

    bool compare(int r1, int r2) const {
      WStandardItem *item1 = item->child(r1, column);
      WStandardItem *item2 = item->child(r2, column);

      if (item1)
	if (item2)
	  return (*item1) < (*item2);
	else
	  return UNSPECIFIED_RESULT == -1;
      else
	if (item2)
	  return UNSPECIFIED_RESULT != -1;
	else
	  return false;
    }
#else
    int compare(int r1, int r2) const {
      WStandardItem *item1 = item->child(r1, column);
      WStandardItem *item2 = item->child(r2, column);

      int result;

      if (item1)
	if (item2)
          result = item1->compare(*item2);
	else
	  result = -UNSPECIFIED_RESULT;
      else
	if (item2)
	  result = UNSPECIFIED_RESULT;
	else
	  result = 0;

      if (order == DescendingOrder)
	result = -result;

      return result;
    }
#endif // WT_TARGET_JAVA

    WStandardItem *item;
    int            column;
    SortOrder      order;
  };

}

namespace Wt {

/*
 * As per the contract of a WAbstractItemModel:
 *  rowCount() > 0 => columnCount() > 0
 * but it is possible to have
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
    columns_ = new ColumnList();
#ifndef WT_TARGET_JAVA
    columns_->insert(columns_->end(), columns, Column(rows));
#else // WT_TARGET_JAVA
    for (int i = 0; i < columns; ++i) {
      Column c;
      c.insert(c.end(), rows, static_cast<WStandardItem *>(0));
      columns_->push_back(c);
    }
#endif // WT_TARGET_JAVA
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
  if (role == EditRole)
    role = DisplayRole;

  data_[role] = d;

  if (model_) {
    WModelIndex self = index();
    model_->dataChanged().emit(self, self);
    model_->itemChanged().emit(this);
  }
}

boost::any WStandardItem::data(int role) const
{
  DataMap::const_iterator i = data_.find(role);

  if (i != data_.end())
    return i->second;
  else
    if (role == EditRole)
      return data(DisplayRole);
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

  return asString(d);
}

void WStandardItem::setIcon(const std::string& uri)
{
  setData(uri, DecorationRole);
}

std::string WStandardItem::icon() const
{
  boost::any d = data(DecorationRole);

  if (!d.empty() && d.type() == typeid(std::string))
    return boost::any_cast<std::string>(d);
  else
    return std::string();
}

void WStandardItem::setLink(const WLink& link)
{
  setData(link, LinkRole);
}

WLink WStandardItem::link() const
{
  boost::any d = data(LinkRole);

  if (!d.empty() && d.type() == typeid(WLink))
    return boost::any_cast<WLink>(d);
  else
    return WLink(std::string());
}

void WStandardItem::setInternalPath(const std::string& internalpath)
{
  setLink(WLink(WLink::InternalPath, internalpath));
}

std::string WStandardItem::internalPath() const
{
  WLink l = link();

  if (!l.isNull())
    return l.internalPath().toUTF8();
  else
    return std::string();
}

void WStandardItem::setUrl(const std::string& url)
{
  setLink(WLink(url));
}

std::string WStandardItem::url() const
{
  WLink l = link();

  if (!l.isNull())
    return l.url();
  else
    return std::string();
}

void WStandardItem::setFlags(WFlags<ItemFlag> flags)
{
  if (flags_ != flags) {
    flags_ = flags;
    signalModelDataChange();
  }
}

WFlags<ItemFlag> WStandardItem::flags() const
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
  if (!isCheckable() && checkable) {
    flags_ |= ItemIsUserCheckable;
    if (data(CheckStateRole).empty())
      setChecked(false);
    signalModelDataChange();
  } if (isCheckable() && !checkable) {
    flags_.clear(ItemIsUserCheckable);
    signalModelDataChange();
  }
}

bool WStandardItem::isCheckable() const
{
  return flags_ & ItemIsUserCheckable;
}

void WStandardItem::setChecked(bool checked)
{
  boost::any d = data(CheckStateRole);
  if (d.empty() || isChecked() != checked)
    setCheckState(checked ? Checked : Unchecked);
}

void WStandardItem::setCheckState(CheckState state)
{
  boost::any d = data(CheckStateRole);
  if (d.empty() || checkState() != state || data(CheckStateRole).empty()) {
    if (isTristate())
      setData(boost::any(state), CheckStateRole);
    else
      setData(boost::any(state == Checked), CheckStateRole);
  }
}

bool WStandardItem::isChecked() const
{
  return checkState() == Checked;
}

CheckState WStandardItem::checkState() const
{
  boost::any d = data(CheckStateRole);

  if (d.empty())
    return Unchecked;
  else if (d.type() == typeid(bool))
    return boost::any_cast<bool>(d) ? Checked : Unchecked;
  else if (d.type() == typeid(CheckState))
    return boost::any_cast<CheckState>(d);
  else
    return Unchecked;
}

void WStandardItem::setTristate(bool tristate)
{
  if (tristate)
    flags_ |= ItemIsTristate;
  else
    flags_.clear(ItemIsTristate);
}

bool WStandardItem::isTristate() const
{
  return flags_ & ItemIsTristate;
}

void WStandardItem::setEditable(bool editable)
{
  if (!isEditable() && editable) {
    flags_ |= ItemIsEditable;
    signalModelDataChange();
  }
  else if (isEditable() && !editable) {
    flags_.clear(ItemIsEditable);
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
  unsigned rc = rowCount();

  if (!columns_) {
    setRowCount(items.size());

    (*columns_)[0] = items;
    for (unsigned i = 0; i < items.size(); ++i)
      if (items[i])
	adoptChild(i, column, items[i]);
  } else {
    if (rc < items.size()) {
      setRowCount(items.size());
      rc = items.size();
    }

    if (model_)
      model_->beginInsertColumns(index(), column, column);

    columns_->insert(columns_->begin() + column, items);
    for (unsigned i = 0; i < items.size(); ++i)
      if (items[i])
	adoptChild(i, column, items[i]);

    if (items.size() < rc) {
      std::vector<WStandardItem *>& inserted = (*columns_)[column];
      inserted.resize(rc);
    }

    renumberColumns(column + 1);

    if (model_)
      model_->endInsertColumns();
  }
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

#ifndef WT_TARGET_JAVA
    columns_->insert(columns_->begin() + column, count, Column(rc));
#else
    for (int i = 0; i < count; ++i) {
      Column c;
      c.insert(c.end(), rc, static_cast<WStandardItem *>(0));
      columns_->insert(columns_->begin() + column + i, c);
    }
#endif

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
#ifndef WT_TARGET_JAVA
  std::vector<WStandardItem *> r(1);
#else
  std::vector<WStandardItem *> r;
  r.push_back(0);
#endif

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

  if (model_) {
    WModelIndex self = item->index();
    model_->dataChanged().emit(self, self);
    // model_->itemChanged().emit(item);
  }
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
    WModelIndex idx = result->index();
    
    if (result->hasChildren())
      model_->beginRemoveRows(result->index(), 0, result->rowCount() - 1);
    
    orphanChild(result);
    (*columns_)[column][row] = 0;
    
    if (result->hasChildren())
      model_->endRemoveRows();

    model_->dataChanged().emit(idx, idx);
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

#ifndef WT_TARGET_JAVA
  std::vector<WStandardItem *> result(columnCount());
#else
  std::vector<WStandardItem *> result;
  result.insert(result.end(), columnCount(), static_cast<WStandardItem *>(0));
#endif

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

  result->data_ = DataMap(data_);
  result->flags_ = flags_;

  return result;
}

void WStandardItem::sortChildren(int column, SortOrder order)
{
  if (model_)
    model_->layoutAboutToBeChanged().emit();

  recursiveSortChildren(column, order);

  if (model_)
    model_->layoutChanged().emit();
}

bool WStandardItem::operator< (const WStandardItem& other) const
{
  return compare(other) < 0;
}

int WStandardItem::compare(const WStandardItem& other) const
{
  int role = model_ ? model_->sortRole() : DisplayRole;

  boost::any d1 = data(role);
  boost::any d2 = other.data(role);

  return Wt::Impl::compare(d1, d2);
}

void WStandardItem::recursiveSortChildren(int column, SortOrder order)
{
  if (column < columnCount()) {
#ifndef WT_TARGET_JAVA
    std::vector<int> permutation(rowCount());

    for (unsigned i = 0; i < permutation.size(); ++i)
      permutation[i] = i;
#else
    std::vector<int> permutation;
    for (unsigned i = 0; i < rowCount(); ++i)
      permutation.push_back(i);
#endif // WT_TARGET_JAVA

    Utils::stable_sort(permutation, WStandardItemCompare(this, column, order));

#ifndef WT_TARGET_JAVA
    Column temp(rowCount());
#endif

    for (int c = 0; c < columnCount(); ++c) {
#ifdef WT_TARGET_JAVA
      Column temp;
#endif // WT_TARGET_JAVA
      Column& cc = (*columns_)[c];
      for (int r = 0; r < rowCount(); ++r) {
#ifndef WT_TARGET_JAVA
	temp[r] = cc[permutation[r]];
#else
	temp.push_back(cc[permutation[r]]);
#endif // WT_TARGET_JAVA
	if (temp[r])
	  temp[r]->row_ = r;
      }
      (*columns_)[c] = temp;
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
    model_->dataChanged().emit(self, self);
  }
}

}
