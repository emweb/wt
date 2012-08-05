// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FOLDER_VIEW_H_
#define FOLDER_VIEW_H_

#include <Wt/WTreeView>

/**
 * \addtogroup treeviewdragdrop
 */
/*@{*/

/*! \brief A specialized treeview that supports a custom drop event.
 */
class FolderView : public Wt::WTreeView 
{
public:
  /*! \brief Constant that indicates the mime type for a selection of files.
   *
   * Every kind of dragged data should be identified using a unique mime type.
   */
  static const char *FileSelectionMimeType;

  /*! \brief Constructor.
   */
  FolderView(Wt::WContainerWidget *parent = 0);

protected:
  /*! \brief Drop event.
   */
  virtual void dropEvent(const Wt::WDropEvent &event,
			 const Wt::WModelIndex &target);
};

/*@}*/

#endif // FOLDER_VIEW_H_
