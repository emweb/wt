/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WTree>
#include <Wt/WTreeTableNode>

#include "FileTreeTable.h"

using namespace Wt;

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  app->setTitle("File explorer example");
  app->useStyleSheet("filetree.css");

  FileTreeTable *treeTable = new FileTreeTable(".");
  treeTable->resize(500, 300);
  treeTable->tree()->setSelectionMode(ExtendedSelection);
  treeTable->treeRoot()->setNodeVisible(false);
  treeTable->treeRoot()->setChildCountPolicy(WTreeNode::Enabled);

  app->root()->addWidget(treeTable);

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

