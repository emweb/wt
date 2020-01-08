// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVIRTUALIMAGE_H_
#define WVIRTUALIMAGE_H_

#include <limits>
#include <Wt/WCompositeWidget.h>
#include <Wt/WJavaScriptSlot.h>

namespace Wt {

class WImage;
class WMouseEvent;

/*! \class WVirtualImage Wt/WVirtualImage.h Wt/WVirtualImage.h
 *  \brief An abstract widget that shows a viewport to a virtually large image.
 *
 * %WVirtualImage is an abstract class which renders a large image in
 * small pieces. The large image is broken down, and rendered as a
 * grid of smaller square images parts.
 *
 * The %WVirtualImage may provide interactive navigation using the
 * mouse, by reacting to dragging of the mouse on the image.
 * 
 * The %WVirtualImage renders pieces in and bordering the current
 * viewport. In this way, provided the individual pieces load
 * sufficiently fast, the user has effectively the impression of
 * scrolling through a single large image, without glitches. Whenever
 * the image is navigated, if necessary, new images are rendered to
 * maintain the border. Images that are too far from the current
 * viewport are pruned away, so that browser memory remains bounded.
 *
 * To use this class, you must reimplement one of two virtual methods
 * to specify the contents of each grid piece. Either you provide a
 * suitable WImage for every grid piece, or you provide a WResource
 * which renders the contents for a WImage for every grid piece.
 *
 * The total image dimensions are (0, 0) to (imageWidth, imageHeight)
 * for a finite image, and become unbounded (including negative numbers)
 * for each dimension which is Infinite.
 * 
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 */
class WT_API WVirtualImage : public WCompositeWidget
{
public:
  /*! \brief Special value for imageWidth or imageHeight
   */
  static const ::int64_t Infinite;

  /*! \brief Creates a viewport for a virtual image.
   *
   * You must specify the size of the viewport, and the size of the
   * virtual image. The latter dimensions may be the special value
   * Infinite, indicating that in one or more dimensions, the image
   * size is infinite (in practice limited by the maximum integer value).
   *
   * In addition, you must specify the size of each square grid
   * item. The default is 256 by 256.
   */
  WVirtualImage(int viewPortWidth, int viewPortHeight,
		::int64_t imageWidth, ::int64_t imageHeight,
		int gridImageSize = 256);

  /*! \brief Destructor.
   */
  ~WVirtualImage();

  /*! \brief Regenerates and redraws the image pieces.
   *
   * This method invalidates all current grid images, and recreates
   * them.
   */
  void redrawAll();

  /*! \brief Enables mouse dragging to scroll around the image.
   *
   * The cursor is changed to a 'move' symbol to indicate the interactivity.
   */
  void enableDragging();

  /*! \brief Scrolls the viewport of the image over a distance.
   *
   * \sa scrollTo()
   */
  void scroll(::int64_t dx, ::int64_t dy);

  /*! \brief Scrolls the viewport of the image to a specific coordinate.
   *
   * Scroll the viewport so that its top left coordinate becomes (x, y).
   *
   * \sa scroll()
   */
  void scrollTo(::int64_t x, ::int64_t y);

  /*! \brief Returns the virtual image width.
   *
   * \sa imageHeight(), resizeImage()
   */
  ::int64_t imageWidth() const { return imageWidth_; }

  /*! \brief Returns the virtual image height.
   *
   * \sa imageWidth(), resizeImage()
   */
  ::int64_t imageHeight() const { return imageHeight_; }

  /*! \brief Resizes the virtual image.
   *
   * This sets a new virtual size for the image. The viewport size sets the
   * visible portion of the image.
   *
   * \sa imageWidth(), imageHeight()
   */
  void resizeImage(::int64_t w, ::int64_t h);

  /*! \brief Returns the viewport width.
   *
   * \sa viewPortHeight()
   */
  int viewPortWidth() const { return viewPortWidth_; }

  /*! \brief Returns the viewport height.
   *
   * \sa viewPortWidth()
   */
  int viewPortHeight() const { return viewPortHeight_; }

  /*! \brief Returns the size of a single piece.
   *
   * This is the size of a side of the square pieces that is used to render
   * the visible part of the image.
   */
  int gridImageSize() const { return gridImageSize_; }

  /*! \brief Returns the current top left X coordinate.
   *
   * \sa currentTopLeftY()
   */
  ::int64_t currentTopLeftX() const { return currentX_; }

  /*! \brief Returns the current top left Y coordinate.
   *
   * \sa currentTopLeftX()
   */
  ::int64_t currentTopLeftY() const { return currentY_; }

  /*! \brief Returns the current bottom right X coordinate.
   *
   * \sa currentBottomRightY()
   */
  ::int64_t currentBottomRightX() const { return currentX_ + viewPortWidth_; }

  /*! \brief Returns the current bottom right Y coordinate.
   *
   * \sa currentBottomRightX()
   */
  ::int64_t currentBottomRightY() const { return currentY_ + viewPortHeight_; }

  /*! \brief %Signal emitted whenever the viewport changes.
   *
   * The viewport can be changed by the user dragging the image or through
   * the API methods scrollTo() and scroll().
   */
  Signal< ::int64_t, ::int64_t >& viewPortChanged() { return viewPortChanged_; }

protected:
  /*! \brief Creates a grid image for the given rectangle.
   *
   * Create the image which spans image coordinates with left upper
   * corner (x, y) and given width and height.
   *
   * Width and height will not necesarilly equal gridImageSize(), if the
   * the image is not infinite sized.
   *
   * The default implementation calls render() and creates an image
   * for the resource returned.
   *
   * You should override this method if you wish to serve for example
   * static image content.
   *
   * \sa render()
   */
  virtual std::unique_ptr<WImage>
    createImage(::int64_t x, ::int64_t y, int width, int height);

  /*! \brief %Render a grid image for the given rectangle.
   *
   * Returns a resource that streams an image which renders the
   * rectangle which spans image coordinates with left upper corner
   * (x, y) and given width and height.
   *
   * Width and height will not necesarilly equal to gridImageSize(), if the
   * the image is not infinite sized.
   *
   * The default implementation throws an Exception. You must
   * reimplement this method unless you reimplement createImage().
   *
   * \sa createImage()
   */
  virtual std::unique_ptr<WResource>
    render(::int64_t x, ::int64_t y, int width, int height);

  using WCompositeWidget::render;

private:
  Signal<int64_t, int64_t> viewPortChanged_;

  WContainerWidget *impl_;
  WContainerWidget *contents_;

  struct Rect {
    ::int64_t x1, y1, x2, y2;
    
    Rect(::int64_t x1_, ::int64_t y1_, ::int64_t x2_, ::int64_t y2_)
      : x1(x1_), y1(y1_), x2(x2_), y2(y2_) { }
  };

  typedef std::map< ::int64_t, WImage *> GridMap;
  GridMap grid_;

  int gridImageSize_;

  int viewPortWidth_;
  int viewPortHeight_;
  ::int64_t imageWidth_;
  ::int64_t imageHeight_;

  ::int64_t currentX_;
  ::int64_t currentY_;

  void mouseUp(const WMouseEvent& e);

  Rect neighbourhood(::int64_t x, ::int64_t y, int marginX, int marginY);
  ::int64_t gridKey(::int64_t i, ::int64_t j);
  struct Coordinate {
    ::int64_t i;
    ::int64_t j;
  };
  void decodeKey(::int64_t key, Coordinate& coordinate);
  void generateGridItems(::int64_t newX, ::int64_t newY);
  void cleanGrid();
  bool visible(::int64_t i, ::int64_t j) const;

  void internalScrollTo(::int64_t x, ::int64_t y, bool moveViewPort);
};

}

#endif // WVIRTUALIMAGE_H_
