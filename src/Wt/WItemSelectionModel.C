/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WItemSelectionModel"
#include "Wt/WAbstractItemModel"

#include <boost/lexical_cast.hpp>
#include "boost/any.hpp"

#include <string>

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
  if (selectionBehavior_ == SelectRows) {
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

   //Check that all selected mime types are the same
    for (WModelIndexSet::const_iterator i = selection_.begin();
         i != selection_.end(); ++i) {
      WModelIndex mi = *i;

      if (!(mi.flags() & ItemIsDragEnabled))
        return std::string();

      boost::any mimeTypeData = mi.data(MimeTypeRole);
      if (!mimeTypeData.empty()) {
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
