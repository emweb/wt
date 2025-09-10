/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDataInfo.h"

#include "Wt/WException.h"

namespace Wt {

WDataInfo::WDataInfo()
{ }

WDataInfo::WDataInfo(const std::string& url, const std::string& filePath)
  : url_(url),
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

void WDataInfo::setUrl(const std::string& url)
{
  url_ = url;
}

std::string WDataInfo::url() const
{
  if (hasUrl()) {
    return url_;
  }
  return WAbstractDataInfo::url();
}

void WDataInfo::setDataUri(const std::string& dataUri)
{
  dataUri_ = dataUri;
}

std::string WDataInfo::dataUri() const
{
  if (hasDataUri()) {
    return dataUri_;
  }
  return WAbstractDataInfo::dataUri();
}

}