/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WStringListModel>
#include <Utils.h>

#include <functional>

namespace Wt {

WStringListModel::WStringListModel(WObject *parent)
  : WAbstractListModel(parent)
{ }

WStringListModel::WStringListModel(const std::vector<WString>& strings,
				   WObject *parent)
  : WAbstractListModel(parent),
    strings_(strings)
{ }

WStringListModel::~WStringListModel()
{ }

void WStringListModel::setStringList(const std::vector<WString>& strings)
{
  int currentSize = strings_.size();
  int newSize = strings.size();

  if (newSize > currentSize)
    beginInsertRows(WModelIndex(), currentSize, newSize - 1);
  else if (newSize < currentSize)
    beginRemoveRows(WModelIndex(), newSize, currentSize - 1);

  strings_ = strings;

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
  return parent.isValid() ? 0 : strings_.size();
}

boost::any WStringListModel::data(const WModelIndex& index, int role) const
{
  return role == DisplayRole ? boost::any(strings_[index.row()]) : boost::any();
}

bool WStringListModel::setData(const WModelIndex& index,
			       const boost::any& value, int role)
{
  if (role == EditRole)
    role = DisplayRole;

  if (role == DisplayRole) {
    strings_[index.row()] = asString(value);
    dataChanged().emit(index, index);
    return true;
  } else
    return false;
}

WFlags<ItemFlag> WStringListModel::flags(const WModelIndex& index) const
{
  return ItemIsSelectable | ItemIsEditable;
}

bool WStringListModel::insertRows(int row, int count, const WModelIndex& parent)
{
  if (!parent.isValid()) {
    beginInsertRows(parent, row, row + count - 1);
    strings_.insert(strings_.begin() + row, count, WString());
    endInsertRows();

    return true;
  } else
    return false;
}

bool WStringListModel::removeRows(int row, int count, const WModelIndex& parent)
{
  if (!parent.isValid()) {
    beginRemoveRows(parent, row, row + count - 1);
    strings_.erase(strings_.begin() + row, strings_.begin() + row + count);
    endRemoveRows();

    return true;
  } else
    return false;
}

void WStringListModel::sort(int column, SortOrder order)
{
  layoutAboutToBeChanged().emit();

  if (order == AscendingOrder)
    Utils::sort(strings_);
  else
    Utils::sort(strings_, std::greater<WString>());

  layoutChanged().emit();
}

}
