// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCANVAS_PAINT_DEVICE_H_
#define WCANVAS_PAINT_DEVICE_H_

#include <Wt/WBrush.h>
#include <Wt/WFont.h>
#include <Wt/WObject.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPainterPath.h>
#include <Wt/WPen.h>
#include <Wt/WPointF.h>
#include <Wt/WShadow.h>
#include <Wt/WTransform.h>

#include <sstream>

namespace Wt {

class DomElement;
class WTransform;
class ServerSideFontMetrics;

/*! \class WCanvasPaintDevice Wt/WCanvasPaintDevice.h Wt/WCanvasPaintDevice.h
 *  \brief A paint device for rendering using the HTML 5 &lt;canvas&gt; element.
 *
 * The %WCanvasPaintDevice is used by WPaintedWidget to render to the
 * browser using the HTML 5 &lt;canvas&gt; element. You usually will
 * not use the device directly, but rather rely on WPaintedWidget to
 * use this device when appropriate.
 *
 * \note Older browsers do not have text support in &lt;canvas&gt;.
 * Text is then rendered in an overlayed DIV and a consequence text is
 * not subject to rotation and scaling components of the current
 * transformation (but does take into account translation). On most
 * browser you can use the WSvgImage or WVmlImage paint devices which
 * do support text natively.
 *
 * \ingroup painting
 */
class WT_API WCanvasPaintDevice : public WObject, public WPaintDevice
{
public:
  /*! \brief Create a canvas paint device.
   */
  WCanvasPaintDevice(const WLength& width, const WLength& height,
		     bool paintUpdate = false);
  virtual ~WCanvasPaintDevice();

  virtual WFlags<PaintDeviceFeatureFlag> features() const override;
  virtual void setChanged(WFlags<PainterChangeFlag> flags) override;
  virtual void drawArc(const WRectF& rect, double startAngle, double spanAngle)
    override;
  virtual void drawImage(const WRectF& rect, const std::string& imgUri,
                         int imgWidth, int imgHeight, const WRectF& sourceRect) override;
  virtual void drawLine(double x1, double y1, double x2, double y2) override;
  virtual void drawPath(const WPainterPath& path) override;
  virtual void drawStencilAlongPath(const WPainterPath &stencil,
				    const WPainterPath &path,
                                    bool softClipping);
  virtual void drawRect(const WRectF& rectangle) override;
  virtual void drawText(const WRectF& rect,
			WFlags<AlignmentFlag> alignmentFlags,
			TextFlag textFlag,
			const WString& text,
                        const WPointF *clipPoint) override;
  virtual void drawTextOnPath(const WRectF &rect,
			      WFlags<AlignmentFlag> alignmentFlags,
			      const std::vector<WString> &text,
			      const WTransform &transform,
			      const WPainterPath &path,
			      double angle, double lineHeight,
			      bool softClipping);
  virtual WTextItem measureText(const WString& text, double maxWidth = -1,
				bool wordWrap = false) override;
  virtual WFontMetrics fontMetrics() override;
  virtual void init() override;
  virtual void done() override;
  virtual bool paintActive() const override { return painter_ != nullptr; }

  void render(const std::string &paintedWidgetJsRef,
              const std::string& canvasId,
              DomElement* text,
              const std::string& updateAreasJs);
  void renderPaintCommands(std::stringstream& js, const std::string& element);

  virtual WLength width() const override { return width_; }
  virtual WLength height() const override { return height_; }

protected:
  virtual WPainter *painter() const override { return painter_; }
  virtual void setPainter(WPainter *painter) override { painter_ = painter; }

private:
  enum class TextMethod { MozText, Html5Text, DomText };

  WLength     width_, height_;
  WPainter   *painter_;
  WFlags<PainterChangeFlag> changeFlags_;
  bool        paintUpdate_;

  TextMethod  textMethod_;

  bool        currentNoPen_, currentNoBrush_, lastTransformWasIdentity_;

  WTransform  currentTransform_;
  WBrush      currentBrush_;
  WPen        currentPen_;
  WShadow     currentShadow_;
  WFont       currentFont_;
  WPointF     pathTranslation_;
  WPainterPath currentClipPath_;
  WTransform currentClipTransform_;
  bool currentClippingEnabled_;
  ServerSideFontMetrics *fontMetrics_;

  std::stringstream js_;
  std::vector<DomElement *> textElements_;
  std::vector<std::string> images_;

  void finishPath();
  void renderTransform(std::stringstream& s, const WTransform& t);
  void renderStateChanges(bool resetPathTranslation);
  void resetPathTranslation();
  void drawPlainPath(std::stringstream& s, const WPainterPath& path);

  int createImage(const std::string& imgUri);

  TextMethod textMethod() const { return textMethod_; }

  friend class WWidgetCanvasPainter;
};

}

#endif // WCANVAS_PAINT_DEVICE_H_
