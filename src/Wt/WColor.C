/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WColor"
#include "Wt/WLogger"
#include "Wt/WStringStream"
#include "WebUtils.h"
#include "ColorUtils.h"

#include <cmath>

namespace Wt {

LOGGER("WColor");

WColor::WColor()
  : default_(true),
    red_(0),
    green_(0),
    blue_(0),
    alpha_(255)
{ }

WColor::WColor(int r, int g, int b, int a)
  : default_(false),
    red_(r),
    green_(g),
    blue_(b),
    alpha_(a)
{ }

WColor::WColor(const WString& name)
  : default_(false),
    red_(-1),
    green_(-1),
    blue_(-1),
    alpha_(255),
    name_(name)
{ 
  WColor c = Wt::Color::parseCssColor(name.toUTF8());
  this->setRgb(c.red(), c.green(), c.blue(), c.alpha());
  // setRgb() erases name_
  name_ = name;
}

WColor::WColor(Wt::GlobalColor name)
{
  switch(name) {
  case Wt::white:       setRgb(0xff, 0xff, 0xff); break;
  case Wt::black:       setRgb(0x00, 0x00, 0x00); break;
  case Wt::red:         setRgb(0xff, 0x00, 0x00); break;
  case Wt::darkRed:     setRgb(0x80, 0x00, 0x00); break;
  case Wt::green:       setRgb(0x00, 0xff, 0x00); break;
  case Wt::darkGreen:   setRgb(0x00, 0x80, 0x00); break;
  case Wt::blue:        setRgb(0x00, 0x00, 0xff); break;
  case Wt::darkBlue:    setRgb(0x00, 0x00, 0x80); break;
  case Wt::cyan:        setRgb(0x00, 0xff, 0xff); break;
  case Wt::darkCyan:    setRgb(0x00, 0x80, 0x80); break;
  case Wt::magenta:     setRgb(0xff, 0x00, 0xff); break;
  case Wt::darkMagenta: setRgb(0x80, 0x00, 0x80); break;
  case Wt::yellow:      setRgb(0xff, 0xff, 0x00); break;
  case Wt::darkYellow:  setRgb(0x80, 0x80, 0x00); break;
  case Wt::gray:        setRgb(0xa0, 0xa0, 0xa4); break;
  case Wt::darkGray:    setRgb(0x80, 0x80, 0x80); break;
  case Wt::lightGray:   setRgb(0xc0, 0xc0, 0xc0); break;
  case Wt::transparent: setRgb(0x00, 0x00, 0x00, 0x00); break;
  }
}

bool WColor::operator==(const WColor& other) const
{
  return ((default_ == other.default_)
	  && (red_ == other.red_)
	  && (green_ == other.green_)
	  && (blue_ == other.blue_)
	  && (alpha_ == other.alpha_)
	  && (name_ == other.name_));
}

bool WColor::operator!=(const WColor& other) const
{
  return !(*this == other);
}

int WColor::red() const
{
  if (red_ == -1) {
    LOG_ERROR("red(): color component not available.");
    return 0;
  }

  return red_;
}

int WColor::green() const
{
  if (green_ == -1) {
    LOG_ERROR("green(): color component not available.");
    return 0;
  }

  return green_;
}

int WColor::blue() const
{
  if (blue_ == -1) {
    LOG_ERROR("blue(): color component not available.");
    return 0;
  }

  return blue_;
}

void WColor::setRgb(int red, int green, int blue, int alpha)
{
  default_ = false;
  name_ = WString();

  red_ = red;
  green_ = green;
  blue_ = blue;
  alpha_ = alpha;
}

void WColor::setName(const WString& name)
{
  default_ = false;
  name_ = name;

  red_ = green_ = blue_ = -1;
  alpha_ = 255;
}

const std::string WColor::cssText(bool withAlpha) const
{
  if (default_)
    return std::string();
  else {
    if (!name_.empty())
      return name_.toUTF8();
    else {
      WStringStream s;

#ifndef WT_TARGET_JAVA
      char buf[30];
#else
      char *buf;
#endif

      if (alpha_ != 255 && withAlpha) {
	s << "rgba(" << red_
	  << ',' << green_
	  << ',' << blue_
	  << ',' << Utils::round_css_str(alpha_ / 255., 2, buf) << ')';
      }	else
	s << "rgb(" << red_ << ',' << green_ << ',' << blue_ << ')';

      return s.c_str();
    }
  }
}

void WColor::toHSL(WT_ARRAY double hsl[3]) const
{
  double h = 0.0, s = 0.0, l = 0.0;
  double r = red() / 255.0;
  double g = green() / 255.0;
  double b = blue() / 255.0;
  double cmax = std::max(r, std::max(g, b));
  double cmin = std::min(r, std::min(g, b));
  double delta = cmax - cmin;
  l = (cmax + cmin) / 2.0;
  s = delta == 0 ? 0 : delta / (1.0 - std::fabs(2.0 * l - 1.0));
  if (delta == 0) {
    h = 0;
  } else if (cmax == r) {
    if (g >= b) {
      h = 60.0 * (g - b) / delta;
    } else {
      h = 60.0 * ((g - b) / delta + 6.0);
    }
  } else if (cmax == g) {
    h = 60.0 * ((b - r) / delta + 2.0);
  } else if (cmax == b) {
    h = 60.0 * ((r - g) / delta + 4.0);
  }
  hsl[0] = h; hsl[1] = s; hsl[2] = l;
}

WColor WColor::fromHSL(double h, double s, double l, int alpha)
{
  double c = (1.0 - std::fabs(2.0 * l - 1.0)) * s;
  double x = c * (1.0 - std::fabs(std::fmod(h / 60.0, 2.0) - 1.0));
  double m = l - c/2.0;
  double r, g, b;
  if (0.0 <= h && h < 60.0) {
    r = c;
    g = x;
    b = 0;
  } else if (60.0 <= h && h < 120.0) {
    r = x;
    g = c;
    b = 0;
  } else if (120.0 <= h && h < 180.0) {
    r = 0;
    g = c;
    b = x;
  } else if (180.0 <= h && h < 240.0) {
    r = 0;
    g = x;
    b = c;
  } else if (240.0 <= h && h < 300.0) {
    r = x;
    g = 0;
    b = c;
  } else /* 300 <= h < 360 */ {
    r = c;
    g = 0;
    b = x;
  }
  return WColor((int)((r + m) * 255), (int)((g + m) * 255), (int)((b + m) * 255), alpha);
}

}
