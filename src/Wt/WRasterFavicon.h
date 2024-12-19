// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRASTER_FAVICON_H_
#define WRASTER_FAVICON_H_

#include "Wt/WFavicon.h"
#include "Wt/WRasterImage.h"
#include "Wt/WBrush.h"

namespace Wt {

class WT_API WRasterFavicon : public WFavicon
{
public:
  explicit WRasterFavicon(std::shared_ptr<WAbstractDataInfo> info,
                          const WLength& width = 256,
                          const WLength& height = 256,
                          const std::string& type = "png");

  WRasterFavicon(std::shared_ptr<WAbstractDataInfo> info, const std::string& type);

  void setdefaultFavicon(std::shared_ptr<WAbstractDataInfo> info);
  std::shared_ptr<WAbstractDataInfo> defaultFavicon() const { return defaultFaviconInfo_; }

  std::string url() const override { return favicon_->url(); }

  void setBigCirleBrush(const WBrush& brush);
  WBrush bigCirleBrush() const { return bigBrush_; }

  void setSmallCirleBrush(const WBrush& brush);
  WBrush smallCirleBrush() const { return smallBrush_; }

protected:
  WRasterImage* favicon() const { return favicon_.get(); }
  void doUpdate() override;
  void doReset() override;

private:
  std::unique_ptr<WRasterImage> favicon_;
  std::shared_ptr<WAbstractDataInfo> defaultFaviconInfo_;
  WBrush bigBrush_, smallBrush_;

  void init();
};

}
#endif //WRASTER_FAVICON_H_