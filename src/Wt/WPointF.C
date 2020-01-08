/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPointF.h"

#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"

#include "Wt/Json/Array.h"
#include "Wt/Json/Value.h"

#include "web/WebUtils.h"

namespace Wt {

LOGGER("WPointF");

WPointF::WPointF()
  : x_(0), y_(0)
{ }

WPointF& WPointF::operator= (const WPointF& rhs)
{
  WJavaScriptExposableObject::operator=(rhs);

  x_ = rhs.x();
  y_ = rhs.y();

  return *this;
}

#ifdef WT_TARGET_JAVA
WPointF WPointF::clone() const
{
  return WPointF(*this);
}
#endif

void WPointF::setX(double x)
{
  checkModifiable();
  x_ = x;
}

void WPointF::setY(double y)
{
  checkModifiable();
  y_ = y;
}

bool WPointF::operator== (const WPointF& other) const
{
  if (!sameBindingAs(other)) return false;

  return (x_ == other.x_) && (y_ == other.y_);
}

bool WPointF::operator!= (const WPointF& other) const
{
  return !(*this == other);
}

WPointF& WPointF::operator+= (const WPointF& other)
{
  checkModifiable();

  x_ += other.x_;
  y_ += other.y_;

  return *this;
}

std::string WPointF::jsValue() const
{
  char buf[30];
  WStringStream ss;
  ss << '[';
  ss << Utils::round_js_str(x_, 3, buf) << ',';
  ss << Utils::round_js_str(y_, 3, buf) << ']';
  return ss.str();
}

WPointF WPointF::swapHV(double width) const
{
  WPointF result(width - y(), x());
  
  if (isJavaScriptBound()) {
    WStringStream ss;
    char buf[30];
    ss << "((function(p){return [";
    ss << Utils::round_js_str(width, 3, buf) << "-p[1],p[0]];})(" << jsRef() + "))";
    result.assignBinding(*this, ss.str());
  }

  return result;
}

WPointF WPointF::inverseSwapHV(double width) const
{
  WPointF result(y(), width - x());

  if (isJavaScriptBound()) {
    WStringStream ss;
    char buf[30];
    ss << "((function(p){return [";
    ss << "p[1]," << Utils::round_js_str(width, 3, buf) << "-p[0]];})(" << jsRef() + "))";
    result.assignBinding(*this, ss.str());
  }

  return result;
}

void WPointF::assignFromJSON(const Json::Value &value)
{
  try {
#ifndef WT_TARGET_JAVA
    const Json::Array &ar = value;
#else
    const Json::Array &ar = static_cast<Json::Array&>(value);
#endif
    if (ar.size() == 2 &&
	!ar[0].toNumber().isNull() &&
	!ar[1].toNumber().isNull()) {
      x_ = ar[0].toNumber().orIfNull(x_);
      y_ = ar[1].toNumber().orIfNull(y_);
    } else {
      LOG_ERROR("Couldn't convert JSON to WPointF");
    }
  } catch (std::exception &e) {
    LOG_ERROR("Couldn't convert JSON to WPointF: " + std::string(e.what()));
  }
}

}
