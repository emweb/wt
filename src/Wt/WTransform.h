// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTRANSFORM_H_
#define WTRANSFORM_H_

#include <Wt/WDllDefs.h>
#include <Wt/WJavaScriptExposableObject.h>

namespace Wt {

class WPainterPath;
class WPointF;
class WRectF;

/*! \class WTransform Wt/WTransform.h Wt/WTransform.h
 *  \brief A value class that defines a 2D affine transformation matrix.
 *
 * The matrix is encoded using 6 parameters:
 * \code
 * m11  m12   0
 * m21  m22   0
 * dx   dy    1
 * \endcode
 * 
 * In this representation, dx() (= m31()) and dy() (= m32()) represent
 * the translation components, and m<i>xy</i> represent a 2D matrix
 * that contains the scale, rotation (and skew) components.
 *
 * The transformation is used to represent a tansformed coordinate
 * system, and provides methods to rotate(), scale(), shear() or
 * translate() this coordinate system.
 *
 * There are also 2 methods to decompose an arbitrary matrix into
 * elementary operations:
 * - decomposeTranslateRotateScaleSkew() 
 * decomposes into a <i>T</i> &#x2218; <i>R</i> &#x2218; <i>Sxx</i>
 * &#x2218; <i>Sxy</i>
 * - decomposeTranslateRotateScaleRotate()
 * decomposes into a <i>T</i> &#x2218; <i>R1</i> &#x2218; <i>Sxx</i>
 * &#x2218; <i>R2</i>
 * 
 * with <i>T</i> a translation, <i>R</i> a rotation, <i>Sxx</i> a scale, and <i>Sxy</i> a skew component.
 *
 * <h3>JavaScript exposability</h3>
 *
 * A %WTransform is JavaScript exposable. If a %WTransform \link isJavaScriptBound()
 * is JavaScript bound\endlink, it can be accessed in your custom JavaScript
 * code through \link WJavaScriptHandle::jsRef() its handle's jsRef()\endlink.
 * A transform is represented as a JavaScript array, e.g. a WTransform(m11, m12, m21, m22, dx, dy)
 * will be represented in JavaScript by:
 * \code
 * [m11, m12, m21, m22, dx, dy]
 * \endcode
 *
 * As an exception to the general rule that \link isJavaScriptBound() JavaScript bound\endlink objects
 *	 should not be modified, %WTransform does support many modifications. These modifications will then
 *	 accumulate in the JavaScript representation of the transform.
 *
 * \ingroup painting
 */
class WT_API WTransform : public WJavaScriptExposableObject
{
public:
  /*! \brief Default constructor.
   *
   * Creates the identity transformation matrix.
   */
  WTransform();

  /*! \brief Construct a custom matrix by specifying the parameters.
   *
   * Creates a matrix from the specified parameters.
   */
  WTransform(double m11, double m12, double m21, double m22,
	     double dx, double dy);

  /*! \brief Copy constructor.
   */
  WTransform(const WTransform &other);

#ifdef WT_TARGET_JAVA
  WTransform& operator=(const WJavaScriptExposableObject& rhs);
#endif

  /*! \brief Assignment operator.
   *
   * Copies the transformation from the \p rhs.
   */
  WTransform& operator=(const WTransform& rhs);

#ifdef WT_TARGET_JAVA
  /*! \brief Clone method.
   *
   * Clones this WTransform object.
   */
  WTransform clone() const;
#endif

  /*! \brief Comparison operator.
   *
   * Returns \c true if the transforms are exactly the same.
   */
  bool operator==(const WTransform& rhs) const;

  /*! \brief Comparison operator.
   *
   * Returns \c true if the transforms are different.
   */
  bool operator!=(const WTransform& rhs) const;

  /*! \brief Identity check.
   *
   * Returns true if the transform represents an identity transformation.
   *
   * \note This is always false if the transform is \link isJavaScriptBound() JavaScript bound\endlink.
   */
  bool isIdentity() const;
  
  /*! \brief Returns the horizontal scaling factor.
   */
  double m11() const { return m_[M11]; }

  /*! \brief Returns the vertical shearing factor.
   */
  double m12() const { return m_[M21]; }

  /*! \brief Returns m13 = 0.
   */
  double m13() const { return 0; }

  /*! \brief Returns the horizontal shearing factor.
   */
  double m21() const { return m_[M12]; }

  /*! \brief Returns the vertical scaling factor.
   */
  double m22() const { return m_[M22]; }

  /*! \brief Returns m23 = 0.
   */
  double m23() const { return 0; }

  /*! \brief Returns the horizontal translation factor.
   *
   * Is equivalent to dx()
   */
  double m31() const { return m_[M13]; }

  /*! \brief Returns the vertical translation factor.
   *
   * Is equivalent to dy()
   */
  double m32() const { return m_[M23]; }

  /*! \brief Returns m33 = 1.
   */
  double m33() const { return 1; }

  /*! \brief Returns the horizontal translation factor.
   *
   * Is equivalent to m31()
   */
  double dx() const { return m_[DX]; }

  /*! \brief Returns the vertical translation factor.
   *
   * Is equivalent to m32()
   */
  double dy() const { return m_[DY]; }

  /*! \brief Applys the transformation to a point.
   *
   * Returns the transformed point.
   *
   * \note If this transform or the given point
   *	   \link isJavaScriptBound() are JavaScript bound\endlink,
   * 	   the resulting point will also be JavaScript bound.
   *
   * \sa map(double x, double y, double *tx, double *ty) const
   */
  WPointF map(const WPointF& p) const;

  /*! \brief Applys the transformation to a point.
   *
   * Sets the point (<i>tx</i>, \p ty) to the transformation of
   * the point (<i>x</i>, \p y).
   *
   * \sa map(const WPointF&) const
   */
  void map(double x, double y, double *tx, double *ty) const;

  /*! \brief Applies the transformation to a rectangle.
   *
   * Since the rectangle is aligned with X and Y axes, this may
   * increase the size of the rectangle even for a transformation that
   * only rotates.
   *
   * \note If this transform or the given rectangle
   *	   \link isJavaScriptBound() are JavaScript bound\endlink,
   * 	   the resulting rectangle will also be JavaScript bound.
   */
  WRectF map(const WRectF& rect) const;

  /*! \brief Applies the transformation to a painter path.
   *
   * This will transform all individual points according to
   * the transformation. The radius of arcs will be unaffected.
   *
   * \note If this transform or the given path
   *	   \link isJavaScriptBound() are JavaScript bound\endlink,
   * 	   the resulting path will also be JavaScript bound.
   */
  WPainterPath map(const WPainterPath& path) const;

  /*! \brief Resets the transformation to the identity.
   *
   * \throws WException if the transform \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa isIdentity(), WTransform()
   */
  void reset();

  /*! \brief Rotates the transformation.
   *
   * Applies a clock-wise rotation to the current transformation
   * matrix, over \p angle degrees.
   *
   * \sa rotateRadians()
   */
  WTransform& rotate(double angle);

  /*! \brief Rotates the transformation.
   *
   * Applies a clock-wise rotation to the current transformation
   * matrix, over \p angle radians.
   *
   * \sa rotate()
   */
  WTransform& rotateRadians(double angle);

  /*! \brief Scales the transformation.
   *
   * Scales the current transformation.
   *
   * \sa shear()
   */
  WTransform& scale(double sx, double sy);

  /*! \brief Shears the transformation.
   *
   * Shears the current transformation.
   *
   * \sa scale(), rotate()
   */
  WTransform& shear(double sh, double sv);

  /*! \brief Translates the transformation.
   *
   * Translates the current transformation.
   */
  WTransform& translate(double dx, double dy);
  
  /*! \brief Translates the transformation.
   *
   * Translates the current transformation.
   *
   * \note If this transform or the given point
   *	   \link isJavaScriptBound() are JavaScript bound\endlink,
   * 	   the resulting transform will also be JavaScript bound.
   */
  WTransform& translate(const WPointF& p);

  /*! \brief Adds a transform that is conceptually applied after this transform.
   */
  WTransform& operator*= (const WTransform& rhs);

  /*! \brief Multiply 2 transform objects.
   */
  WTransform operator* (const WTransform& rhs) const;

  /*! \brief Returns the determinant.
   */
  double determinant() const;

  /*! \brief Returns the adjoint.
   */
  WTransform adjoint() const;

  /*! \brief Returns the inverted transformation.
   *
   * Returns \p this if the transformation could not be inverted
   * (determinant() == 0), and logs an error instead.
   */
  WTransform inverted() const;

  /*! \brief Result of a TRSS decomposition
   *
   * \sa decomposeTranslateRotateScaleSkew()
   */
  struct TRSSDecomposition {
    double dx, //!< X component of translation
      dy,      //!< Y component of translation
      alpha,   //!< Rotation angle (radians)
      sx,      //!< X component of scale
      sy,      //!< Y component of scale
      sh;      //!< Shear (in Y direction)
  };

  /*! \brief Decomposes the transformation
   *
   * Decomposes the transformation into elementary operations:
   * translation (<i>dx</i>, \p dy), followed by rotation
   * (<i>alpha</i>), followed by scale (<i>sx</i>, \p sy) and
   * vertical shearing factor (\p sh). The angle is expressed in
   * radians.
   *
   * This performs a <a
   * href="http://en.wikipedia.org/wiki/Gram_schmidt">Gram-Schmidt
   * orthonormalization</a>.
   */
  void decomposeTranslateRotateScaleSkew(TRSSDecomposition& result) const;

  /*! \brief Result of a TRSR decomposition
   *
   * \sa decomposeTranslateRotateScaleRotate()
   */
  struct TRSRDecomposition {
    double dx, //!< X component of translation
      dy,      //!< Y component of translation
      alpha1,  //!< First rotation angle (radians)
      sx,      //!< X component of scale
      sy,      //!< Y component of scale
      alpha2;  //!< Second rotation angle (radians)
  };

  /*! \brief Decomposes the transformation
   *
   * Decomposes the transformation into elementary operations:
   * translation (<i>dx</i>, \p dy), followed by rotation
   * (<i>alpha2</i>), followed by scale (<i>sx</i>, \p sy) and
   * again a rotation (\p alpha2). The angles are expressed in
   * radians.
   *
   * This performs a <a
   * href="http://en.wikipedia.org/wiki/Singular_value_decomposition">Singular
   * Value Decomposition (SVD)</a>.
   */
  void decomposeTranslateRotateScaleRotate(TRSRDecomposition& result) const;

  /*! \brief Utility method to convert degrees to radians.
   */
  static double degreesToRadians(double angle);

  /*! \brief A constant that represents the identity transform.
   *
   * \sa isIdentity()
   */
  static const WTransform Identity;

  bool closeTo(const WTransform& other) const;

  virtual std::string jsValue() const override;

protected:
  virtual void assignFromJSON(const Json::Value &value) override;

private:
  // we use row,column indices; prepend transformations to the left,
  // and transform column point vectors: X' = M.X

  // by row: real 2x2 matrix:
  static const int M11 = 0;
  static const int M12 = 1;
  static const int M21 = 2;
  static const int M22 = 3;

  static const int M13 = 4;
  static const int DX = 4;
  static const int M23 = 5;
  static const int DY = 5;

  double m_[6];
};

}

#endif // WTRANSFORM_H_
