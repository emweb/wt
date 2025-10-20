/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractDataInfo.h"
#include "Wt/WException.h"

namespace Wt {

std::string WAbstractDataInfo::filePath() const
{
  throw WException("WAbstractDataInfo::filePath(): missing file path.");
}

std::string WAbstractDataInfo::url() const
{
  throw WException("WAbstractDataInfo::url(): missing URL.");
}

std::string WAbstractDataInfo::dataUri() const
{
  throw WException("WAbstractDataInfo::dataUri(): missing data URI.");
}

std::string WAbstractDataInfo::name() const
{
  if (hasUrl()) {
    return url();
  } else if (hasFilePath()) {
    return filePath();
  } else if (hasDataUri()) {
    return dataUri();
  }
  return "";
}

}