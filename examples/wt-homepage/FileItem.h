// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef EXAMPLE_ITEM_H
#define EXAMPLE_ITEM_H

#include <string.h>

#include "Wt/WStandardItem"
#include "Wt/WStandardItemModel"
#include "Wt/WString"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

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
  static const int ContentsRole = Wt::UserRole;
  static const int FilePathRole = Wt::UserRole + 1;
  static const int FileNameRole = Wt::UserRole + 2;
  
  FileItem(const std::string& iconUri, const Wt::WString& text,
	   const std::string& fileName)
    : WStandardItem(iconUri, text)
  { 
    setData(fileName, FileNameRole);
    setData(fileName, FilePathRole);
  }
};
 
#endif // FILE_ITEM_H
