/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBrush.h"

#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"

#include "Wt/Json/Array.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Value.h"

namespace Wt {

LOGGER("WBrush");

WBrush::WBrush()
  : style_(BrushStyle::None),
    color_(StandardColor::Black)
{ }

WBrush::WBrush(BrushStyle style)
  : style_(style),
    color_(StandardColor::Black)
{ }

WBrush::WBrush(const WColor& color)
  : style_(BrushStyle::Solid),
    color_(color)
{ }

WBrush::WBrush(StandardColor color)
  : style_(BrushStyle::Solid),
    color_(color)
{ }

WBrush::WBrush(const WGradient& gradient)
  : style_(BrushStyle::Gradient),
    gradient_(gradient)
{ }

#ifdef WT_TARGET_JAVA
WBrush WBrush::clone() const
{
  WBrush result;

  if (isJavaScriptBound()) result.assignBinding(*this);

  result.color_ = color_;
  result.gradient_ = gradient_;
  result.style_ = style_;

  return result;
}
#endif // WT_TARGET_JAVA

void WBrush::setColor(const WColor& color)
{
  checkModifiable();
  color_ = color;
  if (style_ == BrushStyle::Gradient)
    style_ = BrushStyle::Solid;
}

void WBrush::setGradient(const WGradient& gradient)
{
  checkModifiable();
  if (!gradient_.isEmpty()) {
    gradient_ = gradient;
    style_ = BrushStyle::Gradient;
  }
}

void WBrush::setStyle(BrushStyle style)
{
  checkModifiable();
  style_ = style;
}

WBrush &WBrush::operator=(const WBrush &rhs)
{
  WJavaScriptExposableObject::operator=(rhs);

  color_ = rhs.color_;
  gradient_ = rhs.gradient_;
  style_ = rhs.style_;

  return *this;
}

bool WBrush::operator==(const WBrush& other) const
{
  return
       sameBindingAs(other)
    && color_ == other.color_
    && style_ == other.style_
    && gradient_ == other.gradient_;
}

bool WBrush::operator!=(const WBrush& other) const
{
  return !(*this == other);
}

std::string WBrush::jsValue() const
{
  WStringStream ss;
  ss << "{\"color\":["
    << color_.red() << ","
    << color_.green() << ","
    << color_.blue() << ","
    << color_.alpha() << "]}";
  return ss.str();
}

void WBrush::assignFromJSON(const Json::Value &value)
{
  try {
#ifndef WT_TARGET_JAVA
    const Json::Object &o = value;
    const Json::Value &color = o.get("color");
    const Json::Array &col = color;
#else
    const Json::Object &o = static_cast<const Json::Object &>(value);
    const Json::Value &color = o.get("color");
    const Json::Array &col = static_cast<const Json::Array &>(color);
#endif
    if (col.size() == 4 &&
	!col[0].toNumber().isNull() &&
	!col[1].toNumber().isNull() &&
	!col[2].toNumber().isNull() &&
	!col[3].toNumber().isNull()) {
      color_ = WColor(col[0].toNumber().orIfNull(0),
		      col[1].toNumber().orIfNull(0),
		      col[2].toNumber().orIfNull(0),
		      col[3].toNumber().orIfNull(255));
    } else {
      LOG_ERROR("Couldn't convert JSON to WBrush");
    }
  } catch (std::exception& e) {
    LOG_ERROR("Couldn't convert JSON to WBrush: " + std::string(e.what()));
  }
}

}
