// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
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
  Specificity(bool valid = true);
  Specificity(int a, int b, int c, int d);

  bool isValid() const { return valid_; }
  bool operator==(const Specificity& other) const;

  void setA(int a);
  void setB(int b);
  void setC(int c);
  void setD(int d);

  bool isSmallerThen(const Specificity& other) const;
  bool isGreaterThen(const Specificity& other) const;
  bool isSmallerOrEqualThen(const Specificity& other) const;
  bool isGreaterOrEqualThen(const Specificity& other) const;

private:
#ifndef WT_TARGET_JAVA
  unsigned value_; // using 4 LSB bytes only
#else
  long long value_;
#endif

  bool valid_;
};

  }
}

#endif
