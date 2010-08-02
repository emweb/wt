/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WItemSelectionModel"
#include "Wt/WAbstractItemModel"

namespace Wt {

WItemSelectionModel::WItemSelectionModel(WAbstractItemModel *model,
					 WObject *parent)
  : WObject(parent),
    model_(model),
    selectionBehavior_(SelectRows)
{ 
  if (model_) {
    model_->layoutAboutToBeChanged()
      .connect(this, &WItemSelectionModel::modelLayoutAboutToBeChanged);
    model_->layoutChanged()
      .connect(this, &WItemSelectionModel::modelLayoutChanged);
  }
}

void WItemSelectionModel::setSelectionBehavior(SelectionBehavior behavior)
{
  selectionBehavior_ = behavior;
}

bool WItemSelectionModel::isSelected(const WModelIndex& index) const
{
  return selection_.find(index) != selection_.end();
}

void WItemSelectionModel::modelLayoutAboutToBeChanged()
{
  WModelIndex::encodeAsRawIndexes(selection_);
}

void WItemSelectionModel::modelLayoutChanged()
{
  selection_ = WModelIndex::decodeFromRawIndexes(selection_);
}

}
