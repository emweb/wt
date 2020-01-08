// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FILETREETABLE_H_
#define FILETREETABLE_H_

#include <Wt/WTreeTable.h>

#include <boost/filesystem/path.hpp>

/**
 * \defgroup fileexplorer File Explorer example
 */
/*@{*/

/*! \brief A tree table that displays a file tree.
 *
 * The table allows one to browse a path, and all its subdirectories,
 * using a tree table. In addition to the file name, it shows file size
 * and modification date.
 *
 * The table use FileTreeTableNode objects to display the actual content
 * of the table. 
 *
 * The tree table uses the LazyLoading strategy of WTreeNode to dynamically
 * load contents for the tree.
 *
 * This widget is part of the %Wt File Explorer example.
 */
class FileTreeTable : public Wt::WTreeTable
{
public:
  /*! \brief Construct a new FileTreeTable.
   *
   * Create a new FileTreeTable to browse the given path.
   */
  FileTreeTable(const boost::filesystem::path& path);
};

/*@}*/

#endif // FILETREETABLE_H_
