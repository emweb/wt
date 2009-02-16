/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSvgImage"
#include "Wt/WPainter"
#include "Wt/WPainterPath"
#include "Wt/WRectF"
#include "Wt/WWebWidget"

#include <math.h>
#include <sstream>
#include <boost/lexical_cast.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SVG ""
//#define SVG "svg:"

namespace {
  double adjust360(double d) {
    if (fabs(d - 360) < 0.01)
      return 359.5;
    else if (fabs(d + 360) < 0.01)
      return -359.5;
    else 
      return d;
  }

  double myround(double d) {
    return static_cast<int>(d * 1000) / 1000.;
  }

  bool fequal(double d1, double d2) {
    return fabs(d1 - d2) < 1E-5;
  }
}

namespace Wt {

int WSvgImage::nextClipId_ = 0;

WSvgImage::WSvgImage(const WLength& width, const WLength& height,
		     WObject *parent)
  : WVectorImage(width, height),
    WResource(parent),
    newGroup_(true),
    newClipPath_(false),
    busyWithPath_(false),
    currentClipId_(-1)
{ }

void WSvgImage::init()
{ }

void WSvgImage::done()
{
  finishPath();
}

void WSvgImage::drawArc(const WRectF& rect, double startAngle, double spanAngle)
{
  if (fabs(spanAngle - 360.0) < 0.01) {
    finishPath();
    makeNewGroup();

    std::stringstream tmp;

    tmp << "<"SVG"ellipse "
	<< " cx=\""<< myround(rect.center().x())
	<< "\" cy=\"" << myround(rect.center().y())
	<< "\" rx=\"" << myround(rect.width() / 2)
	<< "\" ry=\"" << myround(rect.height() / 2)
	<< "\" />";

    shapes_ += tmp.str();
  } else {
    WPainterPath path;

    path.arcMoveTo(rect.x(), rect.y(), rect.width(), rect.height(), startAngle);
    path.arcTo(rect.x(), rect.y(), rect.width(), rect.height(), startAngle,
	       spanAngle);

    drawPath(path);
  }
}

void WSvgImage::setChanged(int flags)
{
  if (flags != 0)
    newGroup_ = true;

  if (flags & Clipping)
    newClipPath_ = true;
}

void WSvgImage::makeNewGroup()
{
  if (!newGroup_)
    return;

  if (!newClipPath_) {
    if (currentBrush_ == painter()->brush()
	&& currentPen_ == painter()->pen()) {

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
	  return;
	}
      } else {
	if (currentFont_ == painter()->font()
	    && currentTransform_ == f) {
	  newGroup_ = false;

	  return;
	}
      }
    }
  }

  newGroup_ = false;

  finishPath();

  std::stringstream tmp;

  tmp << "</"SVG"g>";

  currentTransform_ = painter()->combinedTransform();

  if (newClipPath_) {
    tmp << "</"SVG"g>";
    if (painter()->hasClipping()) {
      currentClipId_ = nextClipId_++;
      tmp << "<"SVG"defs><"SVG"clipPath id=\"clip" << currentClipId_ << "\">";
      drawPlainPath(tmp, painter()->clipPath());
      tmp << '"';
      busyWithPath_ = false;

      const WTransform& t = painter()->clipPathTransform();
      if (!t.isIdentity()) {
	tmp << " transform=\"matrix("
	    <<        myround(t.m11())
	    << ' ' << myround(t.m12())
	    << ' ' << myround(t.m21())
	    << ' ' << myround(t.m22())
	    << ' ' << myround(t.m31())
	    << ' ' << myround(t.m32())
	    << ")\"";
      }
      tmp << "/></"SVG"clipPath></"SVG"defs>";
    }

    newClipPath_ = false;

    tmp << "<"SVG"g";
    if (painter()->hasClipping())
      tmp << clipPath();
    tmp << '>';
  }

  currentPen_ = painter()->pen();
  currentBrush_ = painter()->brush();
  currentFont_ = painter()->font();

  tmp << "<"SVG"g style="
      << quote(fillStyle() + strokeStyle() + fontStyle());

  currentTransform_ = painter()->combinedTransform();

  if (!currentTransform_.isIdentity()) {
    tmp << " transform=\"matrix("
	<< myround(currentTransform_.m11())
	<< ' ' << myround(currentTransform_.m12())
	<< ' ' << myround(currentTransform_.m21())
	<< ' ' << myround(currentTransform_.m22())
	<< ' ' << myround(currentTransform_.m31())
	<< ' ' << myround(currentTransform_.m32())
	<< ")\"";
  }

  tmp << '>';

  shapes_ += tmp.str();
}

void WSvgImage::drawPlainPath(std::ostream& out, const WPainterPath& path)
{
  if (!busyWithPath_) {
    out << "<"SVG"path d=\"";
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
      const double x1 = rx * cos(theta1) + cx;
      const double y1 = ry * sin(theta1) + cy;
      const double x2 = rx * cos(theta1 + deltaTheta) + cx;
      const double y2 = ry * sin(theta1 + deltaTheta) + cy;
      const int fa = (fabs(deltaTheta) > M_PI ? 1 : 0);
      const int fs = (deltaTheta > 0 ? 1 : 0);

      if (!fequal(current.x(), x1) || !fequal(current.y(), y1))
	out << "L" << myround(x1 + pathTranslation_.x())
	    << "," << myround(y1 + pathTranslation_.y());

      out << "A" << myround(rx) << "," << myround(ry)
	  << " 0 " << fa << "," << fs
	  << " " << myround(x2 + pathTranslation_.x())
	  << "," << myround(y2 + pathTranslation_.y());
    } else {
      switch (s.type()) {
      case WPainterPath::Segment::MoveTo:
	out << "M";
	break;
      case WPainterPath::Segment::LineTo:
	out << "L";
	break;
      case WPainterPath::Segment::CubicC1:
	out << "C";
	break;
      case WPainterPath::Segment::CubicC2:
      case WPainterPath::Segment::CubicEnd:
	out << " ";
	break;
      case WPainterPath::Segment::QuadC:
	out << "Q";
	break;
      case WPainterPath::Segment::QuadEnd:
	out << " ";
	break;
      default:
	assert(false);
      }

      out << myround(s.x() + pathTranslation_.x()) << ","
	  << myround(s.y() + pathTranslation_.y());
    }
  }
}

void WSvgImage::finishPath()
{
  if (busyWithPath_) {
    busyWithPath_ = false;

    shapes_ += "\" />";
  }
}

void WSvgImage::drawPath(const WPainterPath& path)
{
  makeNewGroup();

  std::stringstream tmp;
  drawPlainPath(tmp, path);
  shapes_ += tmp.str();
}

void WSvgImage::drawImage(const WRectF& rect, const std::string& imageUri,
			  int imgWidth, int imgHeight,
			  const WRectF& srect)
{
  finishPath();
  makeNewGroup();

  WRectF drect = rect;

  std::stringstream tmp;

  bool transformed = false;

  if (drect.width() != srect.width()
      || drect.height() != srect.height()) {
    tmp << "<"SVG"g transform=\"matrix("
	<< myround(drect.width() / srect.width())
	<< " 0 0 " << myround(drect.height() / srect.height())
	<< ' ' << myround(drect.x()) << " " << myround(drect.y()) << ")\">";

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
    tmp <<
      "<"SVG"clipPath id=\"imgClip" << imgClipId << "\">"
      "<"SVG"rect x=\"" << myround(drect.x()) << "\""
           " y=\"" << myround(drect.y()) << "\""
       " width=\"" << myround(drect.width()) << "\""
      " height=\"" << myround(drect.height()) << "\""
      " /></"SVG"clipPath>";
    useClipPath = true;
  }

  tmp <<
    "<"SVG"image xlink:href=\"" << imageUri << "\""
          " x=\"" << myround(x) << "\""
          " y=\"" << myround(y) << "\""
      " width=\"" << myround(width) << "\""
      " height=\"" << myround(height) << "\"";

  if (useClipPath)
    tmp << " clip-path=\"url(#imgClip" << imgClipId << ")\"";

  tmp << "/>";

  if (transformed)
    tmp << "</"SVG"g>";

  shapes_ += tmp.str();
}

void WSvgImage::drawLine(double x1, double y1, double x2, double y2)
{
  WPainterPath path;
  path.moveTo(x1, y1);
  path.lineTo(x2, y2);
  drawPath(path);
}

void WSvgImage::drawText(const WRectF& rect, int flags, const WString& text)
{
  finishPath();
  makeNewGroup();

  std::stringstream tmp;

  tmp << "<"SVG"text";

  // SVG uses fill color to fill text, but we want pen color.
  tmp << " style=\"stroke:none;";
  if (painter()->pen().color() != painter()->brush().color()
      || painter()->brush().style() == NoBrush) {
    const WColor& color = painter()->pen().color();
    tmp << "fill:" + color.cssText() << ";";
    if (color.alpha() != 255)
      tmp << "fill-opacity:" << myround(color.alpha() / 255.) << ";";
  }
  tmp << "\"";

  int horizontalAlign = flags & 0xF;
  int verticalAlign = flags & 0xF0;

  switch (horizontalAlign) {
  case AlignLeft:
    tmp << " x=" << quote(rect.left());
    break;
  case AlignRight:
    tmp << " x=" << quote(rect.right())
	<< " text-anchor=\"end\"";
    break;
  case AlignCenter:
    tmp << " x=" << quote(rect.center().x())
	<< " text-anchor=\"middle\"";
  }

/*
 * Opera doesn't do dominant-baseline yet
 */
#if 0
  switch (verticalAlign) {
  case AlignTop:
    tmp << " y=" << quote(rect.top())
	<< " dominant-baseline=\"text-before-edge\"";
    break;
  case AlignBottom:
    tmp << " y=" << quote(rect.bottom())
	<< " dominant-baseline=\"text-after-edge\"";
    break;
  case AlignMiddle:
    tmp << " y=" << quote(rect.center().y())
	<< " dominant-baseline=\"middle\"";
  }

  tmp << ">" << WWebWidget::escapeText(text, false).toUTF8() << "</"SVG"text>";

#else

  /*
   * Workaround: estimate the location of the default baseline which corresponds
   * with the font baseline.
   */
  double fontSize;
  switch (painter()->font().size()) {
  case WFont::FixedSize:
    fontSize = painter()->font().fixedSize().toPixels();
    break;
  default:
    fontSize = 16;
  }

  double y = rect.center().y();
  switch (verticalAlign) {
  case AlignTop:
    y = rect.top() + fontSize * 0.75; break;
  case AlignMiddle:
    y = rect.center().y() + fontSize * 0.25; break;
  case AlignBottom:
    y = rect.bottom() - fontSize * 0.25 ; break;
  }

  tmp << " y=" << quote(y);

  tmp << ">" << WWebWidget::escapeText(text, false).toUTF8() << "</"SVG"text>";
#endif

  shapes_ += tmp.str();
}

std::string WSvgImage::quote(const std::string& s)
{
  return '"' + s + '"';
}

std::string WSvgImage::quote(double d)
{
  return quote(boost::lexical_cast<std::string>(myround(d)));
}

std::string WSvgImage::fillStyle() const
{
  std::string result;

  switch (painter()->brush().style()) {
  case NoBrush:
    result += "fill:none;";
    break;
  case SolidPattern: {
    const WColor& color = painter()->brush().color();
    result += "fill:" + color.cssText() + ";";
    if (color.alpha() != 255)
      result += "fill-opacity:"
	+ boost::lexical_cast<std::string>(myround(color.alpha() / 255.)) + ";";
    break;
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
  std::string result;

  const WPen& pen = painter()->pen();

  if (!(painter()->renderHints() & WPainter::Antialiasing))
    result += "shape-rendering:optimizeSpeed;";

  if (pen.style() != NoPen) {
    const WColor& color = pen.color();

    result += "stroke:" + color.cssText() + ";";
    if (color.alpha() != 255)
      result += "stroke-opacity:"
	+ boost::lexical_cast<std::string>(color.alpha() / 255.) + ";";

    WLength w = normalizedPenWidth(pen.width(), true);
    if (w != WLength(1))
      result += "stroke-width:" + w.cssText() + ";";

    switch (pen.capStyle()) {
    case FlatCap:
      break;
    case SquareCap:
      result += "stroke-linecap:square;";
      break;
    case RoundCap:
      result += "stroke-linecap:round;";
    }

    switch (pen.joinStyle()) {
    case MiterJoin:
      break;
    case BevelJoin:
      result += "stroke-linejoin:bevel;";
      break;
    case RoundJoin:
      result += "stroke-linejoin:round;";
    }

    switch (pen.style()) {
    case NoPen:
      break;
    case SolidLine:
      break;
    case DashLine:
      result += "stroke-dasharray:4,2;";
      break;
    case DotLine:
      result += "stroke-dasharray:1,2;";
      break;
    case DashDotLine:
      result += "stroke-dasharray:4,2,1,2;";
      break;
    case DashDotDotLine:
      result += "stroke-dasharray:4,2,1,2,1,2;";
      break;
    }
  }

  return result;
}

std::string WSvgImage::fontStyle() const
{
  return " " + painter()->font().cssText();
}

std::string WSvgImage::rendered()
{
  std::stringstream s;

  ArgumentMap empty;
  streamResourceData(s, empty);

  return s.str();
}

const std::string WSvgImage::resourceMimeType() const
{
  return "image/svg+xml";
}

bool WSvgImage::streamResourceData(std::ostream& stream,
				   const ArgumentMap& arguments)
{
  stream << "<"SVG"svg xmlns=\"http://www.w3.org/2000/svg\""
    " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
    " version=\"1.1\" baseProfile=\"full\""
    " width=\"" << width().cssText() << "\""
    " height=\"" << height().cssText() << "\">"
	 << "<"SVG"g><"SVG"g>" << shapes_
	 << "</"SVG"g></"SVG"g></"SVG"svg>";

  return true;
}

}
