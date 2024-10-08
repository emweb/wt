/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cassert>
#include <cmath>
#include <fstream>
#include <string>

#ifndef WT_TARGET_JAVA
#include "Wt/Render/WTextRenderer.h"
#endif

#include "Wt/WApplication.h"
#include "Wt/WException.h"
#include "Wt/WLineF.h"
#include "Wt/WPainter.h"
#include "Wt/WPainterPath.h"
#include "Wt/WPaintDevice.h"
#include "Wt/WRectF.h"
#include "Wt/WStringStream.h"
#include "Wt/WTransform.h"
#include "Wt/WWebWidget.h"

#include "Wt/WCanvasPaintDevice.h"

#include "FileUtils.h"
#include "ImageUtils.h"
#include "UriUtils.h"

#include <boost/algorithm/string.hpp>

namespace Wt {

namespace {
  static std::vector<WString> splitLabel(WString text)
  {
    std::string s = text.toUTF8();
    std::vector<std::string> splitText;
    boost::split(splitText, s, boost::is_any_of("\n"));
    std::vector<WString> result;
    for (std::size_t i = 0; i < splitText.size(); ++i) {
      result.push_back(splitText[i]);
    }
    return result;
  }

  static double calcYOffset(int lineNb,
                            int nbLines,
                            double lineHeight,
                            WFlags<AlignmentFlag> verticalAlign)
  {
    if (verticalAlign == AlignmentFlag::Middle) {
      return - ((nbLines - 1) * lineHeight / 2.0) + lineNb * lineHeight;
    } else if (verticalAlign == AlignmentFlag::Top) {
      return lineNb * lineHeight;
    } else if (verticalAlign == AlignmentFlag::Bottom) {
      return - (nbLines - 1 - lineNb) * lineHeight;
    } else {
      return 0;
    }
  }
}

#ifndef WT_TARGET_JAVA

class MultiLineTextRenderer final : public Render::WTextRenderer
{
public:
  MultiLineTextRenderer(WPainter& painter, const WRectF& rect)
    : painter_(painter),
      rect_(rect)
  { }

  virtual double pageWidth(WT_MAYBE_UNUSED int page) const override
  {
    return rect_.right();
  }

  virtual double pageHeight(WT_MAYBE_UNUSED int page) const override
  {
    return 1E9;
  }

  virtual double margin(Side side) const override
  {
    switch (side) {
    case Side::Top: return rect_.top(); break;
    case Side::Left: return rect_.left(); break;
    default:
      return 0;
    }
  }

  virtual WPaintDevice *startPage(int page) override
  {
    if (page > 0)
      assert(false);

    return painter_.device();
  }

  virtual void endPage(WT_MAYBE_UNUSED WPaintDevice* device) override
  {
  }

  virtual WPainter *getPainter(WT_MAYBE_UNUSED WPaintDevice* device) override
  {
    return &painter_;
  }

private:
  WPainter& painter_;
  WRectF    rect_;
};

#endif // WT_TARGET_JAVA

WPainter::State::State()
  : renderHints_(None),
    clipping_(false)
{
  currentFont_.setFamily(FontFamily::SansSerif);
  currentFont_.setSize(WLength(10, LengthUnit::Point));
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

WPainter::Image::Image(const std::string& url, int width, int height)
  : width_(width),
    height_(height)
{
  setUrl(url);
}

WPainter::Image::Image(const std::string& url, const std::string& fileName)
{
  setUrl(url);

  if (DataUri::isDataUri(url)) {
    DataUri uri(url);

    WPoint size = Wt::ImageUtils::getSize(uri.data);
    if (size.x() == 0 || size.y() == 0)
      throw WException("data url: (" + uri.mimeType
                       + "): could not determine image size");

    width_ = size.x();
    height_ = size.y();
  } else {
    WPoint size = Wt::ImageUtils::getSize(fileName);

    if (size.x() == 0 || size.y() == 0)
      throw WException("'" + fileName
                       + "': could not determine image size");

    width_ = size.x();
    height_ = size.y();
  }
}

void WPainter::Image::setUrl(const std::string& url)
{
  url_ = url; // url is resolved (to url or filesystem) in the paintdevice
}

WPainter::WPainter()
  : device_(nullptr)
{
  stateStack_.push_back(State());
}

WPainter::WPainter(WPaintDevice *device)
  : device_(nullptr)
{
  begin(device);
}

WPainter::~WPainter()
{
  end();
}

void WPainter::setRenderHint(RenderHint hint, bool on)
{
  int old = s().renderHints_.value();

  if (on)
    s().renderHints_ |= hint;
  else
    s().renderHints_.clear(hint);

  if (device_ && old != s().renderHints_.value())
    device_->setChanged(PainterChangeFlag::Hints);
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

  viewPort_ = WRectF(0, 0, device_->width().value(), device_->height().value());

  window_ = viewPort_;

  recalculateViewTransform();

  return true;
}

bool WPainter::end()
{
  if (!device_)
    return false;

  device_->done();

  device_->setPainter(nullptr);
  device_ = nullptr;

  stateStack_.clear();

  return true;
}

bool WPainter::isActive() const
{
  return device_ != nullptr;
}

void WPainter::save()
{
  stateStack_.push_back(State(stateStack_.back()));
}

void WPainter::restore()
{
  if (stateStack_.size() > 1) {
    WFlags<PainterChangeFlag> flags = None;

    State& last = stateStack_.back();
    State& next = stateStack_[stateStack_.size() - 2];

    if (last.worldTransform_ != next.worldTransform_)
      flags |= PainterChangeFlag::Transform;
    if (last.currentBrush_ != next.currentBrush_)
      flags |= PainterChangeFlag::Brush;
    if (last.currentFont_ != next.currentFont_)
      flags |= PainterChangeFlag::Font;
    if (last.currentPen_ != next.currentPen_)
      flags |= PainterChangeFlag::Pen;
    if (last.currentShadow_ != next.currentShadow_)
      flags |= PainterChangeFlag::Shadow;
    if (last.renderHints_ != next.renderHints_)
      flags |= PainterChangeFlag::Hints;
    if (last.clipPath_ != next.clipPath_)
      flags |= PainterChangeFlag::Clipping;
    if (last.clipping_ != next.clipping_)
      flags |= PainterChangeFlag::Clipping;

    stateStack_.erase(stateStack_.begin() + stateStack_.size() - 1);

    if (!flags.empty() && device_)
      device_->setChanged(flags);
  }
}

void WPainter::drawArc(const WRectF& rectangle, int startAngle, int spanAngle)
{
  device_->drawArc(rectangle.normalized(), startAngle / 16., spanAngle / 16.);
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

void WPainter::drawStencilAlongPath(const WPainterPath &stencil,
                                    const WPainterPath &path,
                                    bool softClipping)
{
  WCanvasPaintDevice *cDevice = dynamic_cast<WCanvasPaintDevice*>(device_);
  if (cDevice) {
    cDevice->drawStencilAlongPath(stencil, path, softClipping);
  } else {
    for (std::size_t i = 0; i < path.segments().size(); ++i) {
      const WPainterPath::Segment &seg = path.segments()[i];
      if (softClipping && !clipPath().isEmpty() &&
          !clipPathTransform().map(clipPath())
            .isPointInPath(worldTransform().map(WPointF(seg.x(),seg.y())))) {
        continue;
      }
      if (seg.type() == LineTo ||
          seg.type() == MoveTo ||
          seg.type() == CubicEnd ||
          seg.type() == QuadEnd) {
        WPointF p = WPointF(seg.x(), seg.y());
        drawPath((WTransform().translate(p)).map(stencil));
      }
    }
  }
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
  device_->drawRect(rectangle);
}

void WPainter::drawRect(double x, double y, double width, double height)
{
  drawRect(WRectF(x, y, width, height));
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
    flags |= AlignmentFlag::Top;
  if (!(flags & AlignHorizontalMask))
    flags |= AlignmentFlag::Left;

  device_->drawText(rectangle.normalized(), flags, TextFlag::SingleLine,
                    text, nullptr);
}

void WPainter::drawText(const WRectF& rectangle,
                        WFlags<AlignmentFlag> alignmentFlags,
                        TextFlag textFlag,
                        const WString& text,
                        const WPointF *clipPoint)
{
  if (textFlag == TextFlag::SingleLine) {
    if (!(alignmentFlags & AlignVerticalMask))
      alignmentFlags |= AlignmentFlag::Top;
    if (!(alignmentFlags & AlignHorizontalMask))
      alignmentFlags |= AlignmentFlag::Left;

    device_->drawText(rectangle.normalized(), alignmentFlags,
                      TextFlag::SingleLine, text, clipPoint);
  } else {
    if (!(alignmentFlags & AlignVerticalMask))
      alignmentFlags |= AlignmentFlag::Top;
    if (!(alignmentFlags & AlignHorizontalMask))
      alignmentFlags |= AlignmentFlag::Left;

    if (device_->features().test(PaintDeviceFeatureFlag::WordWrap))
      device_->drawText(rectangle.normalized(), alignmentFlags, textFlag,
                        text, clipPoint);
    else if (device_->features().test(PaintDeviceFeatureFlag::FontMetrics)) {
#ifndef WT_TARGET_JAVA
      MultiLineTextRenderer renderer(*this, rectangle);

      AlignmentFlag horizontalAlign = alignmentFlags & AlignHorizontalMask;
      AlignmentFlag verticalAlign = alignmentFlags & AlignVerticalMask;

      /*
       * Oh irony: after all these years of hating CSS, we now
       * implemented an XHTML renderer for which we need to use the
       * same silly workarounds to render the text with all possible
       * alignment options
       */
      WStringStream s;
      s << "<table style=\"width:" << (int)rectangle.width() << "px;\""
                  "cellspacing=\"0\"><tr>"
             "<td style=\"padding:0px;height:" << (int)rectangle.height() <<
                         "px;color:" << pen().color().cssText()
                      << ";text-align:";

      switch (horizontalAlign) {
      case AlignmentFlag::Left: s << "left"; break;
      case AlignmentFlag::Right: s << "right"; break;
      case AlignmentFlag::Center: s << "center"; break;
      default: break;
      }

      s << ";vertical-align:";

      switch (verticalAlign) {
      case AlignmentFlag::Top: s << "top"; break;
      case AlignmentFlag::Bottom: s << "bottom"; break;
      case AlignmentFlag::Middle: s << "middle"; break;
      default: break;
      }

      s << ";" << font().cssText(false);

      s << "\">"
         << WWebWidget::escapeText(text, true).toUTF8()
         << "</td></tr></table>";

      save();

      /*
       * FIXME: what if there was already a clip path? We need to combine
       * them ...
       */
      WPainterPath p;
      p.addRect(rectangle.x() + 1, rectangle.y() + 1,
                rectangle.width() - 2, rectangle.height() - 2);
      setClipPath(p);
      setClipping(true);
      renderer.render(WString::fromUTF8(s.str()));
      restore();
#endif // WT_TARGET_JAVA
    } else
      throw WException("WPainter::drawText(): device does not support "
                       "WordWrap or FontMetrics");
  }
}

void WPainter::drawText(double x, double y, double width, double height,
                        WFlags<AlignmentFlag> alignmentFlags,
                        TextFlag textFlag,
                        const WString& text)
{
  drawText(WRectF(x, y, width, height), alignmentFlags, textFlag, text);
}

void WPainter::drawText(double x, double y, double width, double height,
                        WFlags<AlignmentFlag> flags, const WString& text)
{
  drawText(WRectF(x, y, width, height), flags, text);
}

void WPainter::drawTextOnPath(const WRectF &rect,
                              WFlags<AlignmentFlag> alignmentFlags,
                              const std::vector<WString> &text,
                              const WTransform &transform,
                              const WPainterPath &path,
                              double angle, double lineHeight,
                              bool softClipping)
{
  if (!(alignmentFlags & AlignVerticalMask))
    alignmentFlags |= AlignmentFlag::Top;
  if (!(alignmentFlags & AlignHorizontalMask))
    alignmentFlags |= AlignmentFlag::Left;
  WCanvasPaintDevice *cDevice = dynamic_cast<WCanvasPaintDevice*>(device_);
  if (cDevice) {
    cDevice->drawTextOnPath(rect, alignmentFlags, text, transform, path, angle, lineHeight, softClipping);
  } else {
    WPainterPath tpath = transform.map(path);
    for (std::size_t i = 0; i < path.segments().size(); ++i) {
      if (i >= text.size())
        break;
      const WPainterPath::Segment &seg = path.segments()[i];
      const WPainterPath::Segment &tseg = tpath.segments()[i];
      std::vector<WString> splitText = splitLabel(text[i]);
      if (seg.type() == MoveTo ||
          seg.type() == LineTo ||
          seg.type() == QuadEnd ||
          seg.type() == CubicEnd) {
        save();
        setClipping(false);
        translate(tseg.x(), tseg.y());
        rotate(-angle);
        for (std::size_t j = 0; j < splitText.size(); ++j) {
          double yOffset = calcYOffset(j, splitText.size(), lineHeight, alignmentFlags & AlignVerticalMask);
          WPointF p(tseg.x(), tseg.y());
          drawText(WRectF(rect.left(), rect.top() + yOffset, rect.width(), rect.height()),
                              alignmentFlags, TextFlag::SingleLine, splitText[j], softClipping ? &p : 0);
        }
        restore();
      }
    }
  }
}

void WPainter::fillPath(const WPainterPath& path, const WBrush& b)
{
  WBrush oldBrush = WBrush(brush());
  WPen   oldPen = WPen(pen());

  setBrush(b);
  setPen(PenStyle::None);

  drawPath(path);

  setBrush(oldBrush);
  setPen(oldPen);
}

void WPainter::fillRect(const WRectF& rectangle, const WBrush& b)
{
  WBrush oldBrush = WBrush(brush());
  WPen   oldPen = WPen(pen());

  setBrush(b);
  setPen(PenStyle::None);

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
    device_->setChanged(PainterChangeFlag::Brush);
  }
}

void WPainter::setFont(const WFont& f)
{
  if (font() != f) {
    s().currentFont_ = f;
    device_->setChanged(PainterChangeFlag::Font);
  }
}

void WPainter::setPen(const WPen& p)
{
  if (pen() != p) {
    s().currentPen_ = p;
    device_->setChanged(PainterChangeFlag::Pen);
  }
}

void WPainter::setShadow(const WShadow& shadow)
{
  if (this->shadow() != shadow) {
    s().currentShadow_ = shadow;
    device_->setChanged(PainterChangeFlag::Shadow);
  }
}

void WPainter::resetTransform()
{
  s().worldTransform_.reset();

  if (device_)
    device_->setChanged(PainterChangeFlag::Transform);
}

void WPainter::rotate(double angle)
{
  s().worldTransform_.rotate(angle);

  if (device_)
    device_->setChanged(PainterChangeFlag::Transform);
}

void WPainter::scale(double sx, double sy)
{
  s().worldTransform_.scale(sx, sy);

  if (device_)
    device_->setChanged(PainterChangeFlag::Transform);
}

void WPainter::translate(double dx, double dy)
{
  translate(WPointF(dx, dy));
}

void WPainter::translate(const WPointF& p)
{
  s().worldTransform_.translate(p);

  if (device_)
    device_->setChanged(PainterChangeFlag::Transform);
}

void WPainter::setWorldTransform(const WTransform& matrix, bool combine)
{
  if (combine)
    s().worldTransform_ *= matrix;
  else
    s().worldTransform_ = matrix;

  if (device_)
    device_->setChanged(PainterChangeFlag::Transform);
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
    device_->setChanged(PainterChangeFlag::Transform);
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
      device_->setChanged(PainterChangeFlag::Clipping);
  }
}

void WPainter::setClipPath(const WPainterPath& clipPath)
{
  s().clipPath_ = clipPath;
  s().clipPathTransform_ = combinedTransform();

  if (s().clipping_ && device_)
    device_->setChanged(PainterChangeFlag::Clipping);
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

    return WLength(w, LengthUnit::Pixel);
  } else if (w != 0 && !correctCosmetic) {
    // non-cosmetic width -- must be transformed
    const WTransform& t = combinedTransform();
    if (!t.isIdentity()) {
      WTransform::TRSRDecomposition d;
      t.decomposeTranslateRotateScaleRotate(d);

      w *= (std::fabs(d.sx) + std::fabs(d.sy))/2.0;
    }

    return WLength(w, LengthUnit::Pixel);
  } else
    return penWidth;
}


}
