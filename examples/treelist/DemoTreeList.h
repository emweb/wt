// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef DEMO_TREE_LIST
#define DEMO_TREE_LIST

#include <Wt/WContainerWidget.h>

using namespace Wt;

namespace Wt {
  class WPushButton;
}

class TreeNode;

/**
 * \defgroup treelist Treelist example
 */
/*@{*/

/*! \brief A demonstration of the treelist.
 *
 * This is the main class for the treelist example.
 */
class DemoTreeList : public WContainerWidget
{
public:
  /*! \brief Create a DemoTreeList.
   */
  DemoTreeList();

private:
  TreeNode    *tree_;
  TreeNode    *testFolder_;
  int          testCount_;

  WPushButton *addFolderButton_;
  WPushButton *removeFolderButton_;

  /*!\brief Add a folder.
   */
  void addFolder();

  /*!\brief Remove a folder.
   */ 
  void removeFolder();

  /*!\brief Create a "folder" node, and insert in the given parent.
   */
  TreeNode *makeTreeFolder(const std::string name, TreeNode *parent);

  /*!\brief Create a "folder" root.
   */
  std::unique_ptr<TreeNode> makeTreeFolder(const std::string name);

  /*!\brief Create a "file" node, and insert in the given parent.
   */
  TreeNode *makeTreeFile(const std::string name, TreeNode *parent);

  /*!\brief Create a "file" root.
   */
  std::unique_ptr<TreeNode> makeTreeFile(const std::string name);
};

/*@}*/

#endif // DEMO_TREE_LIST
