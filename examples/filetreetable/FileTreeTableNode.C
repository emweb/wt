// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileTreeTableNode.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

#include <Wt/WDateTime.h>
#include <Wt/WIconPair.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WStringUtil.h>
#include <Wt/WText.h>
#include <Wt/WAny.h>

FileTreeTableNode::FileTreeTableNode(const boost::filesystem::path& path)
#if BOOST_FILESYSTEM_VERSION < 3
  : WTreeTableNode(Wt::widen(path.leaf()), createIcon(path)),
#else
  : WTreeTableNode(path.leaf().string(), createIcon(path)),
#endif
    path_(path)
{
  label()->setTextFormat(TextFormat::Plain);

  if (boost::filesystem::exists(path)) {
    if (!boost::filesystem::is_directory(path)) {
      int fsize = (int)boost::filesystem::file_size(path);
      setColumnWidget(1, std::make_unique<WText>(asString(fsize)));
      columnWidget(1)->setStyleClass("fsize");
    } else
      setSelectable(false);

    std::time_t t = boost::filesystem::last_write_time(path);
    Wt::WDateTime dateTime = Wt::WDateTime::fromTime_t(t);
    Wt::WString dateTimeStr = dateTime.toString(Wt::utf8("MMM dd yyyy"));

    setColumnWidget(2, std::make_unique<WText>(dateTimeStr));
    columnWidget(2)->setStyleClass("date");
  }
}

std::unique_ptr<WIconPair> FileTreeTableNode::createIcon(const boost::filesystem::path& path)
{
  if (boost::filesystem::exists(path)
      && boost::filesystem::is_directory(path))
    return std::make_unique<WIconPair>("icons/yellow-folder-closed.png",
                         "icons/yellow-folder-open.png", false);
  else
    return std::make_unique<WIconPair>("icons/document.png",
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
        addChildNode(std::make_unique<FileTreeTableNode>(*i));
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
