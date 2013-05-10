// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef RENDER_SPECIFICITY_H_
#define RENDER_SPECIFICITY_H_

#include <Wt/WDllDefs.h>

#include <string>

namespace Wt {
namespace Render {

class WT_API Specificity
{
public:
  explicit Specificity(bool valid = true);
  Specificity(int a, int b, int c, int d);

  void setValid(bool b);
  void setA(int a);
  void setB(int b);
  void setC(int c);
  void setD(int d);

  bool isValid() const;
  bool isSmallerThen(const Specificity& other) const;
  bool isGreaterThen(const Specificity& other) const;
  bool isSmallerOrEqualThen(const Specificity& other) const;
  bool isGreaterOrEqualThen(const Specificity& other) const;

#ifndef WT_TARGET_JAVA
  operator bool() const { return isValid(); }
#endif

private:
  std::string value_;
};

  }
}

#endif
