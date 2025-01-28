/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <Wt/WLogger.h>
#ifndef WT_WIN32
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#endif // WT_WIN32

#include <chrono>
#include <thread>

using namespace Wt;

void startLogTask(int threadNumber, int numberOfLogs)
{
  for (int i = 0; i < numberOfLogs; ++i) {
    log("info")<<"Testlog of T: " << threadNumber << " -----------------------------------------";
  }
}

BOOST_AUTO_TEST_CASE( single_process_concurent_log_with_lock )
{
  logInstance().setUseLock(true);
  constexpr int numLog = 100000;
  constexpr int numThreads = 4;
  std::thread threads[numThreads];

  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  
  for (int i = 0; i < numThreads; ++i) {
    threads[i] = std::thread(&startLogTask, i+1, numLog);
  }
  for (int i = 0; i < numThreads; ++i) {
    threads[i].join();
  }

  std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

  double ms = (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000;

  std::cerr << "logging " << std::to_string(numLog)
            <<" messages per thread, for "<< numThreads
            << " threads took " << ms
            << "ms in total." << std::endl;
}

BOOST_AUTO_TEST_CASE( single_process_concurent_log_no_lock )
{
  logInstance().setUseLock(false);
  constexpr int numLog = 100000;
  constexpr int numThreads = 4;
  std::thread threads[numThreads];

  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  
  for (int i = 0; i < numThreads; ++i) {
    threads[i] = std::thread(&startLogTask, i+1, numLog);
  }
  for (int i = 0; i < numThreads; ++i) {
    threads[i].join();
  }

  std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

  double ms = (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000;

  std::cerr << "logging " << std::to_string(numLog)
            <<" messages per thread, for "<< numThreads
            << " threads took " << ms
            << "ms in total." << std::endl;
}