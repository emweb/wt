/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WReadOnlyProxyModel"

namespace Wt {

WReadOnlyProxyModel::WReadOnlyProxyModel(WObject *parent)
  : WAbstractProxyModel(parent)
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

}
