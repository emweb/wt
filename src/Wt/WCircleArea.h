// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCIRCLE_AREA_H_
#define WCIRCLE_AREA_H_

#include <Wt/WAbstractArea.h>

namespace Wt {

  class WPointF;
  class WPoint;

/*! \class WCircleArea Wt/WCircleArea.h Wt/WCircleArea.h
 *  \brief A interactive area in a widget, specified by a circle.
 *
 * The area may be added to a WImage or WPaintedWidget to provide
 * interactivity on a circular area of the image. The circle is
 * specified in pixel coordinates.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WImage *image = addWidget(std::make_unique<Wt::WImage>("images/events.png"));
 * Wt::WCircleArea *area = image->addArea(std::make_unique<Wt::WCircleArea>(20, 30, 15));
 *
 * area->clicked().connect(this, &MyWidget::areaClicked);
 * \endcode
 * \endif
 *
 * \sa WImage::addArea(), WPaintedWidget::addArea()
 * \sa WRectArea, WPolygonArea
 */
class WT_API WCircleArea : public WAbstractArea
{
public:
  /*! \brief Default constructor.
   *
   * Specifies a circular area with center (0, 0) and radius 0. 
   */
  WCircleArea();

  /*! \brief Creates a circular area with given geometry. 
   *
   * The arguments are in pixel units.
   */
  WCircleArea(int x, int y, int radius);

  /*! \brief Sets the center.
   */
  void setCenter(const WPoint& point);

  /*! \brief Sets the center.
   */
  void setCenter(const WPointF& point);

  /*! \brief Sets the center.
   */
  void setCenter(int x, int y);

  /*! \brief Returns the center X coordinate.
   */
  int centerX() const { return static_cast<int>(x_); }

  /*! \brief Returns the center Y coordinate.
   */
  int centerY() const { return static_cast<int>(y_); }

  /*! \brief Sets the radius. 
   */
  void setRadius(int radius);

  /*! \brief Returns the radius.
   */
  int radius() const { return static_cast<int>(r_); }

private:
  double x_, y_, r_;

protected:
  virtual bool updateDom(DomElement& element, bool all) override;
  virtual std::string updateAreaCoordsJS() override;
};

}

#endif // WCIRCLE_AREA_H_
