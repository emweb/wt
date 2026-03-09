/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SELENIUM_ELEMENT_H_
#define SELENIUM_ELEMENT_H_

#include <Python.h>

#include "PythonInterpreter.h"

#include <optional>
#include <string>

namespace Selenium {
  /*! \struct Size
   *   \brief Holds the width and height of the Element.
   */
  struct Size {
    int width;
    int height;
  };

  /*! \struct Location
   *   \brief Holds the x and y coordinates of the Element.
   */
  struct Location {
    int x;
    int y;
  };

  /*! \class Element "test/selenium/framework/Element.h"
   *  \brief Holds an instance of a DOM element.
   *
   *  This element has certain properties in the DOM, like a name, a tag, and
   *  many other.
   *
   *  It can be interacted with by clicking, or sending keys to it.
   */
  class Element
  {
  public:
    Element(PyObject* pyElement)
      : pyElement_(pyElement)
    {
      if (pyElement_) {
        Py_INCREF(pyElement_);
      }
    }

    Element(const Element& other)
      : pyElement_(other.pyElement_)
    {
      if (pyElement_) {
        Py_INCREF(pyElement_);
      }
    }

    Element& operator=(const Element& other)
    {
      if (this != &other) {
        if (pyElement_) {
          Py_DECREF(pyElement_);
        }
        pyElement_ = other.pyElement_;
        if (pyElement_) {
          Py_INCREF(pyElement_);
        }
      }
      return *this;
    }

    ~Element()
    {
      if (pyElement_) {
        Py_DECREF(pyElement_);
      }
    }

    //! Retrieves the Elements's ID
    std::string id() const
    {
      // Ought to be a property/value, but that doesn't resolve correctly.
      return getAttributeValue<std::string>("id").value_or("");
    }

    //! Retrieves the Elements's tag name
    std::string tagName() const
    {

      return getProperty<std::string>("tag_name").value_or("");
    }

    //! Retrieves the Elements's class name
    std::string className() const
    {
      return getAttributeValue<std::string>("class").value_or("");
    }

    //! Retrieves the Elements's name attribute
    std::string name() const
    {
      return getAttributeValue<std::string>("name").value_or("");
    }

    //! Retrieves the Elements's value attribute
    std::string value() const
    {
      return getAttributeValue<std::string>("value").value_or("");
    }

    //! Retrieves the Elements's text property
    std::string text() const
    {
      return getProperty<std::string>("text").value_or("");
    }

    //! Retrieves the Elements's CSS value of name \p propertyName
    std::string getCssValue(const std::string& propertyName) const
    {
      return getNamedValue<std::string>("value_of_css_property", propertyName).value_or("");
    }

    //! Retrieves whether the Element is visible
    bool isVisible() const
    {
      return getValue<bool>("is_displayed").value_or(false);
    }

    //! Retrieves the Element's width
    int width() const
    {
      return getPropertyItem<int>("size", "width").value_or(-1);
    }

    //! Retrieves the Element's height
    int height() const
    {
      return getPropertyItem<int>("size", "height").value_or(-1);
    }

    //! Retrieves the Element's X coordinate
    int x() const
    {
      return getPropertyItem<int>("location", "x").value_or(-1);
    }

    //! Retrieves the Element's Y coordinate
    int y() const
    {
      return getPropertyItem<int>("location", "y").value_or(-1);
    }

    //! Retrieves  the Python object it wraps.
    PyObject* pyObject() const { return pyElement_; }

    //! Performs a click on the Element.
    bool click()
    {
      if (!pyElement_) {
        return false;
      }

      PyObject* clickMethod = PythonInterpreter::getAttribute(pyElement_, "click");
      if (!clickMethod) {
        return false;
      }

      PyObject* result = PythonInterpreter::callFunction(clickMethod);
      Py_DECREF(clickMethod);

      if (!result) {
        return false;
      }

      Py_DECREF(result);
      return true;
    }

    //! Sends the keys \p keys to the Element.
    bool sendKeys(const std::string& keys)
    {
      if (!pyElement_) {
        return false;
      }

      PyObject* sendKeysMethod = PythonInterpreter::getAttribute(pyElement_, "send_keys");
      if (!sendKeysMethod) {
        return false;
      }

      PyObject* keysArg = PythonInterpreter::fromUTF8(keys);
      if (!keysArg) {
        Py_DECREF(sendKeysMethod);
        return false;
      }

      PyObject* result = PythonInterpreter::callFunction(sendKeysMethod, keysArg);
      Py_DECREF(keysArg);
      Py_DECREF(sendKeysMethod);

      if (!result) {
        return false;
      }

      Py_DECREF(result);
      return true;
    }

  private:
    PyObject* pyElement_;

    template<typename Type>
    std::optional<Type> getProperty(const std::string& name) const
    {
      PyObject* result = PythonInterpreter::getAttribute(pyElement_, name);

      Type output;
      if (convert(result, output)) {
        Py_DECREF(result);
        return std::make_optional(output);
      } else {
        Py_DECREF(result);
        return std::nullopt;
      }
    }

    template<typename Type>
    std::optional<Type> getPropertyItem(const std::string& name, const std::string& value) const
    {
      PyObject* attribute = PythonInterpreter::getAttribute(pyElement_, name);
      if (!attribute) {
        return std::nullopt;
      }

      PyObject* result = PythonInterpreter::getDictionary(attribute, value.c_str());

      Py_DECREF(attribute);
      Type output;
      if (convert(result, output)) {
        Py_DECREF(result);
        return std::make_optional(output);
      } else {
        Py_DECREF(result);
        return std::nullopt;
      }
    }

    template<typename Type>
    std::optional<Type> getValue(const std::string& name) const
    {
      PyObject* attribute = PythonInterpreter::getAttribute(pyElement_, name);

      if (!attribute) {
        return std::nullopt;
      }

      PyObject* result = PythonInterpreter::callFunction(attribute);

      Py_DECREF(attribute);
      if (!result) {
        return std::nullopt;
      }

      Type output;
      if (convert(result, output)) {
        Py_DECREF(result);
        return std::make_optional(output);
      } else {
        Py_DECREF(result);
        return std::nullopt;
      }
    }

    template<typename Type>
    std::optional<Type> getNamedValue(const std::string& name, const std::string& value) const
    {
      PyObject* getValueMethod = PythonInterpreter::getAttribute(pyElement_, name);
      PyObject* valueString = PythonInterpreter::fromUTF8(value);
      PyObject* result = PythonInterpreter::callFunction(getValueMethod, valueString);

      Py_DECREF(getValueMethod);
      Py_DECREF(valueString);
      if (!result) {
        return std::nullopt;
      }

      Type output;
      if (convert(result, output)) {
        Py_DECREF(result);
        return std::make_optional(output);
      } else {
        Py_DECREF(result);
        return std::nullopt;
      }
    }

    template<typename Type>
    std::optional<Type> getAttributeValue(const std::string& name) const
    {
      return getNamedValue<Type>("get_attribute", name);
    }

    bool convert(PyObject* obj, std::string& output) const
    {
      output = PythonInterpreter::asUTF8(obj).value_or("");
      return true;
    }

    bool convert(PyObject* obj, bool& output) const
    {
      if (!obj) {
        output = false;
        return false;
      }

      output = PythonInterpreter::asBool(obj);
      return true;
    }

    bool convert(PyObject* obj, int& output) const
    {
      if (!obj) {
        output = 0;
        return false;
      }

      output = PythonInterpreter::asInt(obj);
      return true;
    }
  };
} // namespace Selenium

#endif // SELENIUM_ELEMENT_H_
