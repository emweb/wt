// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRASTER_IMAGE_H_
#define WRASTER_IMAGE_H_

#include <Wt/WPaintDevice.h>
#include <Wt/WResource.h>

#include <list>

namespace Wt {

class FontSupport;
class WTransform;

/*! \class WRasterImage Wt/WRasterImage.h Wt/WRasterImage.h
 *  \brief A paint device for rendering to a raster image.
 *
 * A %WRasterImage paint device provides support for creating raster
 * images (such as PNG or GIF images).
 *
 * It implements two main use-cases:
 *
 *  - When used either in conjunction with a WPainter, it supports
 *    vector graphics operations, and can be used to make a PNG or GIF
 *    version of a WPaintedWidget's contents.
 *
 *  - It also provides a low-level API to color individual pixels using
 *    setPixel(), which directly sets the raster pixels.
 *
 * On Linux, the rendering is provided by <a
 * href="http://www.graphicsmagick.org/">GraphicsMagick</a>, and this
 * class is included in the library only if <tt>libgraphicsmagick</tt>
 * was found during the build of the library. As an alternative, Wt
 * supports certain versions of <a href="https://skia.org/">skia</a>. In
 * general skia is a good and performant renderer, but since it does not
 * have releases, and a not very stable API, we don't recommend it. On
 * Windows, Direct2D is used for rendering, which means that no extra
 * dependencies have to be installed.
 *
 * If %Wt is built to use <tt>libpango</tt> for font support, then text
 * rendering is done using this library. On Windows, DirectWrite is used
 * for font support functions. In other cases, you may want to configure
 * TrueType font search directories using addFontCollection().
 *
 * You can use the image as a resource and specialize handleRequest()
 * to paint the contents on the fly. Alternatively can also use
 * write() to serialize to an image file (std::ostream).
 *
 * The latter usage is illustrated by the code below:
 * \code
 * Wt::Chart::WCartesianChart *chart = ...
 *
 * Wt::WRasterImage pngImage("png", 600, 400);
 * {
 *   Wt::WPainter p(&pngImage);
 *   chart->paint(p);
 * }
 * std::ofstream f("chart.png", std::ios::out | std::ios::binary);
 * pngImage.write(f);
 * \endcode
 *
 * This paint device has the following limitations:
 * - drop shadows are (not yet?) supported.
 * 
 * \ingroup painting
 */
class WT_API WRasterImage : public WResource, public WPaintDevice
{
public:
  /*! \brief Creates a raster image.
   * 
   * \p type indicates an image type. The mime type of the resource is
   * <tt>"image/"</tt> \p type.
   *
   * %Wt supports the following image types (amongst others):
   * - png: Portable Network Graphics
   * - gif: Graphics Interchange Format
   * - bmp: Microsoft windows bitmap
   * - jpeg: Joint Photographic Experts Group JFIF format
   */
  WRasterImage(const std::string& type,
	       const WLength& width, const WLength& height);

  /*! \brief Destructor.
   */
  ~WRasterImage();

  /*! \brief Adds a font collection.
   *
   * If %Wt has been configured to use <tt>libpango</tt>, then font
   * matching and character selection is done by libpango, which is
   * seeded with information on installed fonts by fontconfig. In that
   * case, invocations for this method is ignored.
   * 
   * If %Wt has not been configured to use <tt>libpango</tt>, then
   * this method may be used to indicate the location of true type
   * fonts. The main drawback compared to libpango is that font
   * selection is not steered by the need for particular characters,
   * i.e. font selection is independent from the text's need for
   * specific characters. Most truetype fonts provided only partial
   * unicode support. The provided \p directory will be searched for
   * fonts (currently only TrueType ".ttf" or ".ttc" fonts).
   */
  void addFontCollection(const std::string& directory, bool recursive=true);

  virtual WFlags<PaintDeviceFeatureFlag> features() const override;
  virtual void setChanged(WFlags<PainterChangeFlag> flags) override;
  virtual void drawArc(const WRectF& rect, double startAngle,
		       double spanAngle) override;
  virtual void drawImage(const WRectF& rect, const std::string& imgUri,
			 int imgWidth, int imgHeight,
			 const WRectF& sourceRect) override;
  virtual void drawLine(double x1, double y1, double x2, double y2) override;
  virtual void drawRect(const WRectF& rect) override;
  virtual void drawPath(const WPainterPath& path) override;

  virtual void drawText(const WRectF& rect, 
			WFlags<AlignmentFlag> alignmentFlags,
			TextFlag textFlag,
			const WString& text,
			const WPointF *clipPoint) override;
  virtual WTextItem measureText(const WString& text, double maxWidth = -1,
				bool wordWrap = false) override;
  virtual WFontMetrics fontMetrics() override;
  virtual void init() override;
  virtual void done() override;
  virtual bool paintActive() const override { return painter_ != nullptr; }

  virtual WLength width() const override { return width_; }
  virtual WLength height() const override { return height_; }

  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) override;

  /*! \brief Low-level paint method.
   *
   * Use this method to directly set colors for individual pixels, when
   * using the paint device without a painter.
   */
  void setPixel(int x, int y, const WColor& color);

  /*! \brief Low-level paint method.
   *
   * Use this method to directly get the color for an individual pixel, when
   * using the paint device without a painter.
   */
  WColor getPixel(int x, int y);

  /*! \brief Low-level pixel retrieval
   *
   * This is a more efficient alternative than calling getPixel() for every
   * pixel.
   *
   * Parameter data must point to an allocated memory region of width*height
   * 32-bit words. The pixels will be written in row-order from top-left
   * to bottom-right, in RGBA format.
   */
  void getPixels(void *data);

  /*! \brief Clears the image.
   *
   * Fills the image with a white background, or transparent if possible.
   */
  void clear();

protected:
  virtual WPainter *painter() const override { return painter_; }
  virtual void setPainter(WPainter *painter) override { painter_ = painter; }

private:
  WLength width_, height_;
  WPainter *painter_;

  class Impl;
  std::unique_ptr<Impl> impl_;

};

}

#endif // WRASTER_IMAGE_H_
