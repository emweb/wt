/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WCssDecorationStyle.h"
#include "Wt/WImage.h"
#include "Wt/WResource.h"
#include "Wt/WVirtualImage.h"
#include "WebUtils.h"

#include <algorithm>

namespace {

inline ::int64_t clamp(::int64_t v, ::int64_t min, ::int64_t max)
{
  return std::max(min, std::min(v, max));
}

}

namespace Wt {

const ::int64_t WVirtualImage::Infinite
  = std::numeric_limits< ::int64_t >::max();

WVirtualImage::WVirtualImage(int viewPortWidth, int viewPortHeight,
			     ::int64_t imageWidth, ::int64_t imageHeight,
			     int gridImageSize)
  : gridImageSize_(gridImageSize),
    viewPortWidth_(viewPortWidth),
    viewPortHeight_(viewPortHeight),
    imageWidth_(imageWidth),
    imageHeight_(imageHeight),
    currentX_(0),
    currentY_(0)
{
  setImplementation(std::unique_ptr<WContainerWidget>(impl_ = new WContainerWidget()));

  impl_->resize(viewPortWidth_, viewPortHeight_);
  impl_->setPositionScheme(PositionScheme::Relative);

  WContainerWidget *scrollArea
    = impl_->addWidget(std::make_unique<WContainerWidget>());
  scrollArea->resize(WLength(100, LengthUnit::Percentage),
		     WLength(100, LengthUnit::Percentage));
  scrollArea->setPositionScheme(PositionScheme::Absolute);
  scrollArea->setOverflow(Overflow::Hidden);

  contents_ = scrollArea->addWidget(std::make_unique<WContainerWidget>());
  contents_->setPositionScheme(PositionScheme::Absolute);
}

void WVirtualImage::enableDragging()
{
  /*
   * For dragging the virtual image, in client-side JavaScript if available.
   */
  impl_->mouseWentDown().connect("function(obj, event) {"
     "  var pc = " WT_CLASS ".pageCoordinates(event);"
     "  obj.setAttribute('dsx', pc.x);"
     "  obj.setAttribute('dsy', pc.y);"
     "}");

  impl_->mouseMoved().connect("function(obj, event) {"
     """var WT= " WT_CLASS ";"
     """var lastx = obj.getAttribute('dsx');"
     """var lasty = obj.getAttribute('dsy');"
     """if (lastx != null && lastx != '') {"
     ""  "var nowxy = WT.pageCoordinates(event);"
     ""  "var img = " + contents_->jsRef() + ";"
     ""  "img.style.left = (WT.pxself(img, 'left')+nowxy.x-lastx) + 'px';"
     ""  "img.style.top = (WT.pxself(img, 'top')+nowxy.y-lasty) + 'px';"
     ""  "obj.setAttribute('dsx', nowxy.x);"
     ""  "obj.setAttribute('dsy', nowxy.y);"
     """}"
     "}");

  impl_->mouseWentUp().connect("function(obj, event) {"
     + impl_->jsRef() + ".removeAttribute('dsx');"
     "}");

  impl_->mouseWentUp().connect(this, &WVirtualImage::mouseUp);
  impl_->decorationStyle().setCursor(Cursor::OpenHand);
}

WVirtualImage::~WVirtualImage()
{ }

void WVirtualImage::mouseUp(const WMouseEvent& e)
{
  internalScrollTo(currentX_ - e.dragDelta().x, currentY_ - e.dragDelta().y,
		   !WApplication::instance()->environment().ajax());
}

void WVirtualImage::redrawAll()
{
  contents_->clear();
  grid_.clear();

  generateGridItems(currentX_, currentY_);
}

void WVirtualImage::resizeImage(::int64_t w, ::int64_t h)
{
  imageWidth_ = w;
  imageHeight_ = h;

  redrawAll();
}

void WVirtualImage::scrollTo(::int64_t newX, ::int64_t newY)
{
  internalScrollTo(newX, newY, true);
}

void WVirtualImage::internalScrollTo(::int64_t newX, ::int64_t newY,
				     bool moveViewPort)
{
  if (imageWidth_ != Infinite)
    newX = clamp(newX, 0, imageWidth_ - viewPortWidth_);
  if (imageHeight_ != Infinite)
    newY = clamp(newY, 0, imageHeight_ - viewPortHeight_);

  if (moveViewPort) {
    contents_->setOffsets((double)-newX, Side::Left);
    contents_->setOffsets((double)-newY, Side::Top);
  }

  generateGridItems(newX, newY);

  viewPortChanged_.emit(currentX_, currentY_);
}

void WVirtualImage::scroll(::int64_t dx, ::int64_t dy)
{
  scrollTo(currentX_ + dx, currentY_ + dy);
}

std::unique_ptr<WImage> WVirtualImage
::createImage(::int64_t x, ::int64_t y, int width, int height)
{
  auto r = render(x, y, width, height);
  return std::unique_ptr<WImage>
    (new WImage(std::shared_ptr<WResource>(std::move(r)), ""));
}

std::unique_ptr<WResource> WVirtualImage
::render(::int64_t x, ::int64_t y, int width, int height)
{
  throw WException("You should reimplement WVirtualImage::render()");
}

void WVirtualImage::generateGridItems(::int64_t newX, ::int64_t newY)
{
  /*
   * The coordinates of the two extreme corners of the new rendered
   * neighbourhood
   */
  Rect newNb = neighbourhood(newX, newY, viewPortWidth_, viewPortHeight_);  

  ::int64_t i1 = newNb.x1 / gridImageSize_;
  ::int64_t j1 = newNb.y1 / gridImageSize_;
  ::int64_t i2 = newNb.x2 / gridImageSize_ + 1;
  ::int64_t j2 = newNb.y2 / gridImageSize_ + 1;

  for (int invisible = 0; invisible < 2; ++invisible) {
    for (::int64_t i = i1; i < i2; ++i)
      for (::int64_t j = j1; j < j2; ++j) {
	::int64_t key = gridKey(i, j);

	GridMap::iterator it = grid_.find(key);
	if (it == grid_.end()) {
	  bool v = visible(i, j);
	  if ((v && !invisible) || (!v && invisible)) {
	    ::int64_t brx = i * gridImageSize_ + gridImageSize_;
	    ::int64_t bry = j * gridImageSize_ + gridImageSize_;
	    brx = std::min(brx, imageWidth_);
	    bry = std::min(bry, imageHeight_);

            const int width = static_cast<int>(brx - i * gridImageSize_);
            const int height = static_cast<int>(bry - j * gridImageSize_);
            if (width > 0 && height > 0) {
	      std::unique_ptr<WImage> img
	        = createImage(i * gridImageSize_, j * gridImageSize_, width, height);

	      img->setAttributeValue("onmousedown", "return false;");
	      img->setPositionScheme(PositionScheme::Absolute);
	      img->setOffsets((double)i * gridImageSize_, Side::Left);
	      img->setOffsets((double)j * gridImageSize_, Side::Top);

	      grid_[key] = img.get();

	      contents_->addWidget(std::move(img));
            }
	  }
	}
      }
  }

  currentX_ = newX;
  currentY_ = newY;

  cleanGrid();
}

::int64_t WVirtualImage::gridKey(::int64_t i, ::int64_t j)
{
  return i * 1000 + j; // I should consider fixing this properly ...
}

bool WVirtualImage::visible(::int64_t i, ::int64_t j) const
{
  ::int64_t x1 = i * gridImageSize_;
  ::int64_t y1 = j * gridImageSize_;
  ::int64_t x2 = x1 + gridImageSize_;
  ::int64_t y2 = y1 + gridImageSize_;

  return ((x2 >= currentX_) && (y2 >= currentY_)
	  && (x1 <= currentX_ + viewPortWidth_)
	  && (y1 <= currentY_ + viewPortHeight_));
}

void WVirtualImage::decodeKey(::int64_t key, Coordinate& coordinate)
{
  coordinate.i = key / 1000;
  coordinate.j = key % 1000;
}

void WVirtualImage::cleanGrid()
{
  Rect cleanNb = neighbourhood(currentX_, currentY_, 
			       viewPortWidth_ * 3, viewPortHeight_ * 3);

  ::int64_t i1 = cleanNb.x1 / gridImageSize_;
  ::int64_t j1 = cleanNb.y1 / gridImageSize_;
  ::int64_t i2 = cleanNb.x2 / gridImageSize_ + 1;
  ::int64_t j2 = cleanNb.y2 / gridImageSize_ + 1;

  for (GridMap::iterator it = grid_.begin(); it != grid_.end();) {
    Coordinate coordinate;
    decodeKey(it->first, coordinate);

    if (coordinate.i < i1 || coordinate.i > i2 || 
	coordinate.j < j1 || coordinate.j > j2) {
      it->second->removeFromParent();
      Utils::eraseAndNext(grid_, it);
    } else
      ++it;
  }
}

WVirtualImage::Rect WVirtualImage::neighbourhood(::int64_t x, ::int64_t y,
						 int marginX, int marginY)
{
  ::int64_t x1 = x - marginX;

  if (imageWidth_ != Infinite)
    x1 = std::max((::int64_t)0, x1);

  ::int64_t y1 = std::max((::int64_t)0, y - marginY);

  ::int64_t x2 = x + viewPortWidth_ + marginX;
  if (imageWidth_ != Infinite)
    x2 = std::min(imageWidth_, x2);
  
  ::int64_t y2 = std::min(imageHeight_, y + viewPortHeight_ + marginY);

  return Rect(x1, y1, x2, y2);
}

}
