// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRECT_AREA_H_
#define WRECT_AREA_H_

#include <Wt/WAbstractArea.h>
#include <Wt/WRectF.h>

namespace Wt {

/*! \class WRectArea Wt/WRectArea.h Wt/WRectArea.h
 *  \brief A interactive area in a widget, specified by a rectangle.
 *
 * The area may be added to a WImage or WPaintedWidget to provide
 * interactivity on a rectangular area of the image. The rectangle
 * is specified in pixel coordinates.
 * 
 * \if cpp
 * \code
 * auto image = std::make_unique<Wt::WImage>("images/family.jpg");
 * auto face = std::make_unique<Wt::WRectArea>(100, 120, 200, 130);
 * face->setToolTip("Uncle Frank");
 * image->addArea(std::move(face));
 * \endcode
 * \endif
 *
 * \sa WImage::addArea(), WPaintedWidget::addArea()
 * \sa WCircleArea, WPolygonArea
 */
class WT_API WRectArea : public WAbstractArea
{
public:
  /*! \brief Default constructor.
   *
   * The default constructor creates a rectangular area spans the
   * whole widget.
   */
  WRectArea();

  /*! \brief Creates a rectangular area with given geometry.
   *
   * The arguments are in pixel units.
   */  
  WRectArea(int x, int y, int width, int height);

  /*! \brief Creates a rectangular area with given geometry.
   *
   * The arguments are in pixel units.
   */  
  WRectArea(double x, double y, double width, double height);

  /*! \brief Creates a rectangular area with given geometry.
   *
   * The \p rect argument is in pixel units.
   */  
  WRectArea(const WRectF& rect);

  /*! \brief Sets the top-left X coordinate.
   */
  void setX(int x);

  /*! \brief Returns the top-left X coordinate.
   */
  int x() const { return static_cast<int>(x_); }

  /*! \brief Sets the top-left Y coordinate.
   */
  void setY(int y);

  /*! \brief Returns the top-left Y coordinate.
   */
  int y() const { return static_cast<int>(y_); }

  /*! \brief Sets the width.
   */
  void setWidth(int width);

  /*! \brief Returns the width.
   */
  int width() const { return static_cast<int>(width_); }

  /*! \brief Sets the height.
   */
  void setHeight(int height);

  /*! \brief Returns the height.
   */
  int height() const { return static_cast<int>(height_); }

private:
  double x_, y_, width_, height_;

protected:
  virtual bool updateDom(DomElement& element, bool all) override;
  virtual std::string updateAreaCoordsJS() override;
};

}

#endif // WRECT_AREA_H_
