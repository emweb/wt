/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <math.h>

#include "Wt/WPaintDevice"
#include "Wt/WPainter"
#include "Wt/WTransform"

namespace Wt {

WPaintDevice::WPaintDevice(const WLength& width, const WLength& height)
  : width_(width),
    height_(height),
    painter_(0)
{ }

WPaintDevice::~WPaintDevice()
{ }

WLength WPaintDevice::normalizedPenWidth(const WLength& penWidth,
					 bool correctCosmetic) const
{
  double w = penWidth.value();

  if (w == 0 && correctCosmetic) {
    // cosmetic width -- must be untransformed 1 pixel
    const WTransform& t = painter()->combinedTransform();
    if (!t.isIdentity()) {
      double dx, dy, alpha1, sx, sy, alpha2;
      t.decomposeTranslateRotateScaleRotate(dx, dy, alpha1, sx, sy, alpha2);

      w = 2.0/(fabs(sx) + fabs(sy));
    } else
      w = 1.0;

    return WLength(w, WLength::Pixel);
  } else if (w != 0 && !correctCosmetic) {
    // non-cosmetic width -- must be transformed
    const WTransform& t = painter()->combinedTransform();
    if (!t.isIdentity()) {
      double dx, dy, alpha1, sx, sy, alpha2;
      t.decomposeTranslateRotateScaleRotate(dx, dy, alpha1, sx, sy, alpha2);

      w *= (fabs(sx) + fabs(sy))/2.0;
    }

    return WLength(w, WLength::Pixel);
  } else
    return penWidth;
}

}
