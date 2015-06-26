/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBrush"
#include "Wt/WStringStream"

namespace Wt {

WBrush::WBrush()
  : style_(NoBrush),
    color_(black)
{ }

WBrush::WBrush(BrushStyle style)
  : style_(style),
    color_(black)
{ }

WBrush::WBrush(const WColor& color)
  : style_(SolidPattern),
    color_(color)
{ }

WBrush::WBrush(GlobalColor color)
  : style_(SolidPattern),
    color_(color)
{ }

WBrush::WBrush(const WGradient& gradient)
  : style_(GradientPattern),
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
  color_ = color;
  if (style_ == GradientPattern)
    style_ = SolidPattern;
}

void WBrush::setGradient(const WGradient& gradient)
{
  if (!gradient_.isEmpty()) {
    gradient_ = gradient;
    style_ = GradientPattern;
  }
}

void WBrush::setStyle(BrushStyle style)
{
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

}
