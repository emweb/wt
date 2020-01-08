/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Auth/HashFunction.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE( sha1_test )
{
  Auth::SHA1HashFunction f;

  std::string msg = "abcdbcdecdefdefgefghfghighij";
  std::string salt = "salt";

  std::string hash = f.compute(msg, salt);

  BOOST_REQUIRE(hash == "6k2mrY3gUBcF62AUhu0TGDshjrM=");
}
