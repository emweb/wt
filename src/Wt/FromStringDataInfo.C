/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/FromStringDataInfo.h"

#include "UriUtils.h"

namespace Wt {

FromStringDataInfo::FromStringDataInfo(const std::string& url)
  : uri_(url),
    filePath_(),
    isDataUri_(DataUri::isDataUri(url))
{
  if (!isDataUri_) {
    filePath_ = url;
  }
}

FromStringDataInfo::FromStringDataInfo(const std::string& url, const std::string& filePath)
  : uri_(url),
    filePath_(filePath),
    isDataUri_(DataUri::isDataUri(url))
{ }

std::string FromStringDataInfo::filePath() const
{
  if (hasFilePath()) {
    return filePath_;
  }
  return WAbstractDataInfo::filePath();
}

std::string FromStringDataInfo::url() const
{
  if (hasUrl()) {
    return uri_;
  }
  return WAbstractDataInfo::url();
}

std::string FromStringDataInfo::dataUri() const
{
  if (hasDataUri()) {
    return uri_;
  }
  return WAbstractDataInfo::dataUri();
}

}