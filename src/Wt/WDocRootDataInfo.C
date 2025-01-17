/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDocRootDataInfo.h"

#include "Wt/WApplication.h"
#include "Wt/WException.h"

#include <boost/filesystem/path.hpp>

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
    boost::filesystem::path path = boost::filesystem::path(relPath_);
    path = boost::filesystem::path(WApplication::instance()->docRoot()) / path;
    return path.string();
  }
  return WAbstractDataInfo::filePath();
}

std::string WDocRootDataInfo::uri() const
{
  if (hasUri()) {
    return relPath_;
  }
  return WAbstractDataInfo::uri();
}

}