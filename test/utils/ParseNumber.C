/*
 * Copyright (C) 2018 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/WDllDefs.h"
#include "web/WebUtils.h"

BOOST_AUTO_TEST_CASE( ParseNumber_test1 )
{
  BOOST_CHECK_EQUAL(Wt::Utils::stol("2147483647"), 2147483647L);
  BOOST_CHECK_EQUAL(Wt::Utils::stol("2147483647"), 2147483647L);
  BOOST_CHECK_EQUAL(Wt::Utils::stoul("4294967295"), 4294967295UL);
  BOOST_CHECK_EQUAL(Wt::Utils::stoll("9223372036854775807"), 9223372036854775807LL);
  BOOST_CHECK_EQUAL(Wt::Utils::stoull("18446744073709551615"), 18446744073709551615ULL);
  BOOST_CHECK_EQUAL(Wt::Utils::stoi("2147483647"), 2147483647);
  BOOST_CHECK_EQUAL(Wt::Utils::stod("0.25"), 0.25);
  BOOST_CHECK_EQUAL(Wt::Utils::stof("0.125"), 0.125f);
  BOOST_CHECK_EQUAL(Wt::Utils::stod(" 0.875"), 0.875);
  BOOST_CHECK_EQUAL(Wt::Utils::stod(" 0.875 "), 0.875);
  BOOST_CHECK_EQUAL(Wt::Utils::stod("0.875 "), 0.875);
}

BOOST_AUTO_TEST_CASE( ParseNumber_test2 )
{
  // Test if Wt::Utils::sto* is locale-agnostic
  try {
    std::locale::global(std::locale("nl_BE.utf8"));
  } catch (const std::runtime_error &) {
    return; // skip test
  }

  try {
    BOOST_CHECK_THROW(Wt::Utils::stod("0,25"), std::invalid_argument);
    BOOST_CHECK_THROW(Wt::Utils::stof("0,125"), std::invalid_argument);

    BOOST_CHECK_EQUAL(Wt::Utils::stol("2147483647"), 2147483647L);
    BOOST_CHECK_EQUAL(Wt::Utils::stoul("4294967295"), 4294967295UL);
    BOOST_CHECK_EQUAL(Wt::Utils::stoll("9223372036854775807"), 9223372036854775807LL);
    BOOST_CHECK_EQUAL(Wt::Utils::stoull("18446744073709551615"), 18446744073709551615ULL);
    BOOST_CHECK_EQUAL(Wt::Utils::stoi("2147483647"), 2147483647);
    BOOST_CHECK_EQUAL(Wt::Utils::stod("0.25"), 0.25);
    BOOST_CHECK_EQUAL(Wt::Utils::stof("0.125"), 0.125f);
  } catch (const std::invalid_argument &) {
    // reset locale
    std::locale::global(std::locale::classic());
    throw;
  }

  // reset locale
  std::locale::global(std::locale::classic());
}
