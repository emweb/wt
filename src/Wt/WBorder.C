/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WBorder"

namespace Wt {

WBorder::WBorder()
  : width_(Medium),
    style_(None)
{ }

WBorder::WBorder(Style style, Width width, WColor color)
  : width_(width),
    color_(color),
    style_(style)
{ }

WBorder::WBorder(Style style, const WLength& width, WColor color)
  : width_(Explicit),
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

void WBorder::setWidth(Width width, const WLength& explicitWidth)
{
  width_ = width;
  explicitWidth_ = explicitWidth;
}

void WBorder::setColor(WColor color)
{
  color_ = color;
}

void WBorder::setStyle(Style style)
{
  style_ = style;
}

std::string WBorder::cssText() const
{
  std::string style;
  switch (style_) {
  case None:
    return "none";
  case Hidden:
    style = "hidden"; break;
  case Dotted:
    style = "dotted"; break;
  case Dashed:
    style = "dashed"; break;
  case Solid:
    style = "solid"; break;
  case Double:
    style = "double"; break;
  case Groove:
    style = "groove"; break;
  case Ridge:
    style = "ridge"; break;
  case Inset:
    style = "inset"; break;
  case Outset:
    style = "outset"; break;
  }

  std::string width;
  switch (width_) {
  case Thin:
    width = "thin"; break;
  case Medium:
    width = "medium"; break;
  case Thick:
    width = "thick"; break;
  case Explicit:
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
