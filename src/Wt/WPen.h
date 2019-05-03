// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPEN_H_
#define WPEN_H_

#include <Wt/WLength.h>
#include <Wt/WColor.h>
#include <Wt/WGradient.h>
#include <Wt/WJavaScriptExposableObject.h>

namespace Wt {

/*! \class WPen Wt/WPen.h Wt/WPen.h
 *  \brief A value class that defines the style for pen strokes
 *
 * A pen defines the properties of how lines (that may surround
 * shapes) are rendered.
 *
 * A pen with width 0 is a <i>cosmetic</i> pen, and is always rendered
 * as 1 pixel width, regardless of transformations. Otherwized, the
 * pen width is modified by the \link WPainter::worldTransform()
 * transformation\endlink set on the painter.
 *
 * <h3>JavaScript exposability</h3>
 *
 * A %WPen is JavaScript exposable. If a %WPen \link isJavaScriptBound()
 * is JavaScript bound\endlink, it can be accessed in your custom JavaScript
 * code through \link WJavaScriptHandle::jsRef() its handle's jsRef()\endlink.
 * At the moment, only the color() property is exposed, e.g. a pen
 * with the color WColor(10,20,30,255) will be represented in JavaScript as:
 * \code
 * {
 *   color: [10,20,30,255]
 * }
 * \endcode
 *
 * \warning A %WPen that is JavaScript exposed should be modified only through its \link WJavaScriptHandle handle\endlink.
 *	    Any attempt at modifying it will cause an exception to be thrown.
 *
 * \sa WPainter::setPen(), WBrush, WPaintedWidget::createJSPen()
 *
 * \ingroup painting
 */
class WT_API WPen : public WJavaScriptExposableObject
{
public:
  /*! \brief Typedef for enum Wt::PenStyle */
  typedef PenStyle Style;
  /*! \brief Typedef for enum Wt::PenCapStyle */
  typedef PenCapStyle CapStyle;
  /*! \brief Typedef for enum Wt::PenJoinStyle */
  typedef PenJoinStyle JoinStyle;

  /*! \brief Creates a black cosmetic pen.
   *
   * Constructs a black solid pen of 0 width (i.e. cosmetic single
   * pixel width), with \link Wt::PenCapStyle::Square PenCapStyle::Square\endlink line
   * ends and \link Wt::PenJoinStyle::Bevel PenJoinStyle::Bevel\endlink line join style.
   */
  WPen();

  /*! \brief Creates a black pen with a particular style.
   *
   * Constructs a black pen of 0 width (i.e. cosmetic single pixel
   * width), with \link Wt::PenCapStyle::Square PenCapStyle::Square\endlink line ends and
   * \link Wt::PenJoinStyle::Bevel PenJoinStyle::Bevel\endlink line join style.
   *
   * The line style is set to \p style.
   */
  WPen(PenStyle style);

  /*! \brief Creates a solid pen of a particular color.
   *
   * Constructs a solid pen of 0 width (i.e. cosmetic single pixel
   * width), with \link Wt::PenCapStyle::Square PenCapStyle::Square\endlink line ends and
   * \link Wt::PenJoinStyle::Bevel PenJoinStyle::Bevel\endlink line join style.
   *
   * The pen color is set to \p color.
   */
  WPen(const WColor& color);

  /*! \brief Creates a solid pen of a standard color.
   *
   * Constructs a solid pen of 0 width (i.e. cosmetic single pixel
   * width), with \link Wt::PenCapStyle::Square
   * PenCapStyle::Square\endlink line ends and \link
   * Wt::PenJoinStyle::Bevel PenJoinStyle::Bevel\endlink line join
   * style.
   *
   * The pen color is set to \p color.
   */
  WPen(StandardColor color);

  /*! \brief Creates a solid pen with a gradient color.
   *
   * Constructs a solid pen of 0 width (i.e. cosmetic single pixel
   * width), with \link Wt::PenCapStyle::Square PenCapStyle::Square\endlink line ends and
   * \link Wt::PenJoinStyle::Bevel PenJoinStyle::Bevel\endlink line join style.
   *
   * The pen's color is defined by the gradient \p color.
   */
  WPen(const WGradient& gradient);

#ifdef WT_TARGET_JAVA
  /*! \brief Clone method.
   *
   * Clones this pen.
   */
  WPen clone() const;
#endif

  WPen& operator=(const WPen& rhs);

  /*! \brief Comparison operator.
   *
   * Returns \c true if the pens are exactly the same.
   */
  bool operator==(const WPen& other) const;

  /*! \brief Comparison operator.
   *
   * Returns \c true if the pens are different.
   */
  bool operator!=(const WPen& other) const;

  /*! \brief Sets the pen style.
   *
   * The pen style determines the pattern with which the pen is
   * rendered.
   *
   * \throws WException if the pen \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setStyle(PenStyle style);

  /*! \brief Returns the pen style.
   *
   * \sa setStyle(PenStyle)
   */
  PenStyle style() const { return penStyle_; }

  /*! \brief Sets the style for rendering line ends.
   *
   * The cap style configures how line ends are rendered.
   *
   * \throws WException if the pen \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setCapStyle(PenCapStyle style);

  /*! \brief Returns the style for rendering line ends.
   *
   * \sa setCapStyle(PenCapStyle)
   */
  PenCapStyle capStyle() const { return penCapStyle_; }

  /*! \brief Sets the style for rendering line joins.
   *
   * The join style configures how corners are rendered between
   * different segments of a poly-line, rectange or painter path.
   *
   * \throws WException if the pen \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setJoinStyle(PenJoinStyle style);

  /*! \brief Returns the style for rendering line joins.
   *
   * \sa setJoinStyle(PenJoinStyle)
   */
  PenJoinStyle joinStyle() const { return penJoinStyle_; }

  /*! \brief Sets the pen width.
   *
   * A pen width \p must be specified using LengthUnit::Pixel units.
   *
   * \throws WException if the pen \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setWidth(const WLength& width);

  /*! \brief Returns the pen width.
   *
   * \sa setWidth(const WLength&)
   */
  const WLength& width() const { return width_; }

  /*! \brief Sets the pen color.
   *
   * \throws WException if the pen \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \a setGradient()
   */
  void setColor(const WColor& color);

  /*! \brief Returns the pen color.
   *
   * \sa setColor()
   */
  const WColor& color() const { return color_; }

  /*! \brief Sets the pen color as a gradient.
   *
   * \throws WException if the pen \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa setColor()
   */
  void setGradient(const WGradient& gradient);

  /*! \brief Returns the pen color gradient.
   *
   * \sa setGradient()
   */
  const WGradient& gradient() const { return gradient_; }

  virtual std::string jsValue() const override;

protected:
  virtual void assignFromJSON(const Json::Value &value) override;

private:
  PenStyle penStyle_;
  PenCapStyle penCapStyle_;
  PenJoinStyle penJoinStyle_;
  WLength width_;
  WColor color_;
  WGradient gradient_;
};

}

#endif // WPEN_H_
