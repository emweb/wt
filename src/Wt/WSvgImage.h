// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSVG_IMAGE_H_
#define WSVG_IMAGE_H_

#include <Wt/WBrush.h>
#include <Wt/WFont.h>
#include <Wt/WPen.h>
#include <Wt/WPointF.h>
#include <Wt/WRectF.h>
#include <Wt/WResource.h>
#include <Wt/WShadow.h>
#include <Wt/WStringStream.h>
#include <Wt/WTransform.h>
#include <Wt/WVectorImage.h>

namespace Wt {

class ServerSideFontMetrics;

/*! \class WSvgImage Wt/WSvgImage.h Wt/WSvgImage.h
 *  \brief A paint device for rendering using Scalable Vector Graphics (SVG).
 *
 * The %WSvgImage is primarily used by WPaintedWidget to render to the
 * browser in Support Vector Graphics (SVG) format.
 *
 * \if cpp
 * You may also use the %WSvgImage as an independent resource, for example
 * in conjunction with a WAnchor or WImage, or to make a hard copy of an
 * image in SVG format, using write():
 * \code
 * Wt::Chart::WCartesianChart *chart = ...
 *
 * Wt::WSvgImage svgImage(800, 400);
 * Wt::WPainter p(&svgImage);
 * chart->paint(p);
 * p.end();
 * std::ofstream f("chart.svg");
 * svgImage.write(f);
 * \endcode
 * \endif
 *
 * \ingroup painting
 */
class WT_API WSvgImage : public WResource, public WVectorImage
{
public:
  /*! \brief Create an SVG paint device.
   *
   * If \p paintUpdate is \c true, then only an SVG fragment will be
   * rendered that can be used to update the DOM of an existing SVG
   * image, instead of a full SVG image.
   */
  WSvgImage(const WLength& width, const WLength& height,
	    bool paintUpdate = false);

  /*! \brief Destructor.
   */
  ~WSvgImage();

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

  virtual std::string rendered() override;

  virtual WLength width() const override { return width_; }
  virtual WLength height() const override { return height_; }

  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) override;

protected:
  virtual WPainter *painter() const override { return painter_; }
  virtual void setPainter(WPainter *painter) override { painter_ = painter; }

private:
  WLength   width_, height_;
  WPainter *painter_;
  bool      paintUpdate_;
  WFlags<PainterChangeFlag> changeFlags_;

  bool        newGroup_;
  bool        newClipPath_;
  bool        busyWithPath_;
  int         currentClipId_;
  static int  nextClipId_;
  int         currentFillGradientId_;
  int         currentStrokeGradientId_;
  static int  nextGradientId_;

  WTransform  currentTransform_;
  WBrush      currentBrush_;
  WFont       currentFont_;
  WPen        currentPen_;
  WShadow     currentShadow_;
  int         currentShadowId_, nextShadowId_;

  WPointF     pathTranslation_;
  WRectF      pathBoundingBox_;

  WStringStream shapes_;
  ServerSideFontMetrics *fontMetrics_;

  void finishPath();
  void makeNewGroup();
  std::string fillStyle() const;
  std::string strokeStyle() const;
  std::string fontStyle() const;
  std::string clipPath() const;
  int createShadowFilter(WStringStream& out);
  void defineGradient(const WGradient& gradient, int id);

  std::string fillStyle_, strokeStyle_, fontStyle_;

  static std::string quote(double s);
  static std::string quote(const std::string& s);

  void drawPlainPath(WStringStream& s, const WPainterPath& path);

  void streamResourceData(std::ostream& stream);
};

}

#endif // WSVG_IMAGE_H_
