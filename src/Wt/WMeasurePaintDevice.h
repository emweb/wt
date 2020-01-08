// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMEASURE_PAINT_DEVICE_H_
#define WMEASURE_PAINT_DEVICE_H_

#include <Wt/WPaintDevice.h>
#include <Wt/WRectF.h>

namespace Wt {

/*! \brief WMeasurePaintDevice Wt/WMeasurePaintDevice Wt/WMeasurePaintDevice
 *
 * This implements a (pseudo)-paintdevice which measures the bounding
 * rect of whatever is being painted on it, using fontmetrics from the
 * underlying device.
 *
 * The only output of the device is the computation of a bounding rect
 * which is returned by boundingRect().
 */
class WT_API WMeasurePaintDevice : public WPaintDevice
{
public:
  /*! \brief Creates a paint device to measure for the underlying device.
   */
  WMeasurePaintDevice(WPaintDevice *paintDevice);

  /*! \brief Returns the bounding rectangle of everything painted so far.
   *
   * The bounding rect is returned in device coordinates (i.e. after all
   * transformations applied).
   */
  WRectF boundingRect() const { return bounds_; }

  virtual WFlags<PaintDeviceFeatureFlag> features() const override;
  virtual void setChanged(WFlags<PainterChangeFlag> flags) override;
  virtual void drawArc(const WRectF& rect, double startAngle, double spanAngle)
    override;
  virtual void drawImage(const WRectF& rect, const std::string& imgUri,
			 int imgWidth, int imgHeight, const WRectF& sourceRect)
    override;
  virtual void drawLine(double x1, double y1, double x2, double y2) override;
  virtual void drawRect(const WRectF& rectangle) override;
  virtual void drawPath(const WPainterPath& path) override;
  virtual void drawText(const WRectF& rect, 
			WFlags<AlignmentFlag> alignmentFlags,
			TextFlag textFlag,
			const WString& text,
			const WPointF *clipPoint) override;
  virtual WTextItem measureText(const WString& text, double maxWidth = -1,
				bool wordWrap = false) override;
  virtual WFontMetrics fontMetrics() override;
  virtual void init() override;
  virtual void done() override;
  virtual bool paintActive() const override { return painter_ != nullptr; }

  virtual WLength width() const override;
  virtual WLength height() const override;

protected:
  virtual WPainter *painter() const override { return painter_; }
  virtual void setPainter(WPainter *painter) override { painter_ = painter; }

private:
  WPainter *painter_;
  WPaintDevice *device_;

  WRectF bounds_;

  void expandBounds(const WRectF& bounds);
};

}

#endif // WMEASURE_PAINT_DEVICE_H_
