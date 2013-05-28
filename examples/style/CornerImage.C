/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <stdio.h>

#include "CornerImage.h"

#include <Wt/WPainter>
#include <Wt/WRasterImage>

class CornerResource : public WResource
{
public:
  CornerResource(CornerImage *parent)
    : WResource(parent)
  { }

  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) {
    CornerImage *img = dynamic_cast<CornerImage *>(parent());
    WRasterImage device("png", img->radius(), img->radius());
    paint(&device, img);

    device.handleRequest(request, response);
  }

  void paint(WPaintDevice *device, CornerImage *img)
  {
    WPainter painter(device);

    painter.setPen(NoPen);

    painter.setBrush(img->background());
    painter.drawRect(0, 0, img->radius(), img->radius());

    double cx, cy;

    if (img->corner() & Top)
      cy = img->radius() + 0.5;
    else
      cy = -0.5;

    if (img->corner() & Left)
      cx = img->radius() + 0.5;
    else
      cx = -0.5;

    painter.setBrush(img->foreground());
    painter.drawEllipse(cx - img->radius() - 0.5, cy - img->radius() - 0.5,
			2 * img->radius(), 2 * img->radius());    
  }
};

CornerImage::CornerImage(Corner c, WColor fg, WColor bg,
			 int radius, WContainerWidget *parent)
  : WImage(parent),
    corner_(c),
    fg_(fg),
    bg_(bg),
    radius_(radius)
{
  resource_ = new CornerResource(this);
  setImageLink(resource_);
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
