/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WItemSelectionModel"

namespace Wt {

WItemSelectionModel::WItemSelectionModel(WAbstractItemModel *model,
					 WObject *parent)
  : WObject(parent),
    model_(model),
    selectionBehavior_(SelectRows)
{ }

void WItemSelectionModel::setSelectionBehavior(SelectionBehavior behavior)
{
  selectionBehavior_ = behavior;
}

bool WItemSelectionModel::isSelected(const WModelIndex& index) const
{
  return selection_.find(index) != selection_.end();
}

}
