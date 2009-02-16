/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <stdio.h>
#include "gd.h"

#include <Wt/WMemoryResource>

#include "CornerImage.h"

CornerImage::CornerImage(Corner c, WColor fg, WColor bg,
			 int radius, WContainerWidget *parent)
  : WImage(parent),
    corner_(c),
    fg_(fg),
    bg_(bg),
    radius_(radius),
    resource_(0)
{
  compute();
}

CornerImage::~CornerImage()
{
  if (resource_) {
    delete resource_;
  }
}

void CornerImage::setRadius(int radius)
{
  if (radius != radius_) {
    radius_ = radius;
    compute();
  }
}

void CornerImage::setForeground(WColor color)
{
  if (fg_ != color) {
    fg_ = color;
    compute();
  }
}

void CornerImage::compute()
{
  /* We want an anti-aliased image: oversample twice */
  int AA = 2;

  gdImagePtr imBig = gdImageCreate(radius_ * AA, radius_ * AA);

  /* automatically becomes the background color -- gd documentation */
  gdImageColorAllocate(imBig, bg_.red(), bg_.green(), bg_.blue());

  int fgColor
    = gdImageColorAllocate(imBig, fg_.red(), fg_.green(), fg_.blue());

  int cx, cy;

  if (corner_ & Top)
    cy = radius_ * AA - 1;
  else
    cy = 0;

  if (corner_ & Left)
    cx = radius_ * AA - 1;
  else
    cx = 0;

  gdImageFilledArc(imBig, cx, cy, (radius_*2 - 1) * AA, (radius_*2 - 1) * AA,
		   0, 360, fgColor, gdArc);

  /* now create the real image, downsampled by a factor of 2 */
  gdImagePtr im = gdImageCreateTrueColor(radius_, radius_);
  gdImageCopyResampled(im, imBig, 0, 0, 0, 0, im->sx, im->sy,
		       imBig->sx, imBig->sy);

  /* and generate an in-memory png file */
  int size;
  char *data;
  data = (char *) gdImagePngPtr(im, &size);
  if (!data) {
    return;
    /* Error */
  }

  std::vector<char> vdata(data, data + size);
  if (resource_) {
    resource_->setData(vdata);
  } else {
    /* create and set the memory resource that contains the image */
    resource_ = new WMemoryResource("image/png", vdata);
    setResource(resource_);
  }

  gdFree(data);  

  gdImageDestroy(im);
  gdImageDestroy(imBig);
}

