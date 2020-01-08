// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WJAVASCRIPT_HANDLE_H_
#define WJAVASCRIPT_HANDLE_H_

#include <cassert>

#include <Wt/WException.h>
#include <Wt/WJavaScriptExposableObject.h>

namespace Wt {

/*! \class WJavaScriptHandle Wt/WJavaScriptHandle.h Wt/WJavaScriptHandle.h
 *  \brief A handle to a JavaScript representation of an object.
 * 
 *  A %WJavaScriptHandle allows to access and modify an object in JavaScript.
 *  This is useful to avoid server roundtrips when frequently updating something,
 *  e.g. to interact with and animate a WPaintedWidget.
 *
 *  You can use the value() of a %WJavaScriptHandle just as you would normally,
 *  with the exception that it will be \link WJavaScriptExposableObject::isJavaScriptBound()
 *  JavaScript bound\endlink, and so will any copies you make of it. You should
 *  not modify a JavaScript bound object, as this will not change its client side
 *  representation. Use the handle's setValue() method instead.
 *
 *  You can access (and modify) the value of a handle on the client side using jsRef().
 *
 *  You can update the value from the server with setValue(). Changes on the
 *  client side will be synced back to the server.
 *
 *  Currently, only WPaintedWidget allows the use of \link WJavaScriptExposableObject
 *  JavaScript exposable objects\endlink.
 *
 *  \sa WJavaScriptExposableObject, WPaintedWidget
 */
template <typename T>
class WJavaScriptHandle
{
public:
  /*! \brief Create an invalid WJavaScriptHandle
   *
   * The handle will be invalid until a valid WJavaScriptHandle
   * is copy-assigned to it.
   */
  WJavaScriptHandle() noexcept
    : value_(nullptr), id_(0)
  { }

  /*! \brief Copy constructor
   */
  WJavaScriptHandle(const WJavaScriptHandle &handle) noexcept
    : value_(handle.value_), id_(handle.id_)
  {
    assert(value_ == nullptr || 
	   value_->clientBinding_ != nullptr);
  }

  /*! \brief Copy assignment operator
   */
  WJavaScriptHandle &operator=(const WJavaScriptHandle &handle) noexcept
  {
    assert(handle.value_ == nullptr || 
	   handle.value_->clientBinding_ != nullptr);

    value_ = handle.value_;
    id_ = handle.id_;

    return (*this);
  }

  /*! \brief Move constructor
   */
  WJavaScriptHandle(WJavaScriptHandle &&handle) noexcept
    : value_(handle.value_), id_(handle.id_)
  {
    assert(handle.value_ == nullptr ||
           handle.value_->clientBinding_ != nullptr);

    handle.value_ = nullptr;
    handle.id_ = 0;
  }

  /*! \brief Move assignment operator
   */
  WJavaScriptHandle &operator=(WJavaScriptHandle &&handle) noexcept
  {
    if (this == &handle)
      return *this;

    assert(handle.value_ == nullptr ||
           handle.value_->clientBinding_ != nullptr);

    value_ = handle.value_;
    id_ = handle.id_;
    handle.value_ = nullptr;
    handle.id_ = 0;

    return *this;
  }

  /*! \brief Destructor
   */
  ~WJavaScriptHandle()
  { }

  /*! \brief Returns whether this is a valid handle
   *
   * A handle is not valid if it is not connected to a JavaScript
   * representation.  To make a WJavaScriptHandle valid, a valid
   * WJavaScriptHandle has to be copy-assigned to it. The various
   * createJS... methods in WPaintedWidget return a valid handle.
   */
  bool isValid() const noexcept {
    return value_;
  }
  
  /*! \brief Returns the JavaScript representation of the object.
   *
   * You can access and modify the value of this handle through its jsRef().
   *
   * \throw WException The handle is \link isValid() invalid\endlink
   */
  std::string jsRef() const {
    if (!value_) {
      throw WException("Can't retrieve the value of an invalid handle!");
    }
    return value_->clientBinding_->jsRef_;
  }

  /*! \brief Set the value for this handle.
   *
   * The value may not be JavaScript bound, i.e. related to another WJavaScriptHandle.
   * The change to the value will be synced to the client side equivalent.
   *
   * \throw WException The handle is \link isValid() invalid\endlink
   * \throw WException Trying to assign a JavaScript bound value
   */
  void setValue(const T& v)
  {
    if (!value_) {
      throw WException("Can't assign a value to an invalid handle!");
    }
    if (v.isJavaScriptBound()) throw WException("Can not assign a JavaScript bound value to a WJavaScriptHandle!");
    // Rescue the binding from being overridden by the assignment, and deleted
    WJavaScriptExposableObject::JSInfo *binding = value_->clientBinding_;
    value_->clientBinding_ = nullptr;

    bool changed = !value_->closeTo(v);
    (*value_) = v;

    value_->clientBinding_ = binding;
    if (changed)
      value_->clientBinding_->context_->dirty_[id_] = true;
  }

  /*! \brief Get the value for this handle.
   *
   * \warning You should not modify this value or any copy of it on the server side,
   *	      because this will not be synced to the client side. Use setValue() instead.
   *
   * \throw WException The handle is \link isValid() invalid\endlink
   */
  const T& value() const {
    if (!value_) {
      throw WException("Can't retrieve the value from an invalid handle!");
    }
    return *value_;
  }

private:
  friend class WJavaScriptObjectStorage;

  WJavaScriptHandle(int id, T *t) noexcept
    : value_(t), id_(id)
  {
    assert(t != nullptr && t->clientBinding_ != nullptr);
  }

  T *value_;
  int id_;
};

}

#endif // WJAVASCRIPT_HANDLE_H_
