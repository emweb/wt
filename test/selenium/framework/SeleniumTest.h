/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SELENIUMTEST_H_
#define SELENIUMTEST_H_

#include "SeleniumAPI.h"
#include "SeleniumFixture.h"

#include <boost/test/unit_test.hpp>

namespace Selenium {
  // Helper macro to define a Selenium test case with automatic setup
#define SELENIUM_TEST(test_name, app_type) \
    BOOST_AUTO_TEST_CASE(test_name) \
    { \
      Selenium::SeleniumTest<app_type> test; \
      BOOST_REQUIRE(test.startServer()); \

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
    enum class Browser {
      Chrome,
      Firefox
    };

    SeleniumTest(Browser browser = Browser::Chrome,
                const std::string& docroot = ".")
      : fixture_(docroot), browser_(browser)
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

    ~SeleniumTest()
    {
      stopServer();
    }

  private:
    SeleniumFixture fixture_;
    Browser browser_;
  };
}

#endif // SELENIUMTEST_H_
