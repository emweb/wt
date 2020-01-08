// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMATRIX4X4_H_
#define WMATRIX4X4_H_

#include <Wt/WGenericMatrix.h>

namespace Wt {

/*! \class WMatrix4x4 Wt/WMatrix4x4.h Wt/WMatrix4x4.h
 *  \brief A value class that describes a 3D affine transformation matrix.
 *
 * The matrix is a 4x4 matrix encoded using 16 parameters. The matrix
 * stores its data internally in row order.
 *
 * Normally, a transformation matrix (composed translation/rotation/scale,
 * but without perspective) is of this form:
 * \code
 * m00 m01 m02 dx
 * m10 m11 m12 dy
 * m20 m21 m22 dz
 * 0   0   0   1
 * \endcode
 *
 * In this representation, dx, dy and dz (= m(0, 3), m(1, 3) and m(2, 3))
 * represent the translation components, and m(<i>x, y</i>) represent a
 * 3D matrix that contains the scale, rotation (and skew) components. The
 * matrix is also capable of representing perspective projections. In that
 * case, the matrix will not match the form depicted above.
 *
 * In order to calculate the transformed vector w of a 3D vector v by the
 * transformation contained in matrix T, v will be left-multiplied by T:
 * \code
 * w = T * v;
 * \endcode
 * In the formula above, v and w are homogenous 3D column vectors
 * (x, y, z, w), equal to (x/w, y/w, z/w, 1). In normal use cases w is 1,
 * except for vectors that were transformed by a perspective projection
 * matrix.
 *
 * The transformation is used to represent a tansformed coordinate
 * system, and provides methods to rotate(), scale() or
 * translate() this coordinate system.
 *
 * This matrix class is matched to OpenGL's coordinate system and
 * matrix notation. The rotate, translate, scale, lookAt, perspective,
 * frustum and ortho methods of this class behave exactly like
 * their OpenGL equivalents. The only difference is that the storage of this
 * matrix is row-major, while OpenGL uses column-major. This should
 * only be a concern if you need to access the raw data of the matrix,
 * in which case you should use transposed().data() instead.
 * When WWebGL uses this class, it sends the data in the correct order
 * to the client.
 *
 */
class WT_API WMatrix4x4: public WGenericMatrix<double, 4, 4>
{
public:
  /*! \brief Default constructor.
   *
   * Creates the identity transformation matrix.
   */
  WMatrix4x4();

  /*! \brief Copy constructor.
   */
  WMatrix4x4(const WMatrix4x4 &other);

  WMatrix4x4(const WGenericMatrix<double, 4, 4>::MatrixType &m) {
    impl() = m;
  }

  /*! \brief Construct for a WGenericMatrix
   *
   * Creates the identity transformation matrix. As we inherit from
   * WGenericMatrix, most overloaded operators create a WGenericMatrix.
   * This implicit constructor ensures that you will not notice this.
   */
  WMatrix4x4(const WGenericMatrix<double, 4, 4> &other);

  /*! \brief Constructs a matrix from an array of elements.
   *
   * The input array is assumed to be in row-major order. If elements is 0,
   * the matrix is not initialized.
   */
  // Assumes d is ROW order
  explicit WMatrix4x4(double *d);

  /*! \brief Construct a custom matrix by specifying the parameters.
   *
   * Creates a matrix from the specified parameters.
   */
  WMatrix4x4(double m11, double m12, double m13, double m14,
             double m21, double m22, double m23, double m24,
             double m31, double m32, double m33, double m34,
             double m41, double m42, double m43, double m44);

  /*! \brief Returns the determinant.
   */
  double determinant() const;

  /*! \brief Switch between left-hand and right-hand side coordinate systems
   *
   * Equivalent to scale(1, -1, -1)
   */
  void flipCoordinates();

  /*! \brief Construct a perspective projection matrix
   *
   * This function constructs a perspective projection where the
   * camera is located in the origin. The visible volume is determined
   * by whatever that is visible when looking from the origin through the
   * rectangular 'window' defined by the coordinates (l, b, n) and
   * (r, t, n) (parallel to the XY plane). The zone is further delimited
   * by the near and the far clipping planes.
   *
   * The perspective matrix (P) is right-multiplied with the current
   * transformation matrix (M): M * P. Usually, you will want M to be
   * the identity matrix when using this method.
   */
  void frustum(double left, double right, double bottom, double top,
               double nearPlane, double farPlane);

  /*! \brief Returns the inversion of this matrix, if invertible
   *
   * If invertible is not 0, it will contain a bool that indicates if
   * the operation succeeded and the inverse matrix is returned. Else,
   * this method returns the unit matrix.
   */
  WMatrix4x4 inverted(bool *invertible = nullptr) const;

  /*! \brief Apply a transformation to position a camera
   *
   * (eyeX, eyeY, eyeZ) is the position of the camera.
   *
   * The camera looks at (centerX, centerY, centerZ).
   *
   * (upX, upY, upZ) is a vector that is the direction of the up vector.
   *
   * This method applies a rotation and translation transformation to
   * the current matrix so that the given eye becomes (0, 0, 0), the
   * center point is on the negative Z axis, and the up vector lies in the
   * X=0 plane, with its Y component in the positive Y axis direction.
   *
   * The up vector must not be parallel to the line between eye and center.
   * The vectors will be normalized and are not required to be perpendicular.
   *
   * If the lookat transformation matrix is M, and the current value of
   * the Matrix4x4 matrix is T, the resulting matrix after lookAt returns
   * will be T * M.
   *
   * This matrix is often used in conjunction with the
   * perspective() method:
   * \code
   * // First, apply the lookAt transformation
   * projectionMatrix.lookAt(1, 1, 1, 0, 0, 0, 0, 1, 0);
   * // Then apply some perspective
   * projectionMatrix.perspective(90, aspect, 0.1, 10);
   * \endcode
   *
   */
  void lookAt(double eyeX, double eyeY, double eyeZ,
              double centerX, double centerY, double centerZ,
              double upX, double upY, double upZ);

  /*! \brief Create an orhtographic projection matrix for use in OpenGL
   *
   * Create an orthographic projection matrix. The given left, right,
   * bottom, top, near and far points will be linearly mapped to the OpenGL
   * unit cube ((1,1,1) to (-1,-1,-1)).
   *
   * The orthographic matrix (O) is right-multiplied with the current
   * transformation matrix (M): M * O. Usually, you will want M to be
   * the identity matrix when using this method.
   */
  void ortho(double left, double right, double bottom, double top,
             double nearPlane, double farPlane);

  /*! \brief Construct a perspective projection matrix for use in OpenGL
   *
   * The camera is located in the origin and look in the direction of the
   * negative Z axis.
   *
   * Angle is the vertical view angle, in degrees. Aspect is the aspect ratio
   * of the viewport, and near and far are the distances of the front and
   * rear clipping plane from the camera.
   *
   * The perspective matrix (P) is right-multiplied with the current
   * transformation matrix (M): M * P. Usually, you will want M to be
   * the identity matrix when using this method.
   */
  void perspective(double angle, double aspect,
                   double nearPlane, double farPlane);

  /*! \brief Rotates the transformation around a random axis.
   *
   * Applies a rotation to the current transformation
   * matrix, over \p angle degrees. The current matrix (M) is
   * right-multiplied by the rotation matrix: M = M * R
   *
   */
  void rotate(double angle, double x, double y, double z);

  /*! \brief Scales the transformation.
   *
   * Equivalent to scale(xFactor, yFactor, 1);
   *
   * \sa scale(double, double, double)
   */
  void scale(double xFactor, double yFactor) { scale (xFactor, yFactor, 1); }

  /*! \brief Scales the transformation.
   *
   * Equivalent to M * S where M is the current transformation and S is
   * \code
   * x 0 0 0
   * 0 y 0 0
   * 0 0 z 0
   * 0 0 0 1
   * \endcode
   */
  void scale(double x, double y, double z)
  {
    for (unsigned i = 0; i < 4; ++i) {
      (*this)(i, 0) *= x;
      (*this)(i, 1) *= y;
      (*this)(i, 2) *= z;
    }
  }

  /*! \brief Scales the transformation.
   *
   * Equivalent to scale(factor, factor, factor);
   *
   * \sa scale(double, double, double)
   */
  void scale(double factor) { scale(factor, factor, factor); }

  /*! \brief Translates the transformation.
   *
   * Equivalent to translate(x, y, 0)
   */
  void translate(double x, double y)
  {
    translate(x, y, 0);
  }

  /*! \brief Translates the transformation.
   *
   * Translates the current transformation.
   *
   * Equivalent to M * T where M is the current transformation matrix
   * and T is:
   * \code
   * 1 0 0 x
   * 0 1 0 y
   * 0 0 1 z
   * 0 0 0 1
   * \endcode
   */
  void translate(double x, double y, double z);


  /*! \brief Multiply two matrices
   */
  WMatrix4x4 operator*(const WGenericMatrix<double, 4, 4> &r) const
  {
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    using namespace boost::numeric::ublas;
    return (WMatrix4x4)prod(impl(), r.impl());
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return WMatrix4x4(*static_cast<const WGenericMatrix<double, 4, 4>*>(this) * r);
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#else
    return 0;
#endif
  }

#ifdef WT_TARGET_JAVA
  WMatrix4x4 mul(const WGenericMatrix<double, 4, 4> &r);
  void setElement(int i1, int i2, float value);
#endif
};

}

#endif // WMATRIX4X4_H
