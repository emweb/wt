// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef DEMO_TREE_LIST
#define DEMO_TREE_LIST

#include <Wt/WContainerWidget>

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
class DemoTreeList : public Wt::WContainerWidget
{
public:
  /*! \brief Create a DemoTreeList.
   */
  DemoTreeList(Wt::WContainerWidget *parent);

private:
  TreeNode *tree_;
  TreeNode *testMap_;
  int testCount_;

  Wt::WPushButton *addMapButton_;
  Wt::WPushButton *removeMapButton_;

  /*!\brief Add a map.
   */
  void addMap();

  /*!\brief Remove a map.
   */ 
  void removeMap();

  /*!\brief Create a "map" node, and insert in the given parent.
   */
  TreeNode *makeTreeMap(const std::string name, TreeNode *parent);

  /*!\brief Create a "file" node, and insert in the given parent.
   */
  TreeNode *makeTreeFile(const std::string name, TreeNode *parent);
};

/*@}*/

#endif // DEMO_TREE_LIST
