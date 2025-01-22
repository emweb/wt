/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Auth/AuthUtils.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE( create_salt_length_test )
{
  for (int i = 0; i < 3; ++i) {
    int size = 18 + i;
    std::string salt = Auth::Utils::createSalt(size);
    BOOST_CHECK_EQUAL(size, salt.size());
  }
}