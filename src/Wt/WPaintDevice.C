/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPaintDevice.h"

#include "Wt/WException.h"
#include "Wt/WPainterPath.h"
#include "Wt/WRectF.h"
#include "Wt/WPointF.h"

#include "Wt/WPainter.h"

namespace Wt {

WTextItem::WTextItem(const WString& text, double width, double nextWidth)
  : text_(text),
    width_(width),
    nextWidth_(nextWidth)
{ }

WPaintDevice::~WPaintDevice()
{ }

void WPaintDevice::drawImage(const WRectF& rect, const std::string& imageUri,
                             int imgWidth, int imgHeight,
                             const WRectF& sourceRect)
{
  throw WException("DrawImage not implemented.");
}

void WPaintDevice::drawImage(const WRectF& rect,
                             const WAbstractDataInfo* imageInfo,
                             int imgWidth, int imgHeight,
                             const WRectF& sourceRect)
{
  /*
   * Needed for retrocompatibility in the specific case where
   * Render::Block calls drawImage on a custom WPaintDevice that does
   * not implement drawImage with WAbstractDataInfo.
   */
  drawImage(rect, imageInfo->uri(), imgWidth, imgHeight, sourceRect);
}

}
