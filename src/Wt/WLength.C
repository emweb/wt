/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WLength"

#include "Utils.h"

#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include <cstring>

namespace Wt {

WLength WLength::Auto;

WLength::WLength()
  : auto_(true),
    unit_(Pixel),
    value_(-1)
{ }

WLength::WLength(double value, Unit unit)
  : auto_(false),
    unit_(unit),
    value_(value)
{ }

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
    Utils::round_str(value_, 1, buf);
    std::strcat(buf, unitText[unit_]);
    return buf;
#else
    return boost::lexical_cast<std::string>(value_) + unitText[unit_];
#endif
  }
}

double WLength::toPixels() const
{
  static const double pxPerPt = 4.0/3.0;
  static const double unitFactor[]
    = { 16,
	8,                   // approximate: ex/em is 0.46 to 0.56...
	1,
	72 * pxPerPt,        // 72 'CSS'points in an inch
	72 / 2.54 * pxPerPt, // 2.54 cm in an inch
	72 / 25.4 * pxPerPt, // 25.4 mm in an inch
	pxPerPt,
	12 * pxPerPt,        // 12 points per pica
	0.16 };              // 12pt = 16px = 1em = 100%

  if (auto_)
    return 0;
  else
    return value_ * unitFactor[unit_];
}

}
