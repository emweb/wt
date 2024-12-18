/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WRasterFavicon.h"

#include "Wt/WApplication.h"
#include "Wt/WLogger.h"
#include "Wt/WPainter.h"

namespace Wt {

WRasterFavicon::WRasterFavicon(std::shared_ptr<WAbstractDataInfo> info,
                               const WLength& width,
                               const WLength& height,
                               const std::string& type)
  : favicon_(new WRasterImage(type, width, height)),
    defaultFaviconInfo_(info)
{
  init();
}

WRasterFavicon::WRasterFavicon(std::shared_ptr<WAbstractDataInfo> info,
                               const std::string& type)
  : favicon_(new WRasterImage(type, 256, 256)),
    defaultFaviconInfo_(info)
{
  init();
}

void WRasterFavicon::init()
{
  WRasterFavicon::doReset();
  setBigCirleBrush(WBrush(WColor(StandardColor::White)));
  setSmallCirleBrush(WBrush(WColor((StandardColor::Red))));
}

void WRasterFavicon::setdefaultFavicon(std::shared_ptr<WAbstractDataInfo> info)
{
  defaultFaviconInfo_ = info;
  doReset();
  if (isUpdate()) {
    doUpdate();
  }
}

void WRasterFavicon::setBigCirleBrush(const WBrush& brush)
{
  bigBrush_ = brush;
  if (isUpdate()) {
    doUpdate();
  }
}

void WRasterFavicon::setSmallCirleBrush(const WBrush& brush)
{
  smallBrush_ = brush;
  if (isUpdate()) {
    doUpdate();
  }
}

void WRasterFavicon::doUpdate()
{
  Wt::WPainter p(favicon());

  double bRLeft = favicon_->width().toPixels()/2;
  double bRUp = favicon_->height().toPixels()/2;
  double bRWidth = favicon_->width().toPixels()/2;
  double bRHeight = favicon_->height().toPixels()/2;

  p.setBrush(bigBrush_);
  p.drawEllipse(WRectF(bRLeft, bRUp, bRWidth, bRHeight));

  double widthDif = bRWidth/5;
  double heighDif = bRHeight/5;

  p.setBrush(smallBrush_);
  p.drawEllipse(WRectF(bRLeft + widthDif, bRUp + heighDif,
                       bRWidth - widthDif*2, bRHeight - heighDif*2));
}

void WRasterFavicon::doReset()
{
  favicon_->clear();
  Wt::WPainter p(favicon());
  p.drawImage(WRectF(0, 0, favicon_->width().toPixels(), favicon_->height().toPixels()),
              WPainter::Image(defaultFaviconInfo_));
}


}