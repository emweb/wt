/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>

#include <Wt/WAbstractItemModel.h>
#include <Wt/WItemSelectionModel.h>
#include <Wt/WMessageBox.h>

#include "FolderView.h"

const char *FolderView::FileSelectionMimeType
  = "application/x-computers-selection";

FolderView::FolderView()
  : WTreeView()
{
  /*
   * Accept drops for the custom mime type.
   */
  acceptDrops(FileSelectionMimeType);
}

void FolderView::dropEvent(const WDropEvent& event,
                            const WModelIndex& target)
{
  /*
   * We reimplement the drop event to handle the dropping of a
   * selection of computers.
   *
   * The test below would always be true in this case, since we only
   * indicated support for that particular mime type.
   */
  if (event.mimeType() == FileSelectionMimeType) {
    /*
     * The source object for a drag of a selection from a WTreeView is
     * a WItemSelectionModel.
     */
    WItemSelectionModel *selection
      = dynamic_cast<WItemSelectionModel *>(event.source());

#ifdef WT_THREADED
    StandardButton result = WMessageBox::show
      ("Drop event",
       "Move "
       + asString(selection->selectedIndexes().size())
       + " files to folder '"
       + cpp17::any_cast<WString>(target.data(ItemDataRole::Display)).toUTF8()
       + "' ?",
       StandardButton::Yes | StandardButton::No);
#else
    StandardButton result = StandardButton::Yes;
#endif

    if (result == StandardButton::Yes) {
      /*
       * You can access the source model from the selection and
       * manipulate it.
       */
      std::shared_ptr<WAbstractItemModel> sourceModel = selection->model();

      WModelIndexSet toChange = selection->selectedIndexes();

      for (WModelIndexSet::reverse_iterator i = toChange.rbegin();
	   i != toChange.rend(); ++i) {
	WModelIndex index = *i;

	/*
	 * Copy target folder to file. Since we are using a
	 * dynamic WSortFilterProxyModel that filters on folder, this
	 * will also result in the removal of the file from the
	 * current view.
	 */
        std::map<ItemDataRole, cpp17::any> data = model()->itemData(target);
	data[ItemDataRole::Decoration] = index.data(ItemDataRole::Decoration);
	sourceModel->setItemData(index, data);
      }
    }
  }
}
