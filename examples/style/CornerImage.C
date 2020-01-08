/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <stdio.h>

#include "CornerImage.h"

#include <Wt/WPainter.h>
#include <Wt/WRasterImage.h>
#include <Wt/WWidget.h>

class CornerResource : public WResource
{
public:
  CornerResource(CornerImage *img)
    : WResource(),
    img_(img)
  { }

  virtual ~CornerResource()
  {
    beingDeleted();
  }

  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) {
    WRasterImage device("png", img_->radius(), img_->radius());
    paint(&device, img_);

    device.handleRequest(request, response);
  }

  void paint(WPaintDevice *device, CornerImage *img)
  {
    WPainter painter(device);

    painter.setPen(WPen(PenStyle::None));

    painter.setBrush(img->background());
    painter.drawRect(0, 0, img->radius(), img->radius());

    double cx, cy;

    if (img->corner() == Corner::TopLeft ||
	img->corner() == Corner::TopRight)
      cy = img->radius() + 0.5;
    else
      cy = -0.5;

    if (img->corner() == Corner::TopLeft ||
	img->corner() == Corner::BottomLeft)
      cx = img->radius() + 0.5;
    else
      cx = -0.5;

    painter.setBrush(img->foreground());
    painter.drawEllipse(cx - img->radius() - 0.5, cy - img->radius() - 0.5,
			2 * img->radius(), 2 * img->radius());    
  }
private:
  CornerImage *img_;
};

CornerImage::CornerImage(Corner c, WColor fg, WColor bg, int radius)
  : WImage(),
    corner_(c),
    fg_(fg),
    bg_(bg),
    radius_(radius)
{
  resource_ = std::make_shared<CornerResource>(this);
  setImageLink(WLink(resource_));
}

void CornerImage::setRadius(int radius)
{
  if (radius != radius_) {
    radius_ = radius;
    resource_->setChanged();
  }
}

void CornerImage::setForeground(WColor color)
{
  if (fg_ != color) {
    fg_ = color;
    resource_->setChanged();
  }
}
