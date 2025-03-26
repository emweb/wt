/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WRandom.h>
#include <Wt/Auth/HashFunction.h>

#include <chrono>
#include <iostream>

using namespace Wt;

// Requirement: security wt-auth-bcrypt-password-10-iteration
BOOST_AUTO_TEST_CASE( bcrypt_test )
{
  for (int i = 0; i < 11; i++) {
    int numIt = 5+i;
    Auth::BCryptHashFunction f(numIt);

    std::string msg = "secret";
    std::string salt = WRandom::generateId();

    std::string hash = f.compute(msg, salt);

    std::cerr << "bcrypted password: " << hash << std::endl;

    std::chrono::system_clock::time_point
      start = std::chrono::system_clock::now();

    BOOST_REQUIRE(f.verify(msg, salt, hash));

    std::chrono::system_clock::time_point
      end = std::chrono::system_clock::now();

    double ms = (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000;

    std::cerr << "with " << std::to_string(numIt)
              <<" iterations, verify() took: " << ms
              << "ms" << std::endl;
  }
}
