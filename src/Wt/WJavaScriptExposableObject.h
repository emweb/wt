// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WJAVASCRIPT_EXPOSABLE_OBJECT_H_
#define WJAVASCRIPT_EXPOSABLE_OBJECT_H_

#include <Wt/WDllDefs.h>

#include <string>
#include <vector>

namespace Wt {

class WJavaScriptObjectStorage;
class WPaintedWidget;

namespace Json {
  class Value;
}

/*! \class WJavaScriptExposableObject Wt/WJavaScriptExposableObject.h
 *  \brief A JavaScript exposable object
 *
 * A JavaScript bound object (as opposed to being mostly
 * a simple value class) has an equivalent representation
 * in JavaScript. Its value can usually only be modified
 * through a WJavaScriptHandle. There are certain exceptions
 * to this rule. Some methods, notably many WTransform methods,
 * will correctly apply these modifications also on the JavaScript
 * representation.
 *
 *  \sa WJavaScriptHandle
 */
class WT_API WJavaScriptExposableObject {
public:
  WJavaScriptExposableObject();
  WJavaScriptExposableObject(const WJavaScriptExposableObject &other);
#ifndef WT_TARGET_JAVA
  WJavaScriptExposableObject &operator=(const WJavaScriptExposableObject &rhs);
#endif

#ifdef WT_TARGET_JAVA
  virtual WJavaScriptExposableObject clone() const = 0;
#endif

  /*! \brief Returns whether this object is JavaScript bound.
   *
   * An object is JavaScript bound if it is associated with a
   * WJavaScriptHandle. It should not be modified directly
   * on the server side. WJavaScriptHandle::setValue() should
   * be used instead.
   */
  bool isJavaScriptBound() const;

  virtual ~WJavaScriptExposableObject();

  /*! \brief Returns a JavaScript representation of the value of this object.
   *
   * \note The value returned will reflect the current server side value
   *       of the object. If this object is JavaScript bound, this value
   *       may not reflect the actual client side value. If you need access
   *       to the client side value, use jsRef() intead.
   */
  virtual std::string jsValue() const = 0;

  /*! \brief Returns a JavaScript reference to this object.
   *
   * If this object is not JavaScript bound, it will return
   * a JavaScript representation of the value of the object,
   * according to jsValue().
   *
   * \warning This reference is intended as read-only. Attempts
   *	      to modify it may have unintended consequences.
   *	      If you want a JavaScript reference that is modifiable,
   *	      use the \link WJavaScriptHandle::jsRef() jsRef of the
   *	      handle\endlink instead.
   */
  std::string jsRef() const;

  bool closeTo(const WJavaScriptExposableObject& other) const { return false; }

protected:
  bool sameBindingAs(const WJavaScriptExposableObject &rhs) const;
  void assignBinding(const WJavaScriptExposableObject &rhs);
  void assignBinding(const WJavaScriptExposableObject &rhs, const std::string &jsRef);
  // Checks if this object is JavaScript bound,
  // if it is, it will throw an exception.
  void checkModifiable();

  virtual void assignFromJSON(const Json::Value &value) = 0;

private:
  template<typename T> friend class WJavaScriptHandle;
  friend class WJavaScriptObjectStorage;

  struct JSInfo {
    JSInfo(WJavaScriptObjectStorage *context, const std::string &jsRef);
    JSInfo(const JSInfo &other);

    JSInfo &operator=(const JSInfo &rhs);

    bool operator==(const JSInfo &rhs) const;

    WJavaScriptObjectStorage *context_;
    std::string jsRef_;
  };

  JSInfo *clientBinding_;
};

}

#endif // WJAVASCRIPT_EXPOSABLE_OBJECT_H_
