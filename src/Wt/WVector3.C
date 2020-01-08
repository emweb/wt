// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WVector3.h"
#include "Wt/WVector4.h"

#include <cmath>

using namespace Wt;

WVector3::WVector3()
{
  (*this)(0, 0) = 0;
  (*this)(1, 0) = 0;
  (*this)(2, 0) = 0;
}

WVector3::WVector3(double *d)
  : WGenericMatrix<double, 3, 1>(d)
{}

WVector3::WVector3(double x, double y, double z):
  WGenericMatrix<double, 3, 1>(0)
{
  (*this)(0, 0) = x;
  (*this)(1, 0) = y;
  (*this)(2, 0) = z;
}

WVector3::WVector3(const WVector3 &other)
  : WGenericMatrix<double, 3, 1>(other)
{}

WVector3::WVector3(const WVector4 &other)
{
  (*this)(0, 0) = other.x() / other.w();
  (*this)(1, 0) = other.y() / other.w();
  (*this)(2, 0) = other.z() / other.w();
}

WVector3::WVector3(const WGenericMatrix<double, 3, 1> &other)
  : WGenericMatrix<double, 3, 1>(other)
{}

const double &WVector3::x() const
{
  return (*this)(0, 0);
}

const double &WVector3::y() const
{
  return (*this)(1, 0);
}

const double &WVector3::z() const
{
  return (*this)(2, 0);
}

double &WVector3::x()
{
  return (*this)(0, 0);
}

double &WVector3::y()
{
  return (*this)(1, 0);
}

double &WVector3::z()
{
  return (*this)(2, 0);
}

WVector3 WVector3::normalize() const
{
  double norm = length();
  return WVector3(x() / norm, y() / norm, z() / norm);
}

double WVector3::length() const
{
  return std::sqrt(x() * x() + y() * y() + z() * z());
}

WVector3 WVector3::cross(const WVector3 &other) const
{
  return WVector3(
    y() * other.z() - z() * other.y(),
    z() * other.x() - x() * other.z(),
    x() * other.y() - y() * other.x()
  );
}

double WVector3::dot(const WVector3 &other) const
{
  return x() * other.x() + y() * other.y() + z() * other.z();
}
