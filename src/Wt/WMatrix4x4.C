// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WMatrix4x4"
#include <boost/numeric/ublas/lu.hpp>


using namespace Wt;

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

