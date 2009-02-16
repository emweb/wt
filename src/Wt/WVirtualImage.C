/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WContainerWidget"
#include "Wt/WCssDecorationStyle"
#include "Wt/WImage"
#include "Wt/WScrollArea"
#include "Wt/WVirtualImage"

#include "WtException.h"

namespace Wt {

const int64_t WVirtualImage::Infinite
  = std::numeric_limits<int64_t>::max();

WVirtualImage::WVirtualImage(int viewPortWidth, int viewPortHeight,
			     int64_t imageWidth, int64_t imageHeight,
			     int gridImageSize,
			     WContainerWidget *parent)
  : WCompositeWidget(parent),
    gridImageSize_(gridImageSize),
    viewPortWidth_(viewPortWidth),
    viewPortHeight_(viewPortHeight),
    imageWidth_(imageWidth),
    imageHeight_(imageHeight),
    currentX_(0),
    currentY_(0),
    mouseDownJS_(this),
    mouseMovedJS_(this),
    mouseUpJS_(this)
{
  setImplementation(impl_ = new WContainerWidget());

  impl_->resize(viewPortWidth_, viewPortHeight_);
  impl_->setPositionScheme(Relative);

  WScrollArea *scrollArea = new WScrollArea(impl_);
  scrollArea->resize(WLength(100, WLength::Percentage),
		     WLength(100, WLength::Percentage));
  scrollArea->setScrollBarPolicy(WScrollArea::ScrollBarAlwaysOff);
  scrollArea->setPositionScheme(Absolute);

  contents_ = new WContainerWidget();
  contents_->setPositionScheme(Absolute);

  scrollArea->setWidget(contents_);
}

void WVirtualImage::enableDragging()
{
  /*
   * For dragging the virtual image, in client-side JavaScript if available.
   */
  mouseDownJS_.setJavaScript
    ("function(obj, event) {"
     "  var pc = " WT_CLASS ".pageCoordinates(event);"
     "  obj.setAttribute('dsx', pc.x);"
     "  obj.setAttribute('dsy', pc.y);"
     "}");

  mouseMovedJS_.setJavaScript
    ("function(obj, event) {"
     """var WT= " WT_CLASS ";"
     """var lastx = obj.getAttribute('dsx');"
     """var lasty = obj.getAttribute('dsy');"
     """if (lastx != null && lastx != '') {"
     ""  "nowxy = WT.pageCoordinates(event);"
     ""  "img = " + contents_->jsRef() + ";"
     ""  "img.style.left = (WT.pxself(img, 'left')+nowxy.x-lastx) + 'px';"
     ""  "img.style.top = (WT.pxself(img, 'top')+nowxy.y-lasty) + 'px';"
     ""  "obj.setAttribute('dsx', nowxy.x);"
     ""  "obj.setAttribute('dsy', nowxy.y);"
     """}"
     "}");

  mouseUpJS_.setJavaScript
    ("function(obj, event) {"
     + impl_->jsRef() + ".removeAttribute('dsx');"
     "}");

  impl_->mouseWentDown.connect(mouseDownJS_);
  impl_->mouseMoved.connect(mouseMovedJS_);
  impl_->mouseWentUp.connect(mouseUpJS_);

  impl_->mouseWentUp.connect(SLOT(this, WVirtualImage::mouseUp));
  impl_->decorationStyle().setCursor(WCssDecorationStyle::Move);
}

WVirtualImage::~WVirtualImage()
{
  for (GridMap::iterator it = grid_.begin(); it != grid_.end(); ++it) {
    delete it->second->resource();
    delete it->second;
  }
}

void WVirtualImage::mouseUp(const WMouseEvent& e)
{
  internalScrollTo(currentX_ - e.dragDelta().x, currentY_ - e.dragDelta().y,
		   !WApplication::instance()->environment().ajax());
}

void WVirtualImage::redrawAll()
{
  for (GridMap::iterator it = grid_.begin(); it != grid_.end(); ++it) {
    delete it->second->resource();
    delete it->second;
  }

  grid_.clear();

  generateGridItems(currentX_, currentY_);
}

void WVirtualImage::resizeImage(int64_t w, int64_t h)
{
  imageWidth_ = w;
  imageHeight_ = h;

  redrawAll();
}

void WVirtualImage::scrollTo(int64_t newX, int64_t newY)
{
  internalScrollTo(newX, newY, true);
}

void WVirtualImage::internalScrollTo(int64_t newX, int64_t newY,
				     bool moveViewPort)
{
  if (imageWidth_ != Infinite)
    newX = std::min(imageWidth_ - viewPortWidth_,
		    std::max((int64_t)0, newX));
  if (imageHeight_ != Infinite)
    newY = std::min(imageHeight_ - viewPortHeight_,
		    std::max((int64_t)0, newY));

  if (moveViewPort) {
    contents_->setOffsets((double)-newX, Left);
    contents_->setOffsets((double)-newY, Top);
  }

  generateGridItems(newX, newY);

  viewPortChanged.emit(currentX_, currentY_);
}

void WVirtualImage::scroll(int64_t dx, int64_t dy)
{
  scrollTo(currentX_ + dx, currentY_ + dy);
}

WImage *WVirtualImage::createImage(int64_t x, int64_t y,
				   int width, int height)
{
  WResource *r = render(x, y, width, height);
  return new WImage(r, "");
}

WResource *WVirtualImage::render(int64_t x, int64_t y,
				 int width, int height)
{
  throw WtException("You should reimplement WVirtualImage::render()");
}

void WVirtualImage::generateGridItems(int64_t newX, int64_t newY)
{
  /*
   * The coordinates of the two extreme corners of the new rendered
   * neighbourhood
   */
  Rect newNb = neighbourhood(newX, newY, viewPortWidth_, viewPortHeight_);  

  int64_t i1 = newNb.x1 / gridImageSize_;
  int64_t j1 = newNb.y1 / gridImageSize_;
  int64_t i2 = newNb.x2 / gridImageSize_ + 1;
  int64_t j2 = newNb.y2 / gridImageSize_ + 1;

  for (int invisible = 0; invisible < 2; ++invisible) {
    for (int64_t i = i1; i < i2; ++i)
      for (int64_t j = j1; j < j2; ++j) {
	int64_t key = gridKey(i, j);

	GridMap::iterator it = grid_.find(key);
	if (it == grid_.end()) {
	  bool v = visible(i, j);
	  if ((v && !invisible) || (!v && invisible)) {
	    int64_t brx = i * gridImageSize_ + gridImageSize_;
	    int64_t bry = j * gridImageSize_ + gridImageSize_;
	    brx = std::min(brx, imageWidth_);
	    bry = std::min(bry, imageHeight_);

	    WImage *img = createImage(i * gridImageSize_, j * gridImageSize_,
				      (int)(brx - i * gridImageSize_),
				      (int)(bry - j * gridImageSize_));

	    img->setAttributeValue("onmousedown", "return false;");
	    contents_->addWidget(img);
	    img->setPositionScheme(Absolute);
	    img->setOffsets((double)i * gridImageSize_, Left);
	    img->setOffsets((double)j * gridImageSize_, Top);

	    grid_[key] = img;
	  }
	}
      }
  }

  currentX_ = newX;
  currentY_ = newY;

  cleanGrid();
}

int64_t WVirtualImage::gridKey(int64_t i, int64_t j)
{
  return i * 1000 + j; // I should consider fixing this properly ...
}

bool WVirtualImage::visible(int64_t i, int64_t j) const
{
  int64_t x1 = i * gridImageSize_;
  int64_t y1 = j * gridImageSize_;
  int64_t x2 = x1 + gridImageSize_;
  int64_t y2 = y1 + gridImageSize_;

  return ((x2 >= currentX_) && (y2 >= currentY_)
	  && (x1 <= currentX_ + viewPortWidth_)
	  && (y1 <= currentY_ + viewPortHeight_));
}

void WVirtualImage::decodeKey(int64_t key, int64_t& i, int64_t& j)
{
  i = key / 1000;
  j = key % 1000;
}

void WVirtualImage::cleanGrid()
{
  Rect cleanNb = neighbourhood(currentX_, currentY_, 
			       viewPortWidth_ * 3, viewPortHeight_ * 3);

  int64_t i1 = cleanNb.x1 / gridImageSize_;
  int64_t j1 = cleanNb.y1 / gridImageSize_;
  int64_t i2 = cleanNb.x2 / gridImageSize_ + 1;
  int64_t j2 = cleanNb.y2 / gridImageSize_ + 1;

  for (GridMap::iterator it = grid_.begin(); it != grid_.end();) {
    int64_t i, j;
    decodeKey(it->first, i, j);

    if (i < i1 || i > i2 || j < j1 || j > j2) {
      delete it->second->resource();
      delete it->second;
      GridMap::iterator eraseIt = it;
      ++it;
      grid_.erase(eraseIt);
    } else
      ++it;
  }
}

WVirtualImage::Rect WVirtualImage::neighbourhood(int64_t x, int64_t y,
						 int marginX, int marginY)
{
  int64_t x1 = x - marginX;

  if (imageWidth_ != Infinite)
    x1 = std::max((int64_t)0, x1);

  int64_t y1 = std::max((int64_t)0, y - marginY);

  int64_t x2 = x + viewPortWidth_ + marginX;
  if (imageWidth_ != Infinite)
    x2 = std::min(imageWidth_, x2);
  
  int64_t y2 = std::min(imageHeight_, y + viewPortHeight_ + marginY);

  return Rect(x1, y1, x2, y2);
}

}
