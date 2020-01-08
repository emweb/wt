// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WGENERICMATRIX_H_
#define WGENERICMATRIX_H_

#include <Wt/WDllDefs.h>

#ifdef WT_TARGET_JAVA
#define WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif // WT_TARGET_JAVA

#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#ifdef _MSC_VER
// Avoid 64-bit related warnings on MSVC
#pragma warning( push )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4267 )
#endif
#define BOOST_SERIALIZATION_NO_LIB
// Include array_wrapper.hpp in Boost 1.64.0 as a workaround
//
// See Boost issues:
//  - https://svn.boost.org/trac/boost/ticket/12978
//  - https://svn.boost.org/trac/boost/ticket/12982
#include <boost/version.hpp>
#if BOOST_VERSION == 106400
#include <boost/serialization/array_wrapper.hpp>
#endif
#include <boost/numeric/ublas/matrix.hpp>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#include <Wt/WStringStream.h>

#include <algorithm>
#include <array>
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS

#include <ostream>

namespace Wt {

/*! \class WGenericMatrix Wt/WGenericMatrix.h Wt/WGenericMatrix.h
 * \brief A value class that describes a matrix
 *
 * This class represents a fixed-size dense (!= sparse) matrix. It
 * can be templatized to the number of rows and columns, and to the
 * datatype stored (integer types, floatin point types, complex types, ...)
 *
 * The row order of this matrix class is row-major. This means that when
 * accessing the raw data store linearly, you will first encounter all
 * elements of the first row, then the second row, and so on.
 *
 * This template class is used in Wt as base class for transformation
 * matrices, but can also be used as a general matrix class. Efficiency
 * for this use case was considered when this class was implemented, but
 * we recommend that you use a more specialized matrix class library
 * if the algorithms you need exceed what's offered here (for example,
 * if you intend to do many linear algebra computations, you may
 * consider boost ublass, MTL, ...).
 */
template<typename T, std::size_t Rows, std::size_t Cols>
class WGenericMatrix
{
public:
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  typedef boost::numeric::ublas::bounded_matrix<T, Rows, Cols, boost::numeric::ublas::row_major> MatrixType;
  typedef boost::numeric::ublas::bounded_matrix<T, Rows, Cols, boost::numeric::ublas::row_major> ArrayType;
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  using MatrixType = std::array<T, Rows * Cols>;
  using ArrayType = std::array<T, Rows * Cols>;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS

  /*! \brief Construct a identity matrix
   *
   * An identity matrix in this context is a matrix where m(i,i) = 1
   * and m(i,j) = 0, for i != j.
   */
  WGenericMatrix()
  {
    setToIdentity();
  }

  /*! \brief Copy Constructor
   */
  WGenericMatrix(const WGenericMatrix<T, Rows, Cols> &other): m_(other.m_) {}

  /*! \brief Constructs a matrix from an array of elements.
   *
   * The input array is assumed to be in row-major order. If elements is 0,
   * the matrix data is not initialized.
   */
  explicit WGenericMatrix(const T* elements)
  {
    if (elements) {
      for(unsigned int i = 0; i < Rows; ++i)
        for(unsigned int j = 0; j < Cols; ++j)
          at(i,j) = elements[i * Rows + j];
    }
  }

  /*! \brief Returns a const pointer to the internal data store.
   *
   * The array can be indexed with []. You can iterate over the
   * entire data store by using begin() and end() iterators. The
   * row order of the data is row major.
   */
  const ArrayType &constData() const {
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_.data();
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

  /*! \brief Export the matrix data
   *
   * Stores the matrix in an array of Rows*Cols elements of type T,
   * pointed to by data. The data will be stored in row major order.
   */
  void copyDataTo(T *data)
  {
    for(unsigned int i = 0; i < Rows; ++i)
      for (unsigned int j = 0; j < Cols; ++j)
        data[i * Rows + j] = at(i, j);
  }

  /*! \brief Returns a reference to the internal data store.
   *
   * The array can be indexed with []. You can iterate over the
   * entire data store by using begin() and end() iterators. The
   * row order of the data is row major.
   */
  ArrayType &data() {
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_.data();
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

  /*! \brief Returns a const reference to the internal data store.
   *
   * The array can be indexed with []. You can iterate over the
   * entire data store by using begin() and end() iterators. The
   * row order of the data is row major.
   */
  const ArrayType &data() const { 
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_.data(); 
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

  /*! \brief Fills every element of the matrix with the given value
   */
  void fill(T value)
  {
    for (unsigned i = 0; i < Rows; ++i)
      for (unsigned j = 0; j < Cols; ++j)
        at(i,j) = value;
  }

  /*! \brief Identity check.
   *
   * Returns true if the transform represents an identity transformation.
   */
  bool isIdentity() const
  {
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    using namespace boost::numeric::ublas;
    identity_matrix<T> I(Rows > Cols ? Rows : Cols);
    for(unsigned i = 0; i < Rows; ++i)
      for (unsigned j = 0; j < Cols; ++j)
        if (m_(i, j) != I(i, j))
          return false;
    return true;
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    for (std::size_t i = 0; i < Rows; ++i)
      for (std::size_t j = 0; j < Cols; ++j) {
        if (i == j) {
          if (at(i,j) != 1)
            return false;
        } else {
          if (at(i,j) != 0)
            return false;
        }
      }
    return true;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

  /*! \brief Set this matrix to the identity matrix
   *
   * An identity matrix is in this context a matrix where m(i,i) = 1
   * and m(i,j) = 0, for i != j.
   */
  void setToIdentity()
  {
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    using namespace boost::numeric::ublas;
    m_ = project(identity_matrix<T>(Rows > Cols ? Rows : Cols),
        range(0, Rows), range(0, Cols));
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    for (std::size_t i = 0; i < Rows; ++i)
      for (std::size_t j = 0; j < Cols; ++j) {
        at(i,j) = (i == j ? 1 : 0);
      }
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
  }

  /*! \brief Returns the transposed of the matrix
   */
  WGenericMatrix<T, Cols, Rows> transposed() const
  {
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return WGenericMatrix<T, Cols, Rows>(boost::numeric::ublas::trans(m_));
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    WGenericMatrix<T, Cols, Rows> result;
    for (std::size_t i = 0; i < Rows; ++i)
      for (std::size_t j = 0; j < Cols; ++j)
        result(j, i) = at(i, j);
    return result;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

  /*! \brief Equality operator.
   *
   * Returns \c true if the matrices are exactly the same.
   */
  bool operator==(const WGenericMatrix<T, Rows, Cols>& rhs) const
  {
    for(unsigned i = 0; i < Rows; ++i)
      for (unsigned j = 0; j < Cols; ++j)
        if (rhs(i, j) != at(i, j))
          return false;
    return true;
  }

  /*! \brief Inequality operator.
   *
   * Returns \c true if the transforms are different.
   */
  bool operator!=(const WGenericMatrix<T, Rows, Cols> &rhs) const {
    return !(*this == rhs);
  }

  /*! \brief Returns the element at the given position
   */
  const T &operator()(int row, int column) const
  {
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_(row, column);
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_[row * Cols + column];
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

  /*! \brief Returns the element at the given position
   */
  T &operator()(int row, int column) { 
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_(row, column);
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_[row * Cols + column];
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

  /*! \brief Returns the element at the given position
   */
  const T &at(int row, int column) const
  {
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_(row, column);
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_[row * Cols + column];
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

  /*! \brief Returns the element at the given position
   */
  T &at(int row, int column) {
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_(row, column);
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return m_[row * Cols + column];
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  }

#ifdef WT_TARGET_JAVA
  void setElement(int row, int column, float v);
#endif

  /*! \brief Multiply every element of the matrix with the given factor
   */
  WGenericMatrix<T, Rows, Cols> &operator*=(const T &factor)
  {
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    m_ *= factor;
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    for (std::size_t i = 0; i < Rows; ++i)
      for (std::size_t j = 0; j < Cols; ++j)
        at(i,j) = at(i,j) * factor;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return *this;
#endif
  }

  /*! \brief Divide every element of the matrix by the given factor
   */
  WGenericMatrix<T, Rows, Cols> &operator/=(const T &factor)
  {
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    m_ /= factor;
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    for (std::size_t i = 0; i < Rows; ++i)
      for (std::size_t j = 0; j < Cols; ++j)
        at(i,j) = at(i,j) / factor;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return *this;
#endif
  }

  /*! \brief Add the given matrix to this matrix
   */
  WGenericMatrix<T, Rows, Cols> &operator+=(
    const WGenericMatrix<T, Rows, Cols> &rhs)
  {
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    m_ += rhs.m_;
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    for (std::size_t i = 0; i < Rows; ++i)
      for (std::size_t j = 0; j < Cols; ++j)
        at(i,j) = at(i,j) + rhs(i,j);
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return *this;
#endif
  }

  /*! \brief Substract the given matrix from this matrix
   */
  WGenericMatrix<T, Rows, Cols> &operator-=(
    const WGenericMatrix<T, Rows, Cols> &rhs)
  {
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    m_ -= rhs.m_;
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    for (std::size_t i = 0; i < Rows; ++i)
      for (std::size_t j = 0; j < Cols; ++j)
        at(i,j) = at(i,j) - rhs(i,j);
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
    return *this;
#endif
  }

  MatrixType &impl() { return m_; }
  const MatrixType &impl() const { return m_; }
  WGenericMatrix(const MatrixType &m): m_(m) {}

private:
  MatrixType m_;
};

/*! \brief Multiply two matrices
 */
template<typename T, std::size_t A, std::size_t B, std::size_t C>
inline WGenericMatrix<T, A, C> operator*(const WGenericMatrix<T, A, B> &l,
  const WGenericMatrix<T, B, C> &r)
{
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  using namespace boost::numeric::ublas;
  return WGenericMatrix<T, A, C>(prod(l.impl(), r.impl()));
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  WGenericMatrix<T, A, C> result;
  for (std::size_t i = 0; i < A; ++i)
    for (std::size_t j = 0; j < C; ++j) {
      result(i,j) = 0;
      for (std::size_t k = 0; k < B; ++k) {
        result(i,j) += l(i,k) * r(k,j);
      }
    }
  return result;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
}

/*! \brief Print the matrix to an ostream
 */
template<typename T, std::size_t Rows, std::size_t Cols>
std::ostream &operator<<(std::ostream &os,
  const WGenericMatrix<T, Rows, Cols> &m)
{
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return os << m.impl();
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  WStringStream ss{os};
  ss << '[' << (long long)Rows << ',' << (long long)Cols << "](";
  for (std::size_t i = 0; i < Rows; ++i) {
    ss << '(';
    for (std::size_t j = 0; j < Cols; ++j) {
      if (j != 0)
        ss << ',';
      ss << m.at(i,j);
    }
    ss << ')';
  }
  ss << ')';
  return os;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
}

/*! \brief Multiply every element in the matrix with the given factor
 */
template<typename T, std::size_t Rows, std::size_t Cols>
inline WGenericMatrix<T, Rows, Cols> operator*(const T &factor,
  const WGenericMatrix<T, Rows, Cols> &m)
{
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(factor * m.impl());
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(m) *= factor;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
}

/*! \brief Multiply every element in the matrix with the given factor
 */
template<typename T, std::size_t Rows, std::size_t Cols>
inline WGenericMatrix<T, Rows, Cols> operator*(
  const WGenericMatrix<T, Rows, Cols> &m, const T &factor)
{
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(m.impl() * factor);
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(m) *= factor;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
}

/*! \brief Divide every element in the matrix by the given factor
 */
template<typename T, std::size_t Rows, std::size_t Cols>
inline WGenericMatrix<T, Rows, Cols> operator/(
  const WGenericMatrix<T, Rows, Cols> &m, const T &factor)
{
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(m.impl() / factor);
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(m) /= factor;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
}

/*! \brief Add two matrices together
 */
template<typename T, std::size_t Rows, std::size_t Cols>
inline WGenericMatrix<T, Rows, Cols> operator+(
  const WGenericMatrix<T, Rows, Cols> &l,
  const WGenericMatrix<T, Rows, Cols> &r)
{
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(l.impl() + r.impl());
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(l) += r;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
}

/*! \brief Substract two matrices
 */
template<typename T, std::size_t Rows, std::size_t Cols>
inline WGenericMatrix<T, Rows, Cols> operator-(
  const WGenericMatrix<T, Rows, Cols> &l,
  const WGenericMatrix<T, Rows, Cols> &r)
{
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(l.impl() - r.impl());
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(l) -= r;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
}

/*! \brief Negate every element in the matrix
 */
template<typename T, std::size_t Rows, std::size_t Cols>
inline WGenericMatrix<T, Rows, Cols> operator-(
  const WGenericMatrix<T, Rows, Cols> &m)
{
#ifndef WT_TARGET_JAVA
#ifdef WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(-m.impl());
#else // !WT_GENERIC_MATRIX_USE_BOOST_UBLAS
  return WGenericMatrix<T, Rows, Cols>(m) *= -1;
#endif // WT_GENERIC_MATRIX_USE_BOOST_UBLAS
#endif
}

}
#endif // WGENERICMATRIX_H_
