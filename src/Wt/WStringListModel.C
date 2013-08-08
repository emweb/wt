/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WStringListModel"
#include "WebUtils.h"

#include <functional>

namespace {

  using namespace Wt;

struct StringListModelCompare W_JAVA_COMPARATOR(int)
{
  WStringListModel *model_;
  SortOrder order_;

  StringListModelCompare(WStringListModel *model, SortOrder order)
    : model_(model), order_(order)
  { }

#ifndef WT_TARGET_JAVA
  bool operator()(int r1, int r2) const {
    if (order_ == AscendingOrder)
      return compare(r1, r2);
    else
      return compare(r2, r1);
  }

  bool compare(int r1, int r2) const {
    return model_->stringList()[r1] < model_->stringList()[r2];
  }
#else
  int compare(int r1, int r2) const {
    int result = model_->stringList()[r1].compareTo(model_->stringList()[r2]);

    if (order_ == DescendingOrder)
      result = -result;

    return result;
  }
#endif // WT_TARGET_JAVA
};

}

namespace Wt {

WStringListModel::WStringListModel(WObject *parent)
  : WAbstractListModel(parent),
    otherData_(0)
{ }

WStringListModel::WStringListModel(const std::vector<WString>& strings,
				   WObject *parent)
  : WAbstractListModel(parent),
    displayData_(strings),
    otherData_(0)
{ }

WStringListModel::~WStringListModel()
{ 
  delete otherData_;
}

void WStringListModel::setStringList(const std::vector<WString>& strings)
{
  int currentSize = displayData_.size();
  int newSize = strings.size();

  if (newSize > currentSize)
    beginInsertRows(WModelIndex(), currentSize, newSize - 1);
  else if (newSize < currentSize)
    beginRemoveRows(WModelIndex(), newSize, currentSize - 1);

  displayData_ = strings;
  flags_.clear();

  delete otherData_;
  otherData_ = 0;

  if (newSize > currentSize)
    endInsertRows();
  else if (newSize < currentSize)
    endRemoveRows();

  int numChanged = std::min(currentSize, newSize);

  if (numChanged)
    dataChanged().emit(index(0, 0), index(numChanged - 1, 0));
}

void WStringListModel::addString(const WString& string)
{
  insertString(rowCount(), string);
}

void WStringListModel::insertString(int row, const WString& string)
{
  insertRows(row, 1);
  setData(row, 0, string);
}

int WStringListModel::rowCount(const WModelIndex& parent) const
{
  return parent.isValid() ? 0 : displayData_.size();
}

boost::any WStringListModel::data(const WModelIndex& index, int role) const
{
  if (role == DisplayRole)
    return boost::any(displayData_[index.row()]);
  else if (otherData_)
    return (*otherData_)[index.row()][role];
  else
    return boost::any();
}

bool WStringListModel::setData(const WModelIndex& index,
			       const boost::any& value, int role)
{
  if (role == EditRole)
    role = DisplayRole;

  if (role == DisplayRole)
    displayData_[index.row()] = asString(value);
  else {
    if (!otherData_) {
#ifndef WT_TARGET_JAVA
      otherData_ = new std::vector<DataMap>(displayData_.size());
#else
      otherData_ = new std::vector<DataMap>();
      for (int i = 0; i < displayData_.size(); ++i)
	otherData_->push_back(DataMap());
#endif
    }

    (*otherData_)[index.row()][role] = value;
  }

  dataChanged().emit(index, index);

  return true;
}

void WStringListModel::setFlags(int row, WFlags<ItemFlag> flags)
{
  if (flags_.empty())
    flags_.insert(flags_.begin(), rowCount(),
		  ItemIsSelectable | ItemIsEditable);

  flags_[row] = flags;
}

WFlags<ItemFlag> WStringListModel::flags(const WModelIndex& index) const
{
  if (flags_.empty())
    return ItemIsSelectable | ItemIsEditable;
  else
    return flags_[index.row()];
}

bool WStringListModel::insertRows(int row, int count, const WModelIndex& parent)
{
  if (!parent.isValid()) {
    beginInsertRows(parent, row, row + count - 1);
    displayData_.insert(displayData_.begin() + row, count, WString());
    if (!flags_.empty())
      flags_.insert(flags_.begin() + row, count,
		    ItemIsSelectable | ItemIsEditable);
    if (otherData_)
      otherData_->insert(otherData_->begin() + row, count, DataMap());
    endInsertRows();

    return true;
  } else
    return false;
}

bool WStringListModel::removeRows(int row, int count, const WModelIndex& parent)
{
  if (!parent.isValid()) {
    beginRemoveRows(parent, row, row + count - 1);
    displayData_.erase(displayData_.begin() + row,
		       displayData_.begin() + row + count);
    if (!flags_.empty())
      flags_.erase(flags_.begin() + row, flags_.begin() + row + count);
    if (otherData_)
      otherData_->erase(otherData_->begin() + row,
			otherData_->begin() + row + count);
    endRemoveRows();

    return true;
  } else
    return false;
}

void WStringListModel::sort(int column, SortOrder order)
{
  layoutAboutToBeChanged().emit();

  if (!otherData_ && flags_.empty()) {
    if (order == AscendingOrder)
      Utils::sort(displayData_);
    else
      Utils::sort(displayData_, std::greater<WString>());
  } else {
#ifndef WT_TARGET_JAVA
    std::vector<int> permutation(rowCount());
    for (unsigned i = 0; i < permutation.size(); ++i)
      permutation[i] = i;
#else
    std::vector<int> permutation;
    for (unsigned i = 0; i < rowCount(); ++i)
      permutation.push_back(i);
#endif // WT_TARGET_JAVA

    Utils::sort(permutation, StringListModelCompare(this, order));

    std::vector<WString> displayData;
    displayData.resize(rowCount());

    std::vector<WFlags<ItemFlag> > flags;
    if (!flags_.empty())
      flags.resize(rowCount());

    std::vector<DataMap> *otherData = 0;
    if (otherData_) {
      otherData = new std::vector<DataMap>();
      otherData->resize(rowCount());
    }

    for (unsigned i = 0; i < permutation.size(); ++i) {
      displayData[i] = displayData_[permutation[i]];
      if (otherData)
	(*otherData)[i] = (*otherData_)[permutation[i]];
      if (!flags.empty())
	flags[i] = flags_[permutation[i]];
    }

    displayData_ = displayData;
    delete otherData_;
    otherData_ = otherData;
    flags_ = flags;
  }

  layoutChanged().emit();
}

}
