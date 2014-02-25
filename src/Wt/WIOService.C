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
#include <boost/thread.hpp>
#if !defined(WT_WIN32)
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif // !_WIN32
#endif // WT_THREADED

namespace Wt {

LOGGER("WIOService");

class WIOServiceImpl {
public:
  WIOServiceImpl()
  : threadCount_(5),
    work_(0)
#ifdef WT_THREADED
    , blockedThreadCounter_(0)
#endif
  {
  }
  int threadCount_;
  boost::asio::io_service::work *work_;

#ifdef WT_THREADED
  boost::mutex blockedThreadMutex_;
  int blockedThreadCounter_;
#endif

  std::vector<boost::thread *> threads_;

};

WIOService::WIOService()
  : impl_(new WIOServiceImpl())
{ }

WIOService::~WIOService()
{
  stop();
  delete impl_;
}

void WIOService::setThreadCount(int count)
{ 
  impl_->threadCount_ = count;
}

int WIOService::threadCount() const
{
  return impl_->threadCount_;
}

void WIOService::start()
{
  if (!impl_->work_) {
    impl_->work_ = new boost::asio::io_service::work(*this);

#ifdef WT_THREADED

#if !defined(WT_WIN32)
    // Block all signals for background threads.
    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif // _WIN32

    for (int i = 0; i < impl_->threadCount_; ++i) {
      impl_->threads_.push_back
	(new boost::thread(boost::bind(&WIOService::run, this)));
    }

#if !defined(WT_WIN32)
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
  delete impl_->work_;
  impl_->work_ = 0;

#ifdef WT_THREADED
  for (unsigned i = 0; i < impl_->threads_.size(); ++i) {
    impl_->threads_[i]->join();
    delete impl_->threads_[i];
  }

  impl_->threads_.clear();
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
  boost::mutex::scoped_lock l(impl_->blockedThreadMutex_);
  if (impl_->blockedThreadCounter_ >= threadCount() - 1)
    return false;
  else {
    impl_->blockedThreadCounter_++;
    return true;
  }
#else
  return false;
#endif
}

void WIOService::releaseBlockedThread()
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock l(impl_->blockedThreadMutex_);
  if (impl_->blockedThreadCounter_ > 0)
    impl_->blockedThreadCounter_--;
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
