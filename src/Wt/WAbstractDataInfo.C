/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractDataInfo.h"
#include "Wt/WException.h"

#include "Wt/WLogger.h"

namespace Wt {

LOGGER("WAbstractDataInfo");

std::string WAbstractDataInfo::filePath() const
{
  throw WException("WAbstractDataInfo::filePath(): missing file path.");
}

std::string WAbstractDataInfo::uri() const
{
  throw WException("WAbstractDataInfo::uri(): missing URI.");
}

std::string WAbstractDataInfo::name() const
{
  if (hasUri()) {
    return uri();
  } else if (hasFilePath()) {
    return filePath();
  }
  return "";
}

}