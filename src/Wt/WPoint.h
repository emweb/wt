// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPOINT_H_
#define WPOINT_H_

#include <Wt/WDllDefs.h>

namespace Wt {

/*! \class WPoint Wt/WPoint.h Wt/WPoint.h
 *  \brief A value class that defines a 2D point with integer coordinates.
 *
 * \sa WPolygonArea
 */
class WT_API WPoint
{
public:
  /*! \brief Creates a point (0, 0).
   */
  WPoint();

  /*! \brief Creates a point (x, y).
   */
  WPoint(int x, int y)
    : x_(x), y_(y) { }

  /*! \brief Sets the X coordinate.
   */
  void setX(int x) { x_ = x; }

  /*! \brief Sets the Y coordinate.
   */
  void setY(int y) { y_ = y; }

  /*! \brief Returns the X coordinate.
   */
  int x() const { return x_; }

  /*! \brief Returns the Y coordinate.
   */
  int y() const { return y_; }

  /*! \brief Comparison operator.
   */
  bool operator== (const WPoint& other) const;

  /*! \brief Comparison operator.
   */
  bool operator!= (const WPoint& other) const;

  WPoint& operator+= (const WPoint& other);

private:
  int x_, y_;
};

}

#endif // WPOINT_H_
