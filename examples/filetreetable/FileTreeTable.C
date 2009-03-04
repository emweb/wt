// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileTreeTable.h"
#include "FileTreeTableNode.h"

#include <Wt/WText>

using namespace Wt;

FileTreeTable::FileTreeTable(const boost::filesystem::path& path,
			     WContainerWidget *parent)
  : WTreeTable(parent)
{
  addColumn("Size", 80);
  addColumn("Modified", 110);

  header(1)->setStyleClass("fsize");
  header(2)->setStyleClass("date");

  setTreeRoot(new FileTreeTableNode(path), "File");

  treeRoot()->setImagePack("icons/");
  treeRoot()->expand();
}
