// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FROM_STRING_DATA_INFO_H_
#define FROM_STRING_DATA_INFO_H_

#include "Wt/WAbstractDataInfo.h"

namespace Wt {

/*! \brief Internal class
 *
 * DataInfo created from a string like before DataInfo existed.
 */
class FromStringDataInfo : public WAbstractDataInfo
{
public:

  FromStringDataInfo(const std::string& url);
  FromStringDataInfo(const std::string& url, const std::string& filePath);

  std::string filePath() const override;

  std::string url() const override;

  std::string dataUri() const override;

  bool hasFilePath() const override { return !filePath_.empty(); }

  bool hasUrl() const override { return !isDataUri_; }

  virtual bool hasDataUri() const { return isDataUri_; }

private:
  std::string uri_, filePath_;
  bool isDataUri_;
};

}



#endif // FROM_STRING_DATA_INFO_H_