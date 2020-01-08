// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRECTF_H_
#define WRECTF_H_

#include <Wt/WDllDefs.h>
#include <Wt/WJavaScriptExposableObject.h>

namespace Wt {

class WPointF;
class WPainterPath;

/*! \class WRectF Wt/WRectF.h Wt/WRectF.h
 *  \brief A value class that defines a rectangle.
 *
 * The rectangle is defined by a top-left point and a width and height.
 *
 * <h3>JavaScript exposability</h3>
 *
 * A %WRectF is JavaScript exposable. If a %WRectF \link isJavaScriptBound()
 * is JavaScript bound\endlink, it can be accessed in your custom JavaScript
 * code through \link WJavaScriptHandle::jsRef() its handle's jsRef()\endlink.
 * A rectangle is represented in JavaScript as an array of four elements (x,y,width,height), e.g. a rectangle
 * WRectF(10,20,30,40) will be represented in JavaScript as:
 * \code
 * [10, 20, 30, 40]
 * \endcode
 *
 * \warning A %WRectF that is JavaScript exposed should be modified only through its \link WJavaScriptHandle handle\endlink.
 *	    Any attempt at modifying it will cause an exception to be thrown.
 *
 * \sa WPaintedWidget::createJSRect()
 *
 * \ingroup painting
 */
class WT_API WRectF : public WJavaScriptExposableObject
{
public:
  /*! \brief Default constructor.
   *
   * \if cpp
   * Constructs a \p null rectangle.
   * 
   * \sa isNull()
   * \endif
   *
   * \if java
   * Constructs an empty rectangle.
   *
   * \sa isEmpty()
   * \endif
   */
  WRectF();

  /*! \brief Creates a rectangle.
   *
   * Constructs a rectangle with top left point (\p x, \p y)
   * and size \p width x \p height.
   */
  WRectF(double x, double y, double width, double height);

  WRectF(const WRectF& other);

  /*! \brief Creates a rectangle.
   *
   * Constructs a rectangle from the two points \p topLeft and
   * \p bottomRight.
   *
   * If you want to create a rectangle from two arbitrary corner
   * points, you can use this constructor too, but should call
   * normalized() afterwords.
   */
  WRectF(const WPointF& topLeft, const WPointF& bottomRight);

#ifdef WT_TARGET_JAVA
  /*! \brief Internal assign method. 
   */
  WRectF& operator= (const WRectF& rhs);
#endif // WT_TARGET_JAVA

#ifdef WT_TARGET_JAVA
  WRectF clone() const;
#endif

  virtual ~WRectF();

  /*! \brief Comparison operator.
   */
  bool operator== (const WRectF& rhs) const;
  bool operator!= (const WRectF& rhs) const;

  /*! \brief Checks for a <i>null</i> rectangle.
   *
   * A rectangle that isJavaScriptBound() is never null.
   *
   * \sa WRectF()
   */
  bool isNull() const;

  /*! \brief Determines whether or not this rectangle is empty. 
   *
   * A rectangle is empty if its width and height are zero.
   *
   * A rectangle that isJavaScriptBound() is JavaScript bound\endlink is never empty.
   */
  bool isEmpty() const;

  /*! \brief Sets the X-position of the left side.
   *
   * The right side of the rectangle does not move, and as a result,
   * the rectangle may be resized.
   *
   * \throws WException if the rectangle \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setX(double x);

  /*! \brief Sets the Y-position of the top side.
   *
   * The bottom side of the rectangle does not move, and as a result,
   * the rectangle may be resized.
   *
   * \throws WException if the rectangle \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setY(double y);

  /*! \brief Sets the width.
   *
   * The right side of the rectangle may move, but this does not
   * affect the left side.
   *
   * \throws WException if the rectangle \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setWidth(double width) { width_ = width; }

  /*! \brief Sets the Y-position of the top side.
   *
   * The bottom side of the rectangle may move, but this does not
   * affect the Y position of the top side.
   *
   * \throws WException if the rectangle \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setHeight(double height) { height_ = height; }

  /*! \brief Returns the X-position (left side offset).
   *
   * This is equivalent to left().
   *
   * \sa y(), left()
   */
  double x() const { return x_; }

  /*! \brief Returns the Y-position (top side offset).
   *
   * This is equivalent to top().
   *
   * \sa x(), top()
   */
  double y() const { return y_; }

  /*! \brief Returns the width.
   *
   * \sa height()
   */
  double width() const { return width_; }

  /*! \brief Returns the height.
   *
   * \sa width()
   */
  double height() const { return height_; }

  /*! \brief Returns the X position (left side offset).
   *
   * \sa x(), right()
   */
  double left() const { return x_; }

  /*! \brief Returns the Y position (top side offset).
   *
   * \sa y(), bottom()
   */
  double top() const { return y_; }

  /*! \brief Returns the the right side offset.
   *
   * \sa left()
   */
  double right() const { return x_ + width_; }

  /*! \brief Returns the bottom side offset.
   *
   * \sa top()
   */
  double bottom() const { return y_ + height_; }

  /*! \brief Returns the top left point.
   *
   * \sa left(), top()
   */
  WPointF topLeft() const;

  /*! \brief Returns the top right point.
   *
   * \sa right(), top()
   */
  WPointF topRight() const;

  /*! \brief Returns the center point.
   */
  WPointF center() const;

  /*! \brief Returns the bottom left point.
   *
   * \sa left(), bottom()
   */
  WPointF bottomLeft() const;

  /*! \brief Returns the bottom right point.
   *
   * \sa right(), bottom()
   */
  WPointF bottomRight() const;

  /*! \brief Tests if a rectangle contains a point.
   *
   * \note This method is not supported if this rectangle isJavaScriptBound()
   */
  bool contains(const WPointF& p) const;

  /*! \brief Tests if a rectangle contains a point.
   *
   * \note This method is not supported if this rectangle isJavaScriptBound()
   */
  bool contains(double x, double y) const;

  /*! \brief Tests if two rectangles intersect.
   */
  bool intersects(const WRectF& other) const;

  /*! \brief Makes the union of to rectangles.
   *
   * \note This method is not supported if this rectangle \link isJavaScriptBound() is JavaScript bound\endlink.
   */
  WRectF united(const WRectF& other) const;

  /*! \brief Returns a normalized rectangle.
   *
   * A normalized rectangle has a positive width and height.
   *
   * This method supports JavaScript bound rectangles.
   */
  WRectF normalized() const;

  virtual std::string jsValue() const override;

  WPainterPath toPath() const;

protected:
  void assignFromJSON(const Json::Value &value) override;

private:
  double x_, y_, width_, height_;

  friend class WTransform;
};

}

#endif // WRECTF_H_
