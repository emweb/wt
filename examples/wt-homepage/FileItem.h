// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef EXAMPLE_ITEM_H
#define EXAMPLE_ITEM_H

#include <string.h>

#include "Wt/WStandardItem.h"
#include "Wt/WStandardItemModel.h"
#include "Wt/WString.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

using namespace Wt;

/*! \class FileItem
 *  \brief WStandardItem which stores a file.
 *
 * The SourceView class reads data from 3 additional roles:
 *  - ContentsRole: file contents
 *  - FilePathRole: the path which holds the file contents
 *  - FileNameRole: the original file name (used to derive the file type)
 */
class FileItem : public Wt::WStandardItem
{
public:
  static const Wt::ItemDataRole ContentsRole;
  static const Wt::ItemDataRole FilePathRole;
  static const Wt::ItemDataRole FileNameRole;
  
  FileItem(const std::string& iconUri, const Wt::WString& text,
	   const std::string& fileName)
    : WStandardItem(iconUri, text)
  { 
    setData(fileName, FileNameRole);
    setData(fileName, FilePathRole);
  }
};
 
#endif // FILE_ITEM_H
