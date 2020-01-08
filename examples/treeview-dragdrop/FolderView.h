// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FOLDER_VIEW_H_
#define FOLDER_VIEW_H_

#include <Wt/WTreeView.h>

using namespace Wt;

/**
 * \addtogroup treeviewdragdrop
 */
/*@{*/

/*! \brief A specialized treeview that supports a custom drop event.
 */
class FolderView : public WTreeView
{
public:
  /*! \brief Constant that indicates the mime type for a selection of files.
   *
   * Every kind of dragged data should be identified using a unique mime type.
   */
  static const char *FileSelectionMimeType;

  /*! \brief Constructor.
   */
  FolderView();

protected:
  /*! \brief Drop event.
   */
  virtual void dropEvent(const WDropEvent &event,
                         const WModelIndex &target);
};

/*@}*/

#endif // FOLDER_VIEW_H_
