// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDOC_ROOT_DATA_INFO_H_
#define WDOC_ROOT_DATA_INFO_H_

#include "Wt/WAbstractDataInfo.h"

namespace Wt {

class WT_API WDocRootDataInfo : public WAbstractDataInfo
{
public:
  WDocRootDataInfo(const std::string& path);

  void setRelativePath(const std::string& path);

  std::string filePath() const override;
  std::string uri() const override;

  bool hasFilePath() const override { return !relPath_.empty(); }
  bool hasUri() const override { return !relPath_.empty(); }

private:
  std::string relPath_;
};

}



#endif // WDOC_ROOT_DATA_INFO_H_