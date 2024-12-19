// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFAVICON_H_
#define WFAVICON_H_

#include "Wt/WDllDefs.h"
#include <string>

namespace Wt {

class WT_API WFavicon
{
public:
  virtual std::string url() const = 0;

  void update();
  void reset();
  bool isUpdate() const { return isUpdate_; }

protected:
  WFavicon();
  virtual void doUpdate();
  virtual void doReset();

private:
  bool isUpdate_;
};

}
#endif //WFAVICON_H_