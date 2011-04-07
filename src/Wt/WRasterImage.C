/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBrush"
#include "Wt/WFontMetrics"
#include "Wt/WPainter"
#include "Wt/WPen"
#include "Wt/WRasterImage"
#include "Wt/WTransform"
#include "Wt/Http/Response"

#include "Wt/FontSupport.h"
#include "web/WtException.h"
#include "web/Utils.h"

#include <cstdio>
#include <cmath>
#include <magick/api.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#if MagickLibVersion < 0x030000
#error GraphicsMagick version must be at least 1.3.0
#error You should upgrade GraphicsMagick or disable WRasterImage with -DENABLE_GM=OFF
#endif

#define MAGICK_IMPLEMENTATION
#define MagickEpsilon 1E-5
#include "alpha_composite.h"
#undef MAGICK_IMPLEMENTATION
#undef MagickEpsilon

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef WIN32
namespace {
  double round(double x)
  {
    return floor(x + 0.5);
  }
}
#endif

namespace {
  static const double EPSILON = 1E-5;

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
    pp->opacity = 255 - static_cast<unsigned char>(color.alpha());
  }

  bool isTranslation(const Wt::WTransform& t) 
  {
    return std::fabs(t.m11() - 1.0) < EPSILON
      && std::fabs(t.m12() - 0.0) < EPSILON
      && std::fabs(t.m21() - 0.0) < EPSILON
      && std::fabs(t.m22() - 1.0) < EPSILON;
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
    painter_(0),
    currentClipPath_(-1),
    currentClipPathRendered_(-1)
{
  w_ = static_cast<unsigned long>(width.toPixels());
  h_ = static_cast<unsigned long>(height.toPixels());

  if (!w_ || !h_)
    throw WtException("Raster image should have non-0 width and height");

  unsigned long bufSize = 4 * w_ * h_;
  pixels_ = new unsigned char[bufSize];
  for (unsigned i = 0; i < w_ * h_; ++i) {
    pixels_[i*4] = pixels_[i*4 + 1] = pixels_[i*4 + 2] = 254;
    pixels_[i*4 + 3] = 0;
  }

  ExceptionInfo exception;
  GetExceptionInfo(&exception);
  image_ = ConstituteImage(w_, h_, "RGBA", CharPixel, pixels_, &exception);

  SetImageType(image_, TrueColorMatteType);
  SetImageOpacity(image_, 254); // 255 seems a special value...
                                // 254 will have to do

  std::string magick = type;
  std::transform(magick.begin(), magick.end(), magick.begin(), toupper);
  strcpy(image_->magick, type.c_str());

  context_ = 0;
  fontSupport_ = 0;
}

void WRasterImage::clear()
{
  PixelPacket *pixel = SetImagePixels(image_, 0, 0, w_, h_);
  for (unsigned i = 0; i < w_ * h_; ++i)
    WColorToPixelPacket(WColor(0, 0, 0, 1), pixel + i);
  SyncImagePixels(image_);
}

WRasterImage::~WRasterImage()
{
  beingDeleted();

  DestroyImage(image_);
  delete[] pixels_;

  delete fontSupport_;
}

void WRasterImage::addFontCollection(const std::string& directory,
				     bool recursive)
{
  fontSupport_->addFontCollection(directory, recursive);
}

WFlags<WPaintDevice::FeatureFlag> WRasterImage::features() const
{
  if (fontSupport_->canRender())
    return HasFontMetrics;
  else
    return 0;
}

void WRasterImage::init()
{
  fontSupport_ = new FontSupport(this);

  /* If the font support can actually render fonts to bitmaps, we use that */
  if (fontSupport_->canRender())
    fontSupport_->setDevice(0);

  internalInit(true);
}

void WRasterImage::internalInit(bool applyChanges)
{
  if (!context_) {
    currentClipPathRendered_ = -1;
    SetImageClipMask(image_, 0);

    context_ = DrawAllocateContext(0, image_);

    DrawPushGraphicContext(context_);

    DrawSetFillRule(context_, NonZeroRule);
    DrawSetTextEncoding(context_, "UTF-8");

    DrawPushGraphicContext(context_); // for painter->clipping();
    DrawPushGraphicContext(context_); // for painter->combinedTransform()

    if (applyChanges)
      setChanged(Clipping | Transform | Pen | Brush | Font | Hints);
  }
}

void WRasterImage::done()
{
  internalDone();

  delete fontSupport_;
  fontSupport_ = 0;
}

void WRasterImage::internalDone()
{
  if (context_) {
    DrawPopGraphicContext(context_); // for painter->combinedTransform()
    DrawPopGraphicContext(context_); // for painter->clipping();
    DrawPopGraphicContext(context_);

    DrawRender(context_);

    DrawDestroyContext(context_);

    context_ = 0;

    SetImageClipMask(image_, 0);
    currentClipPathRendered_ = -1;
  }
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
  /*
   * If it is only Clipping that changes, we may not need to manipulate
   * the context_, since some clipping paths are cached.
   */
  if (flags != Clipping)
    internalInit();

  if (flags & Clipping) {
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
	internalInit(false);

	DrawPopGraphicContext(context_);
	DrawPopGraphicContext(context_);
	DrawPushGraphicContext(context_);

	index = nextIndex + 1;
	DrawPushClipPath
	  (context_,("clip" + boost::lexical_cast<std::string>(index)).c_str());
	drawPath(painter()->clipPath());
	DrawPopClipPath(context_);

	clipPathCache_.pop_back(); // implement LRU
	clipPathCache_.push_front(std::make_pair(painter()->clipPath(), index));
      } else if (context_) {
	DrawPopGraphicContext(context_);
	DrawPopGraphicContext(context_);
	DrawPushGraphicContext(context_);
      }

      currentClipPath_ = index;

      if (context_) {
	const WTransform& t = painter()->clipPathTransform();

	applyTransform(t);

	DrawSetClipUnits(context_, UserSpaceOnUse);
	DrawSetClipPath(context_, currentClipPathName().c_str());

	applyTransform(t.inverted());
      }
    } else {
      currentClipPath_ = -1;

      if (context_) {
	DrawPopGraphicContext(context_);
	DrawPopGraphicContext(context_);
	DrawPushGraphicContext(context_);
      }
    }

    if (context_) {
      DrawPushGraphicContext(context_);
      flags = Transform;
    }
  }

  if (flags != Clipping)
    internalInit();

  if (flags & Transform) {
    setTransform(painter()->combinedTransform());
    flags = Pen | Brush | Font | Hints;
  }

  if (flags & Hints) {
    if (!(painter()->renderHints() & WPainter::Antialiasing))
      DrawSetStrokeAntialias(context_, 0);
    else
      DrawSetStrokeAntialias(context_, 1);
  }

  if (flags & Pen) {
    const WPen& pen = painter()->pen();

    if (pen.style() != NoPen) {
      const WColor& color = pen.color();

      PixelPacket pp;
      WColorToPixelPacket(color, &pp);
      pp.opacity = 0;
      DrawSetStrokeColor(context_, &pp);
      DrawSetStrokeOpacity(context_, color.alpha() / 255.0);

      WLength w = pen.width();
      w = painter()->normalizedPenWidth(w, w.value() == 0);
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

    } else {
      DrawSetStrokeWidth(context_, 0);
      DrawSetStrokeOpacity(context_, 0);
    }
  }

  if (flags & Brush) {
    const WBrush& brush = painter()->brush();

    if (brush.style() != NoBrush) {
      const WColor& color = painter()->brush().color();
      PixelPacket pp;
      WColorToPixelPacket(color, &pp);
      pp.opacity = 0;
      DrawSetFillColor(context_, &pp);
      DrawSetFillOpacity(context_, color.alpha() / 255.0);
    } else
      DrawSetFillOpacity(context_, 0);
  }

  if (flags & Font) {
    const WFont& font = painter()->font();

    std::string name;

    /*
     * First, consult the font support to locate a suitable font
     */
    if (fontSupport_->busy()) {
      /*
       * We have a resolved true type font.
       */
      name = fontSupport_->drawingFontPath();
    } else {
      FontSupport::FontMatch match = fontSupport_->matchFont(font);

      if (match.matched())
	name = match.fileName();
    }

    if (name.empty() || !fontSupport_->canRender()) {
      if (name.empty()) {
	/*
	 * FIXME, does this actually make any sense ?
	 */
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
      }

      DrawSetFont(context_, name.c_str());

      fontSize_ = font.sizeLength(12).toPixels();

      DrawSetFontSize(context_, fontSize_);

      renderText_ = true;
    } else
      renderText_ = false;
  }
}

std::string WRasterImage::currentClipPathName() const
{
  return "clip" + boost::lexical_cast<std::string>(currentClipPath_);
}

void WRasterImage::drawArc(const WRectF& rect,
			   double startAngle, double spanAngle)
{
  internalInit();

  DrawArc(context_, rect.left(), rect.top(), rect.right(), rect.bottom(), 
	  startAngle, startAngle + spanAngle);
}

void WRasterImage::drawImage(const WRectF& rect, const std::string& imgUri,
			     int imgWidth, int imgHeight,
			     const WRectF& srect)
{
  ImageInfo info;
  GetImageInfo(&info);

  ExceptionInfo exception;
  GetExceptionInfo(&exception);

  Image *cImage;
  if (boost::starts_with(imgUri, "data:")) {
    size_t dataEndPos = imgUri.find("data:") + 5;
    size_t commaPos = imgUri.find(",");
    if (commaPos == std::string::npos)
      commaPos = dataEndPos;
    
    std::string mimeTypeEncoding 
      = imgUri.substr(dataEndPos, commaPos - dataEndPos);

    std::string data = imgUri.substr(commaPos + 1);

    bool validMimeType = true;
    if (boost::istarts_with(mimeTypeEncoding, "image/png"))
      strcpy(info.magick, "PNG");
    else if (boost::istarts_with(mimeTypeEncoding, "image/gif"))
      strcpy(info.magick, "GIF");
    else if (boost::istarts_with(mimeTypeEncoding, "image/jpg")
	     || boost::istarts_with(mimeTypeEncoding, "image/jpeg"))
      strcpy(info.magick, "JPG");
    else 
    validMimeType = false;

    if (!validMimeType || !boost::ends_with(mimeTypeEncoding, "base64") 
	|| data.size() == 0)
      throw std::logic_error(std::string("The data URI is not in the expected "
					 "format: ") + imgUri);
    else {
      std::string content = imgUri.substr(commaPos);
      cImage = ReadInlineImage(&info, content.c_str(), &exception);
    }
  } else {
    strncpy(info.filename, imgUri.c_str(), 2048);
    cImage = ReadImage(&info, &exception);
  }

  if (cImage == 0) {
    std::cerr << "WRasterImage::drawImage failed: "
	      << exception.reason << ", "
	      << exception.description << std::endl;
    return;
  }

  RectangleInfo tocrop;
  tocrop.width = srect.width();
  tocrop.height = srect.height();
  tocrop.x = srect.x();
  tocrop.y = srect.y();
  Image *croppedImage = CropImage(cImage, &tocrop, &exception);

  const WTransform& t = painter()->combinedTransform();

  bool directComposite = false;
  if (isTranslation(t)
      && rect.width() == srect.width()
      && rect.height() == srect.height())
    directComposite = true;

  if (!directComposite) {
    internalInit();
    DrawComposite(context_, OverCompositeOp, rect.x(), rect.y(),
		  rect.width(), rect.height(), croppedImage);
  } else {
    internalDone();

    CompositeImage(image_, OverCompositeOp, croppedImage,
		   rect.x() + t.dx(), rect.y() + t.dy());
  }

  DestroyImage(croppedImage);
  DestroyImage(cImage);
}

void WRasterImage::drawLine(double x1, double y1, double x2, double y2)
{
  internalInit();

  DrawLine(context_, x1, y1, x2, y2);
}

void WRasterImage::drawPath(const WPainterPath& path)
{
  internalInit();

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

WColor WRasterImage::getPixel(int x, int y) 
{
  PixelPacket *pixel = GetImagePixels(image_, x, y, 1, 1);
  return WColor(pixel->red, pixel->green, pixel->blue, pixel->opacity);
}

void WRasterImage::drawPlainPath(const WPainterPath& path)
{
  internalInit();

  const std::vector<WPainterPath::Segment>& segments = path.segments();

  if (segments.size() > 0
      && segments[0].type() != WPainterPath::Segment::MoveTo)
    DrawPathMoveToAbsolute(context_, -0.5, -0.5);

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    switch (s.type()) {
    case WPainterPath::Segment::MoveTo:
      DrawPathMoveToAbsolute(context_, s.x() - 0.5, s.y() - 0.5);
      break;
    case WPainterPath::Segment::LineTo:
      DrawPathLineToAbsolute(context_, s.x() - 0.5, s.y() - 0.5);
      break;
    case WPainterPath::Segment::CubicC1: {
      const double x1 = s.x();
      const double y1 = s.y();
      const double x2 = segments[i+1].x();
      const double y2 = segments[i+1].y();
      const double x3 = segments[i+2].x();
      const double y3 = segments[i+2].y();

      DrawPathCurveToAbsolute(context_, x1 - 0.5, y1 - 0.5,
			      x2 - 0.5, y2 - 0.5,
			      x3 - 0.5, y3 - 0.5);

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
	DrawPathLineToAbsolute(context_, x1 - 0.5, y1 - 0.5);

      DrawPathEllipticArcAbsolute(context_, rx, ry, 0, fa, fs,
				  x2 - 0.5, y2 - 0.5);

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

      DrawPathCurveToQuadraticBezierAbsolute(context_, x1 - 0.5, y1 - 0.5,
					     x2 - 0.5, y2 - 0.5);

      i += 1;

      break;
    }
    case WPainterPath::Segment::QuadEnd:
      assert(false);
    }
  }
}

void WRasterImage::drawText(const WRectF& rect, 
			    WFlags<AlignmentFlag> flags,
			    TextFlag textFlag,
			    const WString& text)
{
  if (textFlag == TextWordWrap)
    throw std::logic_error("WRasterImage::drawText() " 
			   "TextWordWrap is not supported");

  if (renderText_) {
    internalInit();

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
    pp.opacity = 0;
    DrawSetFillColor(context_, &pp);
    DrawSetFillOpacity(context_, painter()->pen().color().alpha() / 255.0);
    DrawSetStrokeOpacity(context_, 0);
  
    DrawSetGravity(context_, gravity);

    std::string utf8 = text.toUTF8();
    Utils::replace(utf8, '%', "%%");

    DrawAnnotation(context_, p.x(), p.y(), (const unsigned char *)utf8.c_str());

    DrawPopGraphicContext(context_);

    setChanged(Transform);
  } else {
    WTransform t = painter()->combinedTransform();

    if (painter()->hasClipping())
      setChanged(Clipping);

    internalDone();

    if (currentClipPath_ != currentClipPathRendered_) {
      if (currentClipPath_ != -1) {
	ImageInfo info;
	GetImageInfo(&info);
	DrawInfo *drawInfo = (DrawInfo *)malloc(sizeof(DrawInfo));
	GetDrawInfo(&info, drawInfo);

	AffineMatrix& m = drawInfo->affine;
	m.sx = t.m11();
	m.rx = t.m12();
	m.ry = t.m21();
	m.sy = t.m22();
	m.tx = t.dx();
	m.ty = t.dy();

	drawInfo->clip_units = UserSpaceOnUse;
	DrawClipPath(image_, drawInfo, currentClipPathName().c_str());

	DestroyDrawInfo(drawInfo);	
      } else {
	SetImageClipMask(image_, 0);
      }

      currentClipPathRendered_ = currentClipPath_;
    }

    WRectF renderRect;

    int w, h, x0, y0;
    if (isTranslation(t)) {
      x0 = round(t.dx() + rect.x());
      y0 = round(t.dy() + rect.y());
      w = rect.width();
      h = rect.height();
      renderRect = WRectF(0, 0, rect.width(), rect.height());
      t = WTransform();
    } else {
      x0 = 0;
      y0 = 0;
      w = w_;
      h = h_;
      renderRect = rect;
    }

    FontSupport::Bitmap bitmap(w, h);
    fontSupport_->drawText(painter_->font(), renderRect,
			   t, bitmap, flags, text);

    PixelPacket *pixels = GetImagePixels(image_, 0, 0, w_, h_);

    WColor c = painter()->pen().color();
    PixelPacket pc;
    WColorToPixelPacket(c, &pc);
 
    for (int y = 0; y < h; ++y) {
      int iy = y0 + y;

      if (iy < 0 || iy >= (int)h_)
	continue;

      for (int x = 0; x < w; ++x) {
	int ix = x0 + x;

	if (ix < 0 || ix >= (int)w_)
	  continue;

	PixelPacket *pixel = pixels + (iy * w_ + ix);

	unsigned char bit = bitmap.value(x, y);

	if (bit > 0) {
	  double alpha = (255 - bit) * (255.0 - pc.opacity) / 255.0;
	  AlphaCompositePixel(pixel, &pc, alpha,
			      pixel, pixel->opacity);
	}
      }
    }

    SyncImagePixels(image_);
  }
}

WTextItem WRasterImage::measureText(const WString& text, double maxWidth,
				    bool wordWrap)
{
  if (renderText_)
    throw std::logic_error("WRasterImage::measureText() not supported");
  else
    return fontSupport_->measureText(painter_->font(), text, maxWidth,
				     wordWrap);
}

WFontMetrics WRasterImage::fontMetrics()
{
  if (renderText_)
    throw std::logic_error("WRasterImage::fontMetrics() not supported");
  else
    return fontSupport_->fontMetrics(painter_->font());
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
