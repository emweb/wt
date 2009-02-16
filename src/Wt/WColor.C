/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>
#include "Wt/WColor"

namespace {
  double myround(double d)
  {
    return static_cast<int>(d * 100) / 100.;
  }
}

namespace Wt {

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
{ }

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
    else if (alpha_ != 255 && withAlpha)
      return "rgba(" + boost::lexical_cast<std::string>(red_)
	+ ',' + boost::lexical_cast<std::string>(green_)
	+ ',' + boost::lexical_cast<std::string>(blue_)
	+ ',' + boost::lexical_cast<std::string>(myround(alpha_ / 255.)) + ')';

    else
      return "rgb(" + boost::lexical_cast<std::string>(red_)
	+ ',' + boost::lexical_cast<std::string>(green_)
	+ ',' + boost::lexical_cast<std::string>(blue_) + ')';
  }
}

}
