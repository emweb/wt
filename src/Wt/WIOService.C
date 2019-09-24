/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WIOService.h"
#include "Wt/WLogger.h"

#ifdef WT_THREADED
#include <thread>
#include <mutex>
#if !defined(WT_WIN32)
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif // !_WIN32
#endif // WT_THREADED

namespace Wt {

  namespace asio = AsioWrapper::asio;

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
  asio::io_service::work *work_;

#ifdef WT_THREADED
  std::mutex blockedThreadMutex_;
  int blockedThreadCounter_;

  std::vector<std::unique_ptr<std::thread>> threads_;
#endif

};

WIOService::WIOService()
  : impl_(new WIOServiceImpl()),
    strand_(*this)
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
    impl_->work_ = new asio::io_service::work(*this);

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
	(std::unique_ptr<std::thread>
	 (new std::thread(std::bind(&WIOService::run, this))));
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
  for (unsigned i = 0; i < impl_->threads_.size(); ++i)
    impl_->threads_[i]->join();

  impl_->threads_.clear();
#endif // WT_THREADED

  reset();
}

void WIOService::post(const std::function<void ()>& function)
{
  schedule(std::chrono::milliseconds{0}, function);
}

void WIOService::schedule(std::chrono::steady_clock::duration millis, const std::function<void()>& function)
{
  if (millis.count() == 0)
    strand_.post(function); // guarantees execution order
  else {
    std::shared_ptr<asio::steady_timer> timer = std::make_shared<asio::steady_timer>(*this);
    timer->expires_from_now(millis);
    timer->async_wait
      (std::bind(&WIOService::handleTimeout, this, timer, function,
		 std::placeholders::_1));
  }
}

void WIOService::handleTimeout(const std::shared_ptr<asio::steady_timer>& timer,
			       const std::function<void ()>& function,
			       const AsioWrapper::error_code& e)
{
  if (!e)
    function();

  (void)timer;
}

void WIOService::initializeThread()
{ }

bool WIOService::requestBlockedThread()
{
#ifdef WT_THREADED
  std::unique_lock<std::mutex> l(impl_->blockedThreadMutex_);
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
  std::unique_lock<std::mutex> l(impl_->blockedThreadMutex_);
  if (impl_->blockedThreadCounter_ > 0)
    impl_->blockedThreadCounter_--;
  else
    LOG_ERROR("releaseBlockedThread: oops!");
#endif
}

void WIOService::run()
{
  initializeThread();
  asio::io_service::run();
}

}
