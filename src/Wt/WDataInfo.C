/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDataInfo.h"

#include "Wt/WException.h"

namespace Wt {

WDataInfo::WDataInfo(const std::string& uri, const std::string& filePath)
  : uri_(uri),
    filePath_(filePath)
{ }

void WDataInfo::setFilePath(const std::string& filePath)
{
  filePath_ = filePath;
}

std::string WDataInfo::filePath() const
{
  if (hasFilePath()) {
    return filePath_;
  }
  return WAbstractDataInfo::filePath();
}

void WDataInfo::setUri(const std::string& uri)
{
  uri_ = uri;
}

std::string WDataInfo::uri() const
{
  if (hasUri()) {
    return uri_;
  }
  return WAbstractDataInfo::uri();
}

}