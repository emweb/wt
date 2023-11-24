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

WModelIndex WAbstractTableModel::parent(WT_MAYBE_UNUSED const WModelIndex& parent) const
{
  return WModelIndex();
}

WModelIndex WAbstractTableModel::index(int row, int column,
                                       WT_MAYBE_UNUSED const WModelIndex& index) const
{
  return createIndex(row, column, (void*)nullptr);
}

}
