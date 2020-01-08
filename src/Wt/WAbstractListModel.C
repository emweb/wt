/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WAbstractListModel.h>

namespace Wt {

WAbstractListModel::WAbstractListModel()
  : WAbstractItemModel()
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
  return createIndex(row, column, (void*)nullptr);
}
 
int WAbstractListModel::columnCount(const WModelIndex& parent) const
{
  return parent.isValid() ? 0 : 1;
}

}
