/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WBorder.h"

namespace Wt {

WBorder::WBorder()
  : width_(BorderWidth::Medium),
    style_(BorderStyle::None)
{ }

WBorder::WBorder(BorderStyle style, BorderWidth width, WColor color)
  : width_(width),
    color_(color),
    style_(style)
{ }

WBorder::WBorder(BorderStyle style, const WLength& width, WColor color)
  : width_(BorderWidth::Explicit),
    explicitWidth_(width),
    color_(color),
    style_(style)
{ }

bool WBorder::operator==(const WBorder& other) const
{
  return
    width_ == other.width_
    && color_ == other.color_
    && style_ == other.style_;
}

bool WBorder::operator!=(const WBorder& other) const
{
  return !(*this == other);
}

void WBorder::setWidth(BorderWidth width, const WLength& explicitWidth)
{
  width_ = width;
  explicitWidth_ = explicitWidth;
}

void WBorder::setColor(WColor color)
{
  color_ = color;
}

void WBorder::setStyle(BorderStyle style)
{
  style_ = style;
}

std::string WBorder::cssText() const
{
  std::string style;
  switch (style_) {
  case BorderStyle::None:
    return "none";
  case BorderStyle::Hidden:
    style = "hidden"; break;
  case BorderStyle::Dotted:
    style = "dotted"; break;
  case BorderStyle::Dashed:
    style = "dashed"; break;
  case BorderStyle::Solid:
    style = "solid"; break;
  case BorderStyle::Double:
    style = "double"; break;
  case BorderStyle::Groove:
    style = "groove"; break;
  case BorderStyle::Ridge:
    style = "ridge"; break;
  case BorderStyle::Inset:
    style = "inset"; break;
  case BorderStyle::Outset:
    style = "outset"; break;
  }

  std::string width;
  switch (width_) {
  case BorderWidth::Thin:
    width = "thin"; break;
  case BorderWidth::Medium:
    width = "medium"; break;
  case BorderWidth::Thick:
    width = "thick"; break;
  case BorderWidth::Explicit:
    width = explicitWidth_.cssText();
  }

  return width + " " + style + " " + color_.cssText();
}

#ifdef WT_TARGET_JAVA
WBorder WBorder::clone() const
{
  WBorder b;
  b.width_ = width_;
  b.explicitWidth_ = explicitWidth_;
  b.color_ = color_;
  b.style_ = style_;
  return b;
}
#endif

}
