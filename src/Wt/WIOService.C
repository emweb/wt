/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WIOService"
#include "Wt/WLogger"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#ifdef WT_THREADED
#if !defined(_WIN32)
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif // !_WIN32
#endif // WT_THREADED

namespace Wt {

LOGGER("WIOService");

WIOService::WIOService()
  : threadCount_(5),
    work_(0)
#ifdef WT_THREADED
    , blockedThreadCounter_(0)
#endif
{ }

WIOService::~WIOService()
{
  stop();
}

void WIOService::setThreadCount(int count)
{ 
  threadCount_ = count;
}

int WIOService::threadCount() const
{
  return threadCount_;
}

void WIOService::start()
{
  if (!work_) {
    work_ = new boost::asio::io_service::work(*this);

#ifdef WT_THREADED

#if !defined(_WIN32)
    // Block all signals for background threads.
    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif // _WIN32

    for (int i = 0; i < threadCount_; ++i) {
      threads_.push_back
	(new boost::thread(boost::bind(&WIOService::run, this)));
    }

#if !defined(_WIN32)
    // Restore previous signals.
    pthread_sigmask(SIG_SETMASK, &old_mask, 0);
#endif // _WIN32

#else // !WT_THREADED

    run();

#endif // WT_THREADED

  }
}

void WIOService::stop()
{
  delete work_;
  work_ = 0;

#ifdef WT_THREADED
  for (unsigned i = 0; i < threads_.size(); ++i) {
    threads_[i]->join();
    delete threads_[i];
  }

  threads_.clear();
#endif // WT_THREADED

  reset();
}

void WIOService::post(const boost::function<void ()>& function)
{
  schedule(0, function);
}

void WIOService::schedule(int millis, const boost::function<void()>& function)
{
  if (millis == 0)
    boost::asio::io_service::post(function);
  else {
    boost::asio::deadline_timer *timer = new boost::asio::deadline_timer(*this);
    timer->expires_from_now(boost::posix_time::milliseconds(millis));
    timer->async_wait
      (boost::bind(&WIOService::handleTimeout, this, timer, function,
		   boost::asio::placeholders::error));
  }
}

void WIOService::handleTimeout(boost::asio::deadline_timer *timer,
			       const boost::function<void ()>& function,
			       const boost::system::error_code& e)
{
  if (!e)
    function();

  delete timer;
}

void WIOService::initializeThread()
{ }

bool WIOService::requestBlockedThread()
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock l(blockedThreadMutex_);
  if (blockedThreadCounter_ >= threadCount() - 1)
    return false;
  else {
    blockedThreadCounter_++;
    return true;
  }
#else
  return false;
#endif
}

void WIOService::releaseBlockedThread()
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock l(blockedThreadMutex_);
  if (blockedThreadCounter_ > 0)
    blockedThreadCounter_--;
  else
    LOG_ERROR("releaseBlockedThread: oops!");
#endif
}

void WIOService::run()
{
  initializeThread();
  boost::asio::io_service::run();
}

}
