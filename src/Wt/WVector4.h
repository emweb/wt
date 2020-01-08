// This may look like C code, but its' really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVECTOR4_H_
#define WVECTOR4_H_

#include "Wt/WGenericMatrix.h"

namespace Wt {
class WT_API WVector4: public WGenericMatrix<double, 4, 1>
{
public:
  WVector4();
  WVector4(const WVector4 &other);
  WVector4(const WGenericMatrix<double, 4, 1>::MatrixType &m) {
    impl() = m;
  }
  WVector4(const WGenericMatrix<double, 4, 1> &other);
  explicit WVector4(double *d);
  WVector4(double x, double y, double z, double w);

  const double &x() const;
  const double &y() const;
  const double &z() const;
  const double &w() const;
  double &x();
  double &y();
  double &z();
  double &w();

  WVector4 normalize() const;
  double length() const;
};
}
#endif // WVECTOR4_H_
