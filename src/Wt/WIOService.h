// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCORE_SERVER_H_
#define WCORE_SERVER_H_

#include <Wt/AsioWrapper/io_service.hpp>
#include <Wt/AsioWrapper/steady_timer.hpp>
#include <Wt/AsioWrapper/strand.hpp>
#include <Wt/AsioWrapper/system_error.hpp>

#include <Wt/WDllDefs.h>

#include <chrono>
#include <functional>
#include <memory>

namespace Wt {

class WIOServiceImpl;

/**! \class WIOService Wt/WIOService Wt/WIOService
 *   \brief An I/O service.
 *
 * An I/O service combines (boost::)asio::io_service with a thread pool.
 */
class WT_API WIOService : public AsioWrapper::asio::io_service
{
public:
  /*! \brief Creates a new IO service.
   *
   * \sa setServerConfiguration()
   */
  WIOService();
  virtual ~WIOService();

  /*! \brief Configures the number of threads.
   *
   * This must be configured before the server is started using start().
   *
   * The default thread count is 5 (or is configured by WServer from
   * information in the configuration file).
   */
  void setThreadCount(int number);

  /*! \brief Returns the thread count.
   */
  int threadCount() const;

  /*! \brief Starts the I/O service.
   *
   * This will start the internal thread pool to process work for
   * the I/O service, if not already started.
   */
  void start();

  /*! \brief Stops the I/O service.
   *
   * This will stop the internal thread pool. The method will block until
   * all work has been completed.
   */
  void stop();

  /*! \brief Posts a function into the thread-pool.
   *
   * The function will be executed within a thread of the thread-pool.
   *
   * This method returns immediately.
   */
  void post(const std::function<void ()>& function);

  /*! \brief Schedules a function on the thread-pool.
   *
   * The function will be executed after a time out, specified in
   * milli-seconds, on the thread pool.
   */
  void schedule(std::chrono::steady_clock::duration millis, const std::function<void()>& function);

  /*! \brief Initializes a thread.
   *
   * This function is called for every new thread created, and can be used
   * to configure the thread, e.g. with respect to scheduling priorities.
   */
  virtual void initializeThread();

  // returns false if blocking an extra thread would block the last thread
  // if true is returned, a counter that keeps track of the amount of threads
  // doing blocking operations is incremented
  // Typical use case example: recursive event loop
  bool requestBlockedThread();

  // decrement the blocked thread counter
  void releaseBlockedThread();

private:
  WIOServiceImpl *impl_;
  strand strand_;
  void handleTimeout(const std::shared_ptr<AsioWrapper::asio::steady_timer>& timer,
		     const std::function<void ()>& function,
		     const AsioWrapper::error_code& e);
  void run();
};

}

#endif // WCORE_SERVER_H_ 
