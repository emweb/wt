// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef RENDER_WTEXT_RENDERER_H_
#define RENDER_WTEXT_RENDERER_H_

#include <Wt/WString.h>
#include <Wt/WWebWidget.h>

namespace Wt {
  /*! \brief Namespace for the \ref render
   */
  namespace Render {

class StyleSheet;
class Block;
struct LayoutBox;

/*! \defgroup render XHTML Rendering (Wt::Render)
 *  \brief An XHTML rendering library implemented using the %Wt \ref painting
 *
 * The rendering library contains an (increasingly less limited)
 * XHTML/CSS layout engine.
 */

/*! \class WTextRenderer Wt/Render/WTextRenderer.h Wt/Render/WTextRenderer.h
 *  \brief An XHTML renderering engine.
 *
 * This class implements a rendering engine for a (subset of)
 * XHTML/CSS. Its intended use is to be able to accurately render the
 * output of the WTextEdit widget (although it handles a more general
 * subset of XHTML/CSS than is required to do just that). Its focus is
 * on high-quality rendering of text-like contents.
 *
 * The following are the main features:
 * - decent rendering of inline (text) contents, floats,
 *   tables, images, ordered lists, unordered lists, in any arbitrary
 *   combination, with mixtures of font sizes, etc...
 * - CSS stylesheet support using in-document &lt;style&gt;, inline style
 *   attributes or using setStyleSheetText().
 * - support for relative and absolute positioned layout contexts
 * - support for (true type) fonts, font styles, font sizes and text decorations
 * - support for text alignment options, padding, margins, borders, background
 *   color (only on block elements) and text colors
 * - supports automatic and CSS page breaks (page-break-after or
 *   page-break-before)
 * - supports font scaling (rendering text at another DPI than the rest)
 * - can be used in conjunction with other drawing instructions to the same
 *   paint device.
 *
 * Some of the main limitations are:
 * - only "display: inline" or "display: block" elements are supported.
 *   "display: none" and "display: inline-block" are not (yet) recognized. 
 * - only colors defined in terms of RGB values are supported: CSS named colors
 *   (e.g. 'blue') are not allowed.
 * - Bidi (Right-to-Left) text rendering is not supported
 *
 * The renderer is not CSS compliant (simply because it is still lacking
 * alot of features), but the subset of CSS that is supported is a pragmatic
 * choice. If things are lacking, let us known in the bug tracker.
 *
 * This class is an abstract class. A concrete class implements the
 * pure virtual methods to create an appropriate WPaintDevice for each
 * page and to provide page dimension information.
 *
 * All coordinates and dimensions in the API below are pixel coordinates.
 *
 * \ingroup render
 */
class WT_API WTextRenderer 
{
public:
  /*! \class Node
   *  \brief A rendering box of a layed out DOM node.
   *
   * \sa paintNode()
   */
  class Node {
  public:
    /*! \brief Returns the element type.
     */
    DomElementType type() const;

    /*! \brief Returns an attribute value.
     *
     * This returns an empty string for an undefined attribute.
     */
    std::string attributeValue(const std::string& attribute) const;

    /*! \brief Returns the page.
     */
    int page() const;

    /*! \brief Returns the x position.
     */
    double x() const;

    /*! \brief Returns the y position.
     */
    double y() const;

    /*! \brief Returns the width.
     */
    double width() const;

    /*! \brief Returns the height.
     */
    double height() const;

    /*! \brief Returns the fragment number.
     *
     * A single DOM node can result in multiple layout boxes:
     * e.g. inline contents can be split over multiple lines,
     * resulting in a layout box for each line, and block layout
     * contents can be split over multiple pages, resulting in a
     * layout box per page.
     */
    int fragment() const;

    /*! \brief Returns the fragment count.
     *
     * \sa fragment()
     */
    int fragmentCount() const;

  private:
    WTextRenderer& renderer_;
    Block& block_;
    LayoutBox& lb_;

    Node(Block& block, LayoutBox& lb, WTextRenderer& renderer);

    Block &block() const { return block_; }
    LayoutBox &lb() const { return lb_; }

    friend class Block;
    friend class WTextRenderer;
  };

  /*! \brief Destructor.
   */
  virtual ~WTextRenderer();

  /*! \brief Renders an XHTML fragment.
   *
   * The text is rendered, starting at position \p y, and flowing down
   * the page. New pages are created using \p startPage() to render
   * more contents on a next page. The return value is the position at
   * which rendering stopped on the last page on which was rendered.
   *
   * This \p y position and returned position are <i>text
   * coordinates</i>, which differ from page coordinates in that they exclude
   * margins.
   *
   * The function returns the end position. You may call this function
   * multiple times.
   *
   * Each invocation to render() has the effect of resetting the
   * logical page numbering used by pageWidth(), pageHeight() and
   * startPage() so that the current page is page 0.
   */
  double render(const WString& text, double y = 0);

  /*! \brief Sets the contents of a cascading style sheet (CSS).
   *
   * This sets the text \p contents to be used as CSS. Any previous CSS
   * declarations are discarded. Returns true if parsing was successful,
   * false if otherwise.
   * If parsing failed, the stylesheet text that was already in use will
   * not have been changed. Use getStyleSheetParseErrors to access parse
   * error information.
   *
   * \warning Only the following CSS selector features are supported:
   * - tag selectors: e.g. span or *
   * - class name selectors: .class
   * - id selectors: \#id
   * - descendant selectors: h1 h2 h3 {}
   * - multiples: h1, h2, h3 {}
   * \code
   * h1.a1#one.a2 h3#two.c {}
   * \endcode
   *
   * \sa getStyleSheetParseErrors()
   */
  bool setStyleSheetText(const WString& contents);

  /*! \brief Appends an external cascading style sheet (CSS).
   *
   * This is an overloaded member, provided for convenience. Equivalent to:
   * \code
   * setStyleSheetText(styleSheetText() + <filename_contents>)
   * \endcode
   *
   * \sa setStyleSheetText()
   */
  bool useStyleSheet(const WString& filename);

  /*! \brief Clears the used stylesheet
   *
   * This is an overloaded member, provided for convenience. Equivalent to:
   * \code
   * setStyleSheetText("")
   * \endcode
   *
   * \sa setStyleSheetText()
   */
  void clearStyleSheet();

  /*! \brief Returns the CSS in use.
   *
   * This returns all the CSS declarations in use.
   *
   * \sa setStyleSheetText()
   */
  WString styleSheetText() const;

  /*! \brief Returns all parse error information of the last call to
   * setStyleSheetText.
   *
   * setStyleSheetText stores all parse errors inside. Use
   * getStyleSheetParseErrors to access information about them.
   * Information is newline(\ n) separated.
   *
   * \sa setStyleSheetText(const WString& contents)
  */
  std::string getStyleSheetParseErrors() const;

  /*! \brief Returns the page text width.
   *
   * This returns the width of the page in which text needs to be rendered,
   * excluding horizontal margins, in pixels.
   *
   * \sa textHeight()
   */
  double textWidth(int page) const;

  /*! \brief Returns the page text height.
   *
   * This returns the height of the page in which text needs to be rendered,
   * excluding vertical margins, in pixels.
   *
   * \sa textWidth()
   */
  double textHeight(int page) const;

  /*! \brief Sets the scaling factor used for font rendering.
   *
   * A scaling can be set for text. The scaling factor has as effect
   * that text font sizes are modified by the scale. Also CSS length
   * units that are defined in terms of font units ("em" and "ex") are
   * scaled accordingly.
   *
   * The default value is 1.
   */
  void setFontScale(double scale);

  /*! \brief Returns the font scaling factor.
   *
   * \sa setFontScale().
   */
  double fontScale() const { return fontScale_; }

  /*! \brief Returns the page width.
   *
   * Returns the total page width (in pixel units), including
   * horizontal margins.
   */
  virtual double pageWidth(int page) const = 0;

  /*! \brief Returns the page height.
   *
   * Returns the total page height (in pixel units), including
   * vertical margins.
   */
  virtual double pageHeight(int page) const = 0;

  /*! \brief Returns the margin.
   *
   * Returns the margin at given side (in pixel units).
   */
  virtual double margin(Side side) const = 0;

  /*! \brief Returns a paint device to render a given page.
   *
   * The render() method calls this function once for each page it wants
   * to render.
   */
  virtual WPaintDevice *startPage(int page) = 0;

  /*! \brief Stops painting on the given page.
   */
  virtual void endPage(WPaintDevice *device) = 0;

  /*! \brief Returns a painter for the current page.
   */
  virtual WPainter *getPainter(WPaintDevice *device) = 0;

  /*! \brief Paints an XHTML node.
   *
   * The default implementation paints the node conforming to the
   * XHTML specification.
   *
   * You may want to specialize this method if you wish to customize
   * (or ignore) the rendering for certain nodes or node types, or if you
   * want to capture the actual layout positions for other processing.
   *
   * The node information contains the layout position at which the node
   * is being painted.
   */
  virtual void paintNode(WPainter& painter, const Node& node);

protected:
 /*! \brief Constructor.
  */
  WTextRenderer();

private:
  WPainter *painter_;
  WPaintDevice *device_;
  double fontScale_;
  WString styleSheetText_;
  std::unique_ptr<StyleSheet> styleSheet_;
  std::string error_;

  WPainter *painter() const { return painter_; }

  friend class Block;
};

  }
}

#endif // RENDER_WTEXT_RENDERER_H_
