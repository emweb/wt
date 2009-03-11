/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WAbstractTableModel>

namespace Wt {

WAbstractTableModel::WAbstractTableModel(WObject *parent)
  : WAbstractItemModel(parent)
{ }

WAbstractTableModel::~WAbstractTableModel()
{ }

WModelIndex WAbstractTableModel::parent(const WModelIndex& index) const
{
  return WModelIndex();
}

WModelIndex WAbstractTableModel::index(int row, int column,
				       const WModelIndex& parent) const
{
  return createIndex(row, column, (void *)0);
}

}
