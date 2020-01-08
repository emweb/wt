// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPAINTDEVICE_H_
#define WPAINTDEVICE_H_

#include <Wt/WLength.h>
#include <Wt/WString.h>
#include <Wt/WGlobal.h>

namespace Wt {

/*! \brief Enumeration to communicate painter state changes.
 *
 * \sa setChanged(WFlags<ChangeFlag>)
 */
enum class PainterChangeFlag {
  Pen = 0x1,         //!< Properties of the pen have changed
  Brush = 0x2,       //!< Properties of the brush have changed
  Font = 0x4,        //!< Properties of the font have changed
  Hints = 0x8,       //!< Some render hints have changed
  Transform = 0x10,  //!< The transformation has changed
  Clipping = 0x20,   //!< The clipping has changed
  Shadow = 0x40      //!< Properties of the shadow have changed
};

/*! \brief Enumeration to indicate paint device features.
 *
 * \sa features()
 */
enum class PaintDeviceFeatureFlag {
  WordWrap = 0x1,    //!< Implements drawText() with Wt::TextFlag::WordWrap
  FontMetrics = 0x2  //!< Implements fontMetrics() and measureText()
};

class WFontMetrics;
class WPainter;
class WPainterPath;
class WRectF;

/*! \class WTextItem Wt/WPaintDevice.h Wt/WPaintDevice.h
 *  \brief The result of a font metrics computation.
 *
 * \sa WPaintDevice::measureText()
 */
class WT_API WTextItem
{
public:
  /*! \brief Constructor.
   */
  WTextItem(const WString& text, double width, double nextWidth = -1);

  /*! \brief Returns the measured text.
   *
   * If the item was measured with word breaking enabled, then the text may
   * contain trailing whitespace that is not included in the width().
   */
  WString text() const { return text_; }

  /*! \brief Returns the measured width.
   *
   * Returns the text width, in device local coordinates (pixels).
   */
  double width() const { return width_; }

  /*! \brief Returns the width for a next line-break boundary.
   *
   * Returns the width until the next line-break boundary, or -1 if the
   * underlying word boundary analysis does not support this.
   */
  double nextWidth() const { return nextWidth_; }

private:
  WString text_;
  double width_, nextWidth_;
};

/*! \class WPaintDevice Wt/WPaintDevice.h Wt/WPaintDevice.h
 *  \brief The abstract base class for a paint device
 *
 * A %WPaintDevice is a device on which may be painted using a
 * WPainter. You should never paint directly on a paint device.
 *
 * The device defines the size of the drawing area, using width() and
 * height(). These dimensions must be defined in pixel units. In the
 * future, additional information will be included to convert these pixel
 * units to lengths (using DPI information).
 *
 * You should reimplement this class if you wish to extend the %Wt
 * paint system to paint on other devices than the ones provided by
 * the library.
 *
 * <i>Note: this interface is subject to changes to increase
 * optimization possibilities for the painting using different
 * devices.</i>
 *
 * \sa WPainter
 *
 * \ingroup painting
 */
class WT_API WPaintDevice
{
public:
#ifndef WT_TARGET_JAVA
  /*! \brief Typedef for enum Wt::PaintDeviceFeatureFlag */
  typedef PaintDeviceFeatureFlag FeatureFlag;
#endif // WT_TARGET_JAVA

  /*! \brief Destructor.
   *
   * Frees all resources associated with this device.
   */
  virtual ~WPaintDevice();

  /*! \brief Returns device features.
   */
  virtual WFlags<PaintDeviceFeatureFlag> features() const = 0;

  /*! \brief Returns the device width.
   *
   * The device width, in pixels, establishes the width of the device
   * coordinate system.
   */
  virtual WLength width() const = 0;

  /*! \brief Returns the device height.
   *
   * The device height, in pixels, establishes the height of the device
   * coordinate system.
   */
  virtual WLength height() const = 0;

  /*! \brief Indicates changes in painter state.
   *
   * The \p flags argument is the logical OR of one or more change flags.
   *
   * \sa ChangeFlag
   */
  virtual void setChanged(WFlags<PainterChangeFlag> flags) = 0;

  /*! \brief Draws an arc.
   *
   * The arc describes the segment of an ellipse enclosed by the
   * rect. The segment starts at \p startAngle, and spans an angle
   * given by \p spanAngle. These angles have as unit degree, and are
   * measured counter-clockwise starting from the 3 o'clock position.
   *
   * The arc must be stroked, filled, and transformed using the
   * current painter settings.
   */
  virtual void drawArc(const WRectF& rect, double startAngle, double spanAngle)
    = 0;

  /*! \brief Draws an image.
   *
   * Draws <i>sourceRect</i> from the image with URL \p imageUri
   * and original dimensions <i>imgWidth</i> and \p imgHeight to
   * the location, into the rectangle defined by \p rect.
   *
   * The image is transformed using the current painter settings.
   */
  virtual void drawImage(const WRectF& rect, const std::string& imageUri,
			 int imgWidth, int imgHeight,
			 const WRectF& sourceRect) = 0;

  /*! \brief Draws a line.
   *
   * The line must be stroked and transformed using the current
   * painter settings.
   */
  virtual void drawLine(double x1, double y1, double x2, double y2) = 0;

  /*! \brief Draws a path.
   *
   * The path must be stroked, filled, and transformed using the
   * current painter settings.
   */
  virtual void drawPath(const WPainterPath& path) = 0;

  /*! \brief Draws a rectangle.
   *
   * The rect must be stroked, filled, and transformed using the
   * current painter settings.
   */
  virtual void drawRect(const WRectF& rectangle) = 0;

  /*! \brief Draws text.
   *
   * The text must be rendered, stroked and transformed using the
   * current painter settings.
   *
   * If clipPoint is not null, a check is performed whether
   * the point is inside of the current clip area. If not,
   * the text is not drawn.
   */
  virtual void drawText(const WRectF& rect, 
			WFlags<AlignmentFlag> alignmentFlags,
			TextFlag textFlag,
			const WString& text,
			const WPointF* clipPoint) = 0;

  /*! \brief Measures rendered text size.
   *
   * Returns the bounding rect of the given text when rendered using the
   * current font.
   *
   * If \p maxWidth != -1, then the text is truncated to fit in the
   * width.
   *
   * If \p wordWrap = \c true then text is truncated only at word
   * boundaries. Note that in this case the whitespace at the
   * truncated position is included in the text but not accounted for
   * by the returned width (since usually you will not render the
   * whitespace at the end of a line).
   *
   * Throws a std::logic_error if the underlying device does not
   * provide font metrics.
   */
  virtual WTextItem measureText(const WString& text, double maxWidth = -1,
				bool wordWrap = false) = 0;

  /*! \brief Returns font metrics.
   *
   * This returns font metrics for the current font.
   *
   * Throws a std::logic_error if the underlying device does not
   * provide font metrics.
   */
  virtual WFontMetrics fontMetrics() = 0;

  /*! \brief Initializes the device for painting.
   *
   * This method is called when a WPainter starts painting.
   *
   * \sa WPainter::begin(WPaintDevice *), painter()
   */
  virtual void init() = 0;

  /*! \brief Finishes painting on the device.
   *
   * This method is called when a WPainter stopped painting.
   *
   * \sa WPainter::end()
   */
  virtual void done() = 0;

  /*! \brief Returns whether painting is active.
   *
   * \sa init(), painter()
   */
  virtual bool paintActive() const = 0;

protected:
  /*! \brief Returns the painter that is currently painting on the device.
   *
   * \sa init()
   */
  virtual WPainter *painter() const = 0;

  /*! \brief Sets the painter.
   */
  virtual void setPainter(WPainter *painter) = 0;

  friend class WPainter;
  friend class WPaintedWidget;
  friend class WMeasurePaintDevice;
};

W_DECLARE_OPERATORS_FOR_FLAGS(PainterChangeFlag)
W_DECLARE_OPERATORS_FOR_FLAGS(PaintDeviceFeatureFlag)

}

#endif // WPAINTDEVICE_H_
