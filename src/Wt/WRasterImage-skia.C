/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBrush"
#include "Wt/WException"
#include "Wt/WFontMetrics"
#include "Wt/WLogger"
#include "Wt/WPainter"
#include "Wt/WPen"
#include "Wt/WRasterImage"
#include "Wt/WTransform"
#include "Wt/Http/Response"

#include "Wt/FontSupport.h"
#include "WebUtils.h"
#include "UriUtils.h"

#include <cstdio>
#include <cmath>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <SkBitmap.h>
#include <SkBitmapDevice.h>
#include <SkDashPathEffect.h>
#include <SkImageDecoder.h>
#include <SkImageEncoder.h>
#include <SkStream.h>
#include <SkTypeface.h>

//#include <SkForceLinking.h>


namespace {
  inline SkColor fromWColor(const Wt::WColor &color)
  {
    return SkColorSetARGB(color.alpha(),
			  color.red(), color.green(), color.blue());
  }
}

namespace Wt {

LOGGER("WRasterImage");

class WRasterImage::Impl {
public:
  Impl():
    w_(0),
    h_(0),
    bitmap_(0),
    device_(0),
    canvas_(0)
  {}
  unsigned w_, h_;
  std::string type_;
  SkBitmap *bitmap_;
  SkBitmapDevice *device_;
  SkCanvas *canvas_;
  SkPaint strokePaint_;
  SkPaint fillPaint_;
  SkPaint textPaint_;

  void applyTransform(const WTransform& t);
  void setTransform(const WTransform& t);
  void drawPlainPath(SkPath &p, const WPainterPath& path);
};

WRasterImage::WRasterImage(const std::string& type,
			   const WLength& width, const WLength& height,
			   WObject *parent)
  : WResource(parent),
    width_(width),
    height_(height),
    painter_(0),
    impl_(new Impl)
{
  impl_->type_ = type;
  impl_->w_ = static_cast<unsigned long>(width.toPixels());
  impl_->h_ = static_cast<unsigned long>(height.toPixels());
  
  if (!impl_->w_ || !impl_->h_) {
    impl_->bitmap_ = 0;
    return;
  }
  
  impl_->bitmap_ = new SkBitmap();
  impl_->bitmap_->setConfig(SkBitmap::kARGB_8888_Config, impl_->w_, impl_->h_);
  impl_->bitmap_->allocPixels();
  impl_->bitmap_->eraseARGB(0, 0, 0, 0);
  impl_->device_ = new SkBitmapDevice(*impl_->bitmap_);
  impl_->canvas_ = new SkCanvas(impl_->device_);
  
  impl_->textPaint_.setStyle(SkPaint::kStrokeAndFill_Style);
  impl_->textPaint_.setTextEncoding(SkPaint::kUTF8_TextEncoding);
  impl_->strokePaint_.setStyle(SkPaint::kStroke_Style);
  impl_->fillPaint_.setStyle(SkPaint::kFill_Style);
  
}
  
void WRasterImage::clear()
{
  impl_->canvas_->clear(0);
}

WRasterImage::~WRasterImage()
{
  beingDeleted();
  delete impl_->canvas_;
  delete impl_->device_;
  delete impl_->bitmap_;
  delete impl_;
}

void WRasterImage::addFontCollection(const std::string& directory,
				     bool recursive)
{
#if 0
  fontSupport_->addFontCollection(directory, recursive);
#endif
}
  
WFlags<WPaintDevice::FeatureFlag> WRasterImage::features() const
{
  return HasFontMetrics;
}

void WRasterImage::init()
{
  if (!impl_->w_ || !impl_->h_)
    throw WException("Raster image should have non-0 width and height");

  // save unit matrix & empty clipping
  impl_->canvas_->save();

  setChanged(Clipping | Transform | Pen | Brush | Font | Hints);
}

void WRasterImage::done()
{
  impl_->canvas_->restore();
}

void WRasterImage::Impl::applyTransform(const WTransform& t)
{
  SkMatrix sm;
  sm.setAll(SkDoubleToScalar(t.m11()),
	    SkDoubleToScalar(t.m12()),
	    SkDoubleToScalar(t.dx()),
	    SkDoubleToScalar(t.m21()),
	    SkDoubleToScalar(t.m22()),
	    SkDoubleToScalar(t.dy()),
	    0,
	    0,
	    SkDoubleToScalar(1.0));
  canvas_->concat(sm);
}

void WRasterImage::Impl::setTransform(const WTransform& t)
{
  /*
  std::cout << "WRasterImage::setTransform "
	    << t.m11() << ", "
	    << t.m12() << ", "
	    << t.m21() << ", "
	    << t.m22() << ", "
	    << t.dx() << ", "
	    << t.dy() << ", "
	    << std::endl;
  */
  canvas_->resetMatrix();
  applyTransform(t);
}

void WRasterImage::setChanged(WFlags<ChangeFlag> flags)
{
  if (flags & Clipping) {
    
    if (painter()->hasClipping()) {
      impl_->setTransform(painter()->clipPathTransform());
      SkPath clipPath;
      impl_->drawPlainPath(clipPath, painter()->clipPath());
      impl_->canvas_->clipPath(clipPath, SkRegion::kReplace_Op);
    } else {
      impl_->canvas_->restore();
      impl_->canvas_->save();
    }
    impl_->setTransform(painter()->combinedTransform());
  }

  if (flags & Transform) {
    impl_->setTransform(painter()->combinedTransform());
    flags = Pen | Brush | Font | Hints;
  }

  if (flags & Hints) {
    if (!(painter()->renderHints() & WPainter::Antialiasing)) {
      impl_->strokePaint_.setAntiAlias(false);
      impl_->fillPaint_.setAntiAlias(false);
      impl_->textPaint_.setAntiAlias(false);
    } else {
      impl_->strokePaint_.setAntiAlias(true);
      impl_->fillPaint_.setAntiAlias(true);
      impl_->textPaint_.setAntiAlias(true);
    }
  }

  if (flags & Pen) {
    const WPen& pen = painter()->pen();

    if (pen.style() != NoPen) {
      const WColor& color = pen.color();

      impl_->strokePaint_.setColor(fromWColor(color));

      WLength w = pen.width();
      impl_->strokePaint_.setStrokeWidth(SkIntToScalar(w.toPixels()));

      switch (pen.capStyle()) {
      case FlatCap:
	impl_->strokePaint_.setStrokeCap(SkPaint::kButt_Cap);
	break;
      case SquareCap:
	impl_->strokePaint_.setStrokeCap(SkPaint::kSquare_Cap);
	break;
      case RoundCap:
	impl_->strokePaint_.setStrokeCap(SkPaint::kRound_Cap);
	break;
      }

      switch (pen.joinStyle()) {
      case MiterJoin:
	impl_->strokePaint_.setStrokeJoin(SkPaint::kMiter_Join);
	break;
      case BevelJoin:
	impl_->strokePaint_.setStrokeJoin(SkPaint::kBevel_Join);
	break;
      case RoundJoin:
	impl_->strokePaint_.setStrokeJoin(SkPaint::kRound_Join);
	break;
      }

      SkPathEffect *pe = impl_->strokePaint_.setPathEffect(0);
      if (pe)
	pe->unref();
      switch (pen.style()) {
      case NoPen:
	break;
      case SolidLine:
	break;
      case DashLine: {
	const SkScalar dasharray[] = { SkIntToScalar(4), SkIntToScalar(2) };
	impl_->strokePaint_.setPathEffect(new SkDashPathEffect(dasharray, 2,
							false))->unref();
	break;
      }
      case DotLine: {
	const SkScalar dasharray[] = { SkIntToScalar(1), SkIntToScalar(2) };
	impl_->strokePaint_.setPathEffect(new SkDashPathEffect(dasharray, 2,
							false))->unref();
	break;
      }
      case DashDotLine: {
	const SkScalar dasharray[] = {
	  SkIntToScalar(4),
	  SkIntToScalar(2),
	  SkIntToScalar(1),
	  SkIntToScalar(2)
	};
	impl_->strokePaint_.setPathEffect(new SkDashPathEffect(dasharray, 4,
							false))->unref();
	break;
      }
      case DashDotDotLine: {
	const SkScalar dasharray[] = {
	  SkIntToScalar(4),
	  SkIntToScalar(2),
	  SkIntToScalar(1),
	  SkIntToScalar(2),
	  SkIntToScalar(1),
	  SkIntToScalar(2)
	};
	impl_->strokePaint_.setPathEffect(new SkDashPathEffect(dasharray, 6,
							false))->unref();
	break;
      }
      }

    }
  }

  if (flags & Brush) {
    const WBrush& brush = painter()->brush();
    if (brush.style() != NoBrush) {
      const WColor& color = painter()->brush().color();
      impl_->fillPaint_.setColor(fromWColor(color));
    }
  }

  if (flags & Font) {
    const WFont& font = painter()->font();

    const char *base = 0;
    switch (font.genericFamily()) {
    case WFont::Default:
    case WFont::Serif:
      base = "Times";
      break;
    case WFont::SansSerif:
      base = "Helvetica";
      break;
    case WFont::Monospace:
      base = "Courier";
      break;
    case WFont::Fantasy: // Not really !
      base = "Symbol";
      break;
    case WFont::Cursive: // Not really !
      base = "ZapfDingbats";
    }

    int style = SkTypeface::kNormal;
    if (font.style() != WFont::NormalStyle)
      style |= SkTypeface::kItalic;
    if (font.weight() == WFont::Bold ||
	font.weight() == WFont::Bolder)
      style |= SkTypeface::kBold;

    impl_->textPaint_.setTypeface(SkTypeface::CreateFromName(base,
				    (SkTypeface::Style)style))->unref();
    impl_->textPaint_.setTextSize(SkIntToScalar(font.sizeLength(12).toPixels()));
  }
}

void WRasterImage::drawArc(const WRectF& rect,
			   double startAngle, double spanAngle)
{
  SkRect r = SkRect::MakeLTRB(SkDoubleToScalar(rect.left()),
			      SkDoubleToScalar(rect.top()),
			      SkDoubleToScalar(rect.right()),
			      SkDoubleToScalar(rect.bottom()));
  if (painter()->brush().style() != NoBrush) {
    impl_->canvas_->drawArc(r,
      SkDoubleToScalar(-startAngle),
      SkDoubleToScalar(-spanAngle),
      false, impl_->fillPaint_);
  }
  if (painter()->pen().style() != NoPen) {
    impl_->canvas_->drawArc(r,
      SkDoubleToScalar(-startAngle),
      SkDoubleToScalar(-spanAngle),
      false, impl_->strokePaint_);
  }
}

void WRasterImage::drawImage(const WRectF& rect, const std::string& imgUri,
			     int imgWidth, int imgHeight,
			     const WRectF& srect)
{
  SkBitmap bitmap;
  bool success = false;
  if (DataUri::isDataUri(imgUri)) {
    DataUri uri(imgUri);
    success =
      SkImageDecoder::DecodeMemory(&uri.data[0], uri.data.size(),
				   &bitmap, SkBitmap::kARGB_8888_Config,
				   SkImageDecoder::kDecodePixels_Mode, 0);
    if (!success)
      throw WException("WRasterImage: could not decode data URL (mime type "
		       + uri.mimeType);
  } else {
    success =
      SkImageDecoder::DecodeFile(imgUri.c_str(),
				 &bitmap, SkBitmap::kARGB_8888_Config,
				 SkImageDecoder::kDecodePixels_Mode, 0);
    if (!success)
      throw WException("WRasterImage: could not load file " + imgUri);
  }

  SkRect src = SkRect::MakeLTRB(SkDoubleToScalar(srect.left()),
				SkDoubleToScalar(srect.top()),
				SkDoubleToScalar(srect.right()),
				SkDoubleToScalar(srect.bottom()));
  SkRect dst = SkRect::MakeLTRB(SkDoubleToScalar(rect.left()),
				SkDoubleToScalar(rect.top()),
				SkDoubleToScalar(rect.right()),
				SkDoubleToScalar(rect.bottom()));
  impl_->canvas_->drawBitmapRectToRect(bitmap, &src, dst);
}

void WRasterImage::drawLine(double x1, double y1, double x2, double y2)
{
  impl_->canvas_->drawLine(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
    SkDoubleToScalar(x2), SkDoubleToScalar(y2), impl_->strokePaint_);
}

void WRasterImage::drawPath(const WPainterPath& path)
{
  if (!path.isEmpty()) {
    SkPath p;
    impl_->drawPlainPath(p, path);
    
    if (painter()->brush().style() != NoBrush) {
      impl_->canvas_->drawPath(p, impl_->fillPaint_);
    }
    if (painter()->pen().style() != NoPen) {
      impl_->canvas_->drawPath(p, impl_->strokePaint_);
    }
  }
}

void WRasterImage::setPixel(int x, int y, const WColor& c)
{
  if (painter_)
    throw WException("WRasterImage::setPixel(): cannot be used while a "
		     "painter is active");
  uint8_t* addr = (uint8_t *)impl_->bitmap_->getAddr(x, y);
  addr[0] = c.blue();
  addr[1] = c.green();
  addr[2] = c.red();
  addr[3] = c.alpha();
}

void WRasterImage::getPixels(void *data)
{
  impl_->canvas_->flush();
  uint32_t *pixel = impl_->bitmap_->getAddr32(0, 0);
  unsigned char *d = (unsigned char *)data;
  if (pixel) {
    int i = 0;
    for (int p = 0; p < impl_->w_ * impl_->h_; ++p) {
      d[i++] = SkColorGetR(*pixel);
      d[i++] = SkColorGetG(*pixel);
      d[i++] = SkColorGetB(*pixel);
      d[i++] = SkColorGetA(*pixel);
      pixel++;
    }
  } else {
    throw WException("WRasterImage::getPixels(): error: no data");
  }
}

WColor WRasterImage::getPixel(int x, int y) 
{
  impl_->canvas_->flush();
  SkColor c = impl_->bitmap_->getColor(x, y);
  return WColor(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c), SkColorGetA(c));
}

void WRasterImage::Impl::drawPlainPath(SkPath &p, const WPainterPath& path)
{
  const std::vector<WPainterPath::Segment>& segments = path.segments();

  if (segments.size() > 0
      && segments[0].type() != WPainterPath::Segment::MoveTo)
    p.moveTo(SkDoubleToScalar(0), SkDoubleToScalar(0));

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    switch (s.type()) {
    case WPainterPath::Segment::MoveTo:
      p.moveTo(SkDoubleToScalar(s.x()), SkDoubleToScalar(s.y()));
      break;
    case WPainterPath::Segment::LineTo:
      p.lineTo(SkDoubleToScalar(s.x()), SkDoubleToScalar(s.y()));
      break;
    case WPainterPath::Segment::CubicC1: {
      const double x1 = s.x();
      const double y1 = s.y();
      const double x2 = segments[i+1].x();
      const double y2 = segments[i+1].y();
      const double x3 = segments[i+2].x();
      const double y3 = segments[i+2].y();
      p.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                SkDoubleToScalar(x3), SkDoubleToScalar(y3));
      i += 2;
      break;
    }
    case WPainterPath::Segment::CubicC2:
    case WPainterPath::Segment::CubicEnd:
      assert(false);
    case WPainterPath::Segment::ArcC: {
      const double x = s.x();
      const double y = s.y();
      const double width = segments[i+1].x();
      const double height = segments[i+1].y();
      const double startAngle = segments[i+2].x();
      const double sweepAngle = segments[i+2].y();
      SkRect rect = SkRect::MakeXYWH(SkDoubleToScalar(x - width),
				     SkDoubleToScalar(y - height),
				     SkDoubleToScalar(width * 2.0),
				     SkDoubleToScalar(height * 2.0));
      if (sweepAngle != 360) 
	p.arcTo(rect,
		SkDoubleToScalar(-startAngle), SkDoubleToScalar(-sweepAngle),
		false);
      else
	p.addOval(rect, SkPath::kCCW_Direction);

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
      
      p.quadTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
              SkDoubleToScalar(x2), SkDoubleToScalar(y2));

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
#if 0
  SkRect r = SkRect::MakeLTRB(SkDoubleToScalar(rect.left()),
			      SkDoubleToScalar(rect.top()),
			      SkDoubleToScalar(rect.right()),
			      SkDoubleToScalar(rect.bottom()));
  canvas_->drawRect(r, strokePaint_);
#endif
  std::string txt = text.toUTF8();

  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;
  
  const WTransform& t = painter()->combinedTransform();
  
  WPointF p;
  
  struct SkPaint::FontMetrics metrics;
  impl_->textPaint_.getFontMetrics(&metrics);
  double ascent = SkScalarToFloat(metrics.fAscent);
  double descent = SkScalarToFloat(metrics.fDescent);
  
  switch (verticalAlign) {
  case AlignTop:
    p = rect.topLeft();
    p.setY(p.y() - ascent);
    break;
  case AlignMiddle:
    p = rect.center();
    //p.setY(p.y() + SkScalarToFloat(textPaint_.getTextSize())/2);
    p.setY(p.y() - ascent/2);
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
    impl_->textPaint_.setTextAlign(SkPaint::kLeft_Align);
    p.setX(rect.left());
    break;
  case AlignCenter:
    impl_->textPaint_.setTextAlign(SkPaint::kCenter_Align);
    p.setX(rect.center().x());
    break;
  case AlignRight:
    impl_->textPaint_.setTextAlign(SkPaint::kRight_Align);
    p.setX(rect.right());
    break;
  default:
    break;
  }
  
  impl_->canvas_->drawText(txt.c_str(), txt.size(),
		    SkDoubleToScalar(p.x()), SkDoubleToScalar(p.y()),
		    impl_->textPaint_);
}

WTextItem WRasterImage::measureText(const WString& text, double maxWidth,
				    bool wordWrap)
{
  SkScalar w =
    impl_->textPaint_.measureText(text.toUTF8().c_str(), text.toUTF8().size());
  return WTextItem(text, SkScalarToFloat(w), -1);

  // TODO: what about wordwrap?
}

WFontMetrics WRasterImage::fontMetrics()
{
  struct SkPaint::FontMetrics metrics;
  impl_->textPaint_.getFontMetrics(&metrics);
  // assumed all are to be positive - ascent of skia is negative
  double ascent = -SkScalarToFloat(metrics.fAscent);
  double descent = SkScalarToFloat(metrics.fDescent);
  double leading = SkScalarToFloat(metrics.fLeading);

  return WFontMetrics(painter_->font(), leading, ascent, descent);
}

class RasterStream : public SkWStream {
public:
  RasterStream(std::ostream &os):
    os_(os)
  {
  }

  virtual bool write(const void* buffer, size_t size) {
    os_.write((const char *)buffer, size);
    return os_.good();
  }

private:
  std::ostream &os_;
};

void WRasterImage::handleRequest(const Http::Request& request,
				 Http::Response& response)
{
  response.setMimeType("image/" + impl_->type_);

  RasterStream s(response.out());

  if (impl_->type_ == "png")
    SkImageEncoder::EncodeStream(&s, *impl_->bitmap_, SkImageEncoder::kPNG_Type, 100);
  else if (impl_->type_ == "jpg")
    SkImageEncoder::EncodeStream(&s, *impl_->bitmap_, SkImageEncoder::kJPEG_Type, 100);

}

}
