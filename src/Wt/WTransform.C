/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cassert>
#include <cmath>

#include "Wt/WLogger"
#include "Wt/WPainterPath"
#include "Wt/WPointF"
#include "Wt/WRectF"
#include "Wt/WString"
#include "Wt/WTransform"

#include "Wt/Json/Array"
#include "Wt/Json/Value"

#include "WebUtils.h"

namespace Wt {

LOGGER("WTransform");

const WTransform WTransform::Identity;

WTransform::WTransform()
{
  reset();
}

WTransform::WTransform(double m11, double m12, double m21, double m22,
		       double dx, double dy)
{
  m_[M11] = m11;
  m_[M12] = m21;
  m_[M13] = dx;
  m_[M21] = m12;
  m_[M22] = m22;
  m_[M23] = dy;
}

WTransform::WTransform(const WTransform &other)
  : WJavaScriptExposableObject(other)
{
  for (unsigned i = 0; i < 6; ++i)
    m_[i] = other.m_[i];
}

WTransform& WTransform::operator= (const WTransform& rhs)
{
#ifndef WT_TARGET_JAVA
  WJavaScriptExposableObject::operator=(rhs);
#else
  if (rhs.isJavaScriptBound()) {
    assignBinding(rhs);
  } else {
    clientBinding_ = 0;
  }
#endif

  for (unsigned i = 0; i < 6; ++i)
    m_[i] = rhs.m_[i];

  return *this;
}

#ifdef WT_TARGET_JAVA
WTransform WTransform::clone() const
{
  WTransform result;
  result = *this;
  return result;
}
#endif

bool WTransform::operator== (const WTransform& rhs) const
{
  if (!sameBindingAs(rhs)) return false;

  for (unsigned i = 0; i < 6; ++i)
    if (m_[i] != rhs.m_[i])
      return false;

  return true;
}

bool WTransform::operator!= (const WTransform& rhs) const
{
  return !(*this == rhs);
}

bool WTransform::isIdentity() const
{
  return !isJavaScriptBound() && (m_[M11] == 1.0)
    && (m_[M22] == 1.0)
    && (m_[M21] == 0.0)
    && (m_[M12] == 0.0)
    && (m_[M13] == 0.0)
    && (m_[M23] == 0.0);
}

void WTransform::reset()
{
  checkModifiable();
  m_[M11] = m_[M22] = 1;
  m_[M21] = m_[M12] = m_[M13] = m_[M23] = 0;
}

WPointF WTransform::map(const WPointF& p) const
{
  if (isIdentity()) return p;

  double x, y;
  map(p.x(), p.y(), &x, &y);
  WPointF result(x, y);

  if (isJavaScriptBound() || p.isJavaScriptBound()) {
    const WJavaScriptExposableObject *o = this;
    if (p.isJavaScriptBound()) o = &p;
    result.assignBinding(*o,
	WT_CLASS ".gfxUtils.transform_mult(" + jsRef() + ',' + p.jsRef() + ')');
  }

  return result;
}

void WTransform::map(double x, double y, double *tx, double *ty) const
{
  *tx = m_[M11] * x + m_[M12] * y + m_[M13];
  *ty = m_[M21] * x + m_[M22] * y + m_[M23];
}

WRectF WTransform::map(const WRectF& rect) const
{
  if (isIdentity()) return rect;

  double minX, minY, maxX, maxY;

  WPointF p = map(rect.topLeft());
  minX = maxX = p.x();
  minY = maxY = p.y();

  for (unsigned i = 0; i < 3; ++i) {
    WPointF p2 = map(i == 0 ? rect.bottomLeft()
		     : i == 1 ? rect.topRight()
		     : rect.bottomRight());
    minX = std::min(minX, p2.x());
    maxX = std::max(maxX, p2.x());
    minY = std::min(minY, p2.y());
    maxY = std::max(maxY, p2.y());
  }

  WRectF result(minX, minY, maxX - minX, maxY - minY);

  if (isJavaScriptBound() || rect.isJavaScriptBound()) {
    const WJavaScriptExposableObject *o = this;
    if (rect.isJavaScriptBound()) o = &rect;
    result.assignBinding(*o,
	WT_CLASS ".gfxUtils.transform_mult(" + jsRef() + ',' + rect.jsRef() + ')');
  }

  return result;
}

WPainterPath WTransform::map(const WPainterPath& path) const
{
  if (isIdentity()) return path;

  WPainterPath result;

  if (isJavaScriptBound() || path.isJavaScriptBound()) {
    const WJavaScriptExposableObject *o = this;
    if (!isJavaScriptBound()) o = &path;
    result.assignBinding(*o,
	WT_CLASS ".gfxUtils.transform_apply(" + jsRef() + ',' + path.jsRef() + ')');
  }

  const std::vector<WPainterPath::Segment> &sourceSegments = path.segments();

  for (std::size_t i = 0; i < sourceSegments.size(); ++i) {
    double tx, ty;
    if (sourceSegments[i].type() == WPainterPath::Segment::ArcR ||
	sourceSegments[i].type() == WPainterPath::Segment::ArcAngleSweep) {
      result.segments_.push_back(sourceSegments[i]);
    } else {
      map(sourceSegments[i].x(), sourceSegments[i].y(), &tx, &ty);
      result.segments_.push_back(WPainterPath::Segment(tx, ty, sourceSegments[i].type()));
    }
  }

  return result;
}

WTransform& WTransform::rotateRadians(double angle)
{
  double r11 = std::cos(angle);
  double r12 = -std::sin(angle);
  double r21 = -r12;
  double r22 = r11;

  // note: our public constructor is transposed!
  return *this *= WTransform(r11, r21, r12, r22, 0, 0);
}

WTransform& WTransform::rotate(double angle)
{
  rotateRadians(degreesToRadians(angle));
  return *this;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double WTransform::degreesToRadians(double angle)
{
  return (angle / 180.) * M_PI;  
}

WTransform& WTransform::scale(double sx, double sy)
{
  return *this *= WTransform(sx, 0, 0, sy, 0, 0);
}

WTransform& WTransform::shear(double sh, double sv)
{
  return *this *= WTransform(0, sv, sh, 0, 0, 0);
}

WTransform& WTransform::translate(double dx, double dy)
{
  return *this *= WTransform(1, 0, 0, 1, dx, dy);
}

WTransform& WTransform::translate(const WPointF& p)
{
  bool identity = isIdentity();
  std::string refBefore = jsRef();
  translate(p.x(), p.y());

  if (isJavaScriptBound() || p.isJavaScriptBound()) {
    const WJavaScriptExposableObject *o = this;
    if (!isJavaScriptBound()) o = &p;
    if (identity) {
      assignBinding(*o,
	"((function(){"
	  "var p=" + p.jsRef() + ";"
	  "return [1,0,0,1,p[0],p[1]];"
	"})())");
    } else {
      assignBinding(*o,
	  WT_CLASS ".gfxUtils.transform_mult((function(){"
	    "var p="
	    + p.jsRef() + ";"
	    "return [1,0,0,1,p[0],p[1]];"
	  "})(),(" + refBefore + "))");
    }
  }

  return *this;
}

static double norm(double x1, double x2)
{
  return std::sqrt(x1 * x1 + x2 * x2);
}

double WTransform::determinant() const
{
  return m11() * (m33() * m22() - m32() * m23())
    - m21() * (m33() * m12() - m32() * m13())
    + m31() * (m23() * m12() - m22() * m13());
}

WTransform WTransform::adjoint() const
{
  WTransform res = WTransform(m33() * m22() - m32() * m23(),
		    - (m33() * m12() - m32() * m13()),
		    - (m33() * m21() - m31() * m23()),
		    m33() * m11() - m31() * m13(),
		    m32() * m21() - m31() * m22(),
		    - (m32() * m11() - m31() * m12()));

  if (isJavaScriptBound()) {
    res.assignBinding(*this,
	WT_CLASS ".gfxUtils.transform_adjoint(" + jsRef() + ")");
  }

  return res;
}

WTransform WTransform::inverted() const
{
  double det = determinant();

  if (det != 0) {
    WTransform adj = adjoint();

    WTransform res(adj.m11() / det, adj.m12() / det,
		   adj.m21() / det, adj.m22() / det,
		   adj.m31() / det, adj.m32() / det);
    if (isJavaScriptBound()) {
      res.assignBinding(*this,
	  WT_CLASS ".gfxUtils.transform_inverted(" + jsRef() + ")");
    }
    return res;
  } else {
    LOG_ERROR("inverted(): oops, determinant == 0");

    return *this;
  }
}

void WTransform::decomposeTranslateRotateScaleSkew(TRSSDecomposition& result)
  const
{
  // Performs a Gram Schmidt orthonormalization

  double q1[2];
  // double q2[2]; -- not used ?

  double r11 = norm(m_[M11], m_[M21]);
  q1[0] = m_[M11]/r11;
  q1[1] = m_[M21]/r11;

  double r12 = m_[M12]*q1[0] + m_[M22]*q1[1];
  double r22 = norm(m_[M12] - r12*q1[0], m_[M22] - r12*q1[1]);
  //q2[0] = (m_[M12] - r12 * q1[0])/r22;
  //q2[1] = (m_[M22] - r12 * q1[1])/r22;

  result.alpha = std::atan2(q1[1], q1[0]);

  result.sx = r11;
  result.sy = r22;
  result.sh = r12 / r11;

  result.dx = m_[DX];
  result.dy = m_[DY];
}

static void matrixMultiply(double a11, double a12, double a21, double a22,
			   double b11, double b12, double b21, double b22,
			   WT_ARRAY double *result)
{
  result[0] = a11 * b11 + a12 * b21;
  result[1] = a11 * b12 + a12 * b22;
  result[2] = a21 * b11 + a22 * b21;
  result[3] = a21 * b12 + a22 * b22;
}

static void eigenValues(WT_ARRAY double *m, WT_ARRAY double* l,
			WT_ARRAY double *v)
{
  const double a = m[0];
  const double b = m[1];
  const double c = m[2];
  const double d = m[3];

  double B = - a - d;
  double C = a * d - b * c;
  double Dsqr = B*B - 4*C;
  if (Dsqr <= 0) Dsqr = 0;
  double D = std::sqrt(Dsqr);

  l[0] = -(B + (B < 0 ? -D : D)) / 2.0;
  l[1] = -B - l[0];

  if (std::fabs(l[0] - l[1]) < 1E-5) {
    v[0] = 1;
    v[2] = 0;
    v[1] = 0;
    v[3] = 1;
  } else if (std::fabs(c) > 1E-5) {
    v[0] = d - l[0];
    v[2] = -c;
    v[1] = d - l[1];
    v[3] = -c;
  } else if (std::fabs(b) > 1E-5) {
    v[0] = -b;
    v[2] = a - l[0];
    v[1] = -b;
    v[3] = a - l[1];
  } else {
    if (std::fabs(l[0] - a) < 1E-5) {
      v[0] = 1;
      v[2] = 0;
      v[1] = 0;
      v[3] = 1;
    } else {
      v[0] = 0;
      v[2] = 1;
      v[1] = 1;
      v[3] = 0;
    }
  }

  double v1l = std::sqrt(v[0]*v[0] + v[2]*v[2]);
  v[0] /= v1l;
  v[2] /= v1l;

  double v2l = std::sqrt(v[1]*v[1] + v[3]*v[3]);
  v[1] /= v2l;
  v[3] /= v2l;
}

//#define DEBUG_SVD

void WTransform::decomposeTranslateRotateScaleRotate(TRSRDecomposition& result)
  const
{
  // Performs a Singular Value Decomposition

  double mtm[4];

  LOG_DEBUG("M: \n" << m_[M11] << " " << m_[M12] <<
	    "\n   " << m_[M21] << " " << m_[M22]);

  matrixMultiply(m_[M11], m_[M21], m_[M12], m_[M22],
		 m_[M11], m_[M12], m_[M21], m_[M22],
		 mtm);

  double e[2];
  double V[4];

  eigenValues(mtm, e, V);

  result.sx = std::sqrt(e[0]);
  result.sy = std::sqrt(e[1]);

  LOG_DEBUG("V: \n" << V[M11] << " " << V[M12] <<
	    "\n   " << V[M21] << " " << V[M22]);

  /*
   * if V is no rotation matrix, it contains a reflexion. A rotation
   * matrix has determinant of 1; a matrix that contains a reflexion
   * it has determinant -1. We reflect around the Y axis:
   */
  if (V[0]*V[3] - V[1]*V[2] < 0) {
    result.sx = -result.sx;
    V[0] = -V[0];
    V[2] = -V[2];
  }

  double U[4];

  matrixMultiply(m_[0], m_[1], m_[2], m_[3],
		 V[0], V[1], V[2], V[3],
		 U);
  U[0] /= result.sx;
  U[2] /= result.sx;
  U[1] /= result.sy;
  U[3] /= result.sy;

  LOG_DEBUG("U: \n" << U[M11] << " " << U[M12] <<
	    "\n   " << U[M21] << " " << U[M22]);

  if (U[0]*U[3] - U[1]*U[2] < 0) {
    result.sx = -result.sx;
    U[0] = -U[0];
    U[2] = -U[2];
  }

  result.alpha1 = std::atan2(U[2], U[0]);
  result.alpha2 = std::atan2(V[1], V[0]);

  LOG_DEBUG("alpha1: " << result.alpha1 << ", alpha2: " << result.alpha2
	    << ", sx: " << result.sx << ", sy: " << result.sy);

  /*
  // check our SVD: m_ = U S VT
  double tmp[4], tmp2[4];
  matrixMultiply(U[0], U[1], U[2], U[3],
		 sx, 0, 0, sy,
		 tmp);
  matrixMultiply(tmp[0], tmp[1], tmp[2], tmp[3],
		 V[0], V[2], V[1], V[3],
		 tmp2);

  LOG_DEBUG("check: \n" << 
	    tmp2[0] << " " << tmp2[1] << "\n"
	    tmp2[2] << " " << tmp2[3]);
  */

  result.dx = m_[DX];
  result.dy = m_[DY];
}

WTransform& WTransform::operator*= (const WTransform& Y)
{
  if (isIdentity()) return operator=(Y);
  if (Y.isIdentity()) return *this;

  // conceptually:                  Z = Y * X
  // our transposed representation: Z = X * Y

  const WTransform& X = *this;

  if (isJavaScriptBound() || Y.isJavaScriptBound()) {
    const WJavaScriptExposableObject *o = this;
    if (!isJavaScriptBound()) o = &Y;
    assignBinding(*o,
	WT_CLASS ".gfxUtils.transform_mult(" + jsRef() + ',' + Y.jsRef() + ')');
  }

  double z11 = X.m_[M11] * Y.m_[M11]
             + X.m_[M12] * Y.m_[M21]
          /* + X.m_[M13] * Y.m_[M31]=0*/;

  double z12 = X.m_[M11] * Y.m_[M12]
             + X.m_[M12] * Y.m_[M22]
          /* + X.m_[M13] * Y.m_[M32]=0*/;

  double z13 = X.m_[M11] * Y.m_[M13]
             + X.m_[M12] * Y.m_[M23]
             + X.m_[M13] /* * Y.m_[M33]=1*/;

  double z21 = X.m_[M21] * Y.m_[M11]
             + X.m_[M22] * Y.m_[M21]
          /* + X.m_[M23] * Y.m_[M31]=0*/;

  double z22 = X.m_[M21] * Y.m_[M12]
             + X.m_[M22] * Y.m_[M22]
          /* + X.m_[M23] * Y.m_[M32]=0*/;

  double z23 = X.m_[M21] * Y.m_[M13]
             + X.m_[M22] * Y.m_[M23]
             + X.m_[M23] /* * Y.m_[M33]=1*/;

  m_[M11] = z11;
  m_[M12] = z12;
  m_[M13] = z13;
  m_[M21] = z21;
  m_[M22] = z22;
  m_[M23] = z23;

  return *this;
}

WTransform WTransform::operator* (const WTransform& rhs) const
{
  WTransform result;
  result = *this;
  return result *= rhs;
}

std::string WTransform::jsValue() const
{
  char buf[30];

  WStringStream ss;
  ss << '[';
  ss << Utils::round_js_str(m_[0], 3, buf) << ',';
  ss << Utils::round_js_str(m_[2], 3, buf) << ',';
  ss << Utils::round_js_str(m_[1], 3, buf) << ',';
  ss << Utils::round_js_str(m_[3], 3, buf) << ',';
  ss << Utils::round_js_str(m_[4], 3, buf) << ',';
  ss << Utils::round_js_str(m_[5], 3, buf) << ']';
  return ss.str();
}

void WTransform::assignFromJSON(const Json::Value &value)
{
  try {
#ifndef WT_TARGET_JAVA
    const Json::Array &ar = value;
#else
    const Json::Array &ar = static_cast<Json::Array&>(value);
#endif
    if (ar.size() == 6 &&
	!ar[0].toNumber().isNull() &&
	!ar[1].toNumber().isNull() &&
	!ar[2].toNumber().isNull() &&
	!ar[3].toNumber().isNull() &&
	!ar[4].toNumber().isNull() &&
	!ar[5].toNumber().isNull()) {
      for (std::size_t i = 0; i < 6; ++i) {
	m_[i] = ar[i].toNumber().orIfNull(m_[i]);
      }
    } else {
      LOG_ERROR("Couldn't convert JSON to WTransform");
    }
  } catch (std::exception &e) {
    LOG_ERROR("Couldn't convert JSON to WTransform: " + std::string(e.what()));
  }
}

}
