// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBORDER_H_
#define WBORDER_H_

#include <Wt/WLength.h>
#include <Wt/WColor.h>

namespace Wt {

/*! \brief Enumeration for border width
 */
enum class BorderWidth {
  Thin,    //!< Browser-dependent 'thin' border.
  Medium,  //!< Browser-dependent 'medium' border, default.
  Thick,   //!< Browser-dependent 'thick' border.
  Explicit //!< Explicit width. See also explicitWidth()
};

/*! \brief Enumeration for border style
 */
enum class BorderStyle {
  None,    //!< No border (width ignored), default.
  Hidden,  //!< Invisible border (of specified width). 
  Dotted,  //!< Dotted border
  Dashed,  //!< Dashed border
  Solid,   //!< Solid border
  Double,  //!< Double lined border
  Groove,  //!< Relief border grooved into the canvas
  Ridge,   //!< Relief border coming out of the canvas
  Inset,   //!< Relief border lowering contents into the canvas 
  Outset   //!< Relief border letting contents come out of the canvas
};

/*! \class WBorder Wt/WBorder.h Wt/WBorder.h
 *  \brief A value class that defines the CSS border style of a widget.
 *
 * \ingroup style
 */
class WT_API WBorder
{
public:
  /*! \brief Typedef for enum Wt::BorderWidth */
  typedef BorderWidth Width;
  /*! \brief Typedef for enum Wt::BorderStyle */
  typedef BorderStyle Style;

  /*! \brief Creates a border indicating <i>no border</i>.
   */
  WBorder();

  /*! \brief Creates a border with given style, thickness and color.
   */
  WBorder(BorderStyle style, BorderWidth = BorderWidth::Medium, 
	  WColor color = WColor());

  /*! \brief Creates a border with an absolute width.
   */
  WBorder(BorderStyle style, const WLength& width, WColor color = WColor());

  /*! \brief Comparison operator
   */
  bool operator==(const WBorder& other) const;

  /*! \brief Comparison operator
   */
  bool operator!=(const WBorder& other) const;

  /*! \brief Sets the border width.
   *
   * If width == Explicit, then the width specified in
   * \p explicitWidth is used.
   */
  void setWidth(BorderWidth width,
		const WLength& explicitWidth = WLength::Auto);

  /*! \brief Sets the border color.
   */
  void setColor(WColor color);

  /*! \brief Sets the border style.
   */
  void setStyle(BorderStyle style);

  /*! \brief Returns the border width.
   *
   * \sa setWidth()
   */
  BorderWidth width() const { return width_; }

  /*! \brief Returns the border width when set explicitly.
   *
   * \sa setWidth()
   */
  WLength explicitWidth() const { return explicitWidth_; }

  /*! \brief Returns the border color.
   *
   * \sa setColor()
   */
  WColor color() const { return color_; }

  /*! \brief Returns the border style.
   *
   * \sa setStyle()
   */
  BorderStyle style() const { return style_; }

  /*! \brief Returns the CSS text for this border style.
   */
  std::string cssText() const;

#ifdef WT_TARGET_JAVA
  /*! \brief Clone method.
   *
   * Clones this border.
   */
  WBorder clone() const;
#endif
  
private:
  BorderWidth width_;
  WLength explicitWidth_;
  WColor  color_;
  BorderStyle style_;
};

}

#endif // WLENGTH
