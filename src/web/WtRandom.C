#include "WtRandom.h"

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

#ifdef __linux__
#if 0
/*
 * We get errors from the destructor...
 */
#define __use_random_device__
#endif
#endif
#if defined(WIN32) || defined(__CYGWIN__)
#define __use_random_device__
#include <process.h> // for getpid()
#include <stdlib.h>
#include <windows.h>
#endif

namespace{
  // Random number generator instantiation for the static methods of WtRandom
  Wt::WtRandom wtRandom;
}

namespace Wt {

class WtRandom::Private
{
public:
#ifdef __use_random_device__
  boost::random_device rnd_;
#endif
};

WtRandom::WtRandom():
_p(new WtRandom::Private)
{ 
#ifndef __use_random_device__
  srand48(getpid());
#endif // __use_random_device__
}

WtRandom::~WtRandom()
{
  delete _p;
}

unsigned int WtRandom::rand()
{
#ifdef __use_random_device__
  return _p->rnd_();
#else
  return lrand48();
#endif // __use_random_device__
}

unsigned int WtRandom::getUnsigned()
{
  return wtRandom.rand();
}

double WtRandom::getDouble()
{
  return ((double)wtRandom.rand())/(RAND_MAX);
}

}
