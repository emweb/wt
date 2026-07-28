// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WObject.h>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

namespace {
  class MyObject : public Wt::WObject
  {
  public:
    MyObject()
      : counter(0)
    { }

    void foo() {
      counter += 1;
    }

    std::atomic<int> counter;
  };
}

BOOST_AUTO_TEST_CASE( concurrent_bind_calls )
{
  std::unique_ptr<MyObject> o(new MyObject);

  unsigned nthreads = std::thread::hardware_concurrency();
  if (nthreads == 0) {
    nthreads = 4;
  }

  constexpr int callsPerThread = 10000;

  std::vector<std::thread> threads;
  threads.reserve(nthreads);

  for (unsigned t = 0; t < nthreads; ++t) {
    threads.emplace_back([&o, &callsPerThread]() {
      for (int i = 0; i < callsPerThread; ++i) {
        auto bound = o->bindSafe(&MyObject::foo);
        bound();
      }
    });
  }

  for (auto &th : threads) {
    th.join();
  }

  BOOST_TEST(o->counter == static_cast<int>(nthreads * callsPerThread));
}
