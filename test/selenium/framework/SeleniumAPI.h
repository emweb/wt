/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SELENIUM_API_H_
#define SELENIUM_API_H_

#include <Wt/WException.h>

#include "PythonInterpreter.h"

#include <chrono>
#include <memory>
#include <optional>
#include <Python.h>
#include <string>
#include <thread>
#include <vector>

namespace Wt {
  class WApplication;
}

namespace Selenium {
  /*! \class APIException "test/selenium/framework/SeleniumAPI.h"
   *  \brief The exception throw when calling the SeleniumAPI.
   *
   *  \note This is different from the InterpreterException in that the
   *  interpreter will have returned a valid result, but an interaction
   *  with it may have failed.
   */
  class APIException : public Wt::WException
  {
  public:
    APIException(const std::string& what)
      : Wt::WException(what)
    {
    }
  };

  /*! \class SeleniumAPI "test/selenium/framework/SeleniumAPI.h"
   *  \brief API wrapper of Selenium.
   *
   *  This class is responsible for calling the right Selenium API, and
   *  querying the PythonInterpreter to do the correct "translation".
   *
   *  It is exposed in tests, so that the webdriver can be queried, and
   *  elements can be retrieved.
   */
  class SeleniumAPI
  {
  public:
    enum class Browser {
      Chrome,
      Firefox
    };

    SeleniumAPI()
      : driver_(nullptr)
    {
      // Modules are already imported by PythonInterpreter
    }

    ~SeleniumAPI()
    {
      cleanup();
    }

    //! Set up the driver, with the correct browser and its options.
    bool setupBrowser(const std::string& url, Browser browser = Browser::Chrome)
    {
      cleanup();

      PyObject* webdriverModule = PythonInterpreter::instance().getWebdriverModule();
      if (!webdriverModule) {
        throw new APIException("Failed to get selenium.webdriver module");
      }

      // Get the browser class (Chrome or Firefox)
      const char* browserClass = (browser == Browser::Chrome) ? "Chrome" : "Firefox";
      PyObject* driverClass = PythonInterpreter::getAttribute(webdriverModule, browserClass);

      if (!driverClass) {
        throw new APIException("Failed to get " + std::string(browserClass) + " class");
      }

      // Setup options
      PyObject* options = setupBrowserOptions(browser);
      if (!options) {
        Py_DECREF(driverClass);
        throw new APIException("Failed to get browser options");
      }

      // Create driver instance with empty options
      PyObject* kwargs = PyDict_New();
      PyDict_SetItemString(kwargs, "options", options);
      Py_DECREF(options);

      driver_ = PyObject_Call(driverClass, PyTuple_New(0), kwargs);
      Py_DECREF(kwargs);
      Py_DECREF(driverClass);

      if (!driver_) {
        PyErr_Print();
        PyErr_Clear();
        throw new APIException("Failed to instantiate the driver");
      }

      // Navigate to URL
      return loadUrl(url);
    }

    //! Call the driver's get() function, to retrieve the URL via it.
    bool loadUrl(const std::string& url)
    {
      if (!driver_) {
        throw new APIException("Driver not initialized");
      }

      PyObject* getMethod = PythonInterpreter::getAttribute(driver_, "get");
      if (!getMethod) {
        return false;
      }

      PyObject* urlArg = PythonInterpreter::fromUTF8(url);
      PyObject* result = PythonInterpreter::callFunction(getMethod, urlArg);
      Py_DECREF(urlArg);
      Py_DECREF(getMethod);

      if (!result) {
        throw new APIException("Failed to load URL: " + url);
      }

      Py_DECREF(result);
      return true;
    }

    //! Intial page load, which explicitly waits until the document's
    //  readyState is complete.
    bool waitForPageLoad(int timeoutSeconds = 10)
    {
      if (!driver_) {
        throw new APIException("Driver not initialized");
      }

      // Execute JavaScript to check if page is ready
      PyObject* executeMethod = PythonInterpreter::getAttribute(driver_, "execute_script");
      if (!executeMethod) {
        return false;
      }

      PyObject* script = PythonInterpreter::fromUTF8("return document.readyState");

      auto startTime = std::chrono::steady_clock::now();
      while (true) {
        PyObject* result = PythonInterpreter::callFunction(executeMethod, script);
        if (result) {
          auto state = PythonInterpreter::asUTF8(result);
          if (state.has_value() && state.value() == "complete") {
            Py_DECREF(result);
            Py_DECREF(script);
            Py_DECREF(executeMethod);
            return true;
          }
          Py_DECREF(result);
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::steady_clock::now() - startTime
        );
        if (elapsed.count() >= timeoutSeconds) {
          Py_DECREF(script);
          Py_DECREF(executeMethod);
          throw new APIException("Timeout waiting for page to load");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }

    //! Removes the webdriver
    void cleanup()
    {
      if (driver_) {
        PyObject* quitMethod = PyObject_GetAttrString(driver_, "quit");
        if (quitMethod) {
          PyObject* result = PythonInterpreter::callFunction(quitMethod);
          Py_XDECREF(result);
          Py_DECREF(quitMethod);
        }
        Py_DECREF(driver_);
        driver_ = nullptr;
      }
    }

    PyObject* driver() const { return driver_; }

  private:
    PyObject* setupBrowserOptions(Browser browser)
    {
      const char* optionsClass = (browser == Browser::Chrome) ? "ChromeOptions" : "FirefoxOptions";

      PyObject* optionsClassObj = PyObject_GetAttrString(PythonInterpreter::instance().getWebdriverModule(), optionsClass);

      if (!optionsClassObj) {
        PyErr_Print();
        PyErr_Clear();
        throw new APIException(std::string("Failed to get ") + optionsClass);
      }

      PyObject* options = PythonInterpreter::callFunction(optionsClassObj);
      Py_DECREF(optionsClassObj);

      if (!options) {
        PyErr_Print();
        PyErr_Clear();
        throw new APIException("Failed to create options instance");
      }

      // Add headless argument (and rendering options)
      PyObject* addArgMethod = PyObject_GetAttrString(options, "add_argument");
      if (addArgMethod) {
        if (browser == Browser::Chrome) {
          PyObject* arg = PythonInterpreter::fromUTF8("-headless=new");
          PythonInterpreter::callFunction(addArgMethod, arg);
          Py_DECREF(arg);

          arg = PythonInterpreter::fromUTF8("--disable-gpu");
          PythonInterpreter::callFunction(addArgMethod, arg);
          Py_DECREF(arg);

          arg = PythonInterpreter::fromUTF8("--no-sandbox");
          PythonInterpreter::callFunction(addArgMethod, arg);
          Py_DECREF(arg);
        } else {
          PyObject* arg = PythonInterpreter::fromUTF8("-headless");
          PythonInterpreter::callFunction(addArgMethod, arg);
          Py_DECREF(arg);
        }
        Py_DECREF(addArgMethod);
      }

      return options;
    }

    PyObject* driver_;
  };
} // namespace Selenium

#endif // SELENIUM_API_H_
