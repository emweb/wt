/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WGlobal.h>
#include <memory>
#include <stdexcept>
#include <sys/types.h>
#include <random>

#ifdef WT_THREADED
#include <thread>
#include <mutex>
#endif

#include "Wt/WRandom.h"

#include "Wt/WDllDefs.h"

namespace {
  class RandomDevice
  {
  public:
    std::random_device rnd;
    RandomDevice()
    {
    }
  };

#ifdef WT_THREADED
  std::mutex randomInstanceMutex;
#endif // WT_THREADED

  std::unique_ptr<RandomDevice> instance;
}

namespace Wt {
  
unsigned int WRandom::get()
{
  if (!instance) {
#ifdef WT_THREADED
    std::unique_lock<std::mutex> l(randomInstanceMutex);
    if (!instance)
      instance.reset(new RandomDevice);
#else
    instance.reset(new RandomDevice);
#endif
  }
  try {
    return instance->rnd();
  } catch (std::invalid_argument &e) {
    // Some people fork and reported that random_device does stop working
    // after the fork.
#ifdef WT_THREADED
    std::unique_lock<std::mutex> l(randomInstanceMutex);
#endif
    instance.reset(new RandomDevice);
    // If this still fails, something is really wrong.
    return instance->rnd();
  }
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
