// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPAINTERPATH_H_
#define WPAINTERPATH_H_

#include <Wt/WJavaScriptExposableObject.h>
#include <Wt/WPointF.h>
#include <Wt/WRectF.h>
#include <Wt/WTransform.h>

#include <vector>

namespace Wt {

/*! The segment type
 */
enum SegmentType {
  /*! \brief moveTo segment */
  MoveTo = 0,

  /*! \brief lineTo segment */
  LineTo = 1,

  /*! \brief first control point of cubic bezier curve.
   *
   * Always followed by a CubicC2 and CubicEnd segment 
   */
  CubicC1 = 2,

  /*! \brief second control point of cubic bezier curve
   *
   * Always followed by a CubicEnd segment
   */
  CubicC2 = 3,

  /*! \brief end point of cubic bezier curve
   */
  CubicEnd = 4,

  /*! \brief control point of quadratic bezier curve
   */
  QuadC = 5,

  /*! \brief end point of quadratic bezier curve
   */
  QuadEnd = 6,

  /*! \brief center of an arc
   *
   * Always followed by an ArcR and ArcAngleSweep segment
   */
  ArcC = 7,

  /*! \brief radius of an arc
   * Always followed by an ArcAngleSweep segment
   */
  ArcR = 8,

  /*! \brief the sweep of an arc
   *
   * x = startAngle, y = spanAngle
   */
  ArcAngleSweep = 9
};

/*! \class WPainterPath Wt/WPainterPath.h Wt/WPainterPath.h
 *  \brief A path defining a shape.
 *
 * A painter path represents a (complex) path that may be composed of
 * lines, arcs and bezier curve segments, and painted onto a paint device
 * using WPainter::drawPath().
 *
 * The path that is composed in a painter path may consist of multiple
 * closed sub-paths. Only the last sub-path can be left open.
 *
 * To compose a path, this class maintains a current position, which
 * is the starting point for the next drawing operation. An operation
 * may draw a line (see lineTo()), arc (see arcTo()), or bezier curve
 * (see quadTo() and cubicTo()) from the current position to a new
 * position. A new sub path may be started by moving the current
 * position to a new location (see moveTo()), which automatically
 * closes the previous sub path.
 *
 * When sub paths overlap, the result is undefined (it is dependent on
 * the underlying painting device).
 *
 * Usage example:
 * \if cpp
 * \code
 * Wt::WPainter painter(...);
 *
 * Wt::WPainterPath path(Wt::WPointF(10, 10));
 * path.lineTo(10, 20);
 * path.lineTo(30, 20);
 * path.closeSubPath();
 *
 * painter.setPen(Wt::StandardColor::Red);
 * painter.setBrush(Wt::StandardColor::Blue);
 * painter.drawPath(path);
 * \endcode
 * \elseif java
 * \code
 * WPainter painter = new WPainter();
 *	 
 * WPainterPath path = new WPainterPath(new WPointF(10, 10));
 * path.lineTo(10, 20);
 * path.lineTo(30, 20);
 * path.closeSubPath();
 *		 
 * painter.setPen(new WPen(WColor.red));
 * painter.setBrush(new WBrush(WColor.blue));
 * painter.drawPath(path);
 * \endcode
 * \endif
 *
 * <h3>JavaScript exposability</h3>
 *
 * A %WPainterPath is JavaScript exposable. If a %WPainterPath \link isJavaScriptBound()
 * is JavaScript bound\endlink, it can be accessed in your custom JavaScript
 * code through \link WJavaScriptHandle::jsRef() its handle's jsRef()\endlink.
 *
 * A %WPainterPath is represented in JavaScript as an array of segments, where each segment
 * is defined by a three element array: [x,y,type], where type is the integer representation
 * of the type of a segment.
 *
 * For example, a 10 by 10 square with the top left at (10,10) is represented as:
 * \code
 * [
 *  [10,10,0], // move to (10,10)
 *  [20,10,1], // line to (20,10)
 *  [20,20,1], // line to (20,20)
 *  [10,20,1], // line to (10,20)
 *  [10,10,1]  // line to (10,10)
 * ]
 * \endcode
 *
 * \warning A %WPainterPath that is JavaScript exposed should be modified only through its \link WJavaScriptHandle handle\endlink.
 *	    Any attempt at modifying it will cause an exception to be thrown.
 *
 * \sa WPainter::drawPath(), WPaintedWidget::createJSPainterPath()
 *
 * \ingroup painting
 */
class WT_API WPainterPath : public WJavaScriptExposableObject
{
public:
  /*! \brief Default constructor.
   *
   * Creates an empty path, and sets the current position to (0, 0).
   */
  WPainterPath();

  /*! \brief Construct a new path, and set the initial position.
   *
   * Creates an empty path, and sets the current position to
   * \p startPoint.
   */
  WPainterPath(const WPointF& startPoint);

  /*! \brief Copy constructor.
   */
  WPainterPath(const WPainterPath& path);

  /*! \brief Assignment operator.
   */
  WPainterPath& operator= (const WPainterPath& path);

#ifdef WT_TARGET_JAVA
  WPainterPath clone() const;
#endif

  /*! \brief Returns the current position.
   *
   * Returns the current position, which is the end point of the last
   * move or draw operation, and which well be the start point of the
   * next draw operation.
   */
  WPointF currentPosition() const;

  /*! \brief Returns whether the path is empty.
   *
   * Returns \c true if the path contains no drawing operations. Note that
   * move operations are not considered drawing operations.
   */
  bool isEmpty() const;

  /*! \brief Comparison operator.
   *
   * Returns \c true if the paths are exactly the same.
   */
  bool operator== (const WPainterPath& path) const;  

  /*! \brief Comparison operator.
   *
   * Returns \c true if the paths are different.
   */
  bool operator!= (const WPainterPath& path) const;

  /*! \brief Closes the last sub path.
   *
   * Draws a line from the current position to the start position of
   * the last sub path (which is the end point of the last move
   * operation), and sets the current position to (0, 0).
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void closeSubPath();
  
  /*! \brief Moves the current position to a new location.
   *
   * Moves the current position to a new point, implicitly closing the last
   * sub path, unless \link openSubPathsEnabled() open subpaths are enabled\endlink.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa closeSubPath(), moveTo(double, double), setOpenSubPathsEnabled(bool)
   */
  void moveTo(const WPointF& point);
  
  /*! \brief Moves the current position to a new location.
   *
   * Moves the current position to a new point, implicitly closing the last
   * sub path, unless \link openSubPathsEnabled() open subpaths are enabled\endlink.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa closeSubPath(), moveTo(const WPointF&), setOpenSubPathsEnabled(bool)
   */
  void moveTo(double x, double y);
  
  /*! \brief Draws a straight line.
   *
   * Draws a straight line from the current position to \p point,
   * which becomes the new current position.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa lineTo(double, double)
   */
  void lineTo(const WPointF& point);
  
  /*! \brief Draws a straight line.
   *
   * Draws a straight line from the current position to (\p x,
   * \p y), which becomes the new current position.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa lineTo(const WPointF&)
   */
  void lineTo(double x, double y);
  
  /*! \brief Draws a cubic bezier curve.
   *
   * Draws a cubic bezier curve from the current position to
   * \p endPoint, which becomes the new current position. The
   * bezier curve uses the two control points <i>c1</i> and \p c2.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa cubicTo(double, double, double, double, double, double)
   */
  void cubicTo(const WPointF& c1, const WPointF& c2, const WPointF& endPoint);
  
  /*! \brief Draws a cubic bezier curve.
   *
   * This is an overloaded method provided for convenience.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa cubicTo(const WPointF&, const WPointF&, const WPointF&)
   */
  void cubicTo(double c1x, double c1y, double c2x, double c2y,
	       double endPointx, double endPointy);
  
  /*! \brief Draws an arc.
   *
   * Draws an arc which is a segment of a circle. The circle is
   * defined with center (<i>cx</i>, <i>cy</i>) and \p radius. The
   * segment starts at \p startAngle, and spans an angle given by
   * \p spanAngle. These angles are expressed in degrees, and are
   * measured counter-clockwise starting from the 3 o'clock position.
   *
   * Implicitly draws a line from the current position to the start of
   * the arc, if the current position is different from the start.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa arcMoveTo()
   */
  void arcTo(double cx, double cy, double radius,
	     double startAngle, double spanAngle);

  /*! \brief Moves to a point on an arc.
   *
   * Moves to a point on a circle. The circle is defined with center
   * (<i>cx</i>, <i>cy</i>) and \p radius, and the point is at
   * \p angle degrees measured counter-clockwise starting from the
   * 3 o'clock position.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa arcTo()
   */
  void arcMoveTo(double cx, double cy, double radius, double angle);

  /*! \brief Move to a point on an arc.
   *
   * Moves to a point on an ellipse. The ellipse fits in the
   * rectangle defined by top left position (\p x,
   * <i>y</i>), and size <i>width</i> x \p height, and the point is at
   * \p angle degrees measured counter-clockwise starting from the
   * 3 o'clock position.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa arcTo()
   */
  void arcMoveTo(double x, double y, double width, double height, double angle);

  /*! \brief Draws a quadratic bezier curve
   *
   * Draws a quadratic bezier curve from the current position to
   * \p endPoint, which becomes the new current position. The
   * bezier curve uses the single control point \p c.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa quadTo(double, double, double, double)
   */
  void quadTo(const WPointF& c, const WPointF& endPoint);

  /*! \brief Draws a quadratic bezier curve.
   *
   * This is an overloaded method provided for convenience.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa quadTo(const WPointF&, const WPointF&)
   */
  void quadTo(double cx, double cy, double endPointx, double endPointy);

  /*! \brief Draws an ellipse.
   *
   * This method closes the current sub path, and adds an ellipse that is
   * bounded by the rectangle \p boundingRectangle.
   *
   * \p Note: some renderers only support circles (width == height)
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa addEllipse(double, double, double, double), arcTo()
   */
  void addEllipse(const WRectF& boundingRectangle);

  /*! \brief Draws an ellipse.
   *
   * This method closes the current sub path, and adds an ellipse that is
   * bounded by the rectangle defined by top left position (\p x,
   * <i>y</i>), and size <i>width</i> x \p height.
   *
   * \note Some renderers only support circles (width == height)
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa addEllipse(const WRectF&), arcTo()
   */
  void addEllipse(double x, double y, double width, double height);

  /*! \brief Draws a rectangle.
   *
   * This method closes the current sub path, unless \link openSubPathsEnabled()
   * open subpaths are enabled\endlink, and adds a rectangle
   * that is defined by \p rectangle.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa addRect(double, double, double, double)
   */
  void addRect(const WRectF& rectangle);

  /*! \brief Draws a rectangle.
   *
   * This method closes the current sub path, unless \link openSubPathsEnabled()
   * open subpaths are enabled\endlink, and adds a rectangle
   * that is defined by top left position (<i>x</i>, \p y), and
   * size <i>width</i> x \p height.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa addRect(const WRectF&)
   */
  void addRect(double x, double y, double width, double height);

  /*! \brief Adds a polygon.
   *
   * If the first point is different from the current position, the last
   * sub path is first closed, unless \link openSubPathsEnabled() open
   * subpaths are enabled\endlink, otherwise the last sub path is extended
   * with the polygon.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa moveTo(), lineTo()
   */
  void addPolygon(const std::vector<WPointF>& points);

  /*! \brief Adds a path.
   *
   * Adds an entire \p path to the current path. If the path's
   * begin position is different from the current position, the last
   * sub path is first closed, unless \link openSubPathsEnabled() open
   * subpaths are enabled\endlink, otherwise the last sub path is extended
   * with the path's first sub path.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa connectPath(const WPainterPath&)
   */
  void addPath(const WPainterPath& path);

  /*! \brief Adds a path, connecting.
   *
   * Adds an entire \p path to the current path. If the path's
   * begin position is different from the current position, the last
   * sub path is first closed, unless \link openSubPathsEnabled() open
   * subpaths are enabled\endlink, otherwise the last sub path is extended
   * with the path's first sub path.
   *
   * \throws WException if the path \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa connectPath(const WPainterPath&)
   */
  void connectPath(const WPainterPath& path);

  /*! \brief A segment.
   */
  class Segment 
  {
  public:
    /*! The x parameter
     *
     * Depending on the type(), this is either
     * the x position of the point, or something
     * else.
     */
    double x() const { return x_; }

    /*! The y parameter
     *
     * Depending on the type(), this is either
     * the y position of the point, or something
     * else.
     */
    double y() const { return y_; }
    
    /*! The type of the segment
     */
    SegmentType type() const { return type_; }

    bool operator== (const Segment& other) const;
    bool operator!= (const Segment& other) const;

  private:
    Segment(double x, double y, SegmentType type);

    double x_, y_;
    SegmentType type_;

    friend class WPainterPath;
    friend WPainterPath WTransform::map(const WPainterPath& path) const;
  };

  const std::vector<Segment>& segments() const { return segments_; }

  /* Returns the start position before drawing segment i */
  WPointF positionAtSegment(int i) const;

  bool asRect(WRectF& result) const;

  /*! \brief Returns the bounding box of the control points.
   *
   * Returns the bounding box of all control points. This is guaranteed to
   * be a superset of the actual bounding box.
   *
   * The \p transform is applied to the path first.
   */
  WRectF controlPointRect(const WTransform& transform = WTransform::Identity)
    const;

  /*! \brief Returns a copy of the path where straight lines are moved to be rendered crisply.
   *
   * This is intended to be used on rectangles, or other paths consisting of only
   * straight line, and will nudge every edge a little bit, so that 1px straight lines are
   * rendered as a crisp line.
   *
   * This will also work if the path \link isJavaScriptBound() is JavaScript bound\endlink.
   */
  WPainterPath crisp() const;

  /*! \brief Disables automatically closing subpaths on moveTo
   *
   * By default, open sub paths are disabled, and moveTo and any operation that
   * relies on moveTo will automatically close the last subpath. Enabling this
   * option disables that feature.
   *
   * \sa moveTo(), addPath(), connectPath(), addRect()
   */
  void setOpenSubPathsEnabled(bool enabled = true);

  /*! \brief Returns whether open subpaths are enabled.
   *
   * \sa setOpenSubPathsEnabled(bool)
   */
  bool openSubPathsEnabled() const { return openSubPathsEnabled_; }

  bool isPointInPath(const WPointF &p) const;

  virtual std::string jsValue() const override;

protected:
  virtual void assignFromJSON(const Json::Value &value) override;

private:
  bool isRect_;
  bool openSubPathsEnabled_;
  std::vector<Segment> segments_;

  WPointF getSubPathStart() const;
  WPointF beginPosition() const;

  static WPointF getArcPosition(double cx, double cy, double rx, double ry,
				double angle);

  void arcTo(double x, double y, double width, double height,
	     double startAngle, double sweepLength);

  friend class WSvgImage;
  friend WPainterPath WTransform::map(const WPainterPath& path) const;
};

}

#endif // WPAINTERPATH_H_
