/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WPainterPath.h"

#include <limits>

#include <cmath>
#include <cassert>

#include "Wt/WStringStream.h"

#include "WebUtils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
  double degreesToRadians(double r) {
    return (r / 180.) * M_PI;
  }
}

namespace Wt {

WPainterPath::Segment::Segment(double x, double y, SegmentType type)
  : x_(x), y_(y), type_(type)
{ }

bool WPainterPath::Segment::operator== (const Segment& other) const
{
  return type_ == other.type_
    && x_ == other.x_
    && y_ == other.y_;
}

bool WPainterPath::Segment::operator!= (const Segment& other) const
{
  return !(*this == other);
}

WPainterPath::WPainterPath()
  : isRect_(false),
    openSubPathsEnabled_(false)
{ }

WPainterPath::WPainterPath(const WPointF& startPoint)
  : isRect_(false),
    openSubPathsEnabled_(false)
{
  moveTo(startPoint);
}

WPainterPath::WPainterPath(const WPainterPath& path)
  : WJavaScriptExposableObject(path),
    isRect_(path.isRect_),
    openSubPathsEnabled_(path.openSubPathsEnabled_)
#ifndef WT_TARGET_JAVA
    ,segments_(path.segments_)
#endif
{
  #ifdef WT_TARGET_JAVA
  segments_ = path.segments_;
  #endif
}

WPainterPath& WPainterPath::operator= (const WPainterPath& path)
{
#ifndef WT_TARGET_JAVA
  WJavaScriptExposableObject::operator=(path);
#else
  if (path.isJavaScriptBound()) assignBinding(path);
#endif

  segments_ = path.segments_;
  isRect_ = path.isRect_;

  return *this;
}

#ifdef WT_TARGET_JAVA
WPainterPath WPainterPath::clone() const
{
  return WPainterPath(*this);
}
#endif

WPointF WPainterPath::getArcPosition(double cx, double cy,
				     double rx, double ry,
				     double angle)
{
  /*
   * angles are counter-clockwise, which means against the logic of
   * the downward X-Y system
   */
  double a = -degreesToRadians(angle);

  return WPointF(cx + rx * std::cos(a), cy + ry * std::sin(a));
}

WPointF WPainterPath::beginPosition() const
{
  WPointF result(0, 0);

  for (unsigned int i = 0;
       i < segments_.size() && segments_[i].type() == MoveTo;
       ++i)
    result = WPointF(segments_[i].x(), segments_[i].y());

  return result;
}

WPointF WPainterPath::currentPosition() const
{
  return positionAtSegment(segments_.size());
}

WPointF WPainterPath::positionAtSegment(int index) const
{
  if (index > 0) {
    const Segment& s = segments_[index - 1];
    switch (s.type()) {
    case MoveTo:
    case LineTo:
    case CubicEnd:
    case QuadEnd:
      return WPointF(s.x(), s.y());
    case ArcAngleSweep: {
      int i = segments_.size() - 3;
      double cx = segments_[i].x();
      double cy = segments_[i].y();
      double rx = segments_[i+1].x();
      double ry = segments_[i+1].y();
      double theta1 = segments_[i+2].x();
      double deltaTheta = segments_[i+2].y();

      return getArcPosition(cx, cy, rx, ry, theta1 + deltaTheta);
    }
    default:
      assert(false);
    }
  }

  return WPointF(0, 0);
}

WPointF WPainterPath::getSubPathStart() const
{
  /*
   * Find start point of last sub path, which is the point of the last
   * moveTo operation, or either (0, 0).
   */
  for (int i = segments_.size() - 1; i >= 0; --i)
    if (segments_[i].type() == MoveTo)
      return WPointF(segments_[i].x(), segments_[i].y());

  return WPointF(0, 0);
}

void WPainterPath::closeSubPath()
{
  checkModifiable();
  moveTo(0, 0);
}

bool WPainterPath::isEmpty() const
{
  for (unsigned i = 0; i < segments_.size(); ++i)
    if (segments_[i].type() != MoveTo)
      return false;

  return true;
}

bool WPainterPath::operator==(const WPainterPath& path) const
{
  if (segments_.size() != path.segments_.size())
    return false;

  for (unsigned i = 0; i < segments_.size(); ++i)
    if (segments_[i] != path.segments_[i])
      return false;

  return true;
}

bool WPainterPath::operator!=(const WPainterPath& path) const
{
  return !(*this == path);
}

void WPainterPath::moveTo(const WPointF& point)
{
  moveTo(point.x(), point.y());
}

void WPainterPath::moveTo(double x, double y)
{
  checkModifiable();
  /*
   * first close previous sub path
   */
  if (!openSubPathsEnabled_ &&
      !segments_.empty() && 
      segments_.back().type() != MoveTo) {
    WPointF startP = getSubPathStart();
    WPointF currentP = currentPosition();

    if (startP != currentP)
      lineTo(startP.x(), startP.y());
  }

  segments_.push_back(Segment(x, y, MoveTo));  
}

void WPainterPath::lineTo(const WPointF& point)
{
  lineTo(point.x(), point.y());
}

void WPainterPath::lineTo(double x, double y)
{
  checkModifiable();
  segments_.push_back(Segment(x, y, LineTo));
}

void WPainterPath::cubicTo(const WPointF& c1, const WPointF& c2,
			   const WPointF& endPoint)
{
  cubicTo(c1.x(), c1.y(), c2.x(), c2.y(), endPoint.x(), endPoint.y());
}

void WPainterPath::cubicTo(double c1x, double c1y, double c2x, double c2y,
			   double endPointx, double endPointy)
{
  checkModifiable();
  segments_.push_back(Segment(c1x, c1y, CubicC1));
  segments_.push_back(Segment(c2x, c2y, CubicC2));
  segments_.push_back(Segment(endPointx, endPointy, CubicEnd));
}

void WPainterPath::arcTo(double cx, double cy, double radius,
			 double startAngle, double sweepLength)
{
  arcTo(cx - radius, cy - radius, radius * 2, radius * 2,
	startAngle, sweepLength);
}

void WPainterPath::arcTo(double x, double y, double width, double height,
			 double startAngle, double sweepLength)
{
  checkModifiable();
  segments_.push_back(Segment(x + width/2, y + height/2, ArcC));
  segments_.push_back(Segment(width/2, height/2, ArcR));
  segments_.push_back(Segment(startAngle, sweepLength, 
			      ArcAngleSweep));
}

void WPainterPath::arcMoveTo(double cx, double cy, double radius, double angle)
{
  moveTo(getArcPosition(cx, cy, radius, radius, angle));
}

void WPainterPath::arcMoveTo(double x, double y, double width, double height,
			     double angle)
{
  moveTo(getArcPosition(x + width/2, y + height/2, width/2, height/2, angle));
}

void WPainterPath::quadTo(double cx, double cy,
			  double endPointX, double endPointY)
{
  checkModifiable();
  segments_.push_back(Segment(cx, cy, QuadC));
  segments_.push_back(Segment(endPointX, endPointY, QuadEnd));
}

void WPainterPath::quadTo(const WPointF& c, const WPointF& endPoint)
{
  quadTo(c.x(), c.y(), endPoint.x(), endPoint.y());
}

void WPainterPath::addEllipse(double x, double y, double width, double height)
{
  moveTo(x + width, y + height/2);
  arcTo(x, y, width, height, 0, 360);
}

void WPainterPath::addEllipse(const WRectF& rect)
{
  addEllipse(rect.x(), rect.y(), rect.width(), rect.height());
}

void WPainterPath::addRect(double x, double y, double width, double height)
{
  checkModifiable();
  if (isEmpty())
    isRect_ = true;

  moveTo(x, y);
  lineTo(x + width, y);
  lineTo(x + width, y + height);
  lineTo(x, y + height);
  lineTo(x, y);
}

void WPainterPath::addRect(const WRectF& rectangle)
{
  addRect(rectangle.x(), rectangle.y(), rectangle.width(), rectangle.height());
}

void WPainterPath::addPolygon(const std::vector<WPointF>& points)
{
  checkModifiable();
  if (!points.empty()) {
    unsigned i = 0;
    if (currentPosition() != points[0]) 
      moveTo(points[i++]);

    for (; i < points.size(); ++i)
      lineTo(points[i]);
  }
}

void WPainterPath::addPath(const WPainterPath& path)
{
  checkModifiable();
  if (currentPosition() != path.beginPosition())
    moveTo(path.beginPosition());

  Utils::insert(segments_, path.segments_);
}

void WPainterPath::connectPath(const WPainterPath& path)
{
  checkModifiable();
  if (currentPosition() != path.beginPosition())
    lineTo(path.beginPosition());

  addPath(path);
}

bool WPainterPath::asRect(WRectF& result) const
{
  if (isRect_) {
    if (segments_.size() == 4) {
      result.setX(0);
      result.setY(0);
      result.setWidth(segments_[0].x());
      result.setHeight(segments_[1].y());
      return true;
    } else if (segments_.size() == 5
	       && segments_[0].type() == MoveTo) {
      result.setX(segments_[0].x());
      result.setY(segments_[0].y());
      result.setWidth(segments_[1].x() - segments_[0].x());
      result.setHeight(segments_[2].y() - segments_[0].y());
      return true;
    } else
      return false;
  } else
    return false;
}

WRectF WPainterPath::controlPointRect(const WTransform& transform) const
{
  if (isEmpty())
    return WRectF();
  else {
    bool identity = transform.isIdentity();

    double minX, minY, maxX, maxY;
    minX = minY = std::numeric_limits<double>::max();
    maxX = maxY = std::numeric_limits<double>::min();

    for (unsigned i = 0; i < segments_.size(); ++i) {
      const Segment& s = segments_[i];

      switch (s.type()) {
      case MoveTo:
      case LineTo:
      case CubicC1:
      case CubicC2:
      case CubicEnd:
      case QuadC:
      case QuadEnd: {
	if (identity) {
	  minX = std::min(s.x(), minX);
 	  minY = std::min(s.y(), minY);
	  maxX = std::max(s.x(), maxX);
	  maxY = std::max(s.y(), maxY);
	} else {
	  WPointF p = transform.map(WPointF(s.x(), s.y()));
	  minX = std::min(p.x(), minX);
 	  minY = std::min(p.y(), minY);
	  maxX = std::max(p.x(), maxX);
	  maxY = std::max(p.y(), maxY);
	}
	break;
      }
      case ArcC: {
	const Segment& s2 = segments_[i+1];

	if (identity) {
	  WPointF tl(s.x() - s2.x(), s.y() - s2.y());
	  minX = std::min(tl.x(), minX);
	  minY = std::min(tl.y(), minY);

	  WPointF br(s.x() + s2.x(), s.y() + s2.y());
	  maxX = std::max(br.x(), maxX);
	  maxY = std::max(br.y(), maxY);
	} else {
	  WPointF p1 = transform.map(WPointF(s.x(), s.y()));
	  WPointF p2 = transform.map(WPointF(s2.x(), s2.y()));

	  WPointF tl(p1.x() - p2.x(), p1.y() - p2.y());
	  minX = std::min(tl.x(), minX);
	  minY = std::min(tl.y(), minY);

	  WPointF br(p1.x() + p2.x(), p1.y() + p2.y());
	  maxX = std::max(br.x(), maxX);
	  maxY = std::max(br.y(), maxY);
	}

	i += 2;
	break;
      }
      default:
	assert(false);
      }
    }

    return WRectF(minX, minY, maxX - minX, maxY - minY);
  }
}

std::string WPainterPath::jsValue() const
{
  char buf[30];
  WStringStream ss;
  ss << '[';
  for (std::size_t i = 0; i < segments_.size(); ++i) {
    const Segment &s = segments_[i];
    if (i != 0) ss << ',';
    ss << '[';
    ss << Utils::round_js_str(s.x(), 3, buf) << ',';
    ss << Utils::round_js_str(s.y(), 3, buf) << ',';
    ss << (int)s.type()
       << ']';
  }
  ss << ']';
  return ss.str();
}

WPainterPath WPainterPath::crisp() const
{
  WPainterPath result;

  if (isJavaScriptBound()) {
    result.assignBinding(*this,
	WT_CLASS ".gfxUtils.path_crisp(" + jsRef() + ')');
  }

  for (std::size_t i = 0; i < segments_.size(); ++i) {
    const Segment &segment = segments_[i];
    double hx = std::floor(segment.x()) + 0.5;
    double hy = std::floor(segment.y()) + 0.5;
    result.segments_.push_back(Segment(hx, hy, segment.type()));
  }

  return result;
}

void WPainterPath::setOpenSubPathsEnabled(bool enabled)
{
  openSubPathsEnabled_ = enabled;
}

bool WPainterPath::isPointInPath(const WPointF &p) const
{
  bool res = false;
  double ax = 0.0, ay = 0.0;
  double px = p.x(), py = p.y();
  for (std::size_t i = 0; i < segments_.size(); ++i) {
    double bx = ax;
    double by = ay;
    if (segments_[i].type() == ArcC) {
      WPointF arcPos = getArcPosition(segments_[i].x(), segments_[i].y(),
				      segments_[i+1].x(), segments_[i+1].y(),
				      segments_[i+2].x());
      bx = arcPos.x();
      by = arcPos.y();
    } else if (segments_[i].type() == ArcAngleSweep) {
      WPointF arcPos = getArcPosition(segments_[i-2].x(), segments_[i-2].y(),
				      segments_[i-1].x(), segments_[i-1].y(),
				      segments_[i].x() + segments_[i].y());
      bx = arcPos.x();
      by = arcPos.y();
    } else if (segments_[i].type() != ArcR) {
      bx = segments_[i].x();
      by = segments_[i].y();
    }
    if (segments_[i].type() != MoveTo) {
      if ( (ay > py) != (by > py) &&
	   (px < (bx - ax) * (py - ay) / (by - ay) + ax) ) {
	res = !res;
      }
    }
    ax = bx;
    ay = by;
  }
  return res;
}

void WPainterPath::assignFromJSON(const Json::Value &value) {}

}
