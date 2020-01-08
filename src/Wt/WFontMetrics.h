// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFONTMETRICS_H_
#define WFONTMETRICS_H_

#include <Wt/WFont.h>

namespace Wt {

/*! \class WFontMetrics Wt/WFontMetrics.h Wt/WFontMetrics.h
 *  \brief A value class that describes font metrics for a font.
 *
 * This class provides font metrics for a given font. It is returned
 * by an implementation of WPaintDevice::fontMetrics(), and may differ
 * between devices.
 *
 * All methods return pixel dimensions.
 *
 * \sa WPaintDevice
 */
class WT_API WFontMetrics
{
public:
  /*! \brief Creates a font metrics information object.
   */
  WFontMetrics(const WFont& font, double leading, double ascent,
	       double descent);

  /*! \brief Returns the font for which these font metrics were computed.
   */
  const WFont& font() const { return font_; }

  /*! \brief Returns the font size.
   *
   * This is the same as:
   * \code
   * font().size().sizeLength()
   * \endcode
   *
   * e.g.~for a font with size set to 16px, this returns 16.
   */
  double size() const;

  /*! \brief Returns the font height.
   *
   * The font height is the total height of a text line. It is usually
   * a bit bigger than the font size to have natural line spacing.
   */
  double height() const { return leading_ + ascent_ + descent_; }

  /*! \brief Returns the font leading length.
   *
   * This is vertical space provided on top of the ascent (empty space
   * which serves as natural line spacing).
   */
  double leading() const { return leading_; }

  /*! \brief Returns the font ascent length.
   *
   * This is vertical space which corresponds to the maximum height of a
   * character over the baseline (although many fonts violate this for
   * some glyphs).
   */
  double ascent() const { return ascent_; } 

  /*! \brief Returns the font descent length.
   *
   * This is vertical space which corresponds to the maximum height of a
   * character under the baseline (although many fonts violate this for
   * some glyphs).
   */
  double descent() const { return descent_; }

private:
  WFont font_;
  double leading_, ascent_, descent_;
};

}

#endif // WFONT_METRICS_H_
