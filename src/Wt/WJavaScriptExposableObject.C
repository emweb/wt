// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WJavaScriptExposableObject.h"
#include "Wt/WException.h"
#include "Wt/WLogger.h"

#include <cassert>

namespace Wt {

WJavaScriptExposableObject::WJavaScriptExposableObject()
  : clientBinding_(nullptr)
{ }

WJavaScriptExposableObject::WJavaScriptExposableObject(const WJavaScriptExposableObject &other)
#ifndef WT_TARGET_JAVA
  : clientBinding_(other.clientBinding_ ? 
		   new JSInfo(*other.clientBinding_) : nullptr)
#else
  : clientBinding_(other.clientBinding_)
#endif
{ }

#ifndef WT_TARGET_JAVA
WJavaScriptExposableObject &WJavaScriptExposableObject::operator=(const WJavaScriptExposableObject &rhs)
{
  if (clientBinding_ != nullptr && rhs.clientBinding_ != clientBinding_) {
    delete clientBinding_;
  }
  if (rhs.clientBinding_ != nullptr) {
    clientBinding_ = new JSInfo(*rhs.clientBinding_);
  } else {
    clientBinding_ = nullptr;
  }

  return *this;
}
#endif

bool WJavaScriptExposableObject::isJavaScriptBound() const
{
  return clientBinding_;
}

WJavaScriptExposableObject::~WJavaScriptExposableObject()
{
  delete clientBinding_;
}

std::string WJavaScriptExposableObject::jsRef() const
{
  if (clientBinding_) return clientBinding_->jsRef_;
  else return jsValue();
}

bool WJavaScriptExposableObject::sameBindingAs(const WJavaScriptExposableObject &rhs) const
{
  if (!clientBinding_ && !rhs.clientBinding_) return true; // No binding
  else if (clientBinding_ && rhs.clientBinding_) return (*clientBinding_) == (*rhs.clientBinding_);
  else return false; // One is bound, the other is not
}

void WJavaScriptExposableObject::assignBinding(const WJavaScriptExposableObject &rhs)
{
  assert(rhs.clientBinding_ != nullptr);
  if (&rhs != this) {
    if (clientBinding_) delete clientBinding_;
#ifndef WT_TARGET_JAVA
    clientBinding_ = new WJavaScriptExposableObject::JSInfo(*rhs.clientBinding_);
#else
    clientBinding_ = rhs.clientBinding_;
#endif
  }
}

void WJavaScriptExposableObject::assignBinding(const WJavaScriptExposableObject &rhs,
					       const std::string &jsRef)
{
  assert(rhs.clientBinding_ != nullptr);
  if (&rhs != this) {
    if (clientBinding_) delete clientBinding_;
    clientBinding_ = new WJavaScriptExposableObject::JSInfo(*rhs.clientBinding_);
  }
  clientBinding_->jsRef_ = jsRef;
}

void WJavaScriptExposableObject::checkModifiable()
{
  if (isJavaScriptBound()) {
    throw WException("Trying to modify a JavaScript bound object!");
  }
}

WJavaScriptExposableObject::JSInfo::JSInfo(WJavaScriptObjectStorage *context, const std::string &jsRef)
  : context_(context), jsRef_(jsRef)
{ }

WJavaScriptExposableObject::JSInfo::JSInfo(const WJavaScriptExposableObject::JSInfo &other)
  : context_(other.context_), jsRef_(other.jsRef_)
{ }

WJavaScriptExposableObject::JSInfo &WJavaScriptExposableObject::JSInfo::operator= (const WJavaScriptExposableObject::JSInfo &rhs)
{
  context_ = rhs.context_;
  jsRef_ = rhs.jsRef_;
  return *this;
}

bool WJavaScriptExposableObject::JSInfo::operator== (const WJavaScriptExposableObject::JSInfo &rhs) const
{
  return context_ == rhs.context_ && jsRef_ == rhs.jsRef_;
}

}
