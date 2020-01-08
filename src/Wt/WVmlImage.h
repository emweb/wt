// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVML_IMAGE_H_
#define WVML_IMAGE_H_

#include <Wt/WBrush.h>
#include <Wt/WPen.h>
#include <Wt/WRectF.h>
#include <Wt/WShadow.h>
#include <Wt/WTransform.h>
#include <Wt/WVectorImage.h>
#include <Wt/WResource.h>

#include <sstream>

namespace Wt {

class ServerSideFontMetrics;

/*! \class WVmlImage Wt/WVmlImage.h Wt/WVmlImage.h
 *  \brief A paint device for rendering using the VML pseudo-standard.
 *
 * The %WVmlImage is used by WPaintedWidget to render to the browser
 * using the Vector Markup Language (VML) (to support graphics on
 * Internet Explorer browsers).
 *
 * \note The current implementation has only limited support for
 * clipping: only rectangular areas aligned with the X/Y axes can be used
 * as clipping path.
 *
 * \ingroup painting
 */
class WT_API WVmlImage : public WVectorImage
{
public:
  /*! \brief Create a VML paint device.
   *
   * If \p paintUpdate is \c true, then only a VML fragment will be
   * rendered that can be used to update the DOM of an existing VML
   * image, instead of a full VML image.
   */
  WVmlImage(const WLength& width, const WLength& height, bool paintUpdate);
  virtual ~WVmlImage();

  virtual WFlags<PaintDeviceFeatureFlag> features() const override;
  virtual void setChanged(WFlags<PainterChangeFlag> flags) override;
  virtual void drawArc(const WRectF& rect, double startAngle, double spanAngle) override;
  virtual void drawImage(const WRectF& rect, const std::string& imgUri,
                         int imgWidth, int imgHeight, const WRectF& sourceRect) override;
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

  virtual std::string rendered() override;

  virtual WLength width() const override { return width_; }
  virtual WLength height() const override { return height_; }

protected:
  virtual WPainter *painter() const override { return painter_; }
  virtual void setPainter(WPainter *painter) override { painter_ = painter; }

private:
  WLength    width_, height_;
  WPainter  *painter_;
  bool       paintUpdate_;

  bool       penBrushShadowChanged_;
  bool       clippingChanged_;

  WBrush     currentBrush_;
  WPen       currentPen_;
  WShadow    currentShadow_;
  ServerSideFontMetrics *fontMetrics_;

  struct ActivePath {
    std::string path;
    WRectF      bbox;

    ActivePath()
      : bbox(0, 0, 0, 0)
    { }
  };

  std::vector<ActivePath> activePaths_;

  std::stringstream rendered_;

  void        finishPaths();
  void        processClipping();
  std::string fillElement(const WBrush& brush) const;
  std::string strokeElement(const WPen& pen) const;
  std::string skewElement(const WTransform& transform) const;
  std::string shadowElement(const WShadow& shadow) const;
  std::string createShadowFilter() const;

  static std::string colorAttributes(const WColor& color);
  static std::string quote(double s);
  static std::string quote(const std::string& s);

  void startClip(const WRectF& rect);
  void stopClip();

  WRectF currentRect_;
};

}

#endif // WVML_IMAGE_H_
