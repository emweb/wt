// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FILETREETABLENODE_H_
#define FILETREETABLENODE_H_

#include <Wt/WTreeTableNode.h>

#include <boost/filesystem/path.hpp>

using namespace Wt;

/**
 * @addtogroup fileexplorer
 */
/*@{*/

/*! \brief A single node in a file tree table.
 *
 * The node manages the details about one file, and if the file is a
 * directory, populates a subtree with nodes for every directory item.
 *
 * The tree node reimplements Wt::WTreeTableNode::populate() to populate
 * a directory node only when the node is expanded. In this way, only
 * directories that are actually browsed are loaded from disk.
 */
class FileTreeTableNode : public WTreeTableNode
{
public:
  /*! \brief Construct a new node for the given file.
   */
  FileTreeTableNode(const boost::filesystem::path& path);

private:
  //! The path.
  boost::filesystem::path path_;

  //! Reimplements WTreeNode::populate to read files within a directory.
  virtual void populate() override;

  //! Reimplements WTreeNode::expandable
  virtual bool expandable() override;

  //! Create the iconpair for representing the path.
  static std::unique_ptr<WIconPair> createIcon(const boost::filesystem::path& path);
};

/*@}*/

#endif // FILETREETABLENODE_H_
