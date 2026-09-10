/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SELENIUM_ACTION_CHAINS_H_
#define SELENIUM_ACTION_CHAINS_H_

#include "SeleniumAPI.h"

namespace Selenium {
  /*! \class ActionChains "test/selenium/framework/ActionChains.h"
   *  \brief Queues native browser actions for execution with perform().
   *
   *  Use within a Selenium test while its browser and Python interpreter
   *  are alive. Each instance owns an independent action queue.
   */
  class ActionChains
  {
  public:
    explicit ActionChains(const SeleniumAPI& api)
      : actions_(nullptr)
    {
      if (!api.driver()) {
        throw APIException("Driver not initialized");
      }

      PyObject* actionsClass = PyObject_GetAttrString(PythonInterpreter::instance().getWebdriverModule(), "ActionChains");
      if (!actionsClass) {
        checkResult(nullptr, "get ActionChains class");
      }
      actions_ = PyObject_CallFunctionObjArgs(actionsClass, api.driver(), nullptr);
      Py_DECREF(actionsClass);
      if (!actions_) {
        checkResult(nullptr, "create ActionChains");
      }
    }

    ActionChains(const ActionChains&) = delete;
    ActionChains& operator=(const ActionChains&) = delete;

    ~ActionChains()
    {
      Py_DECREF(actions_);
    }

    //! Queues a double click at the center of the element.
    ActionChains& doubleClick(const Element& element)
    {
      checkResult(PyObject_CallMethod(actions_, "double_click", "O", element.pyObject()), "queue double click");
      return *this;
    }

    //! Queues scrolling from the element's center by the pixel deltas.
    ActionChains& scrollFromElement(const Element& element, int deltaX, int deltaY)
    {
      PyObject* wheelModule = PyImport_ImportModule("selenium.webdriver.common.actions.wheel_input");
      if (!wheelModule) {
        checkResult(nullptr, "import wheel input");
      }
      PyObject* originClass = PyObject_GetAttrString(wheelModule, "ScrollOrigin");
      Py_DECREF(wheelModule);
      if (!originClass) {
        checkResult(nullptr, "get ScrollOrigin class");
      }
      PyObject* origin = PyObject_CallMethod(originClass, "from_element", "O", element.pyObject());
      Py_DECREF(originClass);
      if (!origin) {
        checkResult(nullptr, "create scroll origin");
      }
      PyObject* result = PyObject_CallMethod(actions_, "scroll_from_origin", "Oii", origin, deltaX, deltaY);
      Py_DECREF(origin);
      checkResult(result, "queue scroll");
      return *this;
    }

    //! Executes the queued actions, consuming them so they are not replayed.
    void perform()
    {
      checkResult(PyObject_CallMethod(actions_, "perform", nullptr), "perform actions");
    }

  private:
    PyObject* actions_;

    static void checkResult(PyObject* result, const char* operation)
    {
      if (!result) {
        PyErr_Print();
        throw APIException(std::string("Failed to ") + operation);
      }
      Py_DECREF(result);
    }
  };
} // namespace Selenium

#endif // SELENIUM_ACTION_CHAINS_H_
