/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WReadOnlyProxyModel.h"

namespace Wt {

WReadOnlyProxyModel::WReadOnlyProxyModel()
{ }

WModelIndex WReadOnlyProxyModel::mapFromSource(const WModelIndex& sourceIndex)
  const
{
  return sourceIndex;
}

WModelIndex WReadOnlyProxyModel::mapToSource(const WModelIndex& proxyIndex)
  const
{
  return proxyIndex;
}

int WReadOnlyProxyModel::columnCount(const WModelIndex& parent) const
{
  return sourceModel()->columnCount(parent);
}

int WReadOnlyProxyModel::rowCount(const WModelIndex& parent) const
{
  return sourceModel()->rowCount(parent);
}

WModelIndex WReadOnlyProxyModel::parent(const WModelIndex& index) const
{
  return sourceModel()->parent(index);
}

WModelIndex WReadOnlyProxyModel::index(int row, int column,
				       const WModelIndex& parent) const
{
  return sourceModel()->index(row, column, parent);
}

bool WReadOnlyProxyModel::setData(const WModelIndex& index, const cpp17::any& value,
                                  ItemDataRole role)
{
  return false;
}

bool WReadOnlyProxyModel::setItemData(const WModelIndex& index,
				      const DataMap& values)
{
  return false;
}

bool WReadOnlyProxyModel::setHeaderData(int section, Orientation orientation,
                                        const cpp17::any& value, ItemDataRole role)
{
  return false;
}

bool WReadOnlyProxyModel::insertColumns(int column, int count,
					const WModelIndex& parent)
{
  return false;
}

bool WReadOnlyProxyModel::removeColumns(int column, int count,
					const WModelIndex& parent)
{
  return false;
}

void WReadOnlyProxyModel::dropEvent(const WDropEvent& e, DropAction action,
				    int row, int column,
				    const WModelIndex& parent)
{ }

}
