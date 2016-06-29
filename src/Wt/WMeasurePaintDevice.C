/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WMeasurePaintDevice"
#include "Wt/WFontMetrics"
#include "Wt/WPainter"
#include "Wt/WPainterPath"
#include "Wt/WRectF"

#include <algorithm>
#include <iostream>

namespace Wt {

WMeasurePaintDevice::WMeasurePaintDevice(WPaintDevice *paintDevice)
  : painter_(0),
    device_(paintDevice)
{ }

WFlags<WPaintDevice::FeatureFlag> WMeasurePaintDevice::features() const
{
  return device_->features();
}

void WMeasurePaintDevice::init()
{
  if (!device_->painter()) {
    device_->setPainter(painter_);
    device_->init();
  } else
    device_->painter()->save();
}

void WMeasurePaintDevice::done()
{
  if (painter_ == device_->painter()) {
    device_->done();
    device_->setPainter(0);
  } else
    device_->painter()->restore();
}

void WMeasurePaintDevice::expandBounds(const WRectF& bounds)
{
  WTransform transform = painter()->combinedTransform();
  WRectF bbox = transform.map(bounds);

  // FIXME: take into account clipping

  if (!bounds_.isNull())
    bounds_ = bounds_.united(bbox);
  else
    bounds_ = bbox;
}

void WMeasurePaintDevice::drawArc(const WRectF& rect, double startAngle,
				  double spanAngle)
{
  WPainterPath p;

  double r = std::max(rect.width(), rect.height()) / 2;
  double cx = rect.center().x();
  double cy = rect.center().y();
  p.arcMoveTo(cx, cy, r, startAngle);
  p.arcTo(cx, cy, r, startAngle, spanAngle);

  expandBounds(p.controlPointRect());
}

void WMeasurePaintDevice::drawImage(const WRectF& rect,
				    const std::string& imageUri,
				    int imgWidth, int imgHeight,
				    const WRectF& sourceRect)
{
  expandBounds(rect);
}

void WMeasurePaintDevice::drawPath(const WPainterPath& path)
{
  expandBounds(path.controlPointRect());
}

void WMeasurePaintDevice::drawLine(double x1, double y1, double x2, double y2)
{
  expandBounds(WRectF(x1, y1, x2-x1, y2-y1));
}

void WMeasurePaintDevice::drawText(const WRectF& rect,
				   WFlags<AlignmentFlag> flags,
				   TextFlag textFlag, const WString& text,
				   const WPointF *clipPoint)
{
  if (clipPoint && painter() && !painter()->clipPath().isEmpty()) {
    if (!painter()->clipPathTransform().map(painter()->clipPath())
	  .isPointInPath(painter()->worldTransform().map(*clipPoint)))
      return;
  }

  double w = 0, h = 0;
  WString line = text;

  WFontMetrics fm = fontMetrics();

  for (;;) {
    WTextItem t = measureText(line, rect.width(),
			      textFlag == TextWordWrap ? true : false);

    h += fm.height();
    w = std::max(w, t.width());

    if (t.text() == line)
      break;
    else
      line = WString
	::fromUTF8(line.toUTF8().substr(t.text().toUTF8().length()));
  }

  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  double x, y;

  switch (horizontalAlign) {
  case AlignLeft:
    x = rect.left(); break;
  case AlignCenter:
    x = rect.left() + (rect.width() - w) / 2; break;
  case AlignRight:
  default:
    x = rect.right() - w; break;
  }

  switch (verticalAlign) {
  case AlignTop:
    y = rect.top(); break;
  case AlignMiddle:
    y = rect.top() + (rect.height() - h) / 2; break;
  case AlignBottom:
  default:
    y = rect.bottom() - h; break;
  }

  expandBounds(WRectF(x, y, w, h));
}

WTextItem WMeasurePaintDevice::measureText(const WString& text, double maxWidth,
					   bool wordWrap)
{
  return device_->measureText(text, maxWidth, wordWrap);
}

WFontMetrics WMeasurePaintDevice::fontMetrics()
{
  return device_->fontMetrics();
}

void WMeasurePaintDevice::setChanged(WFlags<ChangeFlag> flags)
{
  if (device_->painter() != painter_ && (flags & Font))
	  device_->painter()->setFont(painter_->font());
  device_->setChanged(flags);
}

WLength WMeasurePaintDevice::width() const
{
  return device_->width();
}

WLength WMeasurePaintDevice::height() const
{
  return device_->height();
}

}
