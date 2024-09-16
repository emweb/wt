// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileTreeTableNode.h"

#include <Wt/WDateTime.h>
#include <Wt/WIconPair.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WStringUtil.h>
#include <Wt/WText.h>
#include <Wt/WAny.h>
#include <web/FileUtils.h>

#include <Wt/cpp17/filesystem.hpp>

#include <iostream>

FileTreeTableNode::FileTreeTableNode(const Wt::cpp17::filesystem::path& path)
  : WTreeTableNode(path.filename().string(), createIcon(path)),
    path_(path)
{
  label()->setTextFormat(TextFormat::Plain);

  if (Wt::cpp17::filesystem::exists(path)) {
    if (!Wt::cpp17::filesystem::is_directory(path)) {
      int fsize = (int)Wt::cpp17::filesystem::file_size(path);
      setColumnWidget(1, std::make_unique<WText>(asString(fsize)));
      columnWidget(1)->setStyleClass("fsize");
    } else
      setSelectable(false);

    std::time_t t = std::chrono::system_clock::to_time_t(Wt::FileUtils::lastWriteTime(path.string()));
    Wt::WDateTime dateTime = Wt::WDateTime::fromTime_t(t);
    Wt::WString dateTimeStr = dateTime.toString(Wt::utf8("MMM dd yyyy"));

    setColumnWidget(2, std::make_unique<WText>(dateTimeStr));
    columnWidget(2)->setStyleClass("date");
  }
}

std::unique_ptr<WIconPair> FileTreeTableNode::createIcon(const Wt::cpp17::filesystem::path& path)
{
  if (Wt::cpp17::filesystem::exists(path)
      && Wt::cpp17::filesystem::is_directory(path))
    return std::make_unique<WIconPair>("icons/yellow-folder-closed.png",
                         "icons/yellow-folder-open.png", false);
  else
    return std::make_unique<WIconPair>("icons/document.png",
                         "icons/yellow-folder-open.png", false);
}

void FileTreeTableNode::populate()
{
  if (Wt::cpp17::filesystem::is_directory(path_)) {
    std::set<Wt::cpp17::filesystem::path> paths;
    Wt::cpp17::filesystem::directory_iterator end_itr;

    for (Wt::cpp17::filesystem::directory_iterator i(path_); i != end_itr; ++i)
      try {
        paths.insert(*i);
      } catch (Wt::cpp17::filesystem::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
      }

    for (std::set<Wt::cpp17::filesystem::path>::iterator i = paths.begin();
      i != paths.end(); ++i)
      try {
        addChildNode(std::make_unique<FileTreeTableNode>(*i));
      } catch (Wt::cpp17::filesystem::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
      }
  }
}

bool FileTreeTableNode::expandable()
{
  if (!populated()) {
    return Wt::cpp17::filesystem::is_directory(path_);
  } else
    return WTreeTableNode::expandable();
}
