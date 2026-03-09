/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef PYTHONINTERPRETER_H_
#define PYTHONINTERPRETER_H_

#include <Wt/WException.h>

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

  private:
    PythonInterpreter()
      : webdriverModule_(nullptr)
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
    }

    PyObject* webdriverModule_;
  };
}

#endif // PYTHONINTERPRETER_H_
