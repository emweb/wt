/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SELENIUM_WAIT_H_
#define SELENIUM_WAIT_H_

#include "PythonInterpreter.h"

#include <chrono>
#include <functional>
#include <Python.h>
#include <string>

namespace Selenium {
  /*! \class SeleniumWait "test/selenium/framework/SeleniumWait.h"
   *  \brief The class managing awaiting changes based on certain conditions.
   *
   * Either a function can be waited upon, which will wait until it returns
   * true. Or if the timeout was reached.
   * Likewise waiting functions have been added for Element instances
   * specifically.
   */
  class SeleniumWait
  {
  public:
    //! Instantiate with the global timeout configuration
    SeleniumWait(PyObject* driver, std::chrono::seconds timeout = std::chrono::seconds(10))
      : driver_(driver),
        timeout_(timeout)
    {
      if (driver_) {
        Py_INCREF(driver_);
      }
    }

    ~SeleniumWait()
    {
      if (driver_) {
        Py_DECREF(driver_);
      }
    }

    /*! Wait until the condition function returns true, or until the timeout
     * is reached. The global timeout can be overridden here.
     */
    bool until(const std::function<bool()>& condition,
               std::chrono::seconds seconds = std::chrono::seconds(0))
    {
      PyObject* waitModule = PythonInterpreter::instance().getWaitModule();
      if (!waitModule) {
        throw new APIException("Failed to get the wait module");
      }

      PyObject* waitClass = PythonInterpreter::getAttribute(waitModule, "WebDriverWait");
      if (!waitClass) {
        throw new APIException("Failed to get the wait module");
      }

      int timeOut = timeout_.count();
      if (seconds > std::chrono::seconds(0)) {
        timeOut = seconds.count();
      }

      PyObject* timeoutArg = PythonInterpreter::fromInt(timeOut);
      PyObject* wait = PythonInterpreter::callFunction(waitClass, driver_, timeoutArg);
      Py_DECREF(timeoutArg);
      Py_DECREF(waitClass);

      if (!wait) {
        PyErr_Clear();
        throw new APIException("Failed to create the wait module");
      }

      /*! Create a Python callable that wraps our C++ condition function
       * We need to pass the condition as a Python callable to wait.until()
       */
      PyObject* conditionCallable = PythonInterpreter::createConditionCallable(condition);
      if (!conditionCallable) {
        Py_DECREF(wait);
        throw new APIException("Failed to create condition");
      }

      PyObject* untilMethod = PythonInterpreter::getAttribute(wait, "until");
      Py_DECREF(wait);

      if (!untilMethod) {
        Py_DECREF(conditionCallable);
        throw new APIException("Failed get 'until' function");
      }

      PyObject* result = PythonInterpreter::callFunction(untilMethod, conditionCallable);
      Py_DECREF(untilMethod);
      Py_DECREF(conditionCallable);

      if (!result) {
        PyErr_Clear();
        throw new APIException("Timeout!");
      }

      Py_DECREF(result);
      return true;
    }

    /*! Wait until the Element's method returns the expected value, or until
     * the timeout is reached. The global timeout can be overridden here.
     *
     * Usage: wait.until(element, &Element::text, "expected text")
     */
    bool until(const Element& element,
               std::string (Element::*method)() const,
               const std::string& expectedValue,
               std::chrono::seconds seconds = std::chrono::seconds(0))
    {
      return until([&]() {
        auto result = (element.*method)();
        return result == expectedValue;
      }, seconds);
    }

    /*! Overload for CSS values that takes the property name.
     * Wait until the Element's method returns the expected value, or until
     * the timeout is reached. The global timeout can be overridden here.
     *
     * Usage: wait.until(element, &Element::getCssValue, "padding-left", "10px")
     */
    bool until(const Element& element,
               std::string (Element::*method)(const std::string&) const,
               const std::string& propertyName,
               const std::string& expectedValue,
               std::chrono::seconds seconds = std::chrono::seconds(0))
    {
      return until([&]() {
        auto result = (element.*method)(propertyName);
        return result == expectedValue;
      }, seconds);
    }

  private:
    PyObject* driver_;
    std::chrono::seconds timeout_;
  };
} // namespace Selenium

#endif // SELENIUM_WAIT_H_
