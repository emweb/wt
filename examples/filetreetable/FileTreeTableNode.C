// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileTreeTableNode.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <time.h>

#include <Wt/WIconPair>
#include <Wt/WStringUtil>
#include <Wt/WText>

using namespace Wt;

FileTreeTableNode::FileTreeTableNode(const boost::filesystem::path& path)
#if BOOST_FILESYSTEM_VERSION < 3
#ifndef WT_NO_STD_WSTRING
  : WTreeTableNode(Wt::widen(path.leaf()), createIcon(path)),
#else
  : WTreeTableNode(path.leaf(), createIcon(path)),
#endif
#else
  : WTreeTableNode(path.leaf().string(), createIcon(path)),
#endif
    path_(path)
{
  label()->setTextFormat(PlainText);

  if (boost::filesystem::exists(path)) {
    if (!boost::filesystem::is_directory(path)) {
      int fsize = (int)boost::filesystem::file_size(path);
      setColumnWidget(1, new WText(boost::lexical_cast<std::string>(fsize)));
      columnWidget(1)->setStyleClass("fsize");
    } else
      setSelectable(false);

    std::time_t t = boost::filesystem::last_write_time(path);
    struct tm ttm;
#if WIN32
    ttm=*localtime(&t);
#else
    localtime_r(&t, &ttm);
#endif

    char c[100];
    strftime(c, 100, "%b %d %Y", &ttm);

    setColumnWidget(2, new WText(c));
    columnWidget(2)->setStyleClass("date");
  }
}

WIconPair *FileTreeTableNode::createIcon(const boost::filesystem::path& path)
{
  if (boost::filesystem::exists(path)
      && boost::filesystem::is_directory(path))
    return new WIconPair("icons/yellow-folder-closed.png",
			 "icons/yellow-folder-open.png", false);
  else
    return new WIconPair("icons/document.png",
			 "icons/yellow-folder-open.png", false);
}

void FileTreeTableNode::populate()
{
  if (boost::filesystem::is_directory(path_)) {
    std::set<boost::filesystem::path> paths;
    boost::filesystem::directory_iterator end_itr;

    for (boost::filesystem::directory_iterator i(path_); i != end_itr; ++i)
      try {
	paths.insert(*i);
      } catch (boost::filesystem::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
      }

    for (std::set<boost::filesystem::path>::iterator i = paths.begin();
      i != paths.end(); ++i)
      try {
        addChildNode(new FileTreeTableNode(*i));
      } catch (boost::filesystem::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
      }
  }
}

bool FileTreeTableNode::expandable()
{
  if (!populated()) {
    return boost::filesystem::is_directory(path_);
  } else
    return WTreeTableNode::expandable();
}
