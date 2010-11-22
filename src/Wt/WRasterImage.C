/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBrush"
#include "Wt/WPainter"
#include "Wt/WPen"
#include "Wt/WRasterImage"
#include "Wt/WTransform"
#include "Wt/Http/Response"

#include "web/WtException.h"
#include "web/Utils.h"

#include <cstdio>
#include <magick/api.h>
#include <boost/lexical_cast.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

  double adjust360(double d) {
    if (std::fabs(d - 360) < 0.01)
      return 359.5;
    else if (std::fabs(d + 360) < 0.01)
      return -359.5;
    else 
      return d;
  }

  bool fequal(double d1, double d2) {
    return std::fabs(d1 - d2) < 1E-5;
  }

  void WColorToPixelPacket(const Wt::WColor& color, PixelPacket *pp)
  {
    pp->red = static_cast<unsigned char>(color.red());
    pp->green = static_cast<unsigned char>(color.green());
    pp->blue = static_cast<unsigned char>(color.blue());
    pp->opacity = 0;
  }

}

namespace Wt {

WRasterImage::WRasterImage(const std::string& type,
			   const WLength& width, const WLength& height,
			   WObject *parent)
  : WResource(parent),
    width_(width),
    height_(height),
    type_(type),
    painter_(0)
{
  w_ = static_cast<unsigned long>(width.toPixels());
  h_ = static_cast<unsigned long>(height.toPixels());

  unsigned long bufSize = 3 * w_ * h_;
  pixels_ = new unsigned char[bufSize];
  for (unsigned i = 0; i < w_ * h_; ++i)
    pixels_[i*3] = pixels_[i*3 + 1] = pixels_[i*3 + 2] = 254;

  ExceptionInfo exception;
  GetExceptionInfo(&exception);
  image_ = ConstituteImage(w_, h_, "RGB", CharPixel, pixels_, &exception);

  std::string magick = type;
  std::transform(magick.begin(), magick.end(), magick.begin(), toupper);
  strcpy(image_->magick, type.c_str());

  context_ = 0;
}

void WRasterImage::clear()
{
  PixelPacket *pixel = SetImagePixels(image_, 0, 0, w_, h_);
  for (unsigned i = 0; i < w_ * h_; ++i)
    WColorToPixelPacket(white, pixel + i);
  SyncImagePixels(image_);
}

WRasterImage::~WRasterImage()
{
  beingDeleted();

  DestroyImage(image_);
  delete[] pixels_;
}

void WRasterImage::init()
{
  context_ = DrawAllocateContext(0, image_);

  DrawPushGraphicContext(context_);

  DrawSetFillRule(context_, NonZeroRule);
  DrawTranslate(context_, -0.5, -0.5);
  DrawSetTextEncoding(context_, "UTF-8");

  DrawPushGraphicContext(context_); // for painter->clipping();
  DrawPushGraphicContext(context_); // for painter->combinedTransform()
}

void WRasterImage::done()
{
  DrawPopGraphicContext(context_); // for painter->combinedTransform()
  DrawPopGraphicContext(context_); // for painter->clipping();
  DrawPopGraphicContext(context_);

  DrawRender(context_);

  DrawDestroyContext(context_);
}

void WRasterImage::applyTransform(const WTransform& t)
{
  AffineMatrix matrix;
  matrix.sx = t.m11();
  matrix.rx = t.m12();
  matrix.ry = t.m21();
  matrix.sy = t.m22();
  matrix.tx = t.dx();
  matrix.ty = t.dy();

  DrawAffine(context_, &matrix);
}

void WRasterImage::setTransform(const WTransform& t)
{
  DrawPopGraphicContext(context_);
  DrawPushGraphicContext(context_);

  applyTransform(t);
}

void WRasterImage::setChanged(WFlags<ChangeFlag> flags)
{
  if (flags & Clipping) {
    DrawPopGraphicContext(context_);
    DrawPopGraphicContext(context_);
    DrawPushGraphicContext(context_);

    if (painter()->hasClipping()) {
      if (clipPathCache_.empty())
	clipPathCache_.resize(3); // keep 3

      int index = -1;
      int nextIndex = 0;

      for (PathCache::iterator i = clipPathCache_.begin();
	   i != clipPathCache_.end(); ++i) {
	if (i->first == painter()->clipPath()) {
	  index = i->second;
	  clipPathCache_.splice(clipPathCache_.begin(), clipPathCache_, i);
	  break;
	} else {
	  nextIndex = std::max(i->second, nextIndex);
	}
      }

      if (index == -1) {
	index = nextIndex + 1;
	DrawPushClipPath
	  (context_,("clip" + boost::lexical_cast<std::string>(index)).c_str());
	drawPath(painter()->clipPath());
	DrawPopClipPath(context_);

	clipPathCache_.pop_back(); // implement LRU
	clipPathCache_.push_front(std::make_pair(painter()->clipPath(), index));
      }

      const WTransform& t = painter()->clipPathTransform();

      applyTransform(t);

      DrawSetClipUnits(context_, UserSpaceOnUse);
      DrawSetClipPath
	(context_, ("clip" + boost::lexical_cast<std::string>(index)).c_str());
    }

    DrawPushGraphicContext(context_);

    flags = Transform;
  }

  if (flags & Transform) {
    setTransform(painter()->combinedTransform());
    flags = Pen | Brush | Font;
  }

  if (flags & Pen) {
    const WPen& pen = painter()->pen();

    if (pen.style() != NoPen) {
      const WColor& color = pen.color();

      PixelPacket pp;
      WColorToPixelPacket(color, &pp);
      DrawSetStrokeColor(context_, &pp);
      DrawSetStrokeOpacity(context_, color.alpha() / 255.0);

      WLength w = painter()->normalizedPenWidth(pen.width(), true);
      DrawSetStrokeWidth(context_, w.toPixels());

      switch (pen.capStyle()) {
      case FlatCap:
	DrawSetStrokeLineCap(context_, ::ButtCap);
	break;
      case SquareCap:
	DrawSetStrokeLineCap(context_, ::SquareCap);
	break;
      case RoundCap:
	DrawSetStrokeLineCap(context_, ::RoundCap);
	break;
      }

      /*
      switch (pen.joinStyle()) {
      case MiterJoin:
	DrawSetStrokeLineJoin(context_, ::MiterJoin);
	DrawSetStrokeMiterLimit(context_, 3);
	break;
      case BevelJoin:
	DrawSetStrokeLineJoin(context_, ::BevelJoin);
	break;
      case RoundJoin:
	DrawSetStrokeLineJoin(context_, ::RoundJoin);
	break;
      }
      */

      switch (pen.style()) {
      case NoPen:
	break;
      case SolidLine:
	DrawSetStrokeDashArray(context_, 0, 0);
	break;
      case DashLine: {
	const double dasharray[] = { 4, 2 };
	DrawSetStrokeDashArray(context_, 2, dasharray);
	break;
      }
      case DotLine: {
	const double dasharray[] = { 1, 2 };
	DrawSetStrokeDashArray(context_, 2, dasharray);
	break;
      }
      case DashDotLine: {
	const double dasharray[] = { 4, 2, 1, 2 };
	DrawSetStrokeDashArray(context_, 4, dasharray);
	break;
      }
      case DashDotDotLine: {
	const double dasharray[] = { 4, 2, 1, 2, 1, 2 };
	DrawSetStrokeDashArray(context_, 6, dasharray);
	break;
      }
      }

    } else
      DrawSetStrokeOpacity(context_, 0);
  }

  if (flags & Brush) {
    const WBrush& brush = painter()->brush();

    if (brush.style() != NoBrush) {
      const WColor& color = painter()->brush().color();
      PixelPacket pp;
      WColorToPixelPacket(color, &pp);
      DrawSetFillColor(context_, &pp);
      DrawSetFillOpacity(context_, color.alpha() / 255.0);
    } else
      DrawSetFillOpacity(context_, 0);
  }

  if (flags & Font) {
    const WFont& font = painter()->font();

    const char *base;
    const char *italic = 0;
    const char *bold = 0;

    switch (font.genericFamily()) {
    case WFont::Default:
    case WFont::Serif:
      base = "Times";
      italic = "Italic";
      bold = "Bold";
      break;
    case WFont::SansSerif:
      base = "Helvetica";
      italic = "Olbique";
      bold = "Bold";
      break;
    case WFont::Monospace:
      base = "Courier";
      italic = "Oblique";
      bold = "Bold";
      break;
    case WFont::Fantasy: // Not really !
      base = "Symbol";
      break;
    case WFont::Cursive: // Not really !
      base = "ZapfDingbats";
    }

    if (italic)
      switch (font.style()) {
      case WFont::NormalStyle:
	italic = 0;
	break;
      default:
	break;
      }

    if (bold)
      switch (font.weight()) {
      case WFont::NormalWeight:
	bold = 0;
	break;
      case WFont::Bold:
      case WFont::Bolder:
	break;
      default:
	bold = 0;
      }

    std::string name = base;
    if (bold) {
      name += std::string("-") + bold;
      if (italic)
	name += italic;
    } else if (italic)
      name += std::string("-") + italic;

    if (name == "Times")
      name = "Times-Roman";

    DrawSetFont(context_, name.c_str());

    switch (font.size()) {
    case WFont::FixedSize:
      fontSize_ = font.fixedSize().toPixels();
      break;
    default:
      fontSize_ = 12;
    }

    DrawSetFontSize(context_, fontSize_);
  }
}

void WRasterImage::drawArc(const WRectF& rect,
			   double startAngle, double spanAngle)
{
  DrawArc(context_, rect.left(), rect.top(), rect.right(), rect.bottom(), 
	  startAngle, startAngle + spanAngle);
}

void WRasterImage::drawImage(const WRectF& rect, const std::string& imgUri,
			     int imgWidth, int imgHeight,
			     const WRectF& srect)
{
  // TODO: DrawComposite
}

void WRasterImage::drawLine(double x1, double y1, double x2, double y2)
{
  DrawLine(context_, x1, y1, x2, y2);
}

void WRasterImage::drawPath(const WPainterPath& path)
{
  if (!path.isEmpty()) {
    DrawPathStart(context_);

    drawPlainPath(path);

    DrawPathFinish(context_);
  }
}

void WRasterImage::setPixel(int x, int y, const WColor& c)
{
  if (painter_)
    throw WtException("renderPixel: cannot be used while a painter is active");

  PixelPacket *pixel = SetImagePixels(image_, x, y, 1, 1);
  WColorToPixelPacket(c, pixel);
  SyncImagePixels(image_);
}

void WRasterImage::drawPlainPath(const WPainterPath& path)
{ 
  const std::vector<WPainterPath::Segment>& segments = path.segments();

  if (segments.size() > 0
      && segments[0].type() != WPainterPath::Segment::MoveTo)
    DrawPathMoveToAbsolute(context_, 0, 0);

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    switch (s.type()) {
    case WPainterPath::Segment::MoveTo:
      DrawPathMoveToAbsolute(context_, s.x(), s.y());
      break;
    case WPainterPath::Segment::LineTo:
      DrawPathLineToAbsolute(context_, s.x(), s.y());
      break;
    case WPainterPath::Segment::CubicC1: {
      const double x1 = s.x();
      const double y1 = s.y();
      const double x2 = segments[i+1].x();
      const double y2 = segments[i+1].y();
      const double x3 = segments[i+2].x();
      const double y3 = segments[i+2].y();

      DrawPathCurveToAbsolute(context_, x1, y1, x2, y2, x3, y3);

      i += 2;
      break;
    }
    case WPainterPath::Segment::CubicC2:
    case WPainterPath::Segment::CubicEnd:
      assert(false);
    case WPainterPath::Segment::ArcC: {
      WPointF current = path.positionAtSegment(i);
      // See also WSvgImage arc drawing

      const double cx = s.x();
      const double cy = s.y();
      const double rx = segments[i+1].x();
      const double ry = segments[i+1].y();
      const double theta1 = -WTransform::degreesToRadians(segments[i+2].x());
      const double deltaTheta
	= -WTransform::degreesToRadians(adjust360(segments[i+2].y()));

      const double x1 = rx * std::cos(theta1) + cx;
      const double y1 = ry * std::sin(theta1) + cy;
      const double x2 = rx * std::cos(theta1 + deltaTheta) + cx;
      const double y2 = ry * std::sin(theta1 + deltaTheta) + cy;
      const int fa = (std::fabs(deltaTheta) > M_PI ? 1 : 0);
      const int fs = (deltaTheta > 0 ? 1 : 0);

      if (!fequal(current.x(), x1) || !fequal(current.y(), y1))
	DrawPathLineToAbsolute(context_, x1, y1);

      DrawPathEllipticArcAbsolute(context_, rx, ry, 0, fa, fs, x2, y2);

      i += 2;
      break;
    }
    case WPainterPath::Segment::ArcR:
    case WPainterPath::Segment::ArcAngleSweep:
      assert(false);
    case WPainterPath::Segment::QuadC: {
      const double x1 = s.x();
      const double y1 = s.y();
      const double x2 = segments[i+1].x();
      const double y2 = segments[i+1].y();

      DrawPathCurveToQuadraticBezierAbsolute(context_, x1, y1, x2, y2);

      i += 1;

      break;
    }
    case WPainterPath::Segment::QuadEnd:
      assert(false);
    }
  }
}

void WRasterImage::drawText(const WRectF& rect, WFlags<AlignmentFlag> flags,
			    const WString& text)
{
  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  /*
   * For centering horizontally, we use graphicsmagick totally insane
   * gravity system.
   */
  GravityType gravity = NorthWestGravity;

  const WTransform& t = painter()->combinedTransform();

  WPointF p;

  double ascent = 0.8 * fontSize_;
  double descent = fontSize_ - ascent;

  switch (verticalAlign) {
  case AlignTop:
    p = rect.topLeft();
    p.setY(p.y() + ascent);
    break;
  case AlignMiddle:
    p = rect.center();
    p.setY(p.y() + ascent - fontSize_/2);
    break;
  case AlignBottom:
    p = rect.bottomLeft();
    p.setY(p.y() - descent);
    break;
  default:
    break;
  }

  switch (horizontalAlign) {
  case AlignLeft:
    gravity = NorthWestGravity;
    p.setX(rect.left());
    break;
  case AlignCenter:
    gravity = NorthGravity;
    p.setX(rect.center().x());

    p = t.map(p);
    p.setX(p.x() - w_/2);
    p = t.inverted().map(p);
    break;
  case AlignRight:
    gravity = NorthEastGravity;
    p.setX(rect.right());

    p = t.map(p);
    p.setX(w_ - p.x());
    p = t.inverted().map(p);
    break;
  default:
    break;
  }

  DrawPushGraphicContext(context_);

  PixelPacket pp;
  WColorToPixelPacket(painter()->pen().color(), &pp);
  DrawSetFillColor(context_, &pp);
  DrawSetFillOpacity(context_, 1);
  DrawSetStrokeOpacity(context_, 0);
  
  DrawSetGravity(context_, gravity);

  std::string utf8 = text.toUTF8();
  Utils::replace(utf8, '%', "%%");

  DrawAnnotation(context_, p.x(), p.y(), (const unsigned char *)utf8.c_str());

  DrawPopGraphicContext(context_);

  setChanged(Transform);
}

void WRasterImage::handleRequest(const Http::Request& request,
				 Http::Response& response)
{
  response.setMimeType("image/" + type_);

  ImageInfo info;
  GetImageInfo(&info);

  ExceptionInfo exception;
  GetExceptionInfo(&exception);

  std::size_t size;
  void *data = ImageToBlob(&info, image_, &size, &exception);

  response.out().write((const char *)data, size);

  free(data);
}

}
