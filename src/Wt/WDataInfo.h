// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDATA_INFO_H_
#define WDATA_INFO_H_

#include "Wt/WAbstractDataInfo.h"

namespace Wt {

class WT_API WDataInfo : public WAbstractDataInfo
{
public:
  WDataInfo(const std::string& uri, const std::string& filePath);

  void setFilePath(const std::string& filePath);
  std::string filePath() const override;

  void setUri(const std::string& uri);
  std::string uri() const override;

  bool hasFilePath() const override { return !filePath_.empty(); }
  bool hasUri() const override { return !uri_.empty(); }

private:
  std::string uri_, filePath_;
};

}



#endif // WDATA_INFO_H_