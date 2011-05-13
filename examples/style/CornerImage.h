// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CORNER_IMAGE_H_
#define CORNER_IMAGE_H_

#include <Wt/WColor>
#include <Wt/WImage>

using namespace Wt;

class CornerResource;

/**
 * @addtogroup styleexample
 */
/*@{*/

/*! \brief The CornerImage is a painted widget with a rounded corner.
 *
 * The CornerImage is a dynamically generated image which draws an arc
 * of 90°, to represent one of the four corners of a rounded widget.
 *
 * The CornerImage is part of the %Wt style example.
 *
 * \sa RoundedWidget
 */
class CornerImage : public WImage
{
public:
  /*! \brief One of the four corners of a widget.
   */
  enum Corner { TopLeft = (int)Top | (int)Left,         //!< Top left
		TopRight = (int)Top | (int)Right,       //!< Top right
		BottomLeft = (int)Bottom | (int)Left,   //!< Bottom left
		BottomRight = (int)Bottom | (int)Right  //!< Bottom right
  };

  /*! \brief Construct a new CornerImage.
   *
   * Construct a corner image, to draw the specified corner, with
   * the given foreground and background color, and the specified radius.
   *
   * The colors must be constructed using red/green/blue values,
   * using WColor::WColor(int, int, int).
   */
  CornerImage(Corner corner, WColor fg, WColor bg,
	      int radius, WContainerWidget *parent = 0);

  /*! \brief Change the corner radius (and image dimensions).
   */
  void setRadius(int radius);

  /*! \brief Get the corner radius.
   */
  int radius() const { return radius_; }

  /*! \brief Change the foreground color.
   */
  void setForeground(WColor color);

  /*! \brief Get the foreground color.
   */
  WColor foreground() const { return fg_; }

  /*! \brief Change the background color.
   */
  void setBackground(WColor color);

  /*! \brief Get the background color.
   */
  WColor background() const { return bg_; }

  Corner corner() const { return corner_; }

private:
  //! One of the four corners, which this image represents.
  Corner corner_;

  //! Foreground color
  WColor fg_;

  //! Background color
  WColor bg_;

  //! Radius
  int radius_;

  CornerResource *resource_;
};

/*@}*/

#endif // CORNER_IMAGE_H_
