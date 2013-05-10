#include <boost/test/unit_test.hpp>
#include <Wt/Render/Specificity.h>

using namespace Wt::Render;

bool operator<(const Specificity& a, const Specificity& b)
{
  return a.isSmallerThen(b);
}

bool operator>=(const Specificity& a, const Specificity& b)
{
  return a.isGreaterOrEqualThen(b);
}

bool operator>(const Specificity& a, const Specificity& b)
{
  return a.isGreaterThen(b);
}

bool operator<=(const Specificity& a, const Specificity& b)
{
  return a.isSmallerOrEqualThen(b);
}


BOOST_AUTO_TEST_CASE( Specificity_test )
{
  BOOST_REQUIRE(Specificity(0,0,0,2) < Specificity(0,0,0,3));
  BOOST_REQUIRE(Specificity(false)   < Specificity(0,0,0,0));
  BOOST_REQUIRE(Specificity(0,0,0,0) < Specificity(0,0,0,2));
  BOOST_REQUIRE(Specificity(0,0,0,10) < Specificity(0,0,1,0));
  BOOST_REQUIRE(Specificity(0,0,10,0) < Specificity(0,1,0,0));
  BOOST_REQUIRE(Specificity(0,10,0,0) < Specificity(1,0,0,0));

  BOOST_REQUIRE(Specificity(0,0,0,3) > Specificity(0,0,0,2));
  BOOST_REQUIRE(Specificity(0,0,0,0) > Specificity(false));

  BOOST_REQUIRE(Specificity(0,0,0,3) >=Specificity(0,0,0,2));
  BOOST_REQUIRE(Specificity(0,0,0,2) <=Specificity(0,0,0,3));


}
