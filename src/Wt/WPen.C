/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPen"

#include "Wt/WLogger"
#include "Wt/WStringStream"

#include "Wt/Json/Array"
#include "Wt/Json/Object"
#include "Wt/Json/Value"

namespace Wt {

LOGGER("WPen");

WPen::WPen()
  : penStyle_(SolidLine),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(black)
{ }

WPen::WPen(PenStyle style)
  : penStyle_(style),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(black)
{ }

WPen::WPen(const WColor& color)
  : penStyle_(SolidLine),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(color)
{ }

WPen::WPen(GlobalColor color)
  : penStyle_(SolidLine),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(color)
{ }

WPen::WPen(const WGradient& gradient)
  : penStyle_(SolidLine),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(black),
    gradient_(gradient)
{ }

#ifdef WT_TARGET_JAVA
WPen WPen::clone() const
{
  WPen result;
  if (isJavaScriptBound()) result.assignBinding(*this);
  result.penStyle_ = penStyle_;
  result.penCapStyle_ = penCapStyle_;
  result.penJoinStyle_ = penJoinStyle_;
  result.width_ = width_;
  result.color_ = WColor(color_.red(), color_.green(), color_.blue(), color_.alpha());
  return result;
}
#endif // WT_TARGET_JAVA

void WPen::setStyle(PenStyle style)
{
  checkModifiable();
  penStyle_ = style;
}

void WPen::setCapStyle(PenCapStyle style)
{
  checkModifiable();
  penCapStyle_ = style;
}

void WPen::setJoinStyle(PenJoinStyle style)
{
  checkModifiable();
  penJoinStyle_ = style;
}

void WPen::setWidth(const WLength& width)
{
  checkModifiable();
  width_ = width;
}

void WPen::setColor(const WColor& color)
{
  checkModifiable();
  color_ = color;
  gradient_ = WGradient();
}

void WPen::setGradient(const WGradient& gradient)
{
  checkModifiable();
  gradient_ = gradient;
}

WPen& WPen::operator=(const WPen& rhs)
{
  WJavaScriptExposableObject::operator=(rhs);

  penStyle_ = rhs.penStyle_;
  penCapStyle_ = rhs.penCapStyle_;
  penJoinStyle_ = rhs.penJoinStyle_;
  width_ = rhs.width_;
  color_ = rhs.color_;
  gradient_ = rhs.gradient_;

  return *this;
}

bool WPen::operator==(const WPen& other) const
{
  return
       sameBindingAs(other)
    && penStyle_ == other.penStyle_
    && penCapStyle_ == other.penCapStyle_
    && penJoinStyle_ == other.penJoinStyle_
    && width_ == other.width_
    && color_ == other.color_
    && gradient_ == other.gradient_;
}

bool WPen::operator!=(const WPen& other) const
{
  return !(*this == other);
}

void WPen::assignFromJSON(const Json::Value &value)
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
      LOG_ERROR("Couldn't convert JSON to WPen");
    }
  } catch (std::exception &e) {
    LOG_ERROR("Couldn't convert JSON to WPen: " + std::string(e.what()));
  }
}

std::string WPen::jsValue() const
{
  WStringStream ss;
  ss << "{\"color\":["
    << color_.red() << ","
    << color_.green() << ","
    << color_.blue() << ","
    << color_.alpha() << "]}";
  return ss.str();
}

}
