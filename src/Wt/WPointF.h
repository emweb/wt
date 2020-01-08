// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPOINTF_H_
#define WPOINTF_H_

#include <Wt/WDllDefs.h>
#include <Wt/WEvent.h>
#include <Wt/WJavaScriptExposableObject.h>

namespace Wt {

/*! \class WPointF Wt/WPointF.h Wt/WPointF.h
 *  \brief A value class that defines a 2D point.
 *
 * <h3>JavaScript exposability</h3>
 *
 * A %WPointF is JavaScript exposable. If a %WPointF
 * \link isJavaScriptBound() is JavaScript bound\endlink,
 * it can be accessed in your custom JavaScript code
 * through \link WJavaScriptHandle::jsRef() its handle's jsRef()\endlink.
 * A point is represented in JavaScript as an array of two elements, e.g. a point
 * WPointF(10,20) will be represented in JavaScript as:
 * \code
 * [10, 20]
 * \endcode
 *
 * \warning A %WPointF that is JavaScript exposed should be modified only through its \link WJavaScriptHandle handle\endlink.
 *	    Any attempt at modifying it will cause an exception to be thrown.
 *
 * \sa WPaintedWidget::createJSPoint()
 *
 * \ingroup painting
 */
class WT_API WPointF : public WJavaScriptExposableObject
{
public:
  /*! \brief Creates point (0, 0).
   */
  WPointF();

  /*! \brief Creates a point (x, y).
   */
  WPointF(double x, double y)
    : x_(x), y_(y) { }

  /*! \brief Copy constructor.
   */
  WPointF(const WPointF& other)
    : WJavaScriptExposableObject(other),
      x_(other.x()), y_(other.y()) { }

  /*! \brief Creates a point from mouse coordinates.
   */
  WPointF(const Coordinates& other)
    : x_(other.x), y_(other.y) { }

  WPointF& operator= (const WPointF& rhs);

#ifdef WT_TARGET_JAVA
  WPointF clone() const;
#endif

  virtual ~WPointF() {}

  /*! \brief Sets the X coordinate.
   *
   * \throws WException if the point \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setX(double x);

  /*! \brief Sets the Y coordinate.
   *
   * \throws WException if the point \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setY(double y);

  /*! \brief Returns the X coordinate.
   */
  double x() const { return x_; }

  /*! \brief Returns the Y coordinate.
   */
  double y() const { return y_; }

  /*! \brief Comparison operator.
   */
  bool operator== (const WPointF& other) const;

  /*! \brief Comparison operator.
   */
  bool operator!= (const WPointF& other) const;

  WPointF& operator+= (const WPointF& other);

  virtual std::string jsValue() const override;

  WPointF swapHV(double width) const;
  WPointF inverseSwapHV(double width) const;

protected:
  virtual void assignFromJSON(const Json::Value &value) override;

private:
  double x_, y_;

  friend class WTransform;
};

}

#endif // WPOINTF_H_
