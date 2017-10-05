// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPAINTER_H_
#define WPAINTER_H_

#include <Wt/WBrush.h>
#include <Wt/WFont.h>
#include <Wt/WGlobal.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPainterPath.h>
#include <Wt/WPen.h>
#include <Wt/WShadow.h>
#include <Wt/WTransform.h>

namespace Wt {

class WLineF;
class WPaintDevice;
class WPainterPath;
class WPointF;
class WRectF;

/*! \defgroup painting Painting system
    \brief Classes that provide support for vector graphics painting

  %Wt provides a vector graphics painting system, which depending on
  the browser support, uses one of four different methods to paint the
  graphics (inline SVG, inline VML, HTML 5 &lt;canvas&gt; or a raster
  image). Vector graphics has as benefit a lower bandwidth usage
  compared to raster images, indepedent of the image size. To use the
  paint system, you need to specialize WPaintedWidget and use a
  WPainter to paint the contents of the widget inside its
  WPaintedWidget::paintEvent().

  In addition, a PDF backend is included in the library, which can be used
  to make a PDF version of a painting, or to embed a painting in a
  PDF document.

  To use inline SVG, you need to enable xhtml support in your
  configuration file by enabling send-xhtml-mimetype, see \ref
  config_general.
*/

/*! \brief Enumeration for render hints
 */
enum class RenderHint {
  Antialiasing = 0x1,     //!< Antialiasing
  LowQualityShadows = 0x2 //!< Use low-quality shadows (applies only to VML)
};

/*! \class WPainter Wt/WPainter.h Wt/WPainter.h
 *  \brief Vector graphics painting class.
 *
 * The painter class provides a vector graphics interface for
 * painting. It needs to be used in conjunction with a WPaintDevice,
 * onto which it paints. To start painting on a device, either pass
 * the device through the constructor, or use begin().
 *
 * A typical use is to instantiate a %WPainter from within a
 * specialized WPaintedWidget::paintEvent() implementation, to paint
 * on the given paint device, but you can also use a painter to paint
 * directly to a particular paint device of choice, for example to
 * create SVG, PDF or PNG images (as resources).
 *
 * The painter maintains state such as the current \link setPen()
 * pen\endlink, \link setBrush() brush\endlink, \link setFont()
 * font\endlink, \link shadow() shadow\endlink, \link worldTransform()
 * transformation\endlink and clipping settings (see setClipping() and
 * setClipPath()). A particular state can be saved using save() and
 * later restored using restore().
 *
 * The painting system distinguishes between device coordinates,
 * logical coordinates, and local coordinates. The device coordinate
 * system ranges from (0, 0) in the top left corner of the device, to
 * (device->width().toPixels(), device->height().toPixels()) for the
 * bottom right corner. The logical coordinate system defines a
 * coordinate system that may be chosen independent of the geometry of
 * the device, which is convenient to make abstraction of the actual
 * device size. Finally, the current local coordinate system may be
 * different from the logical coordinate system because of a
 * transformation set (using translate(), rotate(), and
 * scale()). Initially, the local coordinate system coincides with the
 * logical coordinate system, which coincides with the device
 * coordinate system.
 *
 * The device coordinates are defined in terms of pixels. Even though
 * most underlying devices are actual vector graphics formats, when
 * used in conjunction with a WPaintedWidget, these vector graphics
 * are rendered by the browser onto a pixel-based canvas (like the
 * rest of the user-interface). The coordinates are defined such that
 * integer values correspond to an imaginary raster which separates
 * the individual pixels, as in the figure below.
 *
 * \image html WPainter.png "The device coordinate system for a 6x5 pixel device"
 *
 * As a consequence, to avoid anti-aliasing effects when drawing
 * straight lines of width one pixel, you will need to use vertices
 * that indicate the middle of a pixel to get a crisp one-pixel wide
 * line, as in the example figure.
 *
 * By setting a viewPort() and a window(), a viewPort transformation
 * is defined which maps logical coordinates onto device
 * coordinates. By changing the world transformation (using
 * setWorldTransform(), or translate(), rotate(), scale() operations),
 * it is defined how current local coordinates map onto logical
 * coordinates.
 *
 * The painter provides support for clipping using an arbitrary \link
 * WPainterPath path\endlink, but not that the %WVmlImage paint device
 * only has limited support for clipping.
 *
 * \if cpp
 * Usage example:
 * \code
 * class MyPaintedWidget : public Wt::WPaintedWidget
 * {
 * public:
 *   MyPaintedWidget()
 *     : foo_(100)
 *   {
 *      resize(200, 200); // provide a default size
 *   }
 *
 *   void setFoo(int foo) {
 *      foo_ = foo;
 *      update(); // trigger a repaint
 *   }
 *
 * protected:
 *   void paintEvent(Wt::WPaintDevice *paintDevice) {
 *     Wt::WPainter painter(paintDevice);
 *     painter.drawLine(20, 20, foo_, foo_);
 *     ...
 *   }
 *
 * private:
 *   int foo_;
 * };
 * \endcode
 * \endif
 *
 * \sa WPaintedWidget::paintEvent(WPaintDevice *)
 *
 * \ingroup painting
 */
class WT_API WPainter
{
public:
  /*! \brief Typedef for enum Wt::PainterChangeFlag */
  typedef PainterChangeFlag ChangeFlag;

  /*! \brief Default constructor.
   *
   * Before painting, you must invoke begin(WPaintDevice *) on a paint device.
   *
   * \sa WPainter(WPaintDevice *)
   */
  WPainter();

  /*! \brief Creates a painter on a given paint device.
   */
  WPainter(WPaintDevice *device);

  /*! \brief Destructor.
   */
  ~WPainter();

  /*! \brief Begins painting on a paint device.
   *
   * \sa end(), isActive()
   */
  bool begin(WPaintDevice *device);

  /*! \brief Returns whether this painter is active on a paint device.
   *
   * \sa begin(WPaintDevice *), end()
   */
  bool isActive() const;

  /*! \brief Ends painting.
   *
   * \if cpp
   * This method is called automatically from the destructor.
   * \endif
   */
  bool end();

  /*! \brief Returns the device on which this painter is active (or 0 if not active).
   *
   * \sa begin(WPaintDevice *), WPainter(WPaintDevice *), isActive()
   */
  WPaintDevice *device() const { return device_; }

  /*! \brief Sets a render hint.
   *
   * Renderers may ignore particular hints for which they have no
   * support.
   */
  void setRenderHint(RenderHint hint, bool on = true);

  /*! \brief Returns the current render hints.
   *
   * Returns the logical OR of render hints currently set.
   *
   * \sa setRenderHint(RenderHint, bool).
   */
  WFlags<RenderHint> renderHints() const { return s().renderHints_; }

  /*! \brief Draws an arc.
   *
   * Draws an arc using the current pen, and fills using the current brush.
   *
   * The arc is defined as a segment from an ellipse, which fits in
   * the <i>rectangle</i>. The segment starts at \p startAngle, and
   * spans an angle given by \p spanAngle. These angles have as
   * unit 1/16th of a degree, and are measured counter-clockwise
   * starting from the 3 o'clock position.
   *
   * \sa drawEllipse(const WRectF&), drawChord(const WRectF&, int, int)
   * \sa drawArc(double, double, double, double, int, int)
   */
  void drawArc(const WRectF& rectangle, int startAngle, int spanAngle);

  /*! \brief Draws an arc.
   *
   * This is an overloaded method for convenience.
   *
   * \sa drawArc(const WRectF&, int, int)
   */
  void drawArc(double x, double y, double width, double height,
	       int startAngle, int spanAngle);

  /*! \brief Draws a chord.
   *
   * Draws an arc using the current pen, and connects start and end
   * point with a line. The area is filled using the current brush.
   *
   * The arc is defined as a segment from an ellipse, which fits in
   * the <i>rectangle</i>. The segment starts at \p startAngle, and
   * spans an angle given by \p spanAngle. These angles have as
   * unit 1/16th of a degree, and are measured counter-clockwise
   * starting at 3 o'clock.
   *
   * \sa drawEllipse(const WRectF&), drawArc(const WRectF&, int, int)
   * \sa drawChord(double, double, double, double, int, int)
   */
  void drawChord(const WRectF& rectangle, int startAngle, int spanAngle);

  /*! \brief Draws a chord.
   *
   * This is an overloaded method for convenience.
   *
   * \sa drawChord(const WRectF&, int, int)
   */
  void drawChord(double x, double y, double width, double height,
		 int startAngle, int spanAngle);

  /*! \brief Draws an ellipse.
   *
   * Draws an ellipse using the current pen and fills it using the
   * current brush.
   *
   * The ellipse is defined as being bounded by the \p rectangle.
   *
   * \sa drawArc(const WRectF&, int, int)
   * \sa drawEllipse(double, double, double, double)
   */
  void drawEllipse(const WRectF& rectangle);

  /*! \brief Draws an ellipse.
   *
   * This is an overloaded method for convenience.
   *
   * \sa drawEllipse(const WRectF&)
   */
  void drawEllipse(double x, double y, double width, double height);

  /*! \brief An image that can be rendered on a WPainter.
   *
   * The image is specified in terms of a URL, and the width and
   * height.
   *
   * \sa drawImage()
   */
  class WT_API Image {
  public:
    /*! \brief Creates an image.
     *
     * Create an image which is located at the <i>uri</i>, and which has
     * dimensions <i>width</i> x <i>height</i>.
     */
    Image(const std::string& url, int width, int height);

    /*! \brief Creates an image.
     *
     * Create an image which is located at <i>uri</i> which is available on
     * the local filesystem as <i>file</i>. The image dimensions are
     * retrieved from the file.
     */
    Image(const std::string& url, const std::string& file);

    /*! \brief Returns the url.
     */
    std::string uri() const { return url_; }

    /*! \brief Returns the image width.
     */
    int width() const { return width_; }

    /*! \brief Returns the image height.
     */
    int height() const { return height_; }

  private:
    std::string url_;
    int width_, height_;

    void setUrl(const std::string& url);
  };

  /*! \brief Draws an image.
   *
   * Draws the \p image so that the top left corner corresponds to
   * \p point.
   *
   * This is an overloaded method provided for convenience.
   */
  void drawImage(const WPointF& point, const Image& image);

  /*! \brief Draws part of an image.
   *
   * Draws the \p sourceRect rectangle from an image to the
   * location \p point.
   *
   * This is an overloaded method provided for convenience.
   */
  void drawImage(const WPointF& point, const Image& image,
		 const WRectF& sourceRect);

  /*! \brief Draws an image inside a rectangle.
   *
   * Draws the <i>image</i> inside \p rect (If necessary, the image
   * is scaled to fit into the rectangle).
   *
   * This is an overloaded method provided for convenience.
   */
  void drawImage(const WRectF& rect, const Image& image);

  /*! \brief Draws part of an image inside a rectangle.
   *
   * Draws the \p sourceRect rectangle from an image inside
   * \p rect (If necessary, the image is scaled to fit into the
   * rectangle).
   */
  void drawImage(const WRectF& rect, const Image& image,
		 const WRectF& sourceRect);

  /*! \brief Draws part of an image.
   *
   * Draws the \p sourceRect rectangle with top left corner
   * (<i>sx</i>, <i>sy</i>) and size <i>sw</i> x \p sh from an
   * image to the location (<i>x</i>, \p y).
   */
  void drawImage(double x, double y, const Image& image,
		 double sx = 0, double sy = 0, double sw = -1, double sh = -1);

  /*! \brief Draws a line.
   *
   * Draws a line using the current pen.
   *
   * \sa drawLine(const WPointF&, const WPointF&),
   *     drawLine(double, double, double, double)
   */  
  void drawLine(const WLineF& line);

  /*! \brief Draws a line.
   *
   * Draws a line defined by two points.
   *
   * \sa drawLine(const WLineF&),
   *     drawLine(double, double, double, double)
   */  
  void drawLine(const WPointF& p1, const WPointF& p2);

  /*! \brief Draws a line.
   *
   * Draws a line defined by two points.
   *
   * \sa drawLine(const WLineF&),
   *     drawLine(const WPointF&, const WPointF&)
   */  
  void drawLine(double x1, double y1, double x2, double y2);

  /*! \brief Draws an array of lines.
   *
   * Draws the \p lineCount first lines from the given array of lines.
   */  
  void drawLines(const WT_ARRAY WLineF *lines, int lineCount);

  /*! \brief Draws an array of lines.
   *
   * Draws \p lineCount lines, where each line is specified using
   * a begin and end point that are read from an array. Thus, the
   * <i>pointPairs</i> array must have at least 2*\p lineCount
   * points.
   */
  void drawLines(const WT_ARRAY WPointF *pointPairs, int lineCount);

  /*! \brief Draws an array of lines.
   *
   * Draws the lines given in the vector.
   */  
  void drawLines(const std::vector<WLineF>& lines);

  /*! \brief Draws an array of lines.
   *
   * Draws a number of lines that are specified by pairs of begin- and
   * endpoints. The vector should hold a number of points that is a
   * multiple of two.
   */  
  void drawLines(const std::vector<WPointF>& pointPairs);

  /*! \brief Draws a (complex) path.
   *
   * Draws and fills the given path using the current pen and brush.
   *
   * \sa strokePath(const WPainterPath&, const WPen&),
   *     fillPath(const WPainterPath&, const WBrush&)
   */  
  void drawPath(const WPainterPath& path);

  /*! \brief Draws a WPainterPath on every anchor point of a path.
   *
   * Draws the first WPainterPath on every anchor point of the second path.
   * When rendering to an HTML canvas, this will cause far less JavaScript
   * to be generated than separate calls to drawPath. Also, it's possible
   * for either path to be
   * \link WJavaScriptExposableObject::isJavaScriptBound JavaScript bound\endlink
   * through, e.g. applying a JavaScript bound WTransform without deforming
   * the other path. This is used by WCartesianChart to draw data series markers
   * that don't change size when zooming in.
   *
   * If one of the anchor points of the path is outside of the current clipping
   * area, the stencil will be drawn if softClipping is disabled, and it will
   * not be drawn when softClipping is enabled.
   */
  void drawStencilAlongPath(const WPainterPath &stencil,
			    const WPainterPath &path,
			    bool softClipping = false);

  /*! \brief Draws a pie.
   *
   * Draws an arc using the current pen, and connects start and end
   * point with the center of the corresponding ellipse. The area is
   * filled using the current brush.
   *
   * The arc is defined as a segment from an ellipse, which fits in
   * the <i>rectangle</i>. The segment starts at \p startAngle, and
   * spans an angle given by \p spanAngle. These angles have as
   * unit 1/16th of a degree, and are measured counter-clockwise
   * starting at 3 o'clock.
   *
   * \sa drawEllipse(const WRectF&), drawArc(const WRectF&, int, int)
   * \sa drawPie(double, double, double, double, int, int)
   */
  void drawPie(const WRectF& rectangle, int startAngle, int spanAngle);

  /*! \brief Draws a pie.
   *
   * This is an overloaded method for convenience.
   *
   * \sa drawPie(const WRectF&, int, int)
   */
  void drawPie(double x, double y, double width, double height,
	       int startAngle, int spanAngle);

  /*! \brief Draws a point.
   *
   * Draws a single point using the current pen. This is implemented
   * by drawing a very short line, centered around the given \p
   * position. To get the result of a single point, you should use a
   * pen with a Wt::PenCapStyle::Square or Wt::PenCapStyle::Round pen cap style.
   *
   * \sa drawPoint(double, double)
   */
  void drawPoint(const WPointF& position);

  /*! \brief Draws a point.
   *
   * This is an overloaded method for convenience.
   *
   * \sa drawPoint(const WPointF&)
   */
  void drawPoint(double x, double y);

  /*! \brief Draws a number of points.
   *
   * Draws the \p pointCount first points from the given array of points.
   *
   * \sa drawPoint(const WPointF&)
   */
  void drawPoints(const WT_ARRAY WPointF *points, int pointCount);

  /*! \brief Draws a polygon.
   *
   * Draws a polygon that is specified by a list of points, using the
   * current pen. The polygon is closed by connecting the last point
   * with the first point, and filled using the current brush.
   *
   * \sa drawPath(const WPainterPath&), drawPolyline()
   */
  void drawPolygon(const WT_ARRAY WPointF *points, int pointCount
		   /*, FillRule fillRule */);

  /*! \brief Draws a polyline.
   *
   * Draws a polyline that is specified by a list of points, using the
   * current pen.
   *
   * \sa drawPath(const WPainterPath&), drawPolygon()
   */
  void drawPolyline(const WT_ARRAY WPointF *points, int pointCount);

  /*! \brief Draws a rectangle.
   *
   * Draws and fills a rectangle using the current pen and brush.
   *
   * \sa drawRect(double, double, double, double)
   */
  void drawRect(const WRectF& rectangle);

  /*! \brief Draws a rectangle.
   *
   * This is an overloaded method for convenience.
   *
   * \sa drawRect(const WRectF&)
   */
  void drawRect(double x, double y, double width, double height);

  /*! \brief Draws a number of rectangles.
   *
   * Draws and fills the \p rectCount first rectangles from the
   * given array, using the current pen and brush.
   *
   * \sa drawRect(const WRectF&)
   */
  void drawRects(const WT_ARRAY WRectF *rectangles, int rectCount);

  /*! \brief Draws a number of rectangles.
   *
   * Draws and fills a list of rectangles using the current pen and
   * brush.
   *
   * \sa drawRect(const WRectF&)
   */
  void drawRects(const std::vector<WRectF>& rectangles);
  
  /*! \brief Draws text.
   *
   * Draws text using inside the rectangle, using the current font. The
   * text is aligned inside the rectangle following alignment
   * indications given in \p flags. The text is drawn using the
   * current transformation, pen color (pen()) and font settings
   * (font()).
   *
   * AlignmentFlags is the logical OR of a horizontal and vertical
   * alignment. Orientation::Horizontal alignment may be one of AlignmentFlag::Left,
   * AlignmentFlag::Center, or AlignmentFlag::Right. Orientation::Vertical alignment is one of
   * AlignmentFlag::Top, AlignmentFlag::Middle or AlignmentFlag::Bottom.
   *
   * TextFlag determines how the text is rendered in the rectangle.
   * Text can be rendered on one line or by wrapping the words within the 
   * rectangle.
   *
   * If a clipPoint is provided, the text will not be drawn
   * if the point is outside of the clipPath().
   *
   * \note If the clip path is not a polygon, i.e. it has rounded segments
   * in it, an approximation will be used instead by having the
   * polygon pass through the control points, and the begin and
   * end point of arcs.
   *
   * \note HtmlCanvas: on older browsers implementing Html5 canvas,
   * text will be rendered horizontally (unaffected by rotation and
   * unaffected by the scaling component of the transformation
   * matrix). In that case, text is overlayed on top of painted shapes
   * (in DOM div's), and is not covered by shapes that are painted
   * after the text. Use the SVG and VML renderers
   * (WPaintedWidget::inlineSvgVml) for the most accurate font
   * rendering. Native HTML5 text rendering is supported on Firefox3+,
   * Chrome2+ and Safari4+.
   *
   * \note TextFlag::WordWrap: using the TextFlag::WordWrap TextFlag is currently only 
   * supported by the SVG backend. The code generated by the SVG backend uses
   * features currently only supported by Inkscape. Inkscape currently supports
   * only Side::Top vertical alignments.
   */
  void drawText(const WRectF& rect, 
		WFlags<AlignmentFlag> alignmentFlags,
		TextFlag textFlag,
		const WString& text,
		const WPointF *clipPoint = nullptr);

  void drawTextOnPath(const WRectF& rect,
		      WFlags<AlignmentFlag> alignmentFlags,
		      const std::vector<WString> &text,
		      const WTransform &transform,
		      const WPainterPath &path,
		      // lineHeight could be calculated inside of this
		      // method, but let's leave it like this for now,
		      // with this method undocumented.
		      double angle, double lineHeight,
		      bool softClipping);

  /*! \brief Draws text.
   *
   * This is an overloaded method for convenience, it will render text on a 
   * single line.
   *
   * \sa drawText(const WRectF&, WFlags<AlignmentFlag>, TextFlag textFlag, const WString&)
   */
  void drawText(const WRectF& rectangle, WFlags<AlignmentFlag> flags,
		const WString& text);
  
  /*! \brief Draws text.
   *
   * This is an overloaded method for convenience.
   *
   * \sa drawText(const WRectF&, WFlags<AlignmentFlag>, const WString&)
   */
  void drawText(double x, double y, double width, double height,
		WFlags<AlignmentFlag> flags, const WString& text);
  
  /*! \brief Draws text.
   *
   * This is an overloaded method for convenience.
   *
   * \sa drawText(const WRectF& rect, 
   *		 WFlags<AlignmentFlag> alignmentFlags,
   *		 TextFlag textFlag,
   *		 const WString& text)
   */
  void drawText(double x, double y, double width, double height, 
		WFlags<AlignmentFlag> alignmentFlags,
		TextFlag textFlag,
		const WString& text);

  /*! \brief Fills a (complex) path.
   *
   * Like drawPath(const WPainterPath&), but does not stroke the path,
   * and fills the path with the given \p brush.
   *
   * \sa drawPath(const WPainterPath&), strokePath(const WPainterPath&, const WPen&)
   */
  void fillPath(const WPainterPath& path, const WBrush& brush);

  /*! \brief Fills a rectangle.
   *
   * Like drawRect(const WRectF&), but does not stroke the rect, and
   * fills the rect with the given \p brush.
   *
   * \sa drawRect(const WRectF&)
   */
  void fillRect(const WRectF& rectangle, const WBrush& brush);

  /*! \brief Fills a rectangle.
   *
   * This is an overloaded method for convenience.
   *
   * \sa fillRect(const WRectF&, const WBrush&)
   */
  void fillRect(double x, double y, double width, double height,
		const WBrush& brush);

  /*! \brief Strokes a path.
   *
   * Like drawPath(const WPainterPath&), but does not fill the path,
   * and strokes the path with the given \p pen.
   *
   * \sa drawPath(const WPainterPath&), fillPath(const WPainterPath&, const WBrush&)
   */
  void strokePath(const WPainterPath& path, const WPen& pen);

  /*! \brief Sets a shadow effect.
   *
   * The shadow effect is applied to all things drawn (paths, text and images).
   *
   * \note With the VML backend (IE), the shadow is not applied to images,
   *       and the shadow color is always StandardColor::Black; only the opacity (alpha)
   *       channel is taken into account.
   * \sa LowQualityShadows
   */
  void setShadow(const WShadow& shadow);

  /*! \brief Returns the current shadow effect.
   *
   * \sa setShadow()
   */
  const WShadow& shadow() const { return s().currentShadow_; }

  /*! \brief Sets the fill style.
   *
   * Changes the fills style for subsequent draw operations.
   *
   * \sa brush(), setPen(const WPen&)
   */
  void setBrush(const WBrush& brush);

  /*! \brief Sets the font.
   *
   * Changes the font for subsequent text rendering. Note that only
   * font sizes that are defined as an explicit size (see
   * FontSize::FixedSize) will render correctly in all devices (SVG, VML,
   * and HtmlCanvas).
   *
   * The list of fonts that will render correctly with VML (on IE<9) are
   * limited to the following:
   * http://www.ampsoft.net/webdesign-l/WindowsMacFonts.html
   *
   * Careful, for a font family that contains a space, you need to add
   * quotes, to WFont::setFamily() e.g.
   *
   * \code
   * WFont mono;
   * mono.setFamily(FontFamily::Monospace, "'Courier New'");
   * mono.setSize(18);
   * \endcode
   *
   * \sa font(), drawText()
   */
  void setFont(const WFont& font);

  /*! \brief Sets the pen.
   *
   * Changes the pen used for stroking subsequent draw operations.
   *
   * \sa pen(), setBrush(const WBrush&)
   */
  void setPen(const WPen& pen);

  /*! \brief Returns the current brush.
   *
   * Returns the brush style that is currently used for filling.
   *
   * \sa setBrush(const WBrush&)
   */
  const WBrush& brush() const { return s().currentBrush_; }

  /*! \brief Returns the current font.
   *
   * Returns the font that is currently used for rendering text.
   * The default font is a 10pt sans serif font.
   *
   * \sa setFont(const WFont&)
   */
  const WFont& font() const { return s().currentFont_; }

  /*! \brief Returns the current pen.
   *
   * Returns the pen that is currently used for stroking.
   *
   * \sa setPen(const WPen&)
   */
  const WPen& pen() const { return s().currentPen_; }

  /*! \brief Enables or disables clipping.
   *
   * Enables are disables clipping for subsequent operations using the
   * current clip path set using setClipPath().
   *
   * \note Clipping support is limited for the VML renderer.
   *    Only clipping with a rectangle is supported for the VML
   *    renderer (see WPainterPath::addRect()). The rectangle must,
   *    after applying the combined transformation system, be aligned
   *    with the window.
   *
   * \sa hasClipping(), setClipPath(const WPainterPath&)
   */
  void setClipping(bool enable);

  /*! \brief Returns whether clipping is enabled.
   *
   * \note Clipping support is limited for the VML renderer.
   *
   * \sa setClipping(bool), setClipPath(const WPainterPath&)
   */
  bool hasClipping() const { return s().clipping_; }

  /*! \brief Sets the clip path.
   *
   * Sets the path that is used for clipping subsequent drawing
   * operations. The clip path is only used when clipping is enabled
   * using setClipping(bool). The path is specified in local
   * coordinates.
   *
   * \note Clipping support is limited for the VML renderer.
   *
   * \sa clipPath(), setClipping(bool)
   */
  void setClipPath(const WPainterPath& clipPath);

  /*! \brief Returns the clip path.
   *
   * The clip path is returned as it was defined: in the local
   * coordinates at time of definition.
   *
   * \sa setClipPath(const WPainterPath&)
   */
  WPainterPath clipPath() const { return s().clipPath_; }

  /*! \brief Resets the current transformation.
   *
   * Resets the current transformation to the identity transformation
   * matrix, so that the logical coordinate system coincides with the
   * device coordinate system.
   */
  void resetTransform();

  /*! \brief Rotates the logical coordinate system.
   *
   * Rotates the logical coordinate system around its origin. The
   * \p angle is specified in degrees, and positive values are
   * clock-wise.
   *
   * \sa scale(double, double), translate(double, double), resetTransform()
   */
  void rotate(double angle);

  /*! \brief Scales the logical coordinate system.
   *
   * Scales the logical coordinate system around its origin, by a factor
   * in the X and Y directions.
   *
   * \sa rotate(double), translate(double, double), resetTransform()
   */
  void scale(double sx, double sy);

  /*! \brief Translates the origin of the logical coordinate system.
   *
   * Translates the origin of the logical coordinate system to a new
   * location relative to the current logical coordinate system.
   *
   * \sa translate(double, double), rotate(double),
   *     scale(double, double), resetTransform()
   */
  void translate(const WPointF& offset);

  /*! \brief Translates the origin of the logical coordinate system.
   *
   * Translates the origin of the logical coordinate system to a new
   * location relative to the logical coordinate system.
   *
   * \sa translate(const WPointF& offset), rotate(double),
   *     scale(double, double), resetTransform()
   */
  void translate(double dx, double dy);

  /*! \brief Sets a transformation for the logical coordinate system.
   *
   * Sets a new transformation which transforms logical coordinates to
   * device coordinates. When \p combine is \c true, the
   * transformation is combined with the current world transformation
   * matrix.
   *
   * \sa worldTransform()
   * \sa rotate(double), scale(double, double), translate(double, double)
   * \sa resetTransform()
   */
  void setWorldTransform(const WTransform& matrix, bool combine = false);

  /*! \brief Returns the current world transformation matrix.
   *
   * \sa setWorldTransform()
   */
  const WTransform& worldTransform() const { return s().worldTransform_; }

  /*! \brief Saves the current state.
   *
   * A copy of the current state is saved on a stack. This state will
   * may later be restored by popping this state from the stack using
   * restore().
   *
   * The state that is saved is the current \link setPen()
   * pen\endlink, \link setBrush() brush\endlink, \link setFont()
   * font\endlink, \link shadow() shadow\endlink, \link
   * worldTransform() transformation\endlink and clipping settings
   * (see setClipping() and setClipPath()).
   *
   * \sa restore()
   */
  void save();

  /*! \brief Returns the last save state.
   *
   * Pops the last saved state from the state stack.
   *
   * \sa save()
   */
  void restore();

  /*! \brief Sets the viewport.
   *
   * Selects the part of the device that will correspond to the logical
   * coordinate system.
   *
   * By default, the viewport spans the entire device: it is the
   * rectangle (0, 0) to (device->width(), device->height()). The
   * window defines how the viewport is mapped to logical coordinates.
   *
   * \sa viewPort(), setWindow(const WRectF&)
   */
  void setViewPort(const WRectF& viewPort);

  /*! \brief Sets the viewport.
   *
   * This is an overloaded method for convenience.
   *
   * \sa setViewPort(const WRectF&)
   */
  void setViewPort(double x, double y, double width, double height);

  /*! \brief Returns the viewport.
   *
   * \sa setViewPort(const WRectF&)
   */
  WRectF viewPort() const { return viewPort_; }

  /*! \brief Sets the window.
   *
   * Defines the viewport rectangle in logical coordinates, and thus how
   * logical coordinates map onto the viewPort.
   *
   * By default, is (0, 0) to (device->width(), device->height()). Thus,
   * the default window and viewport leave logical coordinates identical
   * to device coordinates.
   *
   * \sa window(), setViewPort(const WRectF&)
   */
  void setWindow(const WRectF& window);

  /*! \brief Sets the window.
   *
   * This is an overloaded method for convenience.
   *
   * \sa setWindow(const WRectF&)
   */
  void setWindow(double x, double y, double width, double height);

  /*! \brief Returns the current window.
   *
   * \sa setViewPort(const WRectF&)
   */
  WRectF window() const { return window_; }

  /*! \brief Returns the combined transformation matrix.
   *
   * Returns the transformation matrix that maps coordinates to device
   * coordinates. It is the combination of the current world
   * transformation (which defines the transformation within the
   * logical coordinate system) and the window/viewport transformation
   * (which transforms logical coordinates to device coordinates).
   *
   * \sa setWorldTransform(), setViewPort(), setWindow()
   */
  WTransform combinedTransform() const;

  const WTransform& clipPathTransform() const;

  WLength normalizedPenWidth(const WLength& penWidth, bool correctCosmetic)
    const;

private:
  WPainter(const WPainter&);

  WPaintDevice *device_;
  WRectF        viewPort_, window_;
  WTransform    viewTransform_;

  struct State {
    WTransform    worldTransform_;
    WBrush        currentBrush_;
    WFont         currentFont_;
    WPen          currentPen_;
    WShadow       currentShadow_;
    WFlags<RenderHint> renderHints_;
    WPainterPath  clipPath_;
    WTransform    clipPathTransform_;
    bool          clipping_;

    State();

#ifdef WT_TARGET_JAVA
    State clone();
#endif
  };

  std::vector<State> stateStack_;

  State& s() { return stateStack_.back(); }
  const State& s() const { return stateStack_.back(); }

  void recalculateViewTransform();

  void drawMultilineText(const WRectF& rect, 
			 WFlags<AlignmentFlag> alignmentFlags,
			 const WString& text);
};

}

/*! @} */

#endif // WPAINTER_H_
