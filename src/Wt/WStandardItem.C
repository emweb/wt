/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLink.h"
#include "Wt/WStandardItem.h"
#include "Wt/WStandardItemModel.h"

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

      if (order == SortOrder::Ascending)
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
	  result = 1;
      else
	if (item2)
	  result = -1;
	else
	  result = 0;

      if (order == SortOrder::Descending)
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
  : model_(nullptr),
    parent_(nullptr),
    row_(-1), column_(-1),
    flags_(ItemFlag::Selectable)
{ }

WStandardItem::WStandardItem(const WString& text)
  : model_(nullptr),
    parent_(nullptr),
    row_(-1), column_(-1),
    flags_(ItemFlag::Selectable)
{
  setText(text);
}

WStandardItem::WStandardItem(const std::string& iconUri, const WString& text)
  : model_(nullptr),
    parent_(nullptr),
    row_(-1), column_(-1),
    flags_(ItemFlag::Selectable)
{
  setText(text);
  setIcon(iconUri);
}

WStandardItem::WStandardItem(int rows, int columns)
  : model_(nullptr),
    parent_(nullptr),
    row_(-1), column_(-1),
    flags_(ItemFlag::Selectable)
{
  // create at least one column if we have at least one row
  if (rows > 0)
    columns = std::max(columns, 1);

  if (columns > 0) {
    columns_.reset(new ColumnList());
    for (int i = 0; i < columns; ++i) {
      Column c;
      c.resize(rows);
      columns_->push_back(std::move(c));
    }
  }
}

WStandardItem::~WStandardItem()
{ }

void WStandardItem::setData(const cpp17::any& d, ItemDataRole role)
{
  if (role == ItemDataRole::Edit)
    role = ItemDataRole::Display;

  data_[role] = d;

  if (model_) {
    WModelIndex self = index();
    model_->dataChanged().emit(self, self);
    model_->itemChanged().emit(this);
  }
}

cpp17::any WStandardItem::data(ItemDataRole role) const
{
  DataMap::const_iterator i = data_.find(role);

  if (i != data_.end())
    return i->second;
  else
    if (role == ItemDataRole::Edit)
      return data(ItemDataRole::Display);
    else
      return cpp17::any();
}

void WStandardItem::setText(const WString& text)
{
  setData(cpp17::any(text), ItemDataRole::Display);
}

WString WStandardItem::text() const
{
  cpp17::any d = data(ItemDataRole::Display);

  return asString(d);
}

void WStandardItem::setIcon(const std::string& uri)
{
  setData(uri, ItemDataRole::Decoration);
}

std::string WStandardItem::icon() const
{
  cpp17::any d = data(ItemDataRole::Decoration);

  if (cpp17::any_has_value(d) && d.type() == typeid(std::string))
    return cpp17::any_cast<std::string>(d);
  else
    return std::string();
}

void WStandardItem::setLink(const WLink& link)
{
  setData(link, ItemDataRole::Link);
}

WLink WStandardItem::link() const
{
  cpp17::any d = data(ItemDataRole::Link);

  if (cpp17::any_has_value(d) && d.type() == typeid(WLink))
    return cpp17::any_cast<WLink>(d);
  else
    return WLink(std::string());
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
  setData(styleClass, ItemDataRole::StyleClass);
}

WString WStandardItem::styleClass() const
{
  cpp17::any d = data(ItemDataRole::StyleClass);

  if (cpp17::any_has_value(d) && d.type() == typeid(WString))
    return cpp17::any_cast<WString>(d);
  else
    return WString();
}

void WStandardItem::setToolTip(const WString& toolTip)
{
  setData(toolTip, ItemDataRole::ToolTip);
}

WString WStandardItem::toolTip() const
{
  cpp17::any d = data(ItemDataRole::ToolTip);

  if (cpp17::any_has_value(d) && d.type() == typeid(WString))
    return cpp17::any_cast<WString>(d);
  else
    return WString();
}

void WStandardItem::setCheckable(bool checkable)
{
  if (!isCheckable() && checkable) {
    flags_ |= ItemFlag::UserCheckable;
    if (!cpp17::any_has_value(data(ItemDataRole::Checked)))
      setChecked(false);
    signalModelDataChange();
  } if (isCheckable() && !checkable) {
    flags_.clear(ItemFlag::UserCheckable);
    signalModelDataChange();
  }
}

bool WStandardItem::isCheckable() const
{
  return flags_.test(ItemFlag::UserCheckable);
}

void WStandardItem::setChecked(bool checked)
{
  cpp17::any d = data(ItemDataRole::Checked);
  if (!cpp17::any_has_value(d) || isChecked() != checked)
    setCheckState(checked ? CheckState::Checked : CheckState::Unchecked);
}

void WStandardItem::setCheckState(CheckState state)
{
  cpp17::any d = data(ItemDataRole::Checked);
  if (!cpp17::any_has_value(d) || checkState() != state || 
      !cpp17::any_has_value(data(ItemDataRole::Checked))) {
    if (isTristate())
      setData(cpp17::any(state), ItemDataRole::Checked);
    else
      setData(cpp17::any(state == CheckState::Checked), ItemDataRole::Checked);
  }
}

bool WStandardItem::isChecked() const
{
  return checkState() == CheckState::Checked;
}

CheckState WStandardItem::checkState() const
{
  cpp17::any d = data(ItemDataRole::Checked);

  if (!cpp17::any_has_value(d))
    return CheckState::Unchecked;
  else if (d.type() == typeid(bool))
    return cpp17::any_cast<bool>(d) ? CheckState::Checked : CheckState::Unchecked;
  else if (d.type() == typeid(CheckState))
    return cpp17::any_cast<CheckState>(d);
  else
    return CheckState::Unchecked;
}

void WStandardItem::setTristate(bool tristate)
{
  if (tristate)
    flags_ |= ItemFlag::Tristate;
  else
    flags_.clear(ItemFlag::Tristate);
}

bool WStandardItem::isTristate() const
{
  return flags_.test(ItemFlag::Tristate);
}

void WStandardItem::setEditable(bool editable)
{
  if (!isEditable() && editable) {
    flags_ |= ItemFlag::Editable;
    signalModelDataChange();
  }
  else if (isEditable() && !editable) {
    flags_.clear(ItemFlag::Editable);
    signalModelDataChange();
  }
}

bool WStandardItem::isEditable() const
{
  return flags_.test(ItemFlag::Editable);
}

bool WStandardItem::hasChildren() const
{
  return columns_.get() != nullptr;
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

void WStandardItem
::appendColumn(std::vector<std::unique_ptr<WStandardItem> > items)
{
  insertColumn(columnCount(), std::move(items));
}

void WStandardItem
::insertColumn(int column,
	       std::vector<std::unique_ptr<WStandardItem> > items)
{
  unsigned rc = rowCount();

  if (!columns_) {
    setRowCount(items.size());

    for (unsigned i = 0; i < items.size(); ++i)
      if (items[i])
	adoptChild(i, column, items[i].get());

    (*columns_)[0] = std::move(items);
  } else {
    if (rc < items.size()) {
      setRowCount(items.size());
      rc = items.size();
    }

    if (model_)
      model_->beginInsertColumns(index(), column, column);

    for (unsigned i = 0; i < items.size(); ++i)
      if (items[i])
	adoptChild(i, column, items[i].get());

    columns_->insert(columns_->begin() + column, std::move(items));

    auto& inserted = (*columns_)[column];
    if (inserted.size() < rc) {
      inserted.resize(rc);
    }

    renumberColumns(column + 1);

    if (model_)
      model_->endInsertColumns();
  }
}

void WStandardItem::appendRow(std::vector<std::unique_ptr<WStandardItem> > items)
{
  insertRow(rowCount(), std::move(items));
}

void WStandardItem::insertRow(int row,
			      std::vector<std::unique_ptr<WStandardItem> > items)
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

    std::unique_ptr<WStandardItem> item;
    if (i < items.size())
      item = std::move(items[i]);

    adoptChild(row, i, item.get());
    c.insert(c.begin() + row, std::move(item));
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
      columns_.reset(new ColumnList());

    for (int i = 0; i < count; ++i) {
      Column c;
      c.resize(rc);
      columns_->insert(columns_->begin() + column + i, std::move(c));
    }

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
      for (int j = 0; j < count; ++j)
	c.insert(c.begin() + row + j, std::unique_ptr<WStandardItem>());
    }

    renumberRows(row + count);

    if (model_)
      model_->endInsertRows();
  }
}

void WStandardItem::appendRow(std::unique_ptr<WStandardItem> item)
{
  insertRow(rowCount(), std::move(item));
}

void WStandardItem::insertRow(int row, std::unique_ptr<WStandardItem> item)
{
  std::vector<std::unique_ptr<WStandardItem> > r;
  r.push_back(std::move(item));
  insertRow(row, std::move(r));
}

void WStandardItem::appendRows(std::vector<std::unique_ptr<WStandardItem> >
			       items)
{
  insertRows(rowCount(), std::move(items));
}

void WStandardItem
::insertRows(int row, std::vector<std::unique_ptr<WStandardItem> > items)
{
  for (unsigned i = 0; i < items.size(); ++i) {
    std::vector<std::unique_ptr<WStandardItem> > r;
    r.push_back(std::move(items[i]));
    insertRow(row + i, std::move(r));
  }
}

void WStandardItem::setChild(int row, int column,
			     std::unique_ptr<WStandardItem> item)
{
  if (column >= columnCount())
    setColumnCount(column + 1);

  if (row >= rowCount())
    setRowCount(row + 1);

  adoptChild(row, column, item.get());

  WStandardItem *it = item.get();
  (*columns_)[column][row] = std::move(item);

  if (model_) {
    WModelIndex self = it->index();
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
      auto& c = (*columns_)[i][j];

      if (c)
	c->setModel(model);
    }
}

void WStandardItem::setChild(int row, std::unique_ptr<WStandardItem> item)
{
  setChild(row, 0, std::move(item));
}

WStandardItem *WStandardItem::child(int row, int column) const
{
  if (row < rowCount() && column < columnCount())
    return (*columns_)[column][row].get();
  else
    return nullptr;
}

std::unique_ptr<WStandardItem> WStandardItem::takeChild(int row, int column)
{
  WStandardItem *item = child(row, column);

  std::unique_ptr<WStandardItem> result;
  if (item) {
    WModelIndex idx = item->index();
    
    if (item->hasChildren())
      model_->beginRemoveRows(item->index(), 0, item->rowCount() - 1);
    
    orphanChild(item);
    result = std::move((*columns_)[column][row]);
    
    if (item->hasChildren())
      model_->endRemoveRows();

    model_->dataChanged().emit(idx, idx);
  }

  return result;
}

std::vector<std::unique_ptr<WStandardItem> > WStandardItem
::takeColumn(int column)
{
  if (model_)
    model_->beginRemoveColumns(index(), column, column);

  std::vector<std::unique_ptr<WStandardItem> > result
    = std::move((*columns_)[column]);

  columns_->erase(columns_->begin() + column);

  if (columns_->empty())
    columns_.reset();

  for (unsigned i = 0; i < result.size(); ++i)
    orphanChild(result[i].get());

  renumberColumns(column);

  if (model_)
    model_->endRemoveColumns();

  return result;
}

std::vector<std::unique_ptr<WStandardItem> > WStandardItem::takeRow(int row)
{
  if (model_)
    model_->beginRemoveRows(index(), row, row);

#ifndef WT_TARGET_JAVA
  std::vector<std::unique_ptr<WStandardItem> > result(columnCount());
#else
  std::vector<std::unique_ptr<WStandardItem> > result;
  result.insert(result.end(), columnCount(), std::unique_ptr<WStandardItem>());
#endif

  for (unsigned i = 0; i < result.size(); ++i) {
    Column& c = (*columns_)[i];
    result[i] = std::move(c[row]);
    orphanChild(result[i].get());
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

  columns_->erase(columns_->begin() + column,
		  columns_->begin() + column + count);

  if (columns_->empty())
    columns_.reset();

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

std::unique_ptr<WStandardItem> WStandardItem::clone() const
{
  std::unique_ptr<WStandardItem> result(new WStandardItem());

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
  ItemDataRole role = model_ ? model_->sortRole() : ItemDataRole::Display;

  cpp17::any d1 = data(role);
  cpp17::any d2 = other.data(role);

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
        temp[r] = std::move(cc[permutation[r]]);
#else
	temp.push_back(cc[permutation[r]]);
#endif // WT_TARGET_JAVA
	if (temp[r])
	  temp[r]->row_ = r;
      }
      for (int r = 0; r < rowCount(); ++r) {
	cc[r] = std::move(temp[r]);
      }
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
