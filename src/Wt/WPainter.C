/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <string.h>

#include "Wt/WLineF"
#include "Wt/WPainter"
#include "Wt/WPainterPath"
#include "Wt/WPaintDevice"
#include "Wt/WRectF"
#include "Wt/WTransform"
#include "WtException.h"

namespace Wt {

WPainter::State::State()
  : renderHints_(0),
    clipping_(false)
{
  currentFont_.setFamily(WFont::SansSerif);
  currentFont_.setSize(WFont::FixedSize, WLength(10, WLength::Point));
}

#ifdef WT_TARGET_JAVA
WPainter::State WPainter::State::clone()
{
  State result;

  result.worldTransform_ = worldTransform_;
  result.currentBrush_ = currentBrush_;
  result.currentFont_ = currentFont_;
  result.currentPen_ = currentPen_;
  result.currentShadow_ = currentShadow_;
  result.renderHints_ = renderHints_;
  result.clipPath_ = clipPath_;
  result.clipPathTransform_ = clipPathTransform_;
  result.clipping_ = clipping_;

  return result;
}
#endif // WT_TARGET_JAVA

WPainter::Image::Image(const std::string& uri, int width, int height)
  : uri_(uri),
    width_(width),
    height_(height)
{ }

#ifndef WT_TARGET_JAVA
WPainter::Image::Image(const std::string& uri, const std::string& fileName)
  : uri_(uri)
{
  /*
   * Contributed by Daniel Derr @ ArrowHead Electronics Health-Care
   */
  unsigned char header[25];
  std::ifstream file;
  file.open(fileName.c_str(), std::ios::binary | std::ios::in);

  if (file.good()) {
    file.seekg(0, std::ios::beg);
    file.read((char*)header, 25);
    file.close();

    if (std::memcmp(header, "\211PNG\r\n\032\n", 8) == 0) {
      // PNG FILE
      width_ = ( ( ( int(header[16]) << 8
		     | int(header[17])) << 8
		   | int(header[18])) << 8
		 | int(header[19]));
      height_ = ( ( ( int(header[20]) << 8
		      | int(header[21])) << 8
		    | int(header[22])) << 8
		  | int(header[23]));
    } else if ((std::memcmp(header,"GIF8", 4) == 0)
	       && ((header[4] == '9') || (header[4] == '7'))
	       && (header[5] == 'a')) {
      // GIF FILE
      width_ = int(header[7]) << 8 | int(header[6]);
      height_ = int(header[9]) << 8 | int(header[8]);
    } else {
      throw Wt::WtException("'" + fileName + "': unsupported file format");
    }
  } else
    throw Wt::WtException("'" + fileName + "': could not read");
}
#endif // WT_TARGET_JAVA

WPainter::WPainter()
  : device_(0)
{
  stateStack_.push_back(State());
}

WPainter::WPainter(WPaintDevice *device)
  : device_(0)
{
  begin(device);
}

WPainter::~WPainter()
{
  end();
}

void WPainter::setRenderHint(RenderHint hint, bool on)
{
  int old = s().renderHints_;

  if (on)
    s().renderHints_ |= hint;
  else
    s().renderHints_ &= ~hint;

  if (device_ && old != s().renderHints_)
    device_->setChanged(WPaintDevice::Hints);
}

bool WPainter::begin(WPaintDevice *device)
{
  if (device_)
    return false;

  if (device->paintActive())
    return false;

  stateStack_.clear();
  stateStack_.push_back(State());

  device_ = device;
  device_->setPainter(this);

  device_->init();

  viewPort_.setX(0);
  viewPort_.setY(0);
  viewPort_.setWidth(device_->width().value());
  viewPort_.setHeight(device_->height().value());

  window_ = viewPort_;
  recalculateViewTransform();

  return true;
}

bool WPainter::end()
{
  if (!device_)
    return false;

  device_->done();

  device_->setPainter(0);
  device_ = 0;

  stateStack_.clear();

  return true;
}

bool WPainter::isActive() const
{
  return device_ != 0;
}

void WPainter::save()
{
  stateStack_.push_back(State(stateStack_.back()));
}

void WPainter::restore()
{
  if (stateStack_.size() > 1) {
    WFlags<WPaintDevice::ChangeFlag> flags = 0;

    State& last = stateStack_.back();
    State& next = stateStack_[stateStack_.size() - 2];

    if (last.worldTransform_ != next.worldTransform_)
      flags |= WPaintDevice::Transform;
    if (last.currentBrush_ != next.currentBrush_)
      flags |= WPaintDevice::Brush;
    if (last.currentFont_ != next.currentFont_)
      flags |= WPaintDevice::Font;
    if (last.currentPen_ != next.currentPen_)
      flags |= WPaintDevice::Pen;
    if (last.currentShadow_ != next.currentShadow_)
      flags |= WPaintDevice::Shadow;
    if (last.renderHints_ != next.renderHints_)
      flags |= WPaintDevice::Hints;
    if (last.clipPath_ != next.clipPath_)
      flags |= WPaintDevice::Clipping;
    if (last.clipping_ != next.clipping_)
      flags |= WPaintDevice::Clipping;

    stateStack_.erase(stateStack_.begin() + stateStack_.size() - 1);

    if (flags && device_)
      device_->setChanged(flags);
  }
}

void WPainter::drawArc(const WRectF& rectangle, int startAngle, int spanAngle)
{
  WBrush oldBrush = WBrush(brush());

  setBrush(NoBrush);
  device_->drawArc(rectangle.normalized(), startAngle / 16., spanAngle / 16.);
  setBrush(oldBrush);
}

void WPainter::drawArc(double x, double y, double width, double height,
		       int startAngle, int spanAngle)
{
  drawArc(WRectF(x, y, width, height), startAngle, spanAngle);
}

void WPainter::drawChord(const WRectF& rectangle, int startAngle, int spanAngle)
{
  WTransform oldTransform = WTransform(worldTransform());

  translate(rectangle.center().x(), rectangle.center().y());
  scale(1., rectangle.height() / rectangle.width());

  double start = startAngle / 16.;
  double span = spanAngle / 16.;

  WPainterPath path;
  path.arcMoveTo(0, 0, rectangle.width()/2., start);
  path.arcTo(0, 0, rectangle.width()/2., start, span);
  path.closeSubPath();

  drawPath(path);

  setWorldTransform(oldTransform);
}

void WPainter::drawChord(double x, double y, double width, double height,
			 int startAngle, int spanAngle)
{
  drawChord(WRectF(x, y, width, height), startAngle, spanAngle);
}

void WPainter::drawEllipse(const WRectF& rectangle)
{
  device_->drawArc(rectangle.normalized(), 0, 360);
}

void WPainter::drawEllipse(double x, double y, double width, double height)
{
  drawEllipse(WRectF(x, y, width, height));
}

void WPainter::drawImage(const WPointF& point, const Image& image)
{
  drawImage(WRectF(point.x(), point.y(), image.width(), image.height()),
	    image, WRectF(0, 0, image.width(), image.height()));
}

void WPainter::drawImage(const WPointF& point, const Image& image,
			 const WRectF& sourceRect)
{
  drawImage(WRectF(point.x(), point.y(),
		   sourceRect.width(), sourceRect.height()),
	    image, sourceRect);
}

void WPainter::drawImage(const WRectF& rect, const Image& image)
{
  drawImage(rect, image, WRectF(0, 0, image.width(), image.height()));
}

void WPainter::drawImage(const WRectF& rect, const Image& image,
			 const WRectF& sourceRect)
{
  device_->drawImage(rect.normalized(), image.uri(),
		     image.width(), image.height(),
		     sourceRect.normalized());
}

void WPainter::drawImage(double x, double y, const Image& image,
			 double sx, double sy, double sw, double sh)
{
  if (sw <= 0)
    sw = image.width() - sx;
  if (sh <= 0)
    sh = image.height() - sy;

  device_->drawImage(WRectF(x, y, sw, sh),
		     image.uri(), image.width(), image.height(),
		     WRectF(sx, sy, sw, sh));
}

void WPainter::drawLine(const WLineF& line)
{
  drawLine(line.x1(), line.y1(), line.x2(), line.y2());
}

void WPainter::drawLine(const WPointF& p1, const WPointF& p2)
{
  drawLine(p1.x(), p1.y(), p2.x(), p2.y());
}

void WPainter::drawLine(double x1, double y1, double x2, double y2)
{
  device_->drawLine(x1, y1, x2, y2);
}

void WPainter::drawLines(const WT_ARRAY WLineF *lines, int lineCount)
{
  for (int i = 0; i < lineCount; ++i)
    drawLine(lines[i]);
}

void WPainter::drawLines(const WT_ARRAY WPointF *pointPairs, int lineCount)
{
  for (int i = 0; i < lineCount; ++i)
    drawLine(pointPairs[i*2], pointPairs[i*2 + 1]);
}

void WPainter::drawLines(const std::vector<WLineF>& lines)
{
  for (unsigned i = 0; i < lines.size(); ++i)
    drawLine(lines[i]);
}

void WPainter::drawLines(const std::vector<WPointF>& pointPairs)
{
  for (unsigned i = 0; i < pointPairs.size()/2; ++i)
    drawLine(pointPairs[i*2], pointPairs[i*2 + 1]);
}

void WPainter::drawPath(const WPainterPath& path)
{
  device_->drawPath(path);
}

void WPainter::drawPie(const WRectF& rectangle, int startAngle, int spanAngle)
{
  WTransform oldTransform = WTransform(worldTransform());

  translate(rectangle.center().x(), rectangle.center().y());
  scale(1., rectangle.height() / rectangle.width());

  WPainterPath path(WPointF(0.0, 0.0));
  path.arcTo(0.0, 0.0, rectangle.width() / 2.0,
	     startAngle / 16., spanAngle / 16.);
  path.closeSubPath();

  drawPath(path);

  setWorldTransform(oldTransform);
}

void WPainter::drawPie(double x, double y, double width, double height,
		       int startAngle, int spanAngle)
{
  drawPie(WRectF(x, y, width, height), startAngle, spanAngle);
}

void WPainter::drawPoint(double x, double y)
{ 
  drawLine(x - 0.05, y - 0.05, x + 0.05, y + 0.05);
}

void WPainter::drawPoint(const WPointF& point)
{
  drawPoint(point.x(), point.y());
}

void WPainter::drawPoints(const WT_ARRAY WPointF *points, int pointCount)
{
  for (int i = 0; i < pointCount; ++i)
    drawPoint(points[i]);
}

void WPainter::drawPolygon(const WT_ARRAY WPointF *points, int pointCount
			   /*, FillRule fillRule */)
{
  if (pointCount < 2)
    return;

  WPainterPath path;

  path.moveTo(points[0]);
  for (int i = 1; i < pointCount; ++i)
    path.lineTo(points[i]);

  path.closeSubPath();

  drawPath(path);
}

void WPainter::drawPolyline(const WT_ARRAY WPointF *points, int pointCount)
{
  if (pointCount < 2)
    return;

  WPainterPath path;

  path.moveTo(points[0]);
  for (int i = 1; i < pointCount; ++i)
    path.lineTo(points[i]);

  WBrush oldBrush = WBrush(brush());
  setBrush(WBrush());
  drawPath(path);
  setBrush(oldBrush);
}

void WPainter::drawRect(const WRectF& rectangle)
{
  drawRect(rectangle.x(), rectangle.y(), rectangle.width(), rectangle.height());
}

void WPainter::drawRect(double x, double y, double width, double height)
{
  WPainterPath path(WPointF(x, y));

  path.lineTo(x + width, y);
  path.lineTo(x + width, y + height);
  path.lineTo(x, y + height);
  path.closeSubPath();

  drawPath(path);
}

void WPainter::drawRects(const WT_ARRAY WRectF *rectangles, int rectCount)
{
  for (int i = 0; i < rectCount; ++i)
    drawRect(rectangles[i]);
}

void WPainter::drawRects(const std::vector<WRectF>& rectangles)
{
  for (unsigned i = 0; i < rectangles.size(); ++i)
    drawRect(rectangles[i]);
}

void WPainter::drawText(const WRectF& rectangle, WFlags<AlignmentFlag> flags,
			const WString& text)
{
  if (!(flags & AlignVerticalMask))
    flags |= AlignTop;
  if (!(flags & AlignHorizontalMask))
    flags |= AlignLeft;

  device_->drawText(rectangle.normalized(), flags, text);
}

void WPainter::drawText(double x, double y, double width, double height,
			WFlags<AlignmentFlag> flags, const WString& text)
{
  drawText(WRectF(x, y, width, height), flags, text);
}

void WPainter::fillPath(const WPainterPath& path, const WBrush& b)
{
  WBrush oldBrush = WBrush(brush());
  WPen   oldPen = WPen(pen());

  setBrush(b);
  setPen(NoPen);

  drawPath(path);

  setBrush(oldBrush);
  setPen(oldPen);
}

void WPainter::fillRect(const WRectF& rectangle, const WBrush& b)
{
  WBrush oldBrush = WBrush(brush());
  WPen   oldPen = WPen(pen());

  setBrush(b);
  setPen(NoPen);

  drawRect(rectangle);

  setBrush(oldBrush);
  setPen(oldPen);
}

void WPainter::fillRect(double x, double y, double width, double height,
			const WBrush& brush)
{
  fillRect(WRectF(x, y, width, height), brush);
}

void WPainter::strokePath(const WPainterPath& path, const WPen& p)
{
  WBrush oldBrush = WBrush(brush());
  WPen   oldPen = WPen(pen());

  setBrush(WBrush());
  setPen(p);

  drawPath(path);

  setBrush(oldBrush);
  setPen(oldPen);
}

void WPainter::setBrush(const WBrush& b)
{
  if (brush() != b) {
    s().currentBrush_ = b;
    device_->setChanged(WPaintDevice::Brush);
  }
}

void WPainter::setFont(const WFont& f)
{
  if (font() != f) {
    s().currentFont_ = f;
    device_->setChanged(WPaintDevice::Font);
  }
}

void WPainter::setPen(const WPen& p)
{
  if (pen() != p) {
    s().currentPen_ = p;
    device_->setChanged(WPaintDevice::Pen);
  }
}

void WPainter::setShadow(const WShadow& shadow)
{
  if (this->shadow() != shadow) {
    s().currentShadow_ = shadow;
    device_->setChanged(WPaintDevice::Shadow);
  }
}

void WPainter::resetTransform()
{
  s().worldTransform_.reset();

  if (device_)
    device_->setChanged(WPaintDevice::Transform);
}

void WPainter::rotate(double angle)
{
  s().worldTransform_.rotate(angle);

  if (device_)
    device_->setChanged(WPaintDevice::Transform);
}

void WPainter::scale(double sx, double sy)
{
  s().worldTransform_.scale(sx, sy);

  if (device_)
    device_->setChanged(WPaintDevice::Transform);
}

void WPainter::translate(double dx, double dy)
{
  s().worldTransform_.translate(dx, dy);

  if (device_)
    device_->setChanged(WPaintDevice::Transform);
}

void WPainter::translate(const WPointF& p)
{
  translate(p.x(), p.y());
}

void WPainter::setWorldTransform(const WTransform& matrix, bool combine)
{
  if (combine)
    s().worldTransform_ *= matrix;
  else
    s().worldTransform_ = matrix;

  if (device_)
    device_->setChanged(WPaintDevice::Transform);
}

void WPainter::setViewPort(const WRectF& viewPort)
{
  viewPort_ = viewPort;

  recalculateViewTransform();
}

void WPainter::setViewPort(double x, double y, double width, double height)
{
  setViewPort(WRectF(x, y, width, height));
}

void WPainter::setWindow(const WRectF& window)
{
  window_ = window;

  recalculateViewTransform();
}

void WPainter::setWindow(double x, double y, double width, double height)
{
  setWindow(WRectF(x, y, width, height));
}

void WPainter::recalculateViewTransform()
{
  viewTransform_ = WTransform(); 

  double scaleX = viewPort_.width() / window_.width();
  double scaleY = viewPort_.height() / window_.height();

  viewTransform_.translate(viewPort_.x() - window_.x() * scaleX,
			   viewPort_.y() - window_.y() * scaleY);
  viewTransform_.scale(scaleX, scaleY);

  if (device_)
    device_->setChanged(WPaintDevice::Transform);
}

WTransform WPainter::combinedTransform() const
{
  return viewTransform_ * s().worldTransform_;
}

const WTransform& WPainter::clipPathTransform() const
{
  return s().clipPathTransform_;
}

void WPainter::setClipping(bool enable)
{
  if (s().clipping_ != enable) {
    s().clipping_ = enable;
    if (device_)
      device_->setChanged(WPaintDevice::Clipping);
  }
}

void WPainter::setClipPath(const WPainterPath& clipPath)
{
  s().clipPath_ = clipPath;
  s().clipPathTransform_ = combinedTransform();

  if (s().clipping_ && device_)
    device_->setChanged(WPaintDevice::Clipping);
}

WLength WPainter::normalizedPenWidth(const WLength& penWidth,
				     bool correctCosmetic) const
{
  double w = penWidth.value();

  if (w == 0 && correctCosmetic) {
    // cosmetic width -- must be untransformed 1 pixel
    const WTransform& t = combinedTransform();
    if (!t.isIdentity()) {
      WTransform::TRSRDecomposition d;
      t.decomposeTranslateRotateScaleRotate(d);

      w = 2.0/(std::fabs(d.sx) + std::fabs(d.sy));
    } else
      w = 1.0;

    return WLength(w, WLength::Pixel);
  } else if (w != 0 && !correctCosmetic) {
    // non-cosmetic width -- must be transformed
    const WTransform& t = combinedTransform();
    if (!t.isIdentity()) {
      WTransform::TRSRDecomposition d;
      t.decomposeTranslateRotateScaleRotate(d);

      w *= (std::fabs(d.sx) + std::fabs(d.sy))/2.0;
    }

    return WLength(w, WLength::Pixel);
  } else
    return penWidth;
}


}
