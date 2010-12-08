/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WRandom"

#if WIN32

// Don't link to boost_random
#define BOOST_RANDOM_NO_LIB

// And when compiling, don't add declspec to the classnames.
// We have our own implementation of random_device.cpp (older boosts
// didn't support windows well)
#ifdef BOOST_ALL_DYN_LINK
#undef BOOST_ALL_DYN_LINK
#endif
#ifdef BOOST_RANDOM_DYN_LINK
#undef BOOST_RANDOM_DYN_LINK
#endif

#endif

#include <boost/nondet_random.hpp>

#if defined(__linux__) || defined(__APPLE_CC__)
#define USE_NDT_RANDOM_DEVICE
#endif

#if defined(WIN32) || defined(__CYGWIN__)
#define USE_NDT_RANDOM_DEVICE
#include <process.h> // for getpid()
#include <stdlib.h>
#include <windows.h>
#endif

namespace {
  class RandomDevice
  {
  public:
#ifdef USE_NDT_RANDOM_DEVICE
    boost::random_device rnd;
#else
    RandomDevice()
    {
      srand48(getpid());
    }
#endif
  };

  RandomDevice instance;
}

namespace Wt {
  
unsigned int WRandom::get()
{
#ifdef USE_NDT_RANDOM_DEVICE
  return instance.rnd();
#else
  return lrand48();
#endif
}

std::string WRandom::generateId(int length)
{
  std::string result;

  for (int i = 0; i < length; ++i) {
    // use alphanumerical characters (big and small) and numbers
    int d = get() % (26 + 26 + 10);

    char c = (d < 10 ? ('0' + d)
	      : (d < 36 ? ('A' + d - 10)
		 : 'a' + d - 36));
    result.push_back(c);
  }

  return result;
}


}
