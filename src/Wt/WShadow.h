// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSHADOW_H_
#define WSHADOW_H_

#include <Wt/WColor.h>

namespace Wt {

/*! \class WShadow Wt/WShadow.h Wt/WShadow.h
 *  \brief A value class that defines a shadow style
 *
 * \sa WPainter::setShadow()
 *
 * \ingroup painting
 */
class WT_API WShadow
{
public:
  /*! \brief Default constructor.
   *
   * Constructs a disabled shadow effect (offsetX = offsetY = blur = 0)
   */
  WShadow();

  /*! \brief Constructs a shadow with given offset and color.
   */
  WShadow(double dx, double dy, const WColor& color, double blur);

#ifdef WT_TARGET_JAVA
    /*! \brief Clone method.
   *
   * Clones this shadow.
   */
  WShadow clone() const;
#endif

  /*! \brief Comparison operator.
   *
   * Returns \c true if the shadows are exactly the same.
   */
  bool operator==(const WShadow& other) const;

  /*! \brief Comparison operator.
   *
   * Returns \c true if the shadows are different.
   */
  bool operator!=(const WShadow& other) const;

  /*! \brief Returns whether the shadow effect is nihil.
   */
  bool none() const;

  /*! \brief Sets the shadow offset.
   */
  void setOffsets(double dx, double dy);

  /*! \brief Returns the shadow X offset.
   *
   * \sa setOffsets()
   */
  double offsetX() const { return offsetX_; }

  /*! \brief Returns the shadow Y offset.
   *
   * \sa setOffsets()
   */
  double offsetY() const { return offsetY_; }

  /*! \brief Changes the shadow color.
   *
   * \sa color()
   */
  void setColor(const WColor& color);

  /*! \brief Returns the shadow color.
   *
   * \sa setColor()
   */
  const WColor& color() const { return color_; }

  /*! \brief Sets the blur.
   *
   * \sa blur()
   */
  void setBlur(double blur);

  /*! \brief Returns the blur.
   *
   * \sa setBlur()
   */
  double blur() const { return blur_; }

private:
  WColor color_;
  double offsetX_, offsetY_, blur_;
};

}

#endif // WSHADOW_H_
