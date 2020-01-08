// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPDF_RENDERER_H_
#define WPDF_RENDERER_H_

#include <Wt/Render/WTextRenderer.h>
#include <Wt/WPdfImage.h>

namespace Wt {
  namespace Render {

/*! \class WPdfRenderer Wt/Render/WPdfRenderer.h Wt/Render/WPdfRenderer.h
 *  \brief An XHTML to PDF renderer.
 *
 * This class implements an XHTML to PDF renderer. The rendering engine
 * supports only a subset of XHTML. See the documentation of WTextRenderer
 * for more information.
 *
 * \if cpp
 * The renderer renders to a libharu PDF document (using WPdfImage).
 * \endif
 * \if java
 * The renderer renders to a PDFJet PDF document (using WPdfImage).
 * \endif
 *
 * By default it uses a pixel resolution of 72 DPI, which is the
 * default for a WPdfImage, but differs from the default used by most
 * browsers (which is 96 DPI and has nothing to do with the actual
 * screen resolution). The pixel resolution can be configured using
 * setDpi(). Increasing the resolution has the effect of scaling down
 * the rendering. This can be used in conjunction with setFontScale() to
 * scale the font size differently than other content.
 *
 * \if cpp
 * Usage example:
 *
 * \code
 * extern "C" {
 *   HPDF_STATUS HPDF_UseUTFEncodings(HPDF_Doc pdf);
 * }
 *	
 * HPDF_Doc pdf = HPDF_New(error_handler, 0);
 * HPDF_UseUTFEncodings(pdf); // enables UTF-8 encoding with true type fonts
 * HPDF_Page page = HPDF_AddPage(pdf);
 * HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
 *
 * Render::WPdfRenderer renderer(pdf, page);
 * renderer.setMargin(2.54);
 * renderer.setDpi(96);
 *
 * renderer.render("<p style=\"background-color: #c11\">Hello, world !</p>");
 *
 * HPDF_SaveToFile(pdf, "hello.pdf");
 * HPDF_Free(pdf);
 * \endcode
 * \endif
 *
 * Font information is embedded in the PDF. Fonts supported are native
 * PostScript fonts (Base-14) (only ASCII-7), or true type fonts
 * (Unicode). See addFontCollection() for more information on how
 * fonts are located.
 *
 * \ingroup render
 */
class WT_API WPdfRenderer : public WTextRenderer
{
public:
  /*! \brief Creates a new PDF renderer.
   *
   * The PDF renderer will render on the given \p pdf (starting).
   * If the \p page is not \c 0, then rendering will happen on this
   * first page (and its page sizes will be taken into account).
   *
   * Default margins are 0, and the default DPI is 72.
   */
  WPdfRenderer(HPDF_Doc pdf, HPDF_Page page = nullptr);

  virtual ~WPdfRenderer();

  /*! \brief Sets the page margins.
   *
   * This sets page margins, in \p cm, for one or more \p sides.
   */
  void setMargin(double cm, WFlags<Side> sides = AllSides);

  /*! \brief Sets the resolution.
   *
   * The resolution used between CSS pixels and actual page
   * dimensions. Note that his does not have an effect on the <i>de
   * facto</i> standard CSS resolution of 96 DPI that is used to
   * convert between physical WLength units (like <em>cm</em>,
   * <em>inch</em> and <em>point</em>) and pixels. Instead it has the
   * effect of scaling down or up the rendered XHTML on the page.
   *
   * The dpi setting also affects the pageWidth(), pageHeight(), and
   * margin() pixel calculations.
   *
   * The default resolution is 72 DPI.
   */
  void setDpi(int dpi);

  /*! \brief Adds a font collection.
   *
   * If %Wt has been configured to use <tt>libpango</tt>, then font
   * matching and character selection is done by libpango, and calls
   * to this method are ignored. See WPdfImage::addFontCollection()
   * for more details.
   *
   * If %Wt was not configured to use <tt>libpango</tt>, you will have
   * to add the directories where %Wt should look for fonts. You will
   * also have to specify the required font in the HTML source, e.g.:
   *
   * \if cpp
   * \code
   * Render::WPdfRenderer renderer(pdf, page);
   * // ...
   * renderer.render(Wt::utf8(u8"<p style=\"font-family: 'DejaVuSans', Arial\">\u00e9l\u00e8ve, fen\u00eatre, \u00e2me</p>"));
   * \endcode
   * \endif
   *
   * \if java
   * \code
   * WPdfRenderer renderer = new WPdfRenderer(pdf, page);
   * // ...
   * renderer.render("<p style=\"font-family: 'DejaVuSans', Arial\">\u00E9l\u00E8ve, fen\u00EAtre, \u00E2me</p>");
   * \endcode
   * \endif
   *
   * \sa WPdfImage::addFontCollection()
   */
  void addFontCollection(const std::string& directory, bool recursive=true);

  /*! \brief Sets the current page.
   */
  void setCurrentPage(HPDF_Page page);

  /*! \brief Returns the current page.
   *
   * This returns the page last created using createPage(), or the page
   * set with setCurrentPage() page.
   */
  HPDF_Page currentPage() const { return page_; }

  virtual double pageWidth(int page) const override;
  virtual double pageHeight(int page) const override;

  virtual double margin(Side side) const override;

  virtual WPaintDevice *startPage(int page) override;
  virtual void endPage(WPaintDevice *device) override;
  virtual WPainter *getPainter(WPaintDevice *device) override;

  /*! \brief Creates a new page.
   *
   * The default implementation creates a new page with the same dimensions
   * as the previous page.
   *
   * You may want to specialize this method to add e.g.~headers and footers.
   */
  virtual HPDF_Page createPage(int page);

private:
  struct FontCollection {
    std::string directory;
    bool recursive;
  };

  std::vector<FontCollection> fontCollections_;

  HPDF_Doc  pdf_;
  HPDF_Page page_;

  double margin_[4];
  int dpi_;
  WPainter *painter_;
};

  }
}

#endif // WPDF_RENDER_
