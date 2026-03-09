/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SELENIUMFIXTURE_H_
#define SELENIUMFIXTURE_H_

#include "SeleniumServer.h"

#include "Wt/WApplication.h"

#include <condition_variable>
#include <mutex>
#include <thread>

namespace Selenium
{
  /*! \class SeleniumFixture "test/selenium/framework/SeleniumFixture.h"
   *  \brief The fixture used by all Selenium tests, wrapped by SeleniumTest.
   *
   *  The fixture is responsible for launching and stopping the WServer on
   *  a separate thread.
   */
  class SeleniumFixture
  {
  public:
    using AppCreator = std::function<std::unique_ptr<Wt::WApplication>(const Wt::WEnvironment&)>;

    SeleniumFixture(const std::string& docroot = ".")
      : server_(std::make_unique<SeleniumServer>(docroot)),
      serverThread_(nullptr),
      serverStarted_(false)
    {}

    ~SeleniumFixture()
    {
      stopServer();
    }

    //! Registers an application entrypoint
    void setAppCreator(AppCreator creator)
    {
      server_->addEntryPoint(Wt::EntryPointType::Application, creator);
    }

    /*! \brief Starts the WServer, which is running in another thread.
     *
     * Once the server has started startCondition_ is notified.
     * If the server is stopped, the stopCondition_ is notified,
     * and will ensure the thread is joined again.
     */
    bool startServer()
    {
      if (serverStarted_) {
        return true;
      }

      std::unique_lock<std::mutex> lock(mutex_);

      serverThread_ = std::make_unique<std::thread>([this]() {
        bool started = server_->start();
        serverStarted_ = started;
        startCondition_.notify_one();

        if (started) {
          std::unique_lock<std::mutex> lock(mutex_);
          stopCondition_.wait(lock);
        }
      });

      startCondition_.wait(lock, [this]() { return serverStarted_.load(); });
      return serverStarted_;
    }

    /*! \brief Stops the server.
     *
     * This stops the server instance, and joins the thread after it has been
     * notified of its server shutting down.
     */
    void stopServer()
    {
      if (serverThread_ && serverThread_->joinable()) {
        server_->stop();
        stopCondition_.notify_one();
        serverThread_->join();
      }
    }

    //! Returns the URL of the server (with the relevant port).
    std::string url() const
    {
      return server_->url();
    }

    //! Returns the instance of the SeleniumServer.
    SeleniumServer& server() { return *server_; }

  private:
    std::unique_ptr<SeleniumServer> server_;
    std::unique_ptr<std::thread> serverThread_;

    std::atomic_bool serverStarted_;
    std::mutex mutex_;
    std::condition_variable startCondition_;
    std::condition_variable stopCondition_;
  };
}

#endif // SELENIUMFIXTURE_H_
