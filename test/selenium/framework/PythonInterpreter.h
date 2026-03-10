/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef PYTHONINTERPRETER_H_
#define PYTHONINTERPRETER_H_

#include <Wt/WException.h>

#include <functional>
#include <optional>
#include <Python.h>
#include <string>

#define val(x) #x
#define GETVAL(x) val(x)

namespace Selenium {
  /*! \class InterpreterException "test/selenium/framework/PythonInterpreter.h"
   *  \brief The exception thrown during interpretation errors.
   */
  class InterpreterException : public Wt::WException
  {
  public:
    InterpreterException(const std::string& what)
      : Wt::WException(what)
    {
    }
  };

  /*! \class PythonInterpreter "test/selenium/framework/PythonInterpreter.h"
   *  \brief Class responsible for calling the Python interpreter.
   *
   *  This class will call the Python(3) interpreter, using the C-API. It will
   *  then parse results and bind them to C++ code.
   */
  class PythonInterpreter
  {
  public:
    //! Retrieves the singleton instance
    static PythonInterpreter& instance()
    {
      static PythonInterpreter instance;
      return instance;
    }

    //! Returns the webdriver module
    PyObject* getWebdriverModule() { return webdriverModule_; }
    //! Returns the by module
    PyObject* getByModule() { return byModule_; }
    //! Returns the wait module
    PyObject* getWaitModule() { return waitModule_; }
    //! Returns the expected conditions module
    PyObject* getExpectedConditionsModule() { return expectedConditionsModule_; }

    ~PythonInterpreter()
    {
      if (Py_IsInitialized()) {
        Py_Finalize();
      }
    }

    // Delete copy, move constructors and assign operators
    PythonInterpreter(const PythonInterpreter&) = delete;
    PythonInterpreter& operator=(const PythonInterpreter&) = delete;
    PythonInterpreter(PythonInterpreter&&) = delete;
    PythonInterpreter& operator=(PythonInterpreter&&) = delete;

    //! Retrieves the attribute \p attrName from the object \p obj.
    static PyObject* getAttribute(PyObject* obj, const std::string& attrName)
    {
      if (!obj) {
        throw new InterpreterException("Passed object is null");
      }

      PyObject* attribute = PyObject_GetAttrString(obj, attrName.c_str());
      if (!attribute) {
        PyErr_Print();
        PyErr_Clear();
        throw new InterpreterException("Failed to get attribute '" + attrName + "'");
      }

      return attribute;
    }

    //! Calls the object \p obj using the named \p name argument \p param.
    static PyObject* call(PyObject* object, std::vector<std::string>& names, std::vector<PyObject*> params)
    {
      // Create empty tuple for positional arguments
      PyObject* args = PyTuple_New(0);

      // Create kwargs dictionary
      PyObject* kwargs = PyDict_New();
      for (size_t i = 0; i < names.size() && i < params.size(); ++i) {
        PyDict_SetItemString(kwargs, names[i].c_str(), params[i]);
      }

      // Call the object with empty positional args and keyword args
      PyObject* result = PyObject_Call(object, args, kwargs);

      Py_DECREF(args);
      Py_DECREF(kwargs);

      if (!result) {
        PyErr_Print();
        PyErr_Clear();
        std::string errorMsg = "Failed to call object with parameters: ";
        for (size_t i = 0; i < names.size(); ++i) {
          if (i > 0) errorMsg += ", ";
          errorMsg += names[i];
        }
        throw new InterpreterException(errorMsg);
      }

      return result;
    }

    //! Calls the function \p func using the arguments \p args.
    template<typename... Args>
    static PyObject* callFunction(PyObject* func, Args... args)
    {
      if (!func) {
        throw new InterpreterException("Function is null");
      }

      PyObject* result = PyObject_CallFunctionObjArgs(func, args..., nullptr);
      if (!result) {
        PyErr_Print();
        PyErr_Clear();
        throw new InterpreterException("Function call failed");
      }

      return result;
    }

    // Deallocates the condition (from self)
    static void conditionDealloc(PyObject* self)
    {
      ConditionWrapper* wrapper = reinterpret_cast<ConditionWrapper*>(self);
      if (wrapper->condition) {
        delete wrapper->condition;
      }
      Py_TYPE(self)->tp_free(self);
    }

    // Creates the actual condition from the provided function.
    static PyObject* createConditionCallable(const std::function<bool()>& condition)
    {
      static PyTypeObject conditionType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        "SeleniumCondition",           /* tp_name */
        sizeof(ConditionWrapper),      /* tp_basicsize */
        0,                             /* tp_itemsize */
        conditionDealloc,              /* tp_dealloc */
        0,                             /* tp_print */
        0,                             /* tp_getattr */
        0,                             /* tp_setattr */
        0,                             /* tp_reserved */
        0,                             /* tp_repr */
        0,                             /* tp_as_number */
        0,                             /* tp_as_sequence */
        0,                             /* tp_as_mapping */
        0,                             /* tp_hash */
        conditionCall,                 /* tp_call */
        0,                             /* tp_str */
        0,                             /* tp_getattro */
        0,                             /* tp_setattro */
        0,                             /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,            /* tp_flags */
        "Condition wrapper",           /* tp_doc */
      };

      if (conditionType.tp_name && conditionType.tp_base == nullptr) {
        if (PyType_Ready(&conditionType) < 0) {
          return nullptr;
        }
      }

      ConditionWrapper* wrapper = PyObject_New(ConditionWrapper, &conditionType);
      if (!wrapper) {
        return nullptr;
      }

      wrapper->condition = new std::function<bool()>(condition);
      return reinterpret_cast<PyObject*>(wrapper);
    }

    //! Convert PyObject to UTF-8 string (after type checking).
    static std::optional<std::string> asUTF8(PyObject* obj)
    {
      if (PyUnicode_Check(obj)) {
        return std::string(PyUnicode_AsUTF8(obj));
      }

      if (obj == Py_None) {
        return "";
      }

      PyErr_Print();
      PyErr_Clear();
      throw new InterpreterException("Failed to convert to UTF-8");
    }

    //! Convert UTF-8 string to PyObject.
    static PyObject* fromUTF8(const std::string& str)
    {
      PyObject* pyStr = PyUnicode_FromString(str.c_str());
      if (!pyStr) {
        PyErr_Print();
        PyErr_Clear();
        throw new InterpreterException("Failed to create Python string from UTF-8");
      }

      return pyStr;
    }

    //! Gets the value of the dictionary \p obj with key \p value
    static PyObject* getDictionary(PyObject* obj, const std::string& value)
    {
      PyObject* result = PyDict_GetItemString(obj, value.c_str());
      if (!result) {
        throw new InterpreterException("Failed to retrieve dictionary value: " + value);
      }

      // "Hack" since dictionary retrievals are non-owned.
      Py_INCREF(result);
      return result;
    }

    //! Converts the PyObject to a bool (with type checking)
    static bool asBool(PyObject* obj)
    {
      if (PyBool_Check(obj)) {
        return PyObject_IsTrue(obj);
      }

      if (obj == Py_None) {
        return false;
      }

      PyErr_Print();
      PyErr_Clear();
      return false;
    }

    //! Converts an integer to a PyObject.
    static PyObject* fromInt(int value)
    {
      return PyLong_FromLong(value);
    }

    //! Converts the PyObject to an integer (with type checking)
    static int asInt(PyObject* obj)
    {
      if (PyLong_Check(obj)) {
        return static_cast<int>(PyLong_AsLong(obj));
      }

      if (PyFloat_Check(obj)) {
        return static_cast<int>(PyFloat_AsDouble(obj));
      }


      if (obj == Py_None) {
        return 0;
      }

      PyObject* intObj = PyNumber_Long(obj);
      if (!intObj) {
        PyErr_Print();
        PyErr_Clear();
        return 0;
      }

      long result = PyLong_AsLong(intObj);
      Py_DECREF(intObj);
      return static_cast<int>(result);
    }

  private:
    PythonInterpreter()
      : webdriverModule_(nullptr),
        byModule_(nullptr),
        waitModule_(nullptr),
        expectedConditionsModule_(nullptr)
    {
      initializePython();
      importModules();
    }

    // Intitialize Python using the C-API.
    void initializePython()
    {
      if (!Py_IsInitialized()) {
        Py_Initialize();
        loadFromVirtualEnvironment();
      }
    }

    // Loads the Python3 virtual environment (set up in CMake).
    // See: test/selenium/CMakeLists.txt
    void loadFromVirtualEnvironment()
    {
      std::string venvPath = GETVAL(PYTHON_VENV_PATH);
      // Execute Python code to add venv site-packages to sys.path
      std::string pythonCode =
        "import sys\n"
        "import os\n"
        "import glob\n"
        "venv_path = '" + venvPath + "'\n"
        "site_packages = glob.glob(os.path.join(venv_path, 'lib', 'python3*', 'site-packages'))\n"
        "if site_packages:\n"
        "    sys.path.insert(0, site_packages[0])\n";

      int res = PyRun_SimpleString(pythonCode.c_str());
      PyErr_Clear();
      if (res == -1) {
        throw new InterpreterException("Failed to configure Python virtual environment");
      }
    }

    void importModules()
    {
      webdriverModule_ = PyImport_ImportModule("selenium.webdriver");
      if (!webdriverModule_) {
        PyErr_Print();
        PyErr_Clear();
        throw new InterpreterException("Failed to imprort selenium.webdriver");
      }

      // Import selenium.webdriver.common.by
      byModule_ = PyImport_ImportModule("selenium.webdriver.common.by");
      if (!byModule_) {
        PyErr_Print();
        PyErr_Clear();
        throw new InterpreterException("Failed to import selenium.webdriver.common.by");
      }

      waitModule_ = PyImport_ImportModule("selenium.webdriver.support.wait");
      if (!waitModule_) {
        PyErr_Print();
        PyErr_Clear();
        throw new InterpreterException("Failed to import selenium.webdriver.support.wait");
      }

      expectedConditionsModule_ = PyImport_ImportModule("selenium.webdriver.support.expected_conditions");
      if (!expectedConditionsModule_) {
        PyErr_Print();
        PyErr_Clear();
        throw new InterpreterException("Failed to import selenium.webdriver.support.expected_conditions");
      }
    }

    // Structure to hold the C++ function in Python object
    struct ConditionWrapper {
      PyObject_HEAD
      std::function<bool()>* condition;
    };

    // Creates a call to the condition to self, given the arguments.
    static PyObject* conditionCall(PyObject* self, PyObject* args, PyObject* kwargs)
    {
      ConditionWrapper* wrapper = reinterpret_cast<ConditionWrapper*>(self);
      if (wrapper->condition) {
        try {
          bool result = (*wrapper->condition)();
          if (result) {
            Py_RETURN_TRUE;
          } else {
            Py_RETURN_FALSE;
          }
        } catch (...) {
          Py_RETURN_FALSE;
        }
      }
      Py_RETURN_FALSE;
    }

    PyObject* webdriverModule_;
    PyObject* byModule_;
    PyObject* waitModule_;
    PyObject* expectedConditionsModule_;
  };
}

#endif // PYTHONINTERPRETER_H_
