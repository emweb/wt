/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPointF"

#include "Wt/WStringStream"

#include "web/WebUtils.h"

namespace Wt {

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
    ss << Utils::round_js_str(width, 3, buf) << " - p[1],p[0]];})(" << jsRef() + "))";
    result.assignBinding(*this, ss.str());
  }

  return result;
}

}
