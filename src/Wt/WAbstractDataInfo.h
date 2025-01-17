// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef W_ABSTRACT_DATA_INFO_H_
#define W_ABSTRACT_DATA_INFO_H_

#include "Wt/WDllDefs.h"
#include <string>

namespace Wt {

class WT_API WAbstractDataInfo
{
public:
  virtual std::string filePath() const;
  virtual std::string uri() const;
  virtual std::string name() const;

  virtual bool hasFilePath() const { return false; }
  virtual bool hasUri() const { return false; }
};

}



#endif // W_ABSTRACT_DATA_INFO_H_