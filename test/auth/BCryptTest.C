/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/unit_test.hpp>

#include <Wt/WRandom>
#include <Wt/Auth/HashFunction>

using namespace Wt;

BOOST_AUTO_TEST_CASE( bcrypt_test )
{
  Auth::BCryptHashFunction f(7);

  std::string msg = "secret";
  std::string salt = WRandom::generateId();

  std::string hash = f.compute(msg, salt);

  std::cerr << "bcrypted password: " << hash << std::endl;

  boost::posix_time::ptime
    start = boost::posix_time::microsec_clock::local_time();

  BOOST_REQUIRE(f.verify(msg, salt, hash));

  boost::posix_time::ptime
    end = boost::posix_time::microsec_clock::local_time();

  boost::posix_time::time_duration d = end - start;

  std::cerr << "verify() took: " << (double)d.total_microseconds() / 1000
	    << "ms" << std::endl;
}
