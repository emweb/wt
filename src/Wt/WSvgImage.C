/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WFontMetrics"
#include "Wt/WPainter"
#include "Wt/WPainterPath"
#include "Wt/WRectF"
#include "Wt/WSvgImage"
#include "Wt/WStringStream"
#include "Wt/WWebWidget"
#include "Wt/Http/Response"

#include "WebUtils.h"
#include "ServerSideFontMetrics.h"

#include <cmath>
#include <boost/lexical_cast.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SVG ""
//#define SVG "svg:"

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
}

namespace Wt {

int WSvgImage::nextClipId_ = 0;
int WSvgImage::nextGradientId_ = 0;

WSvgImage::WSvgImage(const WLength& width, const WLength& height,
		     WObject *parent, bool paintUpdate)
  : WResource(parent),
    width_(width),
    height_(height),
    painter_(0),
    paintUpdate_(paintUpdate),
    newGroup_(true),
    newClipPath_(false),
    busyWithPath_(false),
    currentClipId_(-1),
    currentFillGradientId_(-1),
    currentStrokeGradientId_(-1),
    currentShadowId_(-1),
    nextShadowId_(0),
    fontMetrics_(0)
{ }

WSvgImage::~WSvgImage()
{
  beingDeleted();
  delete fontMetrics_;
}

WFlags<WPaintDevice::FeatureFlag> WSvgImage::features() const
{
  if (ServerSideFontMetrics::available())
    return HasFontMetrics | CanWordWrap;
  else
    return CanWordWrap; // Actually, only when outputting to inkscape ...
}

void WSvgImage::init()
{ 
  currentBrush_ = painter()->brush();
  currentPen_ = painter()->pen();
  currentFont_ = painter()->font();

  strokeStyle_ = strokeStyle();
  fillStyle_ = fillStyle();
  fontStyle_ = fontStyle();

  //this is not for clipping, but for settings the initial pen stroke
  //in makeNewGroup()
  newClipPath_ = true;
}

void WSvgImage::done()
{
  finishPath();
}

void WSvgImage::drawArc(const WRectF& rect, double startAngle, double spanAngle)
{
  char buf[30];

  if (std::fabs(spanAngle - 360.0) < 0.01) {
    finishPath();
    makeNewGroup();

    shapes_ << "<" SVG "ellipse "
	    << " cx=\""<< Utils::round_js_str(rect.center().x(), 3, buf);
    shapes_ << "\" cy=\"" << Utils::round_js_str(rect.center().y(), 3, buf);
    shapes_ << "\" rx=\"" << Utils::round_js_str(rect.width() / 2, 3, buf);
    shapes_ << "\" ry=\"" << Utils::round_js_str(rect.height() / 2, 3, buf)
	    << "\" />";
  } else {
    WPainterPath path;

    path.arcMoveTo(rect.x(), rect.y(), rect.width(), rect.height(), startAngle);
    path.arcTo(rect.x(), rect.y(), rect.width(), rect.height(), startAngle,
	       spanAngle);

    drawPath(path);
  }
}

void WSvgImage::setChanged(WFlags<ChangeFlag> flags)
{
  if (flags)
    newGroup_ = true;

  if (flags & Clipping)
    newClipPath_ = true;

  changeFlags_ |= flags;
}

void WSvgImage::makeNewGroup()
{
  if (!newGroup_)
    return;

  bool brushChanged
    = (changeFlags_ & Brush) && (currentBrush_ != painter()->brush());
  bool penChanged
    = changeFlags_ & Hints
    || ((changeFlags_ & Pen) && (currentPen_ != painter()->pen()));
  bool fontChanged
    = (changeFlags_ & Font) && (currentFont_ != painter()->font());
  bool shadowChanged = false;
  if (changeFlags_ & Shadow) {
    if (currentShadowId_ == -1)
      shadowChanged = !painter()->shadow().none();
    else
      shadowChanged = currentShadow_ != painter()->shadow();
  }

  if (shadowChanged)
    newClipPath_ = true;

  if (!newClipPath_) {
    if (!brushChanged && !penChanged) {
      WTransform f = painter()->combinedTransform();

      if (busyWithPath_) {
	if (   fequal(f.m11(), currentTransform_.m11())
	    && fequal(f.m12(), currentTransform_.m12())
	    && fequal(f.m21(), currentTransform_.m21())
	    && fequal(f.m22(), currentTransform_.m22())) {

	  /*
	   * Invert scale/rotate to compute the delta needed
	   * before applying these transformations to get the
	   * same as the global translation.
	   */
	  double det = f.m11() * f.m22() - f.m12() * f.m21();
	  double a11 = f.m22() / det;
	  double a12 = -f.m12() / det;
	  double a21 = -f.m21() / det;
	  double a22 = f.m11() / det;

	  double fdx = f.dx() * a11 + f.dy() * a21;
	  double fdy = f.dx() * a12 + f.dy() * a22;

	  const WTransform& g = currentTransform_;

	  double gdx = g.dx() * a11 + g.dy() * a21;
	  double gdy = g.dx() * a12 + g.dy() * a22;

	  double dx = fdx - gdx;
	  double dy = fdy - gdy;

	  pathTranslation_.setX(dx);
	  pathTranslation_.setY(dy);

	  changeFlags_ = 0;

	  return;
	}
      } else {
	if (!fontChanged && currentTransform_ == f) {
	  newGroup_ = false;

	  changeFlags_ = 0;

	  return;
	}
      }
    }
  }

  newGroup_ = false;

  finishPath();

  char buf[30];

  shapes_ << "</" SVG "g>";

  currentTransform_ = painter()->combinedTransform();

  if (newClipPath_) {
    shapes_ << "</" SVG "g>";
    if (painter()->hasClipping()) {
      currentClipId_ = nextClipId_++;
      shapes_ << "<" SVG "defs><" SVG "clipPath id=\"clip"
	      << currentClipId_ << "\">";

      drawPlainPath(shapes_, painter()->clipPath());

      shapes_ << '"';
      busyWithPath_ = false;

      const WTransform& t = painter()->clipPathTransform();
      if (!t.isIdentity()) {
	shapes_ << " transform=\"matrix("
		<<        Utils::round_js_str(t.m11(), 3, buf);
	shapes_ << ' ' << Utils::round_js_str(t.m12(), 3, buf);
	shapes_ << ' ' << Utils::round_js_str(t.m21(), 3, buf);
	shapes_ << ' ' << Utils::round_js_str(t.m22(), 3, buf);
	shapes_ << ' ' << Utils::round_js_str(t.m31(), 3, buf);
	shapes_ << ' ' << Utils::round_js_str(t.m32(), 3, buf)
		<< ")\"";
      }
      shapes_ << "/></" SVG "clipPath></" SVG "defs>";
    }

    newClipPath_ = false;

    if (shadowChanged) {
      if (!painter()->shadow().none()) {
	if (painter()->shadow() != currentShadow_) {
	  currentShadow_ = painter()->shadow();
	  currentShadowId_ = createShadowFilter(shapes_);
	} else
	  currentShadowId_ = nextShadowId_;
      } else
	currentShadowId_ = -1;
    }

    shapes_ << "<" SVG "g";
    if (painter()->hasClipping())
      shapes_ << clipPath();

    if (currentShadowId_ != -1)
      shapes_ << " filter=\"url(#f" << currentShadowId_ << ")\"";

    shapes_ << '>';
  }

  if (penChanged) {
    currentPen_ = painter()->pen();

    if (!currentPen_.gradient().isEmpty()) {
      currentStrokeGradientId_ = nextGradientId_++;
      defineGradient(currentPen_.gradient(), currentStrokeGradientId_);
    }

    strokeStyle_ = strokeStyle();
  }

  if (brushChanged) {
    currentBrush_ = painter()->brush();

    if (!currentBrush_.gradient().isEmpty()) {
      currentFillGradientId_ = nextGradientId_++;
      defineGradient(currentBrush_.gradient(), currentFillGradientId_);
    }

    fillStyle_ = fillStyle();
  }

  if (fontChanged) {
    currentFont_ = painter()->font();
    fontStyle_ = fontStyle();
  }

  shapes_ << "<" SVG "g style=\""
	  << fillStyle_ << strokeStyle_ << fontStyle_ << '"';

  if (!currentTransform_.isIdentity()) {
    shapes_ << " transform=\"matrix("
	    << Utils::round_js_str(currentTransform_.m11(), 3, buf);
    shapes_ << ' ' << Utils::round_js_str(currentTransform_.m12(), 3, buf);
    shapes_ << ' ' << Utils::round_js_str(currentTransform_.m21(), 3, buf);
    shapes_ << ' ' << Utils::round_js_str(currentTransform_.m22(), 3, buf);
    shapes_ << ' ' << Utils::round_js_str(currentTransform_.m31(), 3, buf);
    shapes_ << ' ' << Utils::round_js_str(currentTransform_.m32(), 3, buf)
	    << ")\"";
  }

  shapes_ << '>';
  
  changeFlags_ = 0;
}

int WSvgImage::createShadowFilter(WStringStream& out)
{
  char buf[30];
  int result = ++nextShadowId_;

  out << "<filter id=\"f" << result
      << "\" width=\"150%\" height=\"150%\">"
      << "<feOffset result=\"offOut\" in=\"SourceAlpha\" dx=\""
      << Utils::round_js_str(currentShadow_.offsetX(), 3, buf) << "\" dy=\"";
  out << Utils::round_js_str(currentShadow_.offsetY(), 3, buf) << "\" />";

  out << "<feColorMatrix result=\"colorOut\" in=\"offOut\" "
      << "type=\"matrix\" values=\"";
  double r = currentShadow_.color().red() / 255.;
  double g = currentShadow_.color().green() / 255.;
  double b = currentShadow_.color().blue() / 255.;
  double a = currentShadow_.color().alpha() / 255.;

  out << "0 0 0 " << Utils::round_js_str(r, 3, buf) << " 0 ";
  out << "0 0 0 " << Utils::round_js_str(g, 3, buf) << " 0 ";
  out << "0 0 0 " << Utils::round_js_str(b, 3, buf) << " 0 ";
  out << "0 0 0 " << Utils::round_js_str(a, 3, buf) << " 0\"/>";
  out << "<feGaussianBlur result=\"blurOut\" in=\"colorOut\" stdDeviation=\""
      << Utils::round_js_str(std::sqrt(currentShadow_.blur()), 3, buf) << "\" />"
    "<feBlend in=\"SourceGraphic\" in2=\"blurOut\" mode=\"normal\" />"
    "</filter>";

  return result;
}

void WSvgImage::defineGradient(const WGradient& gradient, int id)
{
  char buf[30];

  shapes_ << "<defs>";
  bool linear = gradient.style() == LinearGradient;

  // linear or radial + add proper position parameters
  if (linear) {
    shapes_ << "<linearGradient gradientUnits=\"userSpaceOnUse\" ";
    shapes_ << "x1=\"" << gradient.linearGradientVector().x1() << "\" "
	    << "y1=\"" << gradient.linearGradientVector().y1() << "\" "
	    << "x2=\"" << gradient.linearGradientVector().x2() << "\" "
	    << "y2=\"" << gradient.linearGradientVector().y2() << "\" ";
  } else {
    shapes_ << "<radialGradient gradientUnits=\"userSpaceOnUse\" ";
    shapes_ << "cx=\"" << gradient.radialCenterPoint().x() << "\" "
	    << "cy=\"" << gradient.radialCenterPoint().y() << "\" "
	    << "r=\"" << gradient.radialRadius() << "\" "
	    << "fx=\"" << gradient.radialFocalPoint().x() << "\" "
	    << "fy=\"" << gradient.radialFocalPoint().y() << "\" ";
  }

  shapes_ << "id=\"gradient" << id << "\">";

  // add colorstops
  for (unsigned i = 0; i < gradient.colorstops().size(); i++) {
    shapes_ << "<stop ";
    std::string offset =
      boost::lexical_cast<std::string>((int)(gradient.colorstops()[i]
					     .position()*100));
    offset += '%';
    shapes_ << "offset=\"" << offset << "\" ";
    shapes_ << "stop-color=\"" <<
      gradient.colorstops()[i].color().cssText()
	    << "\" ";
    shapes_ << "stop-opacity=\"" <<
      Utils::round_css_str(gradient.colorstops()[i].color().alpha() / 255.,
			   3, buf)
	    << "\" ";
    shapes_ << "/>";
  }

  if (linear)
    shapes_ << "</linearGradient>";
  else
    shapes_ << "</radialGradient>";
  shapes_ << "</defs>";
}

void WSvgImage::drawPlainPath(WStringStream& out, const WPainterPath& path)
{
  char buf[30];

  if (!busyWithPath_) {
    out << "<" SVG "path d=\"";
    busyWithPath_ = true;
    pathTranslation_.setX(0);
    pathTranslation_.setY(0);
  }

  const std::vector<WPainterPath::Segment>& segments = path.segments();

  if (!segments.empty()
      && segments[0].type() != WPainterPath::Segment::MoveTo)
    out << "M0,0";

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    if (s.type() == WPainterPath::Segment::ArcC) {
      WPointF current = path.positionAtSegment(i);

      const double cx = segments[i].x();
      const double cy = segments[i].y();
      const double rx = segments[i+1].x();
      const double ry = segments[i+1].y();
      const double theta1 = -WTransform::degreesToRadians(segments[i+2].x());
      const double deltaTheta
	= -WTransform::degreesToRadians(adjust360(segments[i+2].y()));

      i += 2;

      /*
       * formulas from:
       * http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
       * with phi = 0
       */
      const double x1 = rx * std::cos(theta1) + cx;
      const double y1 = ry * std::sin(theta1) + cy;
      const double x2 = rx * std::cos(theta1 + deltaTheta) + cx;
      const double y2 = ry * std::sin(theta1 + deltaTheta) + cy;
      const int fa = (std::fabs(deltaTheta) > M_PI ? 1 : 0);
      const int fs = (deltaTheta > 0 ? 1 : 0);

      if (!fequal(current.x(), x1) || !fequal(current.y(), y1)) {
	out << 'L' << Utils::round_js_str(x1 + pathTranslation_.x(), 3, buf);
	out << ',' << Utils::round_js_str(y1 + pathTranslation_.y(), 3, buf);
      }

      out << 'A' << Utils::round_js_str(rx, 3, buf);
      out << ',' << Utils::round_js_str(ry, 3, buf);
      out << " 0 " << fa << "," << fs;
      out << ' ' << Utils::round_js_str(x2 + pathTranslation_.x(), 3, buf);
      out << ',' << Utils::round_js_str(y2 + pathTranslation_.y(), 3, buf);
    } else {
      switch (s.type()) {
      case WPainterPath::Segment::MoveTo:
	out << 'M';
	break;
      case WPainterPath::Segment::LineTo:
	out << 'L';
	break;
      case WPainterPath::Segment::CubicC1:
	out << 'C';
	break;
      case WPainterPath::Segment::CubicC2:
      case WPainterPath::Segment::CubicEnd:
	out << ' ';
	break;
      case WPainterPath::Segment::QuadC:
	out << 'Q';
	break;
      case WPainterPath::Segment::QuadEnd:
	out << ' ';
	break;
      default:
	assert(false);
      }

      out << Utils::round_js_str(s.x() + pathTranslation_.x(), 3, buf);
      out << ',' << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf);
    }
  }
}

void WSvgImage::finishPath()
{
  if (busyWithPath_) {
    busyWithPath_ = false;

    shapes_ << "\" />";
  }
}

void WSvgImage::drawPath(const WPainterPath& path)
{
  WRectF bbox = painter()->worldTransform().map(path.controlPointRect());
  if (busyWithPath_) {
    if (pathBoundingBox_.intersects(bbox))
      finishPath();
    else
      pathBoundingBox_ = pathBoundingBox_.united(bbox);
  } else {
    pathBoundingBox_ = bbox;
  }

  makeNewGroup();

  drawPlainPath(shapes_, path);
}

void WSvgImage::drawImage(const WRectF& rect, const std::string& imgUri,
			  int imgWidth, int imgHeight,
			  const WRectF& srect)
{
  finishPath();
  makeNewGroup();

  WApplication *app = WApplication::instance();
  std::string imageUri = imgUri;
  if (app)
    imageUri = app->resolveRelativeUrl(imgUri);

  WRectF drect = rect;

  char buf[30];

  bool transformed = false;

  if (drect.width() != srect.width()
      || drect.height() != srect.height()) {
    shapes_ << "<" SVG "g transform=\"matrix("
	    << Utils::round_js_str(drect.width() / srect.width(), 3, buf);
    shapes_ << " 0 0 " 
	    << Utils::round_js_str(drect.height() / srect.height(), 3, buf);
    shapes_ << ' ' << Utils::round_js_str(drect.x(), 3, buf);
    shapes_ << ' ' << Utils::round_js_str(drect.y(), 3, buf) << ")\">";

    drect = WRectF(0, 0, srect.width(), srect.height());

    transformed = true;
  }

  double scaleX = drect.width() / srect.width();
  double scaleY = drect.height() / srect.height();

  double x = drect.x() - srect.x() * scaleX;
  double y = drect.y() - srect.y() * scaleY;
  //double width = drect.width() * imgWidth / srect.width();
  //double height = drect.height() * imgHeight / srect.height();
  double width = imgWidth;
  double height = imgHeight;

  bool useClipPath = false;

  int imgClipId = nextClipId_++;

  if (WRectF(x, y, width, height) != drect) {
    shapes_ << "<" SVG "clipPath id=\"imgClip" << imgClipId << "\">";
    shapes_ << "<" SVG "rect x=\"" << Utils::round_js_str(drect.x(), 3, buf) << '"';
    shapes_ << " y=\"" << Utils::round_js_str(drect.y(), 3, buf) << '"';
    shapes_ << " width=\"" << Utils::round_js_str(drect.width(), 3, buf) << '"';
    shapes_ << " height=\"" << Utils::round_js_str(drect.height(), 3, buf) << '"';
    shapes_ << " /></" SVG "clipPath>";
    useClipPath = true;
  }

  shapes_ << "<" SVG "image xlink:href=\"" << imageUri << "\"";
  shapes_ << " x=\"" << Utils::round_js_str(x, 3, buf) << '"';
  shapes_ << " y=\"" << Utils::round_js_str(y, 3, buf) << '"';
  shapes_ << " width=\"" << Utils::round_js_str(width, 3, buf) << '"';
  shapes_ << " height=\"" << Utils::round_js_str(height, 3, buf) << '"';

  if (useClipPath)
    shapes_ << " clip-path=\"url(#imgClip" << imgClipId << ")\"";

  shapes_ << "/>";

  if (transformed)
    shapes_ << "</" SVG "g>";
}

void WSvgImage::drawLine(double x1, double y1, double x2, double y2)
{
  WPainterPath path;
  path.moveTo(x1, y1);
  path.lineTo(x2, y2);
  drawPath(path);
}

void WSvgImage::drawText(const WRectF& rect, 
			 WFlags<AlignmentFlag> flags,
			 TextFlag textFlag,
			 const WString& text,
			 const WPointF *clipPoint)
{
  if (clipPoint && painter() && !painter()->clipPath().isEmpty()) {
    if (!painter()->clipPathTransform().map(painter()->clipPath())
	  .isPointInPath(painter()->worldTransform().map(*clipPoint)))
      return;
  }

  finishPath();
  makeNewGroup();

  char buf[30];
  WStringStream style;

  // SVG uses fill color to fill text, but we want pen color.
  style << "style=\"stroke:none;";
  if (painter()->pen().color() != painter()->brush().color()
      || painter()->brush().style() == NoBrush) {
    const WColor& color = painter()->pen().color();
    style << "fill:" + color.cssText() << ';'
	  << "fill-opacity:" 
	  << Utils::round_css_str(color.alpha() / 255., 3, buf)
	  << ';';
  }
  style << '"';

  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  if (textFlag == TextWordWrap) {
    std::string hAlign;
    switch (horizontalAlign) {
    case AlignLeft:
      hAlign = "start";
      break;
    case AlignRight:
      hAlign = "end";
      break;
    case AlignCenter:
      hAlign = "center";
      break;
    case AlignJustify:
      hAlign = "justify";
    default:
      break;
    }

    shapes_ << "<" SVG "flowRoot " << style.str() << ">\n"
	    << "  <" SVG "flowRegion>\n"
	    << "    <" SVG "rect"
	    <<            " width=\"" << rect.width() << "\""
	    <<            " height=\"" << rect.height() << "\""
	    <<            " x=\"" << rect.x() << "\""
	    <<            " y=\"" << rect.y() << "\""
	    << "    />\n"
	    << "  </" SVG "flowRegion>\n"
	    << "  <" SVG "flowPara"
	    <<              " text-align=\"" << hAlign << "\">\n"
	    << " " << WWebWidget::escapeText(text, false).toUTF8() << "\n"
	    << "  </" SVG "flowPara>\n"
	    << "</" SVG "flowRoot>\n";
  } else {
    shapes_ << "<" SVG "text " << style.str();

    switch (horizontalAlign) {
    case AlignLeft:
      shapes_ << " x=" << quote(rect.left());
      break;
    case AlignRight:
      shapes_ << " x=" << quote(rect.right())
	      << " text-anchor=\"end\"";
      break;
    case AlignCenter:
      shapes_ << " x=" << quote(rect.center().x())
	      << " text-anchor=\"middle\"";
      break;
    default:
      break;
    }

    /*
     * Opera doesn't do dominant-baseline yet
     */
#if 0
    switch (verticalAlign) {
    case AlignTop:
      shapes_ << " y=" << quote(rect.top())
	      << " dominant-baseline=\"text-before-edge\"";
      break;
    case AlignBottom:
      shapes_ << " y=" << quote(rect.bottom())
	      << " dominant-baseline=\"text-after-edge\"";
      break;
    case AlignMiddle:
      shapes_ << " y=" << quote(rect.center().y())
	      << " dominant-baseline=\"middle\"";
      break;
    default:
      break;
    }

    shapes << ">" << WWebWidget::escapeText(text, false).toUTF8() 
	   << "</" SVG "text>";

#else

    /*
     * Workaround: estimate the location of the default baseline which
     * corresponds with the font baseline.
     */
    double fontSize = painter()->font().sizeLength(16).toPixels();

    double y = rect.center().y();
    switch (verticalAlign) {
    case AlignTop:
      y = rect.top() + fontSize * 0.75; break;
    case AlignMiddle:
      y = rect.center().y() + fontSize * 0.25; break;
    case AlignBottom:
      y = rect.bottom() - fontSize * 0.25 ; break;
    default:
      break;
    }

  shapes_ << " y=" << quote(y);

  shapes_ << ">" << WWebWidget::escapeText(text, false).toUTF8() 
	  << "</" SVG "text>";
#endif
  }
}

WTextItem WSvgImage::measureText(const WString& text, double maxWidth,
				 bool wordWrap)
{
  if (!fontMetrics_)
    fontMetrics_ = new ServerSideFontMetrics();

  return fontMetrics_->measureText(painter()->font(), text, maxWidth, wordWrap);
}

WFontMetrics WSvgImage::fontMetrics()
{
  if (!fontMetrics_)
    fontMetrics_ = new ServerSideFontMetrics();

  return fontMetrics_->fontMetrics(painter()->font());
}

std::string WSvgImage::quote(const std::string& s)
{
  return '"' + s + '"';
}

std::string WSvgImage::quote(double d)
{
  char buf[30];
  return quote(Utils::round_js_str(d, 3, buf));
}

std::string WSvgImage::fillStyle() const
{
  char buf[30];
  std::string result;

  switch (painter()->brush().style()) {
  case NoBrush:
    result += "fill:none;";
    break;
  case SolidPattern:
    {
      const WColor& color = painter()->brush().color();
      result += "fill:" + color.cssText() + ";";
      if (color.alpha() != 255) {
	result += "fill-opacity:";
	result += Utils::round_css_str(color.alpha() / 255., 3, buf);
	result += ';';
      }

      break;
    }
  case GradientPattern:
    if (!currentBrush_.gradient().isEmpty()) {
      result += "fill:"; 
      result += "url(#gradient";
      result += boost::lexical_cast<std::string>(currentFillGradientId_);
      result += ");";
    }
  }

  return result;
}

std::string WSvgImage::clipPath() const
{
  if (painter()->hasClipping())
    return " clip-path=\"url(#clip"
      + boost::lexical_cast<std::string>(currentClipId_) + ")\"";
  else
    return std::string();
}

std::string WSvgImage::strokeStyle() const
{
  WStringStream result;

#ifndef WT_TARGET_JAVA
  char buf[30];
#else
  char *buf;
#endif

  const WPen& pen = painter()->pen();

  if (!(painter()->renderHints() & WPainter::Antialiasing))
    result << "shape-rendering:optimizeSpeed;";

  if (pen.style() != NoPen) {
    const WColor& color = pen.color();

    if (!pen.gradient().isEmpty()) {
      result << "stroke:url(#gradient"
	     << boost::lexical_cast<std::string>(currentStrokeGradientId_)
	     << ");";
    } else {
      result << "stroke:" << color.cssText() << ';';
      if (color.alpha() != 255)
	result << "stroke-opacity:"
	       << Utils::round_css_str(color.alpha() / 255., 2, buf) << ';';
    }

    WLength w = painter()->normalizedPenWidth(pen.width(), true);
    if (w != WLength(1))
      result << "stroke-width:" << w.cssText() << ";";

    switch (pen.capStyle()) {
    case FlatCap:
      break;
    case SquareCap:
      result << "stroke-linecap:square;";
      break;
    case RoundCap:
      result << "stroke-linecap:round;";
    }

    switch (pen.joinStyle()) {
    case MiterJoin:
      break;
    case BevelJoin:
      result << "stroke-linejoin:bevel;";
      break;
    case RoundJoin:
      result << "stroke-linejoin:round;";
    }

    switch (pen.style()) {
    case NoPen:
      break;
    case SolidLine:
      break;
    case DashLine:
      result << "stroke-dasharray:4,2;";
      break;
    case DotLine:
      result << "stroke-dasharray:1,2;";
      break;
    case DashDotLine:
      result << "stroke-dasharray:4,2,1,2;";
      break;
    case DashDotDotLine:
      result << "stroke-dasharray:4,2,1,2,1,2;";
      break;
    }
  }

  return result.c_str();
}

std::string WSvgImage::fontStyle() const
{
  return painter()->font().cssText(false);
}

std::string WSvgImage::rendered()
{
  std::stringstream s;
  streamResourceData(s);
  return s.str();
}

void WSvgImage::handleRequest(const Http::Request& request,
			      Http::Response& response)
{
  response.setMimeType("image/svg+xml");

#ifndef WT_TARGET_JAVA
  std::ostream& o = response.out();
#else
  std::ostream o(response.out());
#endif // WT_TARGET_JAVA

  streamResourceData(o);
}

void WSvgImage::streamResourceData(std::ostream& stream)
{
  finishPath();

  if (paintUpdate_)
    stream << "<" SVG "g xmlns=\"http://www.w3.org/2000/svg\""
      " xmlns:xlink=\"http://www.w3.org/1999/xlink\"><" SVG "g><" SVG "g>"
	   << shapes_.str()
	   << "</" SVG "g></" SVG "g></" SVG "g>";
  else
    stream << "<" SVG "svg xmlns=\"http://www.w3.org/2000/svg\""
      " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
      " version=\"1.1\" baseProfile=\"full\""
      " width=\"" << width().cssText() << "\""
      " height=\"" << height().cssText() << "\">"
	   << "<" SVG "g><" SVG "g>" << shapes_.str()
	   << "</" SVG "g></" SVG "g></" SVG "svg>";
}

}
