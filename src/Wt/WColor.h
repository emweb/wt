// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCOLOR_H_
#define WCOLOR_H_

#include <Wt/WString.h>
#include <Wt/WDllDefs.h>
#include <Wt/WGlobal.h>

/*! \brief The namespace for %Wt.
 */
namespace Wt {

/*! \class WColor Wt/WColor.h Wt/WColor.h
 *  \brief A value class that defines a color.
 *
 * A color corresponds to a CSS color. You can specify a color either using
 * its red/green/blue components, or from a valid CSS name.
 *
 * The color supports an alpha channel, which determines the degree of
 * transparency. An alpha value of 0 is completely transparent (and thus
 * invisible), while a value of 255 is completely opaque.
 *
 * \ingroup style painting
 */
class WT_API WColor
{
public:
  /*! \brief Creates a default color.
   *
   * The default color is depending on the context, another color (for
   * example from a hierarchical parent in a widget tree), or a
   * completely transparent color.
   */
  WColor();

  /*! \brief Creates a color with given red/green/blue/alpha components.
   *
   * All four components must be specified with a value in the range
   * (0 - 255). The alpha channel determines the degree of
   * transparency. An alpha value of 0 is completely transparent (and
   * thus invisible), while a value of 255 is completely opaque.
   *
   * \sa setRgb(int, int, int, int)
   */
  WColor(int red, int green, int blue, int alpha = 255);

  /*! \brief Creates a color from a CSS name.
   *
   * The \p name may be any valid CSS color name, including names
   * colors such as "aqua", or colors defined as RGB components.
   *
   * Only if the color is defined in terms of RGB components (using
   * the \#rgb, \#rrggbb, rgb() or rgba() formats), will the CSS color
   * be parsed into red, blue and green values. Otherwise, these
   * values are not available, and the color can only be used in media
   * that support CSS colors.
   *
   * See also http://www.w3.org/TR/css3-color
   */
  WColor(const WString& name);

  /*! \brief Creates a predefined color
   *
   * Constructs one of the 16 predefined %Wt colors constants.
   */
  WColor(StandardColor name);

  /*! \brief Sets the red/green/blue/alpha components.
   *
   * All four components must be specified with a value in the range
   * (0 - 255). The alpha channel determines the degree of
   * transparency. An alpha value of 0 is completely transparent (and
   * thus invisible), while a value of 255 is completely opaque.
   */
  void setRgb(int red, int green, int blue, int alpha = 255);

  /*! \brief Sets the CSS name.
   *
   * \sa WColor(const WString&)
   */
  void setName(const WString& name);

  /*! \brief Returns if the color is the default color.
   *
   * \sa WColor()
   */
  bool isDefault() const { return default_; }

  /*! \brief Returns the red component.
   *
   * Only available when the color was specified in terms of the RGB
   * components using setRgb(int, int, int, int), WColor(int, int,
   * int, int) or WColor(const WString& name) when the name was parsable .
   *
   * If not available this method return 0.
   */
  int red() const;

  /*! \brief Returns the green component.
   *
   * Only available when the color was specified in terms of the RGB
   * components using setRgb(int, int, int, int), WColor(int, int,
   * int, int) or WColor(const WString& name) when the name was parsable.
   *
   * If not available this method return 0.
   */
  int green() const;

  /*! \brief Returns the blue component.
   *
   * Only available when the color was specified in terms of the RGB
   * components using setRgb(int, int, int, int), WColor(int, int,
   * int, int) or WColor(const WString& name) when the name was parsable.
   *
   * If not available this method return 0.
   */
  int blue() const;

  /*! \brief Returns the alpha component.
   *
   * Only available when the color was specified in terms of the RGB
   * components using setRgb(int, int, int, int) or WColor(int, int,
   * int, int).
   */
  int alpha() const { return alpha_; }

  /*! \brief Returns the CSS name.
   *
   * Only available when it was set with setName(const WString&) or
   * WColor(const WString& name).
   */
  const WString& name() const { return name_; }

  /*! \brief Comparison operator.
   *
   * Returns \c true if the two colors were defined in exactly the same way.
   * It may return \c false although they actually represent the same color.
   */
  bool operator== (const WColor& other) const;

  /*! \brief Comparison operator.
   *
   * Returns false if the two colors were not defined in exactly the
   * same way. It may return return although they actually represent
   * the same color.
   */
  bool operator!= (const WColor& other) const;

  /*! \brief Returns the color as a CSS property value.
   *
   * This returns the color in rgb() or rgba() notation.
   */
  const std::string cssText(bool withAlpha = false) const;

  void toHSL(WT_ARRAY double hsl[3]) const;
  static WColor fromHSL(double h, double s, double l, int alpha);

private:
  bool default_;
  int red_, green_, blue_, alpha_;
  WString name_;
};

}

#endif // WCOLOR
