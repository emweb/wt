/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WException"
#include "Wt/WLength"
#include "Wt/WLogger"

#include "WebUtils.h"

#include <cstring>

namespace Wt {

LOGGER("WLength");

WLength WLength::Auto;

WLength::WLength()
  : auto_(true),
    unit_(Pixel),
    value_(-1)
{ }

WLength::WLength(const std::string &str)
{
  parseCssString(str.c_str());
}

void WLength::parseCssString(const char *s)
{
  auto_ = false;
  unit_ = Pixel;
  value_ = -1;

  if (std::string("auto") == s) {
    auto_ = true;
    return;
  }

  char *end = 0;
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
    unit_ = FontEm;
  else if (unit == "ex")
    unit_ = FontEx;
  else if (unit.empty() || unit == "px")
    unit_ = Pixel;
  else if (unit == "in")
    unit_ = Inch; 
  else if (unit == "cm")
    unit_ = Centimeter; 
  else if (unit == "mm")
    unit_ = Millimeter; 
  else if (unit == "pt")
    unit_ = Point; 
  else if (unit == "pc")
    unit_ = Pica; 
  else if (unit == "%")
    unit_ = Percentage;
  else {
    LOG_ERROR("unrecognized unit in '" << s << "'");
    auto_ = true;
    value_ = -1;
    unit_ = Pixel;
  }
}

WLength::WLength(double value, Unit unit)
  : auto_(false),
    value_(value)
{ 
  setUnit(unit);
}

void WLength::setUnit(Unit unit)
{
  unit_ = unit;

#ifndef WT_TARGET_JAVA
  if (unit_ < FontEm || unit_ > Percentage)
    throw WException("WLength: improper unit value.");
#endif // WT_TARGET_JAVA
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
    = { "em", "ex", "px", "in", "cm", "mm", "pt", "pc", "%" };

  if (auto_)
    return "auto";
  else {
#ifndef WT_TARGET_JAVA
    char buf[30];
    Utils::round_css_str(value_, 1, buf);
    std::strcat(buf, unitText[unit_]);
    return buf;
#else
    return boost::lexical_cast<std::string>(value_) + unitText[unit_];
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
    if (unit_ == FontEm)
      return value_ * fontSize;
    else if (unit_ == FontEx)
      return value_ * fontSize / 2.0; // approximate: ex/em is 0.46 to 0.56...
    else if (unit_ == Percentage)
      return value_ * fontSize / 100.0; // assuming relative to font size...
    else
      return value_ * unitFactor[unit_ - 2];
}

}
