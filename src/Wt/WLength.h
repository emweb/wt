// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLENGTH_H_
#define WLENGTH_H_

#include <string>
#include <Wt/WDllDefs.h>

namespace Wt {

/*! \brief CSS length unit type
 */
enum class LengthUnit {
  FontEm,         //!< The relative font size (em)
  FontEx,         //!< The height of an 'x' in the font (ex)
  Pixel,          //!< Pixel, relative to canvas resolution (px)
  Inch,           //!< Inch (in)
  Centimeter,     //!< Centimeter (cm)
  Millimeter,     //!< Millimeter (mm)
  Point,          //!< Point (1/72 Inch) (pt)
  Pica,           //!< Pica (12 Point) (pc)
  Percentage,     //!< Percentage (meaning context-sensitive) (%)
  /*! \brief A percentage of the viewport's width (vw)
   *
   * \note Internet Explorer only supports vw since version 9
   */
  ViewportWidth,
  /*! \brief A percentage of the viewport's height (vh)
   *
   * \note Internet Explorer only supports vh since version 9
   */
  ViewportHeight,
  /*! \brief A percentage of the viewport's smaller dimension (vmin)
   *
   * \note Internet Explorer only supports vmin since version 9
   */
  ViewportMin,
  /*! \brief A percentage of the viewport's larger dimension (vmax)
   *
   * \note Not supported on Internet Explorer
   */
  ViewportMax
};

/*! \class WLength Wt/WLength.h Wt/WLength.h
 *  \brief A value class that describes a CSS length.
 *
 * The class combines a value with a unit. There is a special value
 * <i>auto</i> which has a different meaning depending on the context.
 */
class WT_API WLength
{
public:
  /*! \brief Typedef for enum Wt::LengthUnit */
  typedef LengthUnit Unit;

  /*! \brief An 'auto' length.
   *
   * \sa WLength()
   */
  static WLength Auto;

  /*! \brief Creates an 'auto' length
   *
   * Specifies an 'auto' length.
   *
   * \sa Auto
   */
  WLength();

#ifndef WT_TARGET_JAVA
  /*! \brief Creates a length by parsing the argument as a css length string.
  *
  * This supports all CSS length formats that have an API counterpart.
  *
  * This is an overload for the std::string version that accepts a string
  * literal.
  */
  template<std::size_t N>
  WLength(const char(&str)[N]) {
    parseCssString(str);
  }
#endif // WT_TARGET_JAVA

  /*! \brief Creates a length by parsing the argument as a css length string.
   *
   * This supports all CSS length formats that have an API counterpart.
   */
  WLength(const std::string &str);

  /*! \brief Creates a length with value and unit.
   *
   * This constructor is also used for the implicit conversion of a
   * double to a WLength, assuming a pixel unit.
   */
  WLength(double value, LengthUnit unit = LengthUnit::Pixel);

  /*! \brief Returns whether the length is 'auto'.
   *
   * \sa WLength(), Auto
   */
  bool isAuto() const { return auto_; }

  /*! \brief Returns the value.
   *
   * \sa unit()
   */
  double value() const { return value_; }

  /*! \brief Returns the unit.
   *
   * \sa value()
   */
  LengthUnit unit() const { return unit_; }

  /*! \brief Returns the CSS text.
   */
  const std::string cssText() const;

  /*! \brief Comparison operator.
   */
  bool operator== (const WLength& other) const;

  /*! \brief Comparison operator.
   */
  bool operator!= (const WLength& other) const;

  /*! \brief Returns the (approximate) length in pixels.
   *
   * When the length isAuto(), 0 is returned, otherwise the approximate
   * length in pixels.
   *
   * \note For percentages (LengthUnit::Percentage), and units relative to viewport size
   *       (LengthUnit::ViewportWidth, LengthUnit::ViewportHeight, LengthUnit::ViewportMin,
   *        LengthUnit::ViewportMax), a percentage of the font size is used.
   */
  double toPixels(double fontSize = 16.0) const;
  
private:
  bool auto_;
  LengthUnit unit_;
  double value_;

  void setUnit(LengthUnit unit);
  void parseCssString(const char *str);
};

inline Wt::WLength operator*(const Wt::WLength &l, double s)
{
  return Wt::WLength(l.value() * s, l.unit());
}

inline Wt::WLength operator*(double s, const Wt::WLength &l)
{
  return l * s;
}

inline Wt::WLength operator/(const Wt::WLength &l, double s)
{
  return l * (1/s);
}

}

#endif // WLENGTH_H_
