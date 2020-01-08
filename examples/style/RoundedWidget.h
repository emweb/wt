// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef ROUNDED_WIDGET_H_
#define ROUNDED_WIDGET_H_

#include <Wt/WCompositeWidget.h>

#include <array>

#include "CornerImage.h"

namespace Wt {
  class WContainerWidget;
}

/**
 * @addtogroup styleexample
 */
/*@{*/

/*! \brief A widget with rounded corners.
 *
 * This widgets represents a widget for which any combination of its four
 * corners may be rounded. Although rounded corners is not a standard part
 * of the CSS specification, this widget will be rendered identical on
 * all platforms.
 *
 * The contents of the widget is managed inside a WContainerWidget, which
 * is accessed using the contents() method.
 *
 * The radius of the rounded corners, the background color of the image,
 * and the surrounding color may be changed at all times.
 *
 * The RoundedWidget is part of the %Wt style example.
 *
 * \sa CornerImage.
 */
class RoundedWidget : public WCompositeWidget
{
public:
  /*! \brief Construct a widget with any combination of its corners
   *         rounded.
   */
  RoundedWidget(WFlags<Corner> corners = WFlags<Corner>(Corner::TopLeft) |
							Corner::TopRight |
							Corner::BottomLeft |
							Corner::BottomRight);

  /*! \brief Set the widget background color.
   *
   * Because the background color also affects the color of the
   * corner images, the background color cannot be set using the
   * WCssDecorationStyle() of the widget.
   */
  void setBackgroundColor(WColor color);

  /*! \brief Get the widget background color.
   */
  WColor backgroundColor() const { return backgroundColor_; }

  /*! \brief Show or hide rounded corners.
   */
  void enableRoundedCorners(bool how);

  /*! \brief Set the corner radius of the widget.
   */
  void setCornerRadius(int radius);

  /*! \brief Get the corner radius of the widget.
   */
  int cornerRadius() const { return radius_; }

  /*! \brief Set the surrounding color of the widget.
   *
   * This color will be used "outside" the corner, in each of the
   * corner images.
   */
  void setSurroundingColor(WColor color);

  /*! \brief Get the surrounding color of the widget.
   */
  WColor surroundingColor() const { return surroundingColor_; }

  /*! \brief Access the contents container.
   *
   * The contents WContainerWidget represents the contents inside
   * the rounded widget.
   */
  WContainerWidget *contents() const { return contents_; }

private:
  //! Background color
  WColor backgroundColor_;

  //! "Surrounding" color -- maybe we can use a transparent color ?
  WColor surroundingColor_;

  //! Radius
  int radius_;

  //! OR'ed specification of the corners which are to be rounded.
  WFlags<Corner> corners_;

  //! The container widget in which to store the contents.
  WContainerWidget *contents_;

  //! This composite widget is implemented as a WContainerWidget
  WContainerWidget *impl_;

  //! A container at the top which renders the top rounding
  WContainerWidget *top_;

  //! A container at the bottom renders the bottom rounding
  WContainerWidget *bottom_;

  //! Up to four CornerImages for each corner.
  std::array<Wt::Core::observing_ptr<CornerImage>, 4> images_;

  //! Create the implementation.
  void create();

  //! Adjust the image (colors and radius).
  void adjust();
};

/*@}*/

#endif // ROUNDED_WIDGET_H_
