// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WVector4.h"

#include <cmath>

using namespace Wt;

WVector4::WVector4()
{
  (*this)(0, 0) = 0;
  (*this)(1, 0) = 0;
  (*this)(2, 0) = 0;
  (*this)(3, 0) = 1;
}

WVector4::WVector4(double *d)
  : WGenericMatrix<double, 4, 1>(d)
{}

WVector4::WVector4(double x, double y, double z, double w)
{
  (*this)(0, 0) = x;
  (*this)(1, 0) = y;
  (*this)(2, 0) = z;
  (*this)(3, 0) = w;
}

WVector4::WVector4(const WVector4 &other)
  : WGenericMatrix<double, 4, 1>(other)
{}

WVector4::WVector4(const WGenericMatrix<double, 4, 1> &other)
  : WGenericMatrix<double, 4, 1>(other)
{}

const double &WVector4::x() const
{
  return (*this)(0, 0);
}

const double &WVector4::y() const
{
  return (*this)(1, 0);
}

const double &WVector4::z() const
{
  return (*this)(2, 0);
}

const double &WVector4::w() const
{
  return (*this)(3, 0);
}

double &WVector4::x()
{
  return (*this)(0, 0);
}

double &WVector4::y()
{
  return (*this)(1, 0);
}

double &WVector4::z()
{
  return (*this)(2, 0);
}

double &WVector4::w()
{
  return (*this)(3, 0);
}

WVector4 WVector4::normalize() const
{
  double norm = length();
  return WVector4(x() / norm, y() / norm, z() / norm, w() / norm);
}

double WVector4::length() const
{
  // If this is a point, it does not make much sense.
  return std::sqrt(x() * x() + y() * y() + z() * z() + w() * w());
}
