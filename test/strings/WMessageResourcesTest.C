/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WMessageResources.h>
#include <Wt/Exception/WInvalidFormatException.h>
#include <Wt/Exception/WInvalidOperationException.h>


BOOST_AUTO_TEST_CASE(test_WMessageResources_evalPluralCase_overflow_division)
{
  std::string msg = "9088*272*3088*768*36*576*1088*36352*8*752*32*32/(1-2)";
  BOOST_CHECK_EQUAL(Wt::WMessageResources::evalPluralCase(msg, msg.size()), 0);
}

BOOST_AUTO_TEST_CASE(test_WMessageResources_evalPluralCase_overflow_modulo)
{
  std::string msg = "9088*272*3088*768*36*576*1088*36352*8*752*32*32%(1-2)";
  BOOST_CHECK_EQUAL(Wt::WMessageResources::evalPluralCase(msg, msg.size()), 0);
}

BOOST_AUTO_TEST_CASE(test_WMessageResources_evalPluralCase_divide_by_zero)
{
  std::string msg = "9088*272*3088*768*36*576*1088*36352*8*752*32*32/(1-1)";
  BOOST_CHECK_THROW(Wt::WMessageResources::evalPluralCase(msg, msg.size()), Wt::WInvalidOperationException);
}

BOOST_AUTO_TEST_CASE(test_WMessageResources_evalPluralCase_modulo_by_zero)
{
  std::string msg = "9088*272*3088*768*36*576*1088*36352*8*752*32*32%(1-1)";
  BOOST_CHECK_THROW(Wt::WMessageResources::evalPluralCase(msg, msg.size()), Wt::WInvalidOperationException);
}

BOOST_AUTO_TEST_CASE(test_WMessageResources_evalPluralCase_number_overflow)
{
  //This number is larger than the maximum value of int64_t.
  std::string msg = "111111111111111111111111111111111111111/1";
  BOOST_CHECK_THROW(Wt::WMessageResources::evalPluralCase(msg, msg.size()), Wt::WInvalidFormatException);
}