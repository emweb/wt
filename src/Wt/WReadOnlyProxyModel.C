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

bool WReadOnlyProxyModel::setData(WT_MAYBE_UNUSED const WModelIndex& index, WT_MAYBE_UNUSED const cpp17::any& value,
                                  WT_MAYBE_UNUSED ItemDataRole role)
{
  return false;
}

bool WReadOnlyProxyModel::setItemData(WT_MAYBE_UNUSED const WModelIndex& index,
                                     WT_MAYBE_UNUSED  const DataMap& values)
{
  return false;
}

bool WReadOnlyProxyModel::setHeaderData(WT_MAYBE_UNUSED int section, WT_MAYBE_UNUSED Orientation orientation,
                                        WT_MAYBE_UNUSED const cpp17::any& value, WT_MAYBE_UNUSED ItemDataRole role)
{
  return false;
}

bool WReadOnlyProxyModel::insertColumns(WT_MAYBE_UNUSED int column, WT_MAYBE_UNUSED int count, WT_MAYBE_UNUSED const WModelIndex& parent)
{
  return false;
}

bool WReadOnlyProxyModel::removeColumns(WT_MAYBE_UNUSED int column, WT_MAYBE_UNUSED int count, WT_MAYBE_UNUSED const WModelIndex& parent)
{
  return false;
}

void WReadOnlyProxyModel::dropEvent(WT_MAYBE_UNUSED const WDropEvent& e, WT_MAYBE_UNUSED DropAction action,
                                    WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column, WT_MAYBE_UNUSED const WModelIndex& parent)
{ }

}
