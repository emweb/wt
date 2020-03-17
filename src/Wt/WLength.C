/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WLength.h"
#include "Wt/WLogger.h"

#include "WebUtils.h"

#include <cstring>
#include <boost/algorithm/string.hpp>

namespace Wt {

LOGGER("WLength");

WLength WLength::Auto;

WLength::WLength()
  : auto_(true),
    unit_(LengthUnit::Pixel),
    value_(-1)
{ }

WLength::WLength(const std::string &str)
{
  parseCssString(str.c_str());
}

void WLength::parseCssString(const char *s)
{
  auto_ = false;
  unit_ = LengthUnit::Pixel;
  value_ = -1;

  if (std::string("auto") == s) {
    auto_ = true;
    return;
  }

  char *end = nullptr;
#ifndef WT_TARGET_JAVA
  value_ = std::strtod(s, &end);
#else
  Utils::stringToDouble(s, &end, value_);
#endif

  if (s == end) {
    LOG_ERROR("cannot parse CSS length: '" << s << "'");
    auto_ = true;
    return;
  }
  
  std::string unit(end);
  boost::trim(unit);
  
  if (unit == "em")
    unit_ = LengthUnit::FontEm;
  else if (unit == "ex")
    unit_ = LengthUnit::FontEx;
  else if (unit.empty() || unit == "px")
    unit_ = LengthUnit::Pixel;
  else if (unit == "in")
    unit_ = LengthUnit::Inch;
  else if (unit == "cm")
    unit_ = LengthUnit::Centimeter;
  else if (unit == "mm")
    unit_ = LengthUnit::Millimeter;
  else if (unit == "pt")
    unit_ = LengthUnit::Point;
  else if (unit == "pc")
    unit_ = LengthUnit::Pica;
  else if (unit == "%")
    unit_ = LengthUnit::Percentage;
  else if (unit == "vw")
    unit_ = LengthUnit::ViewportWidth;
  else if (unit == "vh")
    unit_ = LengthUnit::ViewportHeight;
  else if (unit == "vmin")
    unit_ = LengthUnit::ViewportMin;
  else if (unit == "vmax")
    unit_ = LengthUnit::ViewportMax;
  else {
    LOG_ERROR("unrecognized unit in '" << s << "'");
    auto_ = true;
    value_ = -1;
    unit_ = LengthUnit::Pixel;
  }
}

WLength::WLength(double value, LengthUnit unit)
  : auto_(false),
    value_(value)
{ 
  setUnit(unit);
}

void WLength::setUnit(LengthUnit unit)
{
  unit_ = unit;
}

bool WLength::operator== (const WLength& other) const
{
  return
       auto_  == other.auto_
    && unit_  == other.unit_
    && value_ == other.value_;
}

bool WLength::operator!= (const WLength& other) const
{
  return !(*this == other);
}

const std::string WLength::cssText() const
{
  static const char *unitText[]
    = { "em", "ex", "px", "in", "cm", "mm", "pt", "pc", "%", "vw", "vh", "vmin", "vmax" };

  if (auto_)
    return "auto";
  else {
#ifndef WT_TARGET_JAVA
    char buf[30];
    Utils::round_css_str(value_, 1, buf);
    if (unit_ == LengthUnit::ViewportMin) {
      WApplication *app = WApplication::instance();
      if (app && app->environment().agentIsIElt(10)) {
        std::strcat(buf, "vm");
      } else {
        std::strcat(buf, "vmin");
      }
    } else {
      std::strcat(buf, unitText[static_cast<unsigned int>(unit_)]);
    }
    return buf;
#else
    return std::to_string(value_) + unitText[static_cast<unsigned int>(unit_)];
#endif
  }
}

double WLength::toPixels(double fontSize) const
{
  static const double pxPerPt = 4.0/3.0;
  static const double unitFactor[]
    = { 1,
	72 * pxPerPt,        // 72 'CSS'points in an inch
	72 / 2.54 * pxPerPt, // 2.54 cm in an inch
	72 / 25.4 * pxPerPt, // 25.4 mm in an inch
	pxPerPt,
	12 * pxPerPt };      // 12 points per pica

  if (auto_)
    return 0;
  else
    if (unit_ == LengthUnit::FontEm)
      return value_ * fontSize;
    else if (unit_ == LengthUnit::FontEx)
      return value_ * fontSize / 2.0; // approximate: ex/em is 0.46 to 0.56...
    else if (unit_ == LengthUnit::Percentage ||
             unit_ == LengthUnit::ViewportWidth ||
             unit_ == LengthUnit::ViewportHeight ||
             unit_ == LengthUnit::ViewportMin ||
             unit_ == LengthUnit::ViewportMax)
      return value_ * fontSize / 100.0; // assuming relative to font size...
    else
      return value_ * unitFactor[static_cast<unsigned int>(unit_) - 2];
}

}
