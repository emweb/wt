/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WItemSelectionModel.h"
#include "Wt/WAbstractItemModel.h"

#include <string>

namespace Wt {

WItemSelectionModel::WItemSelectionModel()
  : selectionBehavior_(SelectionBehavior::Rows)
{ }

WItemSelectionModel
::WItemSelectionModel(const std::shared_ptr<WAbstractItemModel>& model)
  : model_(model),
    selectionBehavior_(SelectionBehavior::Rows)
{ }

void WItemSelectionModel::setSelectionBehavior(SelectionBehavior behavior)
{
  selectionBehavior_ = behavior;
}

bool WItemSelectionModel::isSelected(const WModelIndex& index) const
{
  if (selectionBehavior_ == SelectionBehavior::Rows) {
    for (std::set<WModelIndex>::const_iterator it = selection_.begin() ; 
         it != selection_.end(); ++it ) {
      WModelIndex mi = *it;
      if (mi.row() == index.row() && mi.parent() == index.parent())
	return true;
    }
    return false;
  } else {
    return selection_.find(index) != selection_.end();
  }
}

std::string WItemSelectionModel::mimeType()
{
  std::string retval;

  // Check that all selected mime types are the same

  for (WModelIndexSet::const_iterator i = selection_.begin();
       i != selection_.end(); ++i) {
    WModelIndex mi = *i;

    if (!(mi.flags() & ItemFlag::DragEnabled))
      return std::string();

    cpp17::any mimeTypeData = mi.data(ItemDataRole::MimeType);
    if (cpp17::any_has_value(mimeTypeData)) {
      std::string currentMimeType = asString(mimeTypeData).toUTF8();

      if (!currentMimeType.empty()) {
	if (retval.empty())
	  retval = currentMimeType;
	else if (currentMimeType != retval)
	  return model_->mimeType();
      }
    }
  }

  if (retval.empty())
    return selection_.empty() ? std::string() : model_->mimeType();
  else
    return retval;
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
