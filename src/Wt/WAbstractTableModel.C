/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WAbstractTableModel.h>

namespace Wt {

WAbstractTableModel::WAbstractTableModel()
  : WAbstractItemModel()
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
  return createIndex(row, column, (void*)nullptr);
}

}
