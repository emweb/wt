/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WAbstractListModel>

namespace Wt {

WAbstractListModel::WAbstractListModel(WObject *parent)
  : WAbstractItemModel(parent)
{ }

WAbstractListModel::~WAbstractListModel()
{ }

WModelIndex WAbstractListModel::parent(const WModelIndex& index) const
{
  return WModelIndex();
}

WModelIndex WAbstractListModel::index(int row, int column,
				      const WModelIndex& parent) const
{
  return createIndex(row, column, (void *)0);
}
 
int WAbstractListModel::columnCount(const WModelIndex& parent) const
{
  return parent.isValid() ? 0 : 1;
}

}
