// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPDF_IMAGE_H_
#define WPDF_IMAGE_H_

#include <Wt/WFont.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WResource.h>
#ifdef WT_TARGET_JAVA
#include <Wt/WTransform.h>
#endif //WT_TARGET_JAVA

#include <hpdf.h>

#include <map>

namespace Wt {

  class FontSupport;
  class WTransform;

/*! \class WPdfImage Wt/WPdfImage.h Wt/WPdfImage.h
 *  \brief A paint device for rendering to a PDF.
 *
 * A %WPdfImage paint device should be used in conjunction with a
 * WPainter, and can be used to make a PDF version of a
 * WPaintedWidget's contents.
 *
 * The PDF is generated using <a href="http://libharu.org/">The Haru
 * Free PDF Library</a>, and this class is included in the library
 * only if <tt>libharu</tt> was found during the build of the library.
 *
 * You can use the image as a resource and specialize handleRequest()
 * to paint the contents on the fly. Alternatively can also use
 * write() to serialize to a PDF file (std::ostream). The latter usage
 * is illustrated by the code below:
 *
 * \code Wt::Chart::WCartesianChart
 * *chart = ...
 *
 * Wt::WPdfImage pdfImage("4cm", "3cm");
 * {
 *   Wt::WPainter p(&pdfImage);
 *   chart->paint(p);
 * }
 * std::ofstream f("chart.pdf", std::ios::out | std::ios::binary);
 * pdfImage.write(f);
 * \endcode
 *
 * A constructor is provided which allows the generated PDF image to
 * be embedded directly into a page of a larger <tt>libharu</tt>
 * document, and this approach is used for example by the WPdfRenderer
 * to render XHTML to multi-page PDF files.
 *
 * Font information is embedded in the PDF. Fonts supported are native
 * PostScript fonts (Base-14) (only ASCII-7), or true type fonts
 * (Unicode). See addFontCollection() for more information on how
 * fonts are located and matched to WFont descriptions.
 *
 * This paint device has the following limitations:
 * - images (WPainter::drawImage()) can only be included from local
 * files, and only JPG and PNG images are supported.
 * - drop shadows are not supported.
 *
 * \ingroup painting
 */
class WT_API WPdfImage : public WResource, public WPaintDevice
{
public:
  /*! \brief Create a PDF resource that represents a single-page PDF document.
   *
   * The single page will have a size \p width x \p height.  The PDF
   * will be using the same DPI (72dpi) as is conventionally used for
   * the desktop.
   *
   * The passed width and height (such as 4 cm by 3 cm) can be specified in
   * physical units (e.g. 4cm x 3cm), but this will be converted to pixels using
   * the default DPI used in CSS (96dpi) !
   *
   * \sa write()
   */
  WPdfImage(const WLength& width, const WLength& height);

  /*! \brief Create a PDF paint device to paint inside an existing page.
   *
   * The image will be drawn in the existing page, as an image with lower-left
   * point (\p x, \p y) and size (\p width x \p height).
   */
  WPdfImage(HPDF_Doc pdf, HPDF_Page page,
	    HPDF_REAL x, HPDF_REAL y, HPDF_REAL width, HPDF_REAL height);

  /*! \brief Destructor.
   */
  ~WPdfImage();

  /*! \brief Adds a font collection.
   *
   * If %Wt has been configured to use <tt>libpango</tt>, then font
   * matching and character selection is done by libpango, which is
   * seeded with information on installed fonts by fontconfig. In that
   * case, invocations for this method is ignored. Only TrueType fonts
   * are supported.
   *
   * As of %Wt 3.3.7, only TrueType fonts will be selected by default.
   * Prior to %Wt 3.3.7, you needed to configure fontconfig (which
   * is used by pango) to only return TrueType fonts. This can be
   * done using a fonts.conf configuration %file:
   *
   * \code{.xml}
   * <?xml version='1.0'?>
   * <!DOCTYPE fontconfig SYSTEM 'fonts.dtd'>
   * <fontconfig>
   *   <selectfont>
   *     <rejectfont>
   *       <glob>*.pfb</glob>
   *     </rejectfont>
   *   </selectfont>
   * </fontconfig>
   * \endcode
   *
   * You may need to add more glob patterns to exclude other fonts
   * than TrueType, and also to exclude TrueType fonts which do not
   * work properly with libharu.
   *
   * If %Wt has not been configured to use <tt>libpango</tt>, then
   * this method may be used to indicate the location of TrueType
   * fonts. The main drawback compared to libpango is that font
   * selection is not steered by the need for particular characters,
   * i.e. font selection is independent from the text's need for
   * specific characters. Most TrueType fonts provide only partial
   * unicode support. The provided \p directory will be searched for
   * fonts (currently only TrueType ".ttf" or ".ttc" fonts). TrueType
   * fonts are preferable over Base-14 fonts (which are PDF's default
   * fonts) since they provide partial (or complete) unicode support.
   *
   * When using Base-14 fonts, WString::narrow() will be called on text
   * which may result in loss of information.
   */
  void addFontCollection(const std::string& directory, bool recursive=true);

#ifdef WT_TARGET_JAVA
  void setDeviceTransform(const WTransform& deviceTransform);
#endif //WT_TARGET_JAVA

  virtual WFlags<PaintDeviceFeatureFlag> features() const override;
  virtual void setChanged(WFlags<PainterChangeFlag> flags) override;
  virtual void drawArc(const WRectF& rect, double startAngle, double spanAngle)
    override;
  virtual void drawImage(const WRectF& rect, const std::string& imgUri,
			 int imgWidth, int imgHeight, const WRectF& sourceRect)
    override;
  virtual void drawLine(double x1, double y1, double x2, double y2) override;
  virtual void drawRect(const WRectF& rect) override;
  virtual void drawPath(const WPainterPath& path) override;
  virtual void drawText(const WRectF& rect, 
			WFlags<AlignmentFlag> alignmentFlags, TextFlag textFlag,
			const WString& text, const WPointF *clipPoint)
    override;
  virtual WTextItem measureText(const WString& text, double maxWidth = -1,
				bool wordWrap = false) override;
  virtual WFontMetrics fontMetrics() override;
  virtual void init() override;
  virtual void done() override;
  virtual bool paintActive() const override { return painter_ != nullptr; }

  virtual WLength width() const override { return width_; }
  virtual WLength height() const override { return height_; }

  void errorHandler(HPDF_STATUS error_no, HPDF_STATUS detail_no);

  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) override;

protected:
  virtual WPainter *painter() const override { return painter_; }
  virtual void setPainter(WPainter *painter) override { painter_ = painter; }

private:
  WLength width_, height_;
  WPainter *painter_;
  FontSupport *trueTypeFonts_;

  HPDF_Doc pdf_;
  HPDF_Page page_;
  HPDF_Font font_;
  bool trueTypeFont_;
  std::map<std::string, const char *> ttfFonts_;
  WFont currentFont_;
  std::string currentTtfFont_;
  std::map<int, HPDF_ExtGState> alphaStrokeExtGStateMap_;
  std::map<int, HPDF_ExtGState> alphaFillExtGStateMap_;

  bool myPdf_;
  double x_, y_;

  double fontSize_;

  void paintPath();
  void drawPlainPath(const WPainterPath& path);
  void applyTransform(const WTransform& f);
  void setStrokeColor(WColor color);
  void setFillColor(WColor color);
};

}

#endif // WPDF_IMAGE_H_
