// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVECTOR3_H_
#define WVECTOR3_H_

#include "Wt/WGenericMatrix.h"
#include "Wt/WVector4.h"

namespace Wt {
class WT_API WVector3: public WGenericMatrix<double, 3, 1>
{
public:
  WVector3();
  WVector3(const WVector3 &other);
  WVector3(const WVector4 &other);
  WVector3(const WGenericMatrix<double, 3, 1>::MatrixType &m) {
    impl() = m;
  }
  WVector3(const WGenericMatrix<double, 3, 1> &other);
  explicit WVector3(double *d);
  WVector3(double x, double y, double z);

  const double &x() const;
  const double &y() const;
  const double &z() const;
  double &x();
  double &y();
  double &z();

  WVector3 normalize() const;
  double length() const;
  WVector3 cross(const WVector3 &other) const;
  double dot(const WVector3 &other) const;

#ifdef WT_TARGET_JAVA
  void setElement(int i, double v);
#endif
};
}

#endif // WVECTOR3_H_
