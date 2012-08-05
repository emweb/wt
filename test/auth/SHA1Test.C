/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/unit_test.hpp>

#include <Wt/Auth/HashFunction>

using namespace Wt;

BOOST_AUTO_TEST_CASE( sha1_test )
{
  Auth::SHA1HashFunction f;

  std::string msg = "abcdbcdecdefdefgefghfghighij";
  std::string salt = "salt";

  std::string hash = f.compute(msg, salt);

  BOOST_REQUIRE(hash == "6k2mrY3gUBcF62AUhu0TGDshjrM=");
}
