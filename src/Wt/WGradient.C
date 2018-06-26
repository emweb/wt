#include "Wt/WGradient.h"

namespace Wt {

WGradient::WGradient()
  : style_(GradientStyle::Linear),
    gradientVector_(0, 0, 1, 1),
    center_(0, 0),
    focal_(0, 0),
    radius_(1)
{ }

void WGradient::setLinearGradient(double x0, double y0, double x1, double y1)
{
  style_ = GradientStyle::Linear;
  gradientVector_ = WLineF(x0, y0, x1, y1);
}

void WGradient::setRadialGradient(double cx, double cy, double r,
				  double fx, double fy)
{
  style_ = GradientStyle::Radial;
  center_ = WPointF(cx, cy);
  focal_ = WPointF(fx, fy);
  radius_ = r;
}

void WGradient::addColorStop(double position, const WColor& color)
{
  addColorStop(ColorStop(position, color));
}

void WGradient::addColorStop(const ColorStop& colorstop)
{
  for (unsigned i = 0; i < colorstops_.size(); ++i)
    if (colorstop.position() < colorstops_[i].position()) {
      colorstops_.insert(colorstops_.begin() + i, colorstop);
      return;
    }

  colorstops_.push_back(colorstop);
}

bool WGradient::operator==(const WGradient& other) const
{
  if (style_ != other.style_)
    return false;

  if (colorstops_.size() != other.colorstops_.size())
    return false;

  for (unsigned i = 0; i < colorstops_.size(); i++) {
    if (colorstops_[i] != other.colorstops_[i])
      return false;
  }

  if (style_ == GradientStyle::Linear) {
    return gradientVector_ == other.gradientVector_;
  } else if (style_ == GradientStyle::Radial) {
    return (center_ == other.center_) && 
      (focal_ == other.focal_) &&
      (radius_ == other.radius_);
  } else
    return false;
}

bool WGradient::operator!=(const WGradient& other) const
{
  return !(*this == other);
}

}
