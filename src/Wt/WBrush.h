// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBRUSH_H_
#define WBRUSH_H_

#include <Wt/WColor.h>
#include <Wt/WGradient.h>
#include <Wt/WJavaScriptExposableObject.h>

namespace Wt {

/*! \class WBrush Wt/WBrush.h Wt/WBrush.h
 *  \brief A value class that defines the style for filling a path
 *
 * A brush defines the properties of how areas (the interior of
 * shapes) are filled. A brush is defined either as a solid color
 * or a gradient.
 *
 * <h3>JavaScript exposability</h3>
 *
 * A %WBrush is JavaScript exposable. If a %WBrush \link isJavaScriptBound()
 * is JavaScript bound\endlink, it can be accessed in your custom JavaScript
 * code through \link WJavaScriptHandle::jsRef() its handle's jsRef()\endlink.
 * At the moment, only the color() property is exposed, e.g. a brush
 * with the color WColor(10,20,30,255) will be represented in JavaScript as:
 * \code
 * {
 *   color: [10,20,30,255]
 * }
 * \endcode
 *
 * \warning A %WBrush that is JavaScript exposed should be modified only through its \link WJavaScriptHandle handle\endlink.
 *	    Any attempt at modifying it will cause an exception to be thrown.
 *
 * \sa WPainter::setBrush(), WPen, WPaintedWidget::createJSBrush()
 *
 * \ingroup painting
 */
class WT_API WBrush : public WJavaScriptExposableObject
{
public:
  /*! \brief Typedef for enum Wt::BrushStyle */
  typedef BrushStyle Style;

  /*! \brief Creates a brush.
   *
   * Creates a brush with a \link Wt::BrushStyle::None BrushStyle::None\endlink fill style.
   */
  WBrush();

  /*! \brief Creates a brush with the given style.
   */
  WBrush(BrushStyle style);

  /*! \brief Creates a solid brush of a given color.
   *
   * Creates a solid brush with the indicated \p color.
   */
  WBrush(const WColor& color);

  /*! \brief Creates a solid brush with a standard color.
   *
   * Creates a solid brush with the indicated \p color.
   */
  WBrush(StandardColor color);

  /*! \brief Creates a gradient brush.
   */
  WBrush(const WGradient& gradient);

#ifdef WT_TARGET_JAVA
  /*! \brief Clone method.
   *
   * Clones this brush.
   */
  WBrush clone() const;
#endif

  WBrush &operator=(const WBrush &rhs);

  /*! \brief Comparison operator.
   *
   * Returns \c true if the brushes are exactly the same.
   */
  bool operator==(const WBrush& other) const;

  /*! \brief Comparison operator.
   *
   * Returns \c true if the brushes are different.
   */
  bool operator!=(const WBrush& other) const;

  /*! \brief Sets the brush style.
   *
   * \throws WException if the brush \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa style()
   */
  void setStyle(BrushStyle style);

  /*! \brief Returns the fill style.
   *
   * \sa setStyle(BrushStyle)
   */
  BrushStyle style() const { return style_; }

  /*! \brief Sets the brush color.
   *
   * If the current style is a gradient style, then it is reset to
   * \link Wt::BrushStyle::Solid BrushStyle::Solid\endlink.
   *
   * \throws WException if the brush \link isJavaScriptBound() is JavaScript bound\endlink
   *
   * \sa color()
   */
  void setColor(const WColor& color);

  /*! \brief Returns the brush color.
   *
   * \sa color()
   */
  const WColor& color() const { return color_; }

  /*! \brief Sets the brush gradient.
   *
   * This also sets the style to
   * \link Wt::BrushStyle::Gradient BrushStyle::Gradient\endlink.
   *
   * \throws WException if the brush \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setGradient(const WGradient& gradient);

  /*! \brief Returns the brush gradient.
   */
  const WGradient& gradient() const { return gradient_; }

  virtual std::string jsValue() const override;

protected:
  virtual void assignFromJSON(const Json::Value &value) override;

private:
  BrushStyle style_;
  WColor color_;
  WGradient gradient_;
};

}

#endif // WBRUSH_H_
