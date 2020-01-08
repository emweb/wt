// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPOLYGON_AREA_H_
#define WPOLYGON_AREA_H_

#include <Wt/WAbstractArea.h>
#include <Wt/WPoint.h>
#include <Wt/WPointF.h>

namespace Wt {

/*! \class WPolygonArea Wt/WPolygonArea.h Wt/WPolygonArea.h
 *  \brief An interactive area in a widget, specified by a polygon.
 *
 * The area may be added to a WImage or WPaintedWidget to provide
 * interactivity on a polygon area of the image. The polygon is
 * specified in pixel coordinates, and uses an even-odd winding rule
 * (overlaps create holes).
 * 
 * \if cpp
 * \code
 * auto image = std::make_unique<Wt::WImage>("images/family.jpg");
 * auto face = std::make_unique<Wt::WPolygonArea>();
 * face->addPoint(100, 120);
 * face->addPoint(300, 120);
 * face->addPoint (200, 250);
 * face->setToolTip("Uncle Frank");
 * image->addArea(std::move(face));
 * \endcode
 * \endif
 *
 * The polygon area corresponds to the HTML <tt>&lt;area shape="poly"&gt;</tt>
 * tag.
 *
 * \sa WImage::addArea(), WPaintedWidget::addArea()
 * \sa WCircleArea, WRectArea
 */
class WT_API WPolygonArea : public WAbstractArea
{
public:
  /*! \brief Creates an empty polygon.
   *
   * Defines an empty polygon. 
   */
  WPolygonArea();

  /*! \brief Creates a polygon area with given vertices.
   *
   * The polygon is defined with vertices corresponding to
   * \p points. The polygon is closed by connecting the last point
   * with the first point.
   */
  WPolygonArea(const std::vector<WPoint>& points);

#ifndef WT_TARGET_JAVA
  /*! \brief Creates a polygon area with given vertices.
   *
   * The polygon is defined with vertices corresponding to
   * \p points. The polygon is closed by connecting the last point
   * with the first point.
   */
  WPolygonArea(const std::vector<WPointF>& points);
#endif // WT_TARGET_JAVA

  /*! \brief Adds a point.
   */
  void addPoint(int x, int y);

  /*! \brief Adds a point.
   */
  void addPoint(double x, double y);

  /*! \brief Adds a point.
   */
  void addPoint(const WPoint& point);

  /*! \brief Adds a point.
   */
  void addPoint(const WPointF& point);

  /*! \brief Sets the polygon vertices. 
   *
   * The polygon is defined with vertices corresponding to
   * \p points. The polygon is closed by connecting the last point
   * with the first point.
   */
  void setPoints(const std::vector<WPoint>& points);

#ifndef WT_TARGET_JAVA
  /*! \brief Sets the polygon vertices. 
   *
   * The polygon is defined with vertices corresponding to
   * \p points. The polygon is closed by connecting the last point
   * with the first point.
   */
  void setPoints(const std::vector<WPointF>& points);
#endif // WT_TARGET_JAVA

  /*! \brief Returns the polygon vertices.
   *
   * \sa setPoints(), points()
   */
  const std::vector<WPointF>& pointFs() const { return points_; }

  /*! \brief Returns the polygon vertices.
   *
   * \sa setPoints(), pointFs()
   */
  const std::vector<WPoint>& points() const;

private:
  std::vector<WPointF> points_;
  mutable std::vector<WPoint> pointsIntCompatibility_;

protected:
  virtual bool updateDom(DomElement& element, bool all) override;
  virtual std::string updateAreaCoordsJS() override;
};

}

#endif // WPOLYGON_AREA_H_
