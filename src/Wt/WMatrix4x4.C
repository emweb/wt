// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WMatrix4x4"
#include <boost/numeric/ublas/lu.hpp>


using namespace Wt;

WMatrix4x4::WMatrix4x4()
{}

WMatrix4x4::WMatrix4x4(double *d)
  : WGenericMatrix<double, 4, 4>(d)
{}

WMatrix4x4::WMatrix4x4(double m11, double m12, double m13, double m14,
		       double m21, double m22, double m23, double m24,
		       double m31, double m32, double m33, double m34,
		       double m41, double m42, double m43, double m44):
  WGenericMatrix<double, 4, 4>(0)
{
  (*this)(0, 0) = m11;
  (*this)(0, 1) = m12;
  (*this)(0, 2) = m13;
  (*this)(0, 3) = m14;
  (*this)(1, 0) = m21;
  (*this)(1, 1) = m22;
  (*this)(1, 2) = m23;
  (*this)(1, 3) = m24;
  (*this)(2, 0) = m31;
  (*this)(2, 1) = m32;
  (*this)(2, 2) = m33;
  (*this)(2, 3) = m34;
  (*this)(3, 0) = m41;
  (*this)(3, 1) = m42;
  (*this)(3, 2) = m43;
  (*this)(3, 3) = m44;
}

WMatrix4x4::WMatrix4x4(const WMatrix4x4 &other)
  : WGenericMatrix<double, 4, 4>(other)
{}

WMatrix4x4::WMatrix4x4(const WGenericMatrix<double, 4, 4> &other)
  : WGenericMatrix<double, 4, 4>(other)
{} 

void WMatrix4x4::flipCoordinates()
{
  scale(1, -1, -1);
}

void WMatrix4x4::frustum(double left, double right, double bottom, double top,
			 double nearPlane, double farPlane)
{
  using namespace boost::numeric::ublas;
  WMatrix4x4 f(0);
  f(0, 0) = 2 * nearPlane / (right - left);
  f(0, 1) = 0;
  f(0, 2) = (right + left) / (right - left);
  f(0, 3) = 0;
    
  f(1, 0) = 0;
  f(1, 1) = 2 * nearPlane / (top - bottom);
  f(1, 2) = (top + bottom) / (top - bottom);
  f(1, 3) = 0;
    
  f(2, 0) = 0;
  f(2, 1) = 0;
  f(2, 2) = - (farPlane + nearPlane) / (farPlane - nearPlane);
  f(2, 3) = - 2 * farPlane * nearPlane / (farPlane - nearPlane);
    
  f(3, 0) = 0;
  f(3, 1) = 0;
  f(3, 2) = -1;
  f(3, 3) = 0;
  impl() = prod(impl(), f.impl());
}

void WMatrix4x4::lookAt(double eyeX, double eyeY, double eyeZ,
			double centerX, double centerY, double centerZ,
			double upX, double upY, double upZ)
{
  using namespace boost::numeric::ublas;
  // A 3D vector class would be handy here
  // Compute and normalize lookDir
  double lookDirX = centerX - eyeX;
  double lookDirY = centerY - eyeY;
  double lookDirZ = centerZ - eyeZ;
  double lookDirNorm = std::sqrt(lookDirX*lookDirX + lookDirY*lookDirY + lookDirZ*lookDirZ);
  lookDirX /= lookDirNorm;
  lookDirY /= lookDirNorm;
  lookDirZ /= lookDirNorm;
  // Compute and normalize the 'side' vector: cross product of lookDir and upDir
  double sideX = lookDirY*upZ - upY*lookDirZ;
  double sideY = -(lookDirX*upZ - upX*lookDirZ);
  double sideZ = lookDirX*upY - upX*lookDirY;
  double sideNormal = std::sqrt(sideX*sideX + sideY*sideY + sideZ*sideZ);
  sideX /= sideNormal;
  sideY /= sideNormal;
  sideZ /= sideNormal;
  // Compute the normalized 'up' vector: cross-prod of normalized look
  // and side dirs:
  double upDirX = sideY*lookDirZ - lookDirY*sideZ;
  double upDirY = -(sideX*lookDirZ - lookDirX*sideZ);
  double upDirZ = sideX*lookDirY - lookDirX*sideY;
  WMatrix4x4 l(
	       sideX,     sideY,     sideZ,     -(eyeX*sideX + eyeY*sideY + eyeZ*sideZ),
	       upDirX,    upDirY,    upDirZ,    -(eyeX*upDirX + eyeY*upDirY + eyeZ*upDirZ),
	       -lookDirX, -lookDirY, -lookDirZ, +(+eyeX*lookDirX + eyeY*lookDirY + eyeZ*lookDirZ),
	       0,      0,         0,     1
	       );
  impl() = prod(impl(), l.impl());;
}

void WMatrix4x4::ortho(double left, double right, double bottom, double top,
		       double nearPlane, double farPlane)
{
  using namespace boost::numeric::ublas;
  WMatrix4x4 o(
	       2 / (right - left), 0, 0, - (right + left) / (right - left),
	       0, 2 / (top - bottom), 0, - (top + bottom) / (top - bottom),
	       0, 0, -2 / (farPlane - nearPlane), - (farPlane + nearPlane) / (farPlane - nearPlane),
	       0, 0, 0, 1
	       );
  impl() = prod(impl(), o.impl());;
}

void WMatrix4x4::perspective(double angle, double aspect,
		 double nearPlane, double farPlane)
{
  double halfHeight =
    nearPlane * std::tan(angle / 2 / 180 * 3.14159265358979323846);
  double halfWidth = halfHeight * aspect;
  frustum(-halfWidth, halfWidth, -halfHeight, halfHeight,
	  nearPlane, farPlane);
}

void WMatrix4x4::rotate(double angle, double x, double y, double z)
{
  using namespace boost::numeric::ublas;
  double t = angle / 180.0 * 3.14159265358979323846;
  double norm2 = std::sqrt(x*x + y*y + z*z);
  x /= norm2;
  y /= norm2;
  z /= norm2;
  double cost = std::cos(t);
  double sint = std::sin(t);
  WMatrix4x4 rot(0);
  rot(0,0) = cost + x*x*(1-cost);
  rot(0,1) = x*y*(1-cost) - z*sint;
  rot(0,2) = x*z*(1-cost) + y*sint;
  rot(0,3) = 0;
  rot(1,0) = y*x*(1-cost) + z*sint;
  rot(1,1) = cost + y*y*(1 - cost);
  rot(1,2) = y*z*(1-cost) - x*sint;
  rot(1,3) = 0;
  rot(2,0) = z*x*(1-cost) - y*sint;
  rot(2,1) = z*y*(1-cost) + x*sint;
  rot(2,2) = cost + z*z*(1-cost);
  rot(2,3) = 0;
  rot(3,0) = 0;
  rot(3,1) = 0;
  rot(3,2) = 0;
  rot(3,3) = 1;

  impl() = prod(impl(), rot.impl());;
}

void WMatrix4x4::translate(double x, double y, double z)
{
  using namespace boost::numeric::ublas;
  WMatrix4x4 T;
  T(0, 3) = x;
  T(1, 3) = y;
  T(2, 3) = z;
  impl() = prod(impl(), T.impl());;
}


double WMatrix4x4::determinant() const
{
  using namespace boost::numeric::ublas;
  bounded_matrix<double, 4, 4, row_major> tmp(impl());
  boost::numeric::ublas::permutation_matrix<unsigned> pivots(4);
  double det = 1.0;
  if (boost::numeric::ublas::lu_factorize(tmp, pivots)) {
    // Singular matrix
    det = 0.0;
  } else {
    for(std::size_t i = 0; i < 4; ++i) {
      if (pivots(i) != i)
        det *= -1;
      det *= tmp(i,i);
    }
  }
  return det;
}

WMatrix4x4 WMatrix4x4::inverted(bool *invertible) const
{
  using namespace boost::numeric::ublas;
  WMatrix4x4 retval; // Identity matrix now
  bounded_matrix<double, 4, 4, row_major> tmp(impl());
  boost::numeric::ublas::permutation_matrix<unsigned> pivots(4);

  if (boost::numeric::ublas::lu_factorize(tmp, pivots)) {
    // Singular matrix, not invertible
    if (invertible)
      *invertible = false;
    return retval;
  } else {
    if (invertible)
      *invertible = true;
    boost::numeric::ublas::lu_substitute(tmp, pivots, retval.impl());
    return retval;
  }
}

