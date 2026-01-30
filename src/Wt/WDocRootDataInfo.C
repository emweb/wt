/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDocRootDataInfo.h"

#include "Wt/cpp17/filesystem.hpp"

#include "Wt/WApplication.h"
#include "Wt/WException.h"

namespace Wt {

WDocRootDataInfo::WDocRootDataInfo(const std::string& path)
  : relPath_(path)
{ }

void WDocRootDataInfo::setRelativePath(const std::string& path)
{
  relPath_ = path;
}

std::string WDocRootDataInfo::filePath() const
{
  if (hasFilePath()) {
    cpp17::filesystem::path path = cpp17::filesystem::path(relPath_);
    path = cpp17::filesystem::path(WApplication::instance()->docRoot()) / path;
    return path.string();
  }
  return WAbstractDataInfo::filePath();
}

std::string WDocRootDataInfo::url() const
{
  if (hasUrl()) {
    return relPath_;
  }
  return WAbstractDataInfo::url();
}

}