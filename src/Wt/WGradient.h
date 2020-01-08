// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WGRADIENT_H_
#define WGRADIENT_H_

#include <Wt/WColor.h>
#include <Wt/WLineF.h>
#include <Wt/WPointF.h>

namespace Wt {

/*! \class WGradient Wt/WGradient.h Wt/WGradient.h
 *  \brief A linear or radial gradient.
 *
 * \sa WPen::setGradient(), WBrush::setGradient()
 *
 * \ingroup painting
 */
class WT_API WGradient
{
public:
  /*! \brief A gradient color stop.
   *
   * A color stop is defined by a color and a (relative) position. The
   * interpretation of the position depends on the gradient style.
   *
   * \sa addColorStop()
   */
  class ColorStop {
  public:
    /*! \brief Constructor.
     */
    ColorStop(double position, const WColor& color)
      : position_(position), color_(color)
    { }
  
    /*! \brief Returns the position.
     */
    double position() const { return position_; }

    /*! \brief Returns the color.
     */
    const WColor& color() const { return color_; }

    /*! \brief Comparison operator.
     */
    bool operator==(const ColorStop& other) const {
      return (position_ == other.position_) && (color_ == other.color_);
    }

    /*! \brief Comparison operator.
     */
    bool operator!=(const ColorStop& other) const {
      return !(*this == other);
    }

  private:
    double position_;
    WColor color_;
  };

  /*! \brief Default constructor.
   *
   * Creates an empty linear gradient from (0,0) to (1,1) (without
   * color stops).
   *
   * \sa isEmpty()
   */
  WGradient();

  /*! \brief Returns the gradient style.
   *
   * \sa setLinearGradient(), setRadialGradient()
   */
  GradientStyle style() const { return style_; }

  /*! \brief Returns whether the gradient is empty.
   *
   * A gradient is empty if no color stops are defined.
   */
  bool isEmpty() const { return colorstops_.empty(); }

  /*! \brief Configures a linear gradient.
   *
   * The coordinates describe a line which provides an origin and
   * orientation of the gradient in user-space coordinates.
   */
  void setLinearGradient(double x0, double y0, double x1, double y1);

  /*! \brief Configures a radial gradient.
   *
   * A radial gradient is described by a center, a radial and a focus
   * point. All coordinates are user-space coordinates. 
   */
  void setRadialGradient(double cx, double cy, double r,
			 double fx, double fy);

  /*! \brief Adds a color stop.
   *
   * For a linear gradient, the position is relative to the position on the
   * line (from 0 to 1 corresponding to p0 to p1).
   *
   * For a radial gradient, the position indicates the distance from the
   * center (from 0 to 1 corresponding to center to radius).
   */
  void addColorStop(double position, const WColor& color);

  /*! \brief Adds a color stop.
   *
   * Adds a color stop.
   */
  void addColorStop(const ColorStop& colorstop);

  /*! \brief Removes all color stops.
   *
   * \sa addColorStop()
   */
  void clearColorStops() { colorstops_.clear(); }

  /*! \brief Returns the color stops.
   *
   * \sa addColorStop()
   */
  const std::vector<ColorStop>& colorstops() const { return colorstops_; }

  /*! \brief Returns the line positioning the linear gradient.
   *
   * This returns the line set in setLinearGradient().
   */
  const WLineF& linearGradientVector() const { return gradientVector_; }

  /*! \brief Returns the center of a radial gradient.
   *
   * This returns the center point set in setRadialGradient().
   */
  const WPointF& radialCenterPoint() const { return center_; }

  /*! \brief Returns the focal point of a radial gradient.
   *
   * This returns the focal point set in setRadialGradient().
   */
  const WPointF& radialFocalPoint() const { return focal_; }

  /*! \brief Returns the radius of a radial gradient.
   *
   * This returns the radius set in setRadialGradient().
   */
  double radialRadius() const { return radius_; }

  /*! \brief Comparison operator.
   */
  bool operator==(const WGradient& other) const;

  /*! \brief Comparison operator.
   */
  bool operator!=(const WGradient& other) const;

private:
  GradientStyle style_;
  std::vector<ColorStop> colorstops_;

  // linear gradient parameters
  WLineF gradientVector_;

  // radial gradient parameters
  WPointF center_, focal_;
  double radius_;
};

}

#endif
