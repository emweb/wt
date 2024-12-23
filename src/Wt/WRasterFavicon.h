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

/*! \class WRasterFavicon Wt/WRasterFavicon.h Wt/WRasterFavicon.h
 *  \brief A WFavicon using a WRasterImage as resource.
 *
 * This is a WFavicon that uses a WRasterImage as resource.
 *
 * In its default state, it shows an image present in the docroot,
 * and in its updated state, it shows the same image with a circle.
 *
 * \sa WApplication::setFavicon()
 */
class WT_API WRasterFavicon : public WFavicon
{
public:
  /*! \brief Constructs a WRasterFavicon.
   *
   * Constructs a WRasterFavicon of given image \p type, \p width, and
   * \p height and using the image described by \p info as default
   * favicon.
   *
   * By default, \p type is "png" and \p width and \p height are of 256
   * pixels.
   *
   * \note Like for WRasterImage, the mime type of the favicon is
   *       <tt>"image/"</tt> \p type.
   */
  explicit WRasterFavicon(std::shared_ptr<WAbstractDataInfo> info,
                          const WLength& width = 256,
                          const WLength& height = 256,
                          const std::string& type = "png");

  WRasterFavicon(std::shared_ptr<WAbstractDataInfo> info, const std::string& type);

  /*! \brief Sets the default favicon.
   *
   * Sets the image described by \p info as the new default favicon.
   */
  void setdefaultFavicon(std::shared_ptr<WAbstractDataInfo> info);

  /*! \brief Returns the data info of the default favicon.
   *
   * \sa setdefaultFavicon()
   */
  std::shared_ptr<WAbstractDataInfo> defaultFavicon() const { return defaultFaviconInfo_; }

  /*! \brief Returns the url to the favicon.
   *
   * This returns the url to the WRasterImage, updating the
   * WRasterImage will therefor automatically update the favicon on the
   * client side.
   */
  std::string url() const override { return favicon_->url(); }

  /*! \brief Sets the brush used to paint the outline of the circle.
   *
   * This sets the brush used to paint the outline of the circle added
   * to the favicon when in updated state.
   *
   * \sa update()
   */
  void setOutlineBrush(const WBrush& brush);

  /*! \brief Returns the brush used to paint the outline of the circle.
   *
   * \sa setOutlineBrush()
   */
  WBrush outlineBrush() const { return bigBrush_; }

  /*! \brief Sets the brush used to fill the circle.
   *
   * This sets the brush used to the circle added to the favicon when
   * in updated state.
   *
   * \sa update()
   */
  void setFillBrush(const WBrush& brush);

  /*! \brief Returns the brush used to fill the circle.
   *
   * \sa setFillBrush()
   */
  WBrush fillBrush() const { return smallBrush_; }

protected:
  /*! \brief Returns the WRasterImage.
   *
   * \sa doUpdate(), doReset()
   */
  WRasterImage* favicon() const { return favicon_.get(); }

  /*! \brief Updates the favicon.
   *
   * Draw the circle on the WRasterImage.
   *
   * You can override this function to change what is drawn on the
   * favicon when update is called.
   */
  void doUpdate() override;

  /*! \brief Resets the favicon.
   *
   * Clear the WRasterImage and repaint the default favicon on it.
   */
  void doReset() override;

private:
  std::unique_ptr<WRasterImage> favicon_;
  std::shared_ptr<WAbstractDataInfo> defaultFaviconInfo_;
  WBrush bigBrush_, smallBrush_;

  void init();
  void resetRasterImage();
};

}
#endif //WRASTER_FAVICON_H_