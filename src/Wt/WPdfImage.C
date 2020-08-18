/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBrush.h"
#include "Wt/WException.h"
#include "Wt/WFontMetrics.h"
#include "Wt/WLogger.h"
#include "Wt/WPainter.h"
#include "Wt/WPen.h"
#include "Wt/WPdfImage.h"
#include "Wt/WTransform.h"
#include "Wt/Http/Response.h"

#include "Wt/FontSupport.h"
#include "PdfUtils.h"
#include "UriUtils.h"
#include "ImageUtils.h"
#include "WebUtils.h"

#include <cmath>
#include <ctype.h>
#include <stdio.h>
#include <hpdf.h>
#ifdef WT_WIN32
// Disable warnings about conversions from double to real (data loss)
#pragma warning(disable: 4244)
#define snprintf _snprintf
#endif

namespace {
  void HPDF_STDCALL error_handler(HPDF_STATUS   error_no,
		                  HPDF_STATUS   detail_no,
		                  void         *user_data) {
    Wt::WPdfImage *image = (Wt::WPdfImage *)user_data;

    image->errorHandler(error_no, detail_no);
  }

  const double EPSILON = 1e-4;
}

namespace Wt {

LOGGER("WPdfImage");

WPdfImage::WPdfImage(const WLength& width, const WLength& height)
  : width_(width),
    height_(height),
    painter_(nullptr)
{
  myPdf_ = true;

  pdf_ = HPDF_New(error_handler, this);
  if (!pdf_)
    throw WException("Could not create libharu document.");

  HPDF_SetCompressionMode(pdf_, HPDF_COMP_ALL);

  page_ = HPDF_AddPage(pdf_);

  font_ = nullptr;

  x_ = y_ = 0;

  HPDF_Page_SetWidth(page_, width_.toPixels());
  HPDF_Page_SetHeight(page_, height_.toPixels());
  HPDF_Page_GSave(page_);

  trueTypeFonts_ = new FontSupport(this, FontSupport::TrueTypeOnly);

#if HPDF_VERSION_ID>=20300
  HPDF_UseUTFEncodings(pdf_);
#endif
}

WPdfImage::WPdfImage(HPDF_Doc pdf, HPDF_Page page, HPDF_REAL x, HPDF_REAL y,
		     HPDF_REAL width, HPDF_REAL height)
  : width_(width, LengthUnit::Pixel),
    height_(height, LengthUnit::Pixel),
    painter_(nullptr),
    pdf_(pdf),
    page_(page),
    x_(x),
    y_(y)
{
  myPdf_ = false;

  font_ = nullptr;

  trueTypeFonts_ = new FontSupport(this, FontSupport::TrueTypeOnly);
}

WPdfImage::~WPdfImage()
{
  beingDeleted();

  if (myPdf_) {
    // clear graphics state stack to avoid leaking memory in libharu
    // see bug #3979
    HPDF_Page page = HPDF_GetCurrentPage(pdf_);
    if (page)
      while (HPDF_Page_GetGStateDepth(page) > 1)
        HPDF_Page_GRestore(page);
    HPDF_Free(pdf_);
  }

  delete trueTypeFonts_;
}

WFlags<PaintDeviceFeatureFlag> WPdfImage::features() const
{
  return PaintDeviceFeatureFlag::FontMetrics;
}

void WPdfImage::init()
{
  HPDF_Page_GSave(page_); // for Wt -> HPDF

  HPDF_Page_Concat(page_, 1, 0, 0, -1, 0, height_.toPixels());
  HPDF_Page_Concat(page_, 1, 0, 0, 1, x_, y_);

  HPDF_Page_GSave(page_); // for painter->combinedTransform()

}

void WPdfImage::done()
{
  HPDF_Page_GRestore(page_); // for painter->combinedTransform()
  HPDF_Page_GRestore(page_); // for Wt -> HPDF
}

void WPdfImage::setStrokeColor(WColor color)
{
  HPDF_Page_SetRGBStroke(page_,
                         color.red() / 255.,
                         color.green() / 255.,
                         color.blue() / 255.);

  const std::map<int, HPDF_ExtGState>::const_iterator it =
    alphaStrokeExtGStateMap_.find(color.alpha());
  HPDF_ExtGState gstate;
  if (it == alphaStrokeExtGStateMap_.end()) {
    gstate = HPDF_CreateExtGState(pdf_);
    HPDF_ExtGState_SetAlphaStroke(gstate, color.alpha()/255.);
    alphaStrokeExtGStateMap_[color.alpha()] = gstate;
  } else {
    gstate = it->second;
  }
  HPDF_Page_SetExtGState(page_, gstate);
}

void WPdfImage::setFillColor(WColor color)
{
  HPDF_Page_SetRGBFill(page_,
                       color.red() / 255.,
                       color.green() / 255.,
                       color.blue() / 255.);

  const std::map<int, HPDF_ExtGState>::const_iterator it =
    alphaFillExtGStateMap_.find(color.alpha());
  HPDF_ExtGState gstate;
  if (it == alphaFillExtGStateMap_.end()) {
    gstate = HPDF_CreateExtGState(pdf_);
    HPDF_ExtGState_SetAlphaFill(gstate, color.alpha()/255.);
    alphaFillExtGStateMap_[color.alpha()] = gstate;
  } else {
    gstate = it->second;
  }
  HPDF_Page_SetExtGState(page_, gstate);
}

void WPdfImage::addFontCollection(const std::string& directory, bool recursive)
{
  trueTypeFonts_->addFontCollection(directory, recursive);
}

void WPdfImage::applyTransform(const WTransform& t)
{
  HPDF_Page_Concat(page_, t.m11(), t.m12(), t.m21(),
		   t.m22(), t.dx(), t.dy());
}

void WPdfImage::setChanged(WFlags<PainterChangeFlag> flags)
{
  if (flags.test(PainterChangeFlag::Transform) || flags.test(PainterChangeFlag::Clipping)) {
    HPDF_Page_GRestore(page_);
    HPDF_Page_GSave(page_);

    currentFont_ = WFont();

    if (painter()->hasClipping()) {
      const WTransform& t = painter()->clipPathTransform();

      if (!painter()->clipPath().isEmpty()) {
        applyTransform(t);

        drawPlainPath(painter()->clipPath());
        HPDF_Page_Clip(page_);
        HPDF_Page_EndPath(page_);

        applyTransform(t.inverted());
      }
    }

    applyTransform(painter()->combinedTransform());

    flags = PainterChangeFlag::Pen | PainterChangeFlag::Brush | PainterChangeFlag::Font;
  }

  if (flags.test(PainterChangeFlag::Pen)) {
    const WPen& pen = painter()->pen();

    if (pen.style() != PenStyle::None) {
      setStrokeColor(pen.color());

      // Pen width is already scaled by the PDF transform (Page_Concat)
      // Cosmetic pens are also correctly drawn as 1px
      WLength w = pen.width();

      HPDF_Page_SetLineWidth(page_, w.toPixels());

      switch (pen.capStyle()) {
      case PenCapStyle::Flat:
	HPDF_Page_SetLineCap(page_, HPDF_BUTT_END);
	break;
      case PenCapStyle::Square:
	HPDF_Page_SetLineCap(page_, HPDF_PROJECTING_SCUARE_END); // scuary !
	break;
      case PenCapStyle::Round:
	HPDF_Page_SetLineCap(page_, HPDF_ROUND_END);
	break;
      }

      switch (pen.joinStyle()) {
      case PenJoinStyle::Miter:
	HPDF_Page_SetLineJoin(page_, HPDF_MITER_JOIN);
	break;
      case PenJoinStyle::Bevel:
	HPDF_Page_SetLineJoin(page_, HPDF_BEVEL_JOIN);
	break;
      case PenJoinStyle::Round:
	HPDF_Page_SetLineJoin(page_, HPDF_ROUND_JOIN);
	break;
      }

      switch (pen.style()) {
      case PenStyle::None:
	break;
      case PenStyle::SolidLine:
	HPDF_Page_SetDash(page_, nullptr, 0, 0);
	break;
      case PenStyle::DashLine: {
	const HPDF_UINT16 dash_ptn[] = { 4, 2 };
	HPDF_Page_SetDash(page_, dash_ptn, 2, 0);
	break;
      }
      case PenStyle::DotLine: {
	const HPDF_UINT16 dash_ptn[] = { 1, 2 };
	HPDF_Page_SetDash(page_, dash_ptn, 2, 0);
	break;
      }
      case PenStyle::DashDotLine: {
	const HPDF_UINT16 dash_ptn[] = { 4, 2, 1, 2 };
	HPDF_Page_SetDash(page_, dash_ptn, 4, 0);
	break;
      }
      case PenStyle::DashDotDotLine: {
	const HPDF_UINT16 dash_ptn[] = { 4, 2, 1, 2, 1, 2 };
	HPDF_Page_SetDash(page_, dash_ptn, 6, 0);
	break;
      }
      }
    }
  }

  if (flags.test(PainterChangeFlag::Brush)) {
    const WBrush& brush = painter()->brush();

    if (brush.style() != BrushStyle::None) {
      setFillColor(brush.color());
    }
  }

  if (flags.test(PainterChangeFlag::Font)) {
    const WFont& font = painter()->font();

    if (font == currentFont_ && !trueTypeFonts_->busy())
      return;

    /*
     * First, try a true type font.
     */
    std::string ttfFont;
    if (trueTypeFonts_->busy()) {
      /*
       * We have a resolved true type font.
       */
      ttfFont = trueTypeFonts_->drawingFontPath();
    } else {
      FontSupport::FontMatch match = trueTypeFonts_->matchFont(font);

      if (match.matched())
	ttfFont = match.fileName();
    }

    LOG_DEBUG("font: " << ttfFont);

    if (font == currentFont_ &&
        !ttfFont.empty() &&
        currentTtfFont_ == ttfFont)
      return;

    currentFont_ = font;

    const char *font_name = nullptr;
    font_ = nullptr;

    if (!ttfFont.empty()) {

      bool fontOk = false;

      std::map<std::string, const char *>::const_iterator i
	= ttfFonts_.find(ttfFont);

      if (i != ttfFonts_.end()) {
	font_name = i->second;
	fontOk = true;
      } else if (ttfFont.length() > 4) {
	std::string suffix
	  = Utils::lowerCase(ttfFont.substr(ttfFont.length() - 4));

        try {
          if (suffix == ".ttf") {
            font_name = HPDF_LoadTTFontFromFile(pdf_, ttfFont.c_str(),
              HPDF_TRUE);
          } else if (suffix == ".ttc") {
            /* Oops, pango didn't tell us which font to load ... */
            font_name = HPDF_LoadTTFontFromFile2(pdf_, ttfFont.c_str(),
              0, HPDF_TRUE);
          }
        } catch (const Wt::WException& e) {
          LOG_ERROR("error reading font: '" << ttfFont << "': " << e.what());
        }

	if (!font_name)
	  HPDF_ResetError (pdf_);
	else {
	  ttfFonts_[ttfFont] = font_name;
	  fontOk = true;
	}
      }

      if (!fontOk)
	LOG_ERROR("cannot read font: '" << ttfFont << "': "
		  "expecting a true type font (.ttf, .ttc)");
    }

    if (!font_ && font_name) {
      font_ = HPDF_GetFont (pdf_, font_name, "UTF-8");

      if (!font_)
	HPDF_ResetError (pdf_);
      else {
	trueTypeFont_ = true;
        currentTtfFont_ = ttfFont;
      }
    }

    if (!font_) {
      trueTypeFont_ = false;
      currentTtfFont_.clear();

      std::string name = Pdf::toBase14Font(font);
      font_ = HPDF_GetFont(pdf_, name.c_str(), nullptr);
    }

    fontSize_ = font.sizeLength(12).toPixels();

    HPDF_Page_SetFontAndSize (page_, font_, fontSize_);
  }
}

void WPdfImage::drawArc(const WRectF& rect, double startAngle, double spanAngle)
{
  HPDF_Page_GSave(page_);

  HPDF_Page_Concat(page_, 1, 0, 0, rect.height() / rect.width(),
		   rect.center().x(), rect.center().y());

  double start = startAngle;
  double end = start + spanAngle;

  if (end < start)
    std::swap(start, end);

  if (std::fabs(spanAngle) < (360 - EPSILON) )
    HPDF_Page_Arc(page_, 0, 0, rect.width() / 2, start + 90, end + 90);
  else
    HPDF_Page_Circle(page_, 0, 0, rect.width() / 2);

  paintPath();

  HPDF_Page_GRestore(page_);
}

void WPdfImage::paintPath()
{
  if (painter()->pen().style() != PenStyle::None)
    if (painter()->brush().style() != BrushStyle::None)
      HPDF_Page_FillStroke(page_);
    else
      HPDF_Page_Stroke(page_);
  else
    if (painter()->brush().style() != BrushStyle::None)
      HPDF_Page_Fill(page_);
    else
      HPDF_Page_EndPath(page_);
}

void WPdfImage::drawImage(const WRectF& rect, const std::string& imgUrl,
			  int imgWidth, int imgHeight,
			  const WRectF& srect)
{
  HPDF_Image img = nullptr;

  if (DataUri::isDataUri(imgUrl)) {
#define HAVE_LOAD_FROM_MEM HPDF_MAJOR_VERSION > 2 || (HPDF_MAJOR_VERSION == 2 && (HPDF_MINOR_VERSION >= 2))

#if HAVE_LOAD_FROM_MEM
    DataUri uri(imgUrl);
    if ("image/png" == uri.mimeType)
      img = HPDF_LoadPngImageFromMem(pdf_, 
				     (HPDF_BYTE*)&uri.data[0], 
				     uri.data.size()); 
    else if ("image/jpeg" == uri.mimeType)
      img = HPDF_LoadJpegImageFromMem(pdf_, 
				      (HPDF_BYTE*)&uri.data[0], 
				      uri.data.size());
#else
      LOG_ERROR("drawImage: data URI support requires libharu 2.2.0 or later");
#endif
  } else {
    std::string mimeType = ImageUtils::identifyMimeType(imgUrl);
    if ("image/png" == mimeType)
      img = HPDF_LoadPngImageFromFile2(pdf_, imgUrl.c_str());
    else if ("image/jpeg" == mimeType)
      img = HPDF_LoadJpegImageFromFile(pdf_, imgUrl.c_str());
  } 

  if (!img)
    throw WException("WPdfImage::drawImage(): cannot load image: " + imgUrl);

  double x = rect.x();
  double y = rect.y();
  double width = rect.width();
  double height = rect.height();

  HPDF_Page_GSave(page_);

  if (srect.x() != 0
      || srect.y() != 0
      || srect.width() != imgWidth
      || srect.height() != imgHeight) {
    double scaleX = width / imgWidth;
    double scaleY = height / imgHeight;

    x -= srect.x() * scaleX;
    y -= srect.y() * scaleY;
    width *= scaleX;
    height *= scaleY;

    HPDF_Page_Rectangle(page_, rect.x(), rect.y(), rect.width(), rect.height());
    HPDF_Page_Clip(page_);
  }

  HPDF_Page_Concat(page_, 1, 0, 0, -1, x, y + height); // revert upside-down

  HPDF_Page_DrawImage(page_, img, 0, 0, width, height);

  HPDF_Page_GRestore(page_);
}

void WPdfImage::drawLine(double x1, double y1, double x2, double y2)
{
  if (painter()->pen().style() != PenStyle::None) {
    HPDF_Page_MoveTo(page_, x1, y1);
    HPDF_Page_LineTo(page_, x2, y2);

    HPDF_Page_Stroke(page_);
  }
}

void WPdfImage::drawRect(const WRectF& rect)
{
  drawPath(rect.toPath());
}

void WPdfImage::drawPath(const WPainterPath& path)
{
  if (path.isEmpty())
    return;
  drawPlainPath(path);

  paintPath();
}

void WPdfImage::drawPlainPath(const WPainterPath& path)
{
  if (path.isEmpty())
    return;

  const std::vector<WPainterPath::Segment>& segments = path.segments();

  if (segments.size() > 0
      && segments[0].type() != SegmentType::MoveTo)
    HPDF_Page_MoveTo(page_, 0, 0);

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    switch (s.type()) {
    case SegmentType::MoveTo:
      HPDF_Page_MoveTo(page_, s.x(), s.y());
      break;
    case SegmentType::LineTo:
      HPDF_Page_LineTo(page_, s.x(), s.y());
      break;
    case SegmentType::CubicC1: {
      const double x1 = s.x();
      const double y1 = s.y();
      const double x2 = segments[i+1].x();
      const double y2 = segments[i+1].y();
      const double x3 = segments[i+2].x();
      const double y3 = segments[i+2].y();

      HPDF_Page_CurveTo(page_, x1, y1, x2, y2, x3, y3);

      i += 2;
      break;
    }
    case SegmentType::CubicC2:
    case SegmentType::CubicEnd:
      assert(false);
    case SegmentType::ArcC: {
      const double x = s.x();
      const double y = s.y();
      const double radius = segments[i+1].x();
      const double startAngle = segments[i+2].x();
      double spanAngle = segments[i+2].y();
      if (std::fabs(spanAngle) >= (360 - EPSILON))
        spanAngle = 360;
      const double betweenAngle = startAngle + spanAngle / 2;
      const double endAngle = startAngle + spanAngle;

      HPDF_Page_Arc(page_, x, y, radius, startAngle + 90, betweenAngle + 90);
      HPDF_Page_Arc(page_, x, y, radius, betweenAngle + 90, endAngle + 90);

      i += 2;
      break;
    }
    case SegmentType::ArcR:
    case SegmentType::ArcAngleSweep:
      assert(false);
    case SegmentType::QuadC: {
      const double x1 = s.x();
      const double y1 = s.y();
      const double x2 = segments[i+1].x();
      const double y2 = segments[i+1].y();

      // or CurveTo3 ?? Are these any different ??
      HPDF_Page_CurveTo2(page_, x1, y1, x2, y2);

      i += 1;

      break;
    }
    case SegmentType::QuadEnd:
      assert(false);
    }
  }
}

void WPdfImage::drawText(const WRectF& rect, 
			 WFlags<AlignmentFlag> flags,
			 TextFlag textFlag,
			 const WString& text,
			 const WPointF *clipPoint)
{
  // FIXME: textFlag
  
  if (clipPoint && painter() && !painter()->clipPath().isEmpty()) {
    if (!painter()->clipPathTransform().map(painter()->clipPath())
	  .isPointInPath(painter()->worldTransform().map(*clipPoint)))
      return;
  }

  if (trueTypeFont_ && !trueTypeFonts_->busy())
    trueTypeFonts_->drawText(painter()->font(), rect, flags, text);
  else {
    HPDF_REAL left, top, right, bottom;
    HPDF_TextAlignment alignment = HPDF_TALIGN_LEFT;

    AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
    AlignmentFlag verticalAlign = flags & AlignVerticalMask;

    switch (horizontalAlign) {
    default:
      // should never happen
    case AlignmentFlag::Left:
      left = rect.left();
      right = left + 10000;
      alignment = HPDF_TALIGN_LEFT;
      break;
    case AlignmentFlag::Right:
      right = rect.right();
      left = right - 10000;
      alignment = HPDF_TALIGN_RIGHT;
      break;
    case AlignmentFlag::Center:
      {
	float center = rect.center().x();
	left = center - 5000;
	right = center + 5000;
	alignment = HPDF_TALIGN_CENTER;
	break;
      }
    }

    switch (verticalAlign) {
    default:
      // fall-through ; should never happen
    case AlignmentFlag::Top:
      top = rect.top(); break;
    case AlignmentFlag::Middle:
      // FIXME: use font metrics to center middle of ascent !
      top = rect.center().y() - 0.60 * fontSize_; break;
    case AlignmentFlag::Bottom:
      top = rect.bottom() - fontSize_; break;
    }

    bottom = top + fontSize_;

    if (trueTypeFonts_->busy())
      setChanged(PainterChangeFlag::Font);

    HPDF_Page_GSave(page_);

    // Need to fill text using pen color
    const WColor& penColor = painter()->pen().color();
    setFillColor(penColor);

    // Undo the global inversion
    HPDF_Page_Concat(page_, 1, 0, 0, -1, 0, bottom);

    HPDF_Page_BeginText(page_);

    std::string s = trueTypeFont_ ? text.toUTF8() : text.narrow();

    HPDF_Page_TextRect(page_, left, fontSize_, right, 0, s.c_str(),
		       alignment, nullptr);

    HPDF_Page_EndText(page_);

    HPDF_Page_GRestore(page_);
  }
}

WFontMetrics WPdfImage::fontMetrics()
{
  HPDF_Box bbox = HPDF_Font_GetBBox(font_);

  int ascent = HPDF_Font_GetAscent(font_);
  int descent = -HPDF_Font_GetDescent(font_);

  if (ascent == 0 && descent == 0) {
    ascent = bbox.top;
    descent = -bbox.bottom;
  }

  int leading = bbox.top - ascent;

  return WFontMetrics(painter()->font(),
		      fontSize_ * leading / 1000.0,
		      fontSize_ * ascent / 1000.0,
		      fontSize_ * descent / 1000.0);
}

WTextItem WPdfImage::measureText(const WString& text, double maxWidth,
				 bool wordWrap)
{
  if (trueTypeFont_ && !trueTypeFonts_->busy())
    return trueTypeFonts_->measureText(painter()->font(), text, maxWidth,
				       wordWrap);
  else {
    HPDF_REAL width = 0;

    if (!wordWrap)
      maxWidth = 1E9;
    else
      maxWidth += EPSILON;

    if (trueTypeFonts_->busy())
      setChanged(PainterChangeFlag::Font);

    std::string s = trueTypeFont_ ? text.toUTF8() : text.narrow();

    int bytes = HPDF_Page_MeasureText(page_, s.c_str(), maxWidth, wordWrap,
				      &width);

    if (trueTypeFont_)
      return WTextItem(WString::fromUTF8(s.substr(0, bytes)), width);
    else
      return WTextItem(text.value().substr(0, bytes), width);
  }
}

void WPdfImage::handleRequest(const Http::Request& request,
			      Http::Response& response)
{
  HPDF_SaveToStream(pdf_);

  HPDF_ResetStream(pdf_);

  response.setMimeType("application/pdf");

  for (;;) {
    HPDF_BYTE buf[4096];
    HPDF_UINT32 siz = 4096;
    HPDF_ReadFromStream (pdf_, buf, &siz);
 
    if (siz == 0)
        break;

    response.out().write((const char *)buf, siz);
  }
}

void WPdfImage::errorHandler(HPDF_STATUS error_no,
			     HPDF_STATUS detail_no)
{
  char buf[200];
  snprintf(buf, 200, "WPdfImage error: error_no=%04X, detail_no=%d",
    (unsigned int) error_no, (int) detail_no);

  throw WException(buf);
}

}
