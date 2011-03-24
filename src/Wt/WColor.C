/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <stdlib.h>

#include "Wt/WColor"
#include "Utils.h"
#include "EscapeOStream.h"
#include "WtException.h"

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

int parseRgbArgument(const std::string& argument) 
{
  std::string arg = boost::trim_copy(argument);
  try {
    if (boost::ends_with(arg, "%"))
      return (int) (boost::lexical_cast<double>(arg.substr(0, arg.size() - 1)) 
		    * 255 / 100);
    else 
      return boost::lexical_cast<int>(arg);
  } catch (boost::bad_lexical_cast &e) {
    throw WtException(std::string("WColor: Illegal rgb argument: ") + arg); 
  }
}

WColor::WColor(const WString& name)
  : default_(false),
    name_(name)
{ 
  std::string n = name.toUTF8();
  boost::trim(n);
  
  if (boost::starts_with(n, "#")) {
    if (n.size() - 1 == 3) {
      red_ = strtol(n.substr(1,1).c_str(), 0, 16);
      red_ = red_ * 16 + red_;
      green_ = strtol(n.substr(2,1).c_str(), 0, 16);
      green_ = green_ * 16 + green_;
      blue_ = strtol(n.substr(3,1).c_str(), 0, 16);
      blue_ = blue_ * 16 + blue_;
    } else if (n.size() - 1 == 6) {
      red_ = strtol(n.substr(1,2).c_str(), 0, 16);
      green_ = strtol(n.substr(3,2).c_str(), 0, 16);
      blue_ = strtol(n.substr(5,2).c_str(), 0, 16);
    } else {
      throw WtException(std::string("WColor: Could not parse rgb format: ") 
			+ n);
    }
  } else if (boost::starts_with(n, "rgb")) {
    if (n.size() < 5)
      throw WtException(std::string("WColor: Could not parse rgb format: ") 
			+ n);

    bool alpha = (n[3] == 'a');
    int start_bracket = 3 + alpha;

    if (n[start_bracket] != '(' || n[n.size() - 1] != ')')
      throw WtException(std::string("WColor: Missing brackets in rgb format: ") 
			+ n);

    std::string argumentsStr = n.substr(start_bracket + 1, 
					n.size() - 1 - (start_bracket + 1));

    std::vector<std::string> arguments;
    boost::split(arguments, 
		 argumentsStr,
		 boost::is_any_of(","));

    if (!alpha && arguments.size() != 3)
      throw WtException(std::string("WColor: Invalid argument count: ") + n);

    if (alpha && arguments.size() != 4)
      throw WtException(std::string("WColor: Invalid argument count: ") + n);

    red_ = parseRgbArgument(arguments[0]);
    green_ = parseRgbArgument(arguments[1]);
    blue_ = parseRgbArgument(arguments[2]);

    if (alpha) 
      try {
	alpha_ = boost::lexical_cast<int>(boost::trim_copy(arguments[3]));
      } catch (boost::bad_lexical_cast &e) {
	throw WtException(std::string("WColor: Illegal alpha value: ") + 
			  arguments[3]); 
      } 
    else
      alpha_ = 255;
  } else {
    red_ = -1;
    green_ = -1;
    blue_ = -1;
    alpha_ = 255;
  }
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
  if (red_ == -1)
    throw WtException("WColor: No rgb values are available");
  return red_;
}

int WColor::green() const
{
  if (green_ == -1)
    throw WtException("WColor: No rgb values are available");
  return green_;
}

int WColor::blue() const
{
  if (blue_ == -1)
    throw WtException("WColor: No rgb values are available");
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
      EscapeOStream s;
#ifndef WT_TARGET_JAVA
      char buf[30];
#else
      char *buf;
#endif
      if (alpha_ != 255 && withAlpha) {
	s << "rgba(" << Utils::itoa(red_, buf);
	s << ',' << Utils::itoa(green_, buf);
	s << ',' << Utils::itoa(blue_, buf);
	s << ',' << Utils::round_str(alpha_ / 255., 2, buf) << ')';
      }	else {
	s << "rgb(" << Utils::itoa(red_, buf);
	s << ',' << Utils::itoa(green_, buf);
	s << ',' << Utils::itoa(blue_, buf) << ')';
      }

      return s.c_str();
    }
  }
}

}
