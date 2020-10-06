/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WTree.h>
#include <Wt/WTreeTableNode.h>

#include "FileTreeTable.h"

using namespace Wt;

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app = std::make_unique<WApplication>(env);
  app->setTitle("File explorer example");
  app->useStyleSheet("filetree.css");

  std::unique_ptr<FileTreeTable> treeTable
      = std::make_unique<FileTreeTable>(".");
  treeTable->resize(500, 300);
  treeTable->tree()->setSelectionMode(SelectionMode::Extended);
  treeTable->treeRoot()->setNodeVisible(false);
  treeTable->treeRoot()->setChildCountPolicy(ChildCountPolicy::Enabled);

  app->root()->addWidget(std::move(treeTable));

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

