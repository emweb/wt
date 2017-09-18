#include <boost/test/unit_test.hpp>

#include <Wt/WGenericMatrix.h>
#include <Wt/WMatrix4x4.h>

#include <cmath>

using namespace Wt;

BOOST_AUTO_TEST_CASE( matrix_test_identity )
{
  WGenericMatrix<double, 4, 4> m;
  BOOST_REQUIRE_EQUAL(m(0,0), 1);
  BOOST_REQUIRE_EQUAL(m(1,1), 1);
  BOOST_REQUIRE_EQUAL(m(2,2), 1);
  BOOST_REQUIRE_EQUAL(m(3,3), 1);

  BOOST_REQUIRE_EQUAL(m(0,1), 0);
  BOOST_REQUIRE_EQUAL(m(0,2), 0);
  BOOST_REQUIRE_EQUAL(m(0,3), 0);

  BOOST_REQUIRE_EQUAL(m(1,0), 0);
  BOOST_REQUIRE_EQUAL(m(1,2), 0);
  BOOST_REQUIRE_EQUAL(m(1,3), 0);

  BOOST_REQUIRE_EQUAL(m(2,0), 0);
  BOOST_REQUIRE_EQUAL(m(2,1), 0);
  BOOST_REQUIRE_EQUAL(m(2,3), 0);

  BOOST_REQUIRE_EQUAL(m(3,0), 0);
  BOOST_REQUIRE_EQUAL(m(3,1), 0);
  BOOST_REQUIRE_EQUAL(m(3,2), 0);

  BOOST_REQUIRE(m.isIdentity());

  std::cout << "Identity: " << m << std::endl;
}

BOOST_AUTO_TEST_CASE( matrix_test_transpose )
{
  WGenericMatrix<double, 4, 4> m1;
  m1(0,0) = 1;
  m1(0,1) = 2;
  m1(0,2) = 3;
  m1(0,3) = 4;

  m1(1,0) = 5;
  m1(1,1) = 6;
  m1(1,2) = 7;
  m1(1,3) = 8;

  m1(2,0) = 9;
  m1(2,1) = 10;
  m1(2,2) = 11;
  m1(2,3) = 12;

  m1(3,0) = 13;
  m1(3,1) = 14;
  m1(3,2) = 15;
  m1(3,3) = 16;

  auto m1t = m1.transposed();
  BOOST_REQUIRE_EQUAL(m1t(0,0), 1);
  BOOST_REQUIRE_EQUAL(m1t(0,1), 5);
  BOOST_REQUIRE_EQUAL(m1t(0,2), 9);
  BOOST_REQUIRE_EQUAL(m1t(0,3), 13);
  BOOST_REQUIRE_EQUAL(m1t(1,0), 2);
  BOOST_REQUIRE_EQUAL(m1t(1,1), 6);
  BOOST_REQUIRE_EQUAL(m1t(1,2), 10);
  BOOST_REQUIRE_EQUAL(m1t(1,3), 14);
  BOOST_REQUIRE_EQUAL(m1t(2,0), 3);
  BOOST_REQUIRE_EQUAL(m1t(2,1), 7);
  BOOST_REQUIRE_EQUAL(m1t(2,2), 11);
  BOOST_REQUIRE_EQUAL(m1t(2,3), 15);
  BOOST_REQUIRE_EQUAL(m1t(3,0), 4);
  BOOST_REQUIRE_EQUAL(m1t(3,1), 8);
  BOOST_REQUIRE_EQUAL(m1t(3,2), 12);
  BOOST_REQUIRE_EQUAL(m1t(3,3), 16);

  BOOST_REQUIRE_EQUAL(m1, m1t.transposed());
  BOOST_REQUIRE_EQUAL(m1t.transposed(), m1);

  std::cout << "Original: " << m1 << std::endl;
  std::cout << "Transposed: " << m1t << std::endl;
}

BOOST_AUTO_TEST_CASE( matrix_test_multiply_scalar )
{
  WGenericMatrix<double, 4, 4> m1;
  double s = 4;

  WGenericMatrix<double, 4, 4> m1m = m1 * s;
  BOOST_REQUIRE_EQUAL(m1m(0,0), 4);
  BOOST_REQUIRE_EQUAL(m1m(0,1), 0);
  BOOST_REQUIRE_EQUAL(m1m(0,2), 0);
  BOOST_REQUIRE_EQUAL(m1m(0,3), 0);
  BOOST_REQUIRE_EQUAL(m1m(1,0), 0);
  BOOST_REQUIRE_EQUAL(m1m(1,1), 4);
  BOOST_REQUIRE_EQUAL(m1m(1,2), 0);
  BOOST_REQUIRE_EQUAL(m1m(1,3), 0);
  BOOST_REQUIRE_EQUAL(m1m(2,0), 0);
  BOOST_REQUIRE_EQUAL(m1m(2,1), 0);
  BOOST_REQUIRE_EQUAL(m1m(2,2), 4);
  BOOST_REQUIRE_EQUAL(m1m(2,3), 0);
  BOOST_REQUIRE_EQUAL(m1m(3,0), 0);
  BOOST_REQUIRE_EQUAL(m1m(3,1), 0);
  BOOST_REQUIRE_EQUAL(m1m(3,2), 0);
  BOOST_REQUIRE_EQUAL(m1m(3,3), 4);

  std::cout << "Original: " << m1 << std::endl;
  std::cout << "Scalar: " << s << std::endl;
  std::cout << "Multiplied: " << m1m << std::endl;
}

BOOST_AUTO_TEST_CASE( matrix_test_multiply_matrix4x4_scalar )
{
  WMatrix4x4 m1;
  double s = 4;

  WMatrix4x4 m1m = m1 * s;
  BOOST_REQUIRE_EQUAL(m1m(0,0), 4);
  BOOST_REQUIRE_EQUAL(m1m(0,1), 0);
  BOOST_REQUIRE_EQUAL(m1m(0,2), 0);
  BOOST_REQUIRE_EQUAL(m1m(0,3), 0);
  BOOST_REQUIRE_EQUAL(m1m(1,0), 0);
  BOOST_REQUIRE_EQUAL(m1m(1,1), 4);
  BOOST_REQUIRE_EQUAL(m1m(1,2), 0);
  BOOST_REQUIRE_EQUAL(m1m(1,3), 0);
  BOOST_REQUIRE_EQUAL(m1m(2,0), 0);
  BOOST_REQUIRE_EQUAL(m1m(2,1), 0);
  BOOST_REQUIRE_EQUAL(m1m(2,2), 4);
  BOOST_REQUIRE_EQUAL(m1m(2,3), 0);
  BOOST_REQUIRE_EQUAL(m1m(3,0), 0);
  BOOST_REQUIRE_EQUAL(m1m(3,1), 0);
  BOOST_REQUIRE_EQUAL(m1m(3,2), 0);
  BOOST_REQUIRE_EQUAL(m1m(3,3), 4);

  std::cout << "Original: " << m1 << std::endl;
  std::cout << "Scalar: " << s << std::endl;
  std::cout << "Multiplied: " << m1m << std::endl;
}

BOOST_AUTO_TEST_CASE( matrix_test_multiply_matrix )
{
  WGenericMatrix<double, 4, 4> m1;
  m1(0,0) = 1;
  m1(0,1) = 2;
  m1(0,2) = 3;
  m1(0,3) = 4;

  m1(1,0) = 5;
  m1(1,1) = 6;
  m1(1,2) = 7;
  m1(1,3) = 8;

  m1(2,0) = 9;
  m1(2,1) = 10;
  m1(2,2) = 11;
  m1(2,3) = 12;

  m1(3,0) = 13;
  m1(3,1) = 14;
  m1(3,2) = 15;
  m1(3,3) = 16;

  WGenericMatrix<double, 4, 4> m2;
  m2(0,0) = 1;
  m2(0,1) = -2;
  m2(0,2) = 3;
  m2(0,3) = -4;

  m2(1,0) = 5;
  m2(1,1) = -6;
  m2(1,2) = 7;
  m2(1,3) = -8;

  m2(2,0) = 9;
  m2(2,1) = -10;
  m2(2,2) = 11;
  m2(2,3) = -12;

  m2(3,0) = 13;
  m2(3,1) = -14;
  m2(3,2) = 15;
  m2(3,3) = -16;

  WGenericMatrix<double, 4, 4> m3 = m1 * m2;

  BOOST_REQUIRE_EQUAL(m3(0,0), 90);
  BOOST_REQUIRE_EQUAL(m3(0,1), -100);
  BOOST_REQUIRE_EQUAL(m3(0,2), 110);
  BOOST_REQUIRE_EQUAL(m3(0,3), -120);

  BOOST_REQUIRE_EQUAL(m3(1,0), 202);
  BOOST_REQUIRE_EQUAL(m3(1,1), -228);
  BOOST_REQUIRE_EQUAL(m3(1,2), 254);
  BOOST_REQUIRE_EQUAL(m3(1,3), -280);

  BOOST_REQUIRE_EQUAL(m3(2,0), 314);
  BOOST_REQUIRE_EQUAL(m3(2,1), -356);
  BOOST_REQUIRE_EQUAL(m3(2,2), 398);
  BOOST_REQUIRE_EQUAL(m3(2,3), -440);

  BOOST_REQUIRE_EQUAL(m3(3,0), 426);
  BOOST_REQUIRE_EQUAL(m3(3,1), -484);
  BOOST_REQUIRE_EQUAL(m3(3,2), 542);
  BOOST_REQUIRE_EQUAL(m3(3,3), -600);

  std::cout << "M1: " << m1 << std::endl;
  std::cout << "M2: " << m2 << std::endl;
  std::cout << "M3 = M1 * M2: " << m3 << std::endl;
}

BOOST_AUTO_TEST_CASE( matrix_test_multiply_matrix4x4 )
{
  WMatrix4x4 m1;
  m1(0,0) = 1;
  m1(0,1) = 2;
  m1(0,2) = 3;
  m1(0,3) = 4;

  m1(1,0) = 5;
  m1(1,1) = 6;
  m1(1,2) = 7;
  m1(1,3) = 8;

  m1(2,0) = 9;
  m1(2,1) = 10;
  m1(2,2) = 11;
  m1(2,3) = 12;

  m1(3,0) = 13;
  m1(3,1) = 14;
  m1(3,2) = 15;
  m1(3,3) = 16;

  WMatrix4x4 m2;
  m2(0,0) = 1;
  m2(0,1) = -2;
  m2(0,2) = 3;
  m2(0,3) = -4;

  m2(1,0) = 5;
  m2(1,1) = -6;
  m2(1,2) = 7;
  m2(1,3) = -8;

  m2(2,0) = 9;
  m2(2,1) = -10;
  m2(2,2) = 11;
  m2(2,3) = -12;

  m2(3,0) = 13;
  m2(3,1) = -14;
  m2(3,2) = 15;
  m2(3,3) = -16;

  WMatrix4x4 m3 = m1 * m2;

  BOOST_REQUIRE_EQUAL(m3(0,0), 90);
  BOOST_REQUIRE_EQUAL(m3(0,1), -100);
  BOOST_REQUIRE_EQUAL(m3(0,2), 110);
  BOOST_REQUIRE_EQUAL(m3(0,3), -120);

  BOOST_REQUIRE_EQUAL(m3(1,0), 202);
  BOOST_REQUIRE_EQUAL(m3(1,1), -228);
  BOOST_REQUIRE_EQUAL(m3(1,2), 254);
  BOOST_REQUIRE_EQUAL(m3(1,3), -280);

  BOOST_REQUIRE_EQUAL(m3(2,0), 314);
  BOOST_REQUIRE_EQUAL(m3(2,1), -356);
  BOOST_REQUIRE_EQUAL(m3(2,2), 398);
  BOOST_REQUIRE_EQUAL(m3(2,3), -440);

  BOOST_REQUIRE_EQUAL(m3(3,0), 426);
  BOOST_REQUIRE_EQUAL(m3(3,1), -484);
  BOOST_REQUIRE_EQUAL(m3(3,2), 542);
  BOOST_REQUIRE_EQUAL(m3(3,3), -600);

  std::cout << "M1: " << m1 << std::endl;
  std::cout << "M2: " << m2 << std::endl;
  std::cout << "M3 = M1 * M2: " << m3 << std::endl;
}

void check_matrix_close(const WMatrix4x4 &m1,
                        const WMatrix4x4 &m2,
                        double epsilon)
{
  for (std::size_t i = 0; i < 4; ++i)
    for (std::size_t j = 0; j < 4; ++j)
      BOOST_REQUIRE(std::fabs(m1(i,j) - m2(i,j)) < epsilon);
}

BOOST_AUTO_TEST_CASE( matrix_test_inverse )
{
  WMatrix4x4 m1;
  m1.rotate(30, 1, 0, 0);
  m1.rotate(40, 0, 1, 0);
  m1.rotate(50, 0, 0, 1);

  WMatrix4x4 m2;
  m2.rotate(-50, 0, 0, 1);
  m2.rotate(-40, 0, 1, 0);
  m2.rotate(-30, 1, 0, 0);

  bool invertible = false;
  WMatrix4x4 m1i = m1.inverted(&invertible);

  BOOST_REQUIRE(invertible);
  const double EPSILON = 1E-10;
  check_matrix_close(m1.inverted(), m1i, EPSILON);
  check_matrix_close(m1i.inverted(), m1, EPSILON);
  check_matrix_close(m1i, m2, EPSILON);
  check_matrix_close(m1 * m1i, WMatrix4x4{}, EPSILON);

  std::cout << "Original: " << m1 << std::endl;
  std::cout << "Inverse: " << m1i << std::endl;
  std::cout << "Compare: " << m2 << std::endl;
}

BOOST_AUTO_TEST_CASE( matrix_test_det )
{
  WMatrix4x4 m1;
  m1.rotate(30, 1, 0, 0);
  m1.rotate(40, 0, 1, 0);
  m1.rotate(50, 0, 0, 1);

  const double EPSILON = 1E-10;
  BOOST_REQUIRE(std::fabs(m1.determinant() - 1.0) < EPSILON);

  m1.scale(2.4);

  BOOST_REQUIRE(std::fabs(m1.determinant() - (2.4*2.4*2.4)) < EPSILON);
}
