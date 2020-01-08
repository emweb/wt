// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLINEF_H_
#define WLINEF_H_

#include <Wt/WDllDefs.h>

namespace Wt {

class WPointF;

/*! \class WLineF Wt/WLineF.h Wt/WLineF.h
 *  \brief Utility class that defines a single line.
 *
 * \ingroup painting
 */
class WT_API WLineF
{
public:
  /*! \brief Default constructor.
   *
   * Constructs a line from (<i>x1=0</i>,<i>y1=0</i>) to (<i>x2=0</i>,\p y2=0).
   */
  WLineF();

  /*! \brief Construct a line connecting two points.
   *
   * Constructs a line from <i>p1</i> to \p p2.
   */
  WLineF(const WPointF& p1, const WPointF& p2);

  /*! \brief Construct a line connecting two points.
   *
   * Constructs a line from (<i>x1</i>,<i>y1</i>) to (<i>x2</i>,\p y2).
   */
  WLineF(double x1, double y1, double x2, double y2);

  /*! \brief Returns the X coordinate of the first point.
   *
   * \sa y1(), p1()
   */
  double x1() const { return x1_; }

  /*! \brief Returns the Y coordinate of the first point.
   *
   * \sa x1(), p1()
   */
  double y1() const { return y1_; }

  /*! \brief Returns the X coordinate of the second point.
   *
   * \sa y2(), p2()
   */
  double x2() const { return x2_; }

  /*! \brief Returns the Y coordinate of the second point.
   *
   * \sa x2(), p2()
   */
  double y2() const { return y2_; }

  /*! \brief Returns the first point.
   *
   * \sa x1(), y1()
   */
  WPointF p1() const;

  /*! \brief Returns the second point.
   *
   * \sa x2(), y2()
   */
  WPointF p2() const;

  bool operator==(const WLineF& other) const;

  bool operator!=(const WLineF& other) const;

private:
  double x1_, y1_, x2_, y2_;
};

}

#endif // WLINEF_H_
