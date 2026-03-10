/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SELENIUMTEST_H_
#define SELENIUMTEST_H_

#include "SeleniumAPI.h"
#include "SeleniumFixture.h"
#include "SeleniumWait.h"

#include <boost/test/unit_test.hpp>

namespace Selenium {
  // Helper macro to define a Selenium test case with automatic setup
#define SELENIUM_TEST(test_name, app_type) \
    BOOST_AUTO_TEST_CASE(test_name) \
    { \
      Selenium::SeleniumAPI::Browser type = Selenium::SeleniumAPI::Browser::Chrome; \
      std::string driverPath; \
      int argc = boost::unit_test::framework::master_test_suite().argc; \
      char** argv = boost::unit_test::framework::master_test_suite().argv; \
      if (argc >= 3) { \
        if (strcmp(argv[1], "--browser-type") == 0) { \
          if (strcmp(argv[2], "firefox") == 0) { \
            type = Selenium::SeleniumAPI::Browser::Firefox; \
          } else if (strcmp(argv[2], "chrome") == 0) { \
            type = Selenium::SeleniumAPI::Browser::Chrome; \
          } \
        } \
      } \
      if (argc == 5) { \
        if (strcmp(argv[3], "--browser-driver") == 0) { \
          driverPath = std::string(argv[4]); \
        } \
      } \
      Selenium::SeleniumTest<app_type> test(type, driverPath); \
      BOOST_REQUIRE(test.startServer()); \
      auto& api = test.api(); \
      Selenium::SeleniumWait wait(api.driver(), std::chrono::seconds(10)); \
      auto updateApplication = [&](const std::function<void(app_type*)>& fn) { \
        test.updateApplication(fn); \
      }; \
      \
      (void)api; \
      (void)wait; \
      (void)updateApplication;

#define END_SELENIUM_TEST \
    }

  /*! \class SeleniumTest "test/selenium/framework/SeleniumTest.h"
   *  \brief Wrapper around the SeleniumFixture, used in SELENIUM_TEST.
   *
   * This allows for a WApplication instance to be created, and
   * a server to be started inside a test case upon initialization.
   */
  template<typename AppType>
  class SeleniumTest
  {
  public:
    SeleniumTest(SeleniumAPI::Browser browser,
                 const std::string& driverPath,
                 const std::string& docroot = ".")
      : fixture_(docroot),
        driverPath_(driverPath),
        browser_(browser)
    {
      fixture_.setAppCreator([](const Wt::WEnvironment& env) {
        auto app = std::make_unique<AppType>(env);
        app->enableUpdates(true);
        return app;
      });
    }

    //! Starts the server in the SeleniumFixture.
    bool startServer()
    {
      return fixture_.startServer();
    }

    //! Stops the server in the SeleniumFixture.
    void stopServer()
    {
      fixture_.stopServer();
    }

    //! Retrieves the URL of the server in the SeleniumFixture.
    std::string url() const
    {
      return fixture_.url();
    }

    /*! \brief Initialize the API.
     *
     * This sets up the browser connection, and waits for the initial page to be
     * loaded.
     */
    SeleniumAPI& api()
    {
      if (!api_) {
        api_ = std::make_unique<SeleniumAPI>();
        if (!api_->setupBrowser(fixture_.url(), browser_, driverPath_)) {
          throw std::runtime_error("Failed to setup browser");
        }
        if (!api_->waitForPageLoad()) {
          throw std::runtime_error("Failed to load page");
        }
      }
      return *api_;
    }

    // Updates the application instances in the WServer to run the function.
    // This update is then triggered, so that awaited changes can occur.
    void updateApplication(const std::function<void(AppType*)>& function)
    {
      fixture_.server().postAll([function]() {
        auto app = dynamic_cast<AppType*>(Wt::WApplication::instance());
        if (app) {
          function(app);
          app->triggerUpdate();
        }
      });
    }


    ~SeleniumTest()
    {
      if (api_) {
        api_->cleanup();
      }
      stopServer();
    }

  private:
    std::unique_ptr<SeleniumAPI> api_;
    SeleniumFixture fixture_;
    std::string driverPath_;
    SeleniumAPI::Browser browser_;
  };
}

#endif // SELENIUMTEST_H_
