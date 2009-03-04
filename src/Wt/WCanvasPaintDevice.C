/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCanvasPaintDevice"
#include "Wt/WPainter"
#include "Wt/WPainterPath"
#include "Wt/WRectF"
#include "Wt/WWebWidget"

#include "DomElement.h"
#include "Utils.h"

#include <math.h>
#include <sstream>
#include <boost/lexical_cast.hpp>


namespace Wt {

namespace {

  static const double EPSILON=1E-5;

  WPointF normalizedDegreesToRadians(double angle, double sweep) {
    angle = 360 - angle;
    int i = (int)angle / 360;
    angle -= (i * 360);

    double r1 = WTransform::degreesToRadians(angle);

    if (fabs(sweep - 360) < 0.01)
      sweep = 359.9;
    else if (fabs(sweep + 360) < 0.01)
      sweep = -359.9;

    double a2 = angle - sweep;

    // the code below broke rendering on chrome, seems to be not needed
    // for other browsers ?
    //i = (int)a2 / 360;
    //a2 -= (i * 360);

    double r2 = WTransform::degreesToRadians(a2);

    return WPointF(r1, r2);
  }
}

WCanvasPaintDevice::WCanvasPaintDevice(const WLength& width,
				       const WLength& height,
				       WObject *parent)
  : WObject(parent),
    width_(width),
    height_(height),
    painter_(0)
{ }

void WCanvasPaintDevice::render(const std::string& canvasId,
				DomElement *text)
{
  std::string canvasVar = WT_CLASS ".getElement('" + canvasId + "')";

  std::stringstream tmp;

  tmp <<
    "if(" << canvasVar << ".getContext){"
    "new Wt._p_.ImagePreloader([";

  for (unsigned i = 0; i < images_.size(); ++i) {
    if (i != 0)
      tmp << ',';
    tmp << '\'' << images_[i] << '\'';
  }

  tmp << "],function(images) {"
    "var ctx=" << canvasVar << ".getContext('2d');"
    "ctx.clearRect(0,0," << width().value() << "," << height().value() <<
    ");ctx.save();ctx.save();" << js_ << "ctx.restore();ctx.restore();});}";

  text->callJavaScript(tmp.str());

  for (unsigned i = 0; i < textElements_.size(); ++i)
    text->addChild(textElements_[i]);
}

void WCanvasPaintDevice::init()
{ 
  changeFlags_ = Pen | Brush;
}

void WCanvasPaintDevice::done()
{ }

void WCanvasPaintDevice::drawArc(const WRectF& rect, double startAngle,
				 double spanAngle)
{
  if (rect.width() < EPSILON || rect.height() < EPSILON)
    return;

  renderStateChanges();

  std::stringstream tmp;

  WPointF ra = normalizedDegreesToRadians(startAngle, spanAngle);

  double sx, sy, r, lw;
  if (rect.width() > rect.height()) {
    sx = 1;
    sy = std::max(0.005, rect.height() / rect.width());
    r = rect.width()/2;
  } else {
    sx = std::max(0.005, rect.width() / rect.height());
    sy = 1;
    lw = painter()->normalizedPenWidth(painter()->pen().width(), true).value()
      * 1 / std::min(sx, sy);
    r = rect.height()/2;
  }

  const WPen& pen = painter()->pen();
  if (pen.style() != NoPen)
    lw = painter()->normalizedPenWidth(pen.width(), true).value()
      * 1 / std::min(sx, sy);
  else
    lw = 0;
  r = std::max(0.005, r - lw/2);

  tmp << "ctx.save();"
      << "ctx.translate(" << rect.center().x() << "," << rect.center().y()
      << ");"
      << "ctx.scale(" << sx << "," << sy << ");"
      << "ctx.lineWidth = " << lw << ";"
      << "ctx.beginPath();"
      << "ctx.arc(0,0," << r << ',' << ra.x() << "," << ra.y() << ",true);";

  if (painter()->brush().style() != NoBrush) {
    tmp << "ctx.fill();";
  }

  if (painter()->pen().style() != NoPen) {
    tmp << "ctx.stroke();";
  }

  tmp << "ctx.restore();";

  js_ += tmp.str();
}

int WCanvasPaintDevice::createImage(const std::string& imgUri)
{
  images_.push_back(imgUri);
  return images_.size() - 1;
}

void WCanvasPaintDevice::drawImage(const WRectF& rect,
				   const std::string& imgUri,
				   int imgWidth, int imgHeight,
				   const WRectF& sourceRect)
{
  renderStateChanges();

  int imageIndex = createImage(imgUri);

  char buf[30];
  std::stringstream tmp;
  tmp << "ctx.drawImage(images[" << imageIndex
      << "]," << Utils::round_str(sourceRect.x(), 3, buf);
  tmp << ',' << Utils::round_str(sourceRect.y(), 3, buf);
  tmp << ',' << Utils::round_str(sourceRect.width(), 3, buf);
  tmp << ',' << Utils::round_str(sourceRect.height(), 3, buf);
  tmp << ',' << Utils::round_str(rect.x(), 3, buf);
  tmp << ',' << Utils::round_str(rect.y(), 3, buf);
  tmp << ',' << Utils::round_str(rect.width(), 3, buf);
  tmp << ',' << Utils::round_str(rect.height(), 3, buf) << ");";

  js_ += tmp.str();
}

void WCanvasPaintDevice::drawPlainPath(std::stringstream& out,
				       const WPainterPath& path)
{
  char buf[30];
  const std::vector<WPainterPath::Segment>& segments = path.segments();

  out << "ctx.beginPath();";

  if (segments.size() > 0
      && segments[0].type() != WPainterPath::Segment::MoveTo)
    out << "ctx.moveTo(0,0);";

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    switch (s.type()) {
    case WPainterPath::Segment::MoveTo:
      out << "ctx.moveTo(" << Utils::round_str(s.x(), 3, buf);
      out << ',' << Utils::round_str(s.y(), 3, buf) << ");";
      break;
    case WPainterPath::Segment::LineTo:
      out << "ctx.lineTo(" << Utils::round_str(s.x(), 3, buf);
      out << ',' << Utils::round_str(s.y(), 3, buf) << ");";
      break;
    case WPainterPath::Segment::CubicC1:
      out << "ctx.bezierCurveTo(" << Utils::round_str(s.x(), 3, buf);
      out << ',' << Utils::round_str(s.y(), 3, buf);
      break;
    case WPainterPath::Segment::CubicC2:
      out << ',' << Utils::round_str(s.x(), 3, buf) << ',';
      out << Utils::round_str(s.y(), 3, buf);
      break;
    case WPainterPath::Segment::CubicEnd:
      out << ',' << Utils::round_str(s.x(), 3, buf) << ',';
      out << Utils::round_str(s.y(), 3, buf) << ");";
      break;
    case WPainterPath::Segment::ArcC:
      out << "ctx.arc(" << Utils::round_str(s.x(), 3, buf) << ',';
      out << Utils::round_str(s.y(), 3, buf);
      break;
    case WPainterPath::Segment::ArcR:
      out << ',' << Utils::round_str(s.x(), 3, buf);
      break;
    case WPainterPath::Segment::ArcAngleSweep:
      {
	WPointF r = normalizedDegreesToRadians(s.x(), s.y());

	out << ',' << r.x() << ',' << r.y() << ','
	    << (s.y() > 0 ? "true" : "false")
	    << ");";
      }
      break;
    case WPainterPath::Segment::QuadC: {
      /*
       * quadraticCurveTo() is broken in Firefox 1.5.
       * Therefore, we convert to a bezier Curve instead.
       */
      WPointF current = path.positionAtSegment(i);
      const double cpx = s.x();
      const double cpy = s.y();
      const double x = segments[i+1].x();
      const double y = segments[i+1].y();
      
      const double cp1x = current.x() + 2.0/3.0*(cpx - current.x());
      const double cp1y = current.y() + 2.0/3.0*(cpy - current.y());
      const double cp2x = cp1x + (x - current.x())/3.0;
      const double cp2y = cp1y + (y - current.y())/3.0;

      // and now call cubic Bezier curve to function 
      out << "ctx.bezierCurveTo(" << Utils::round_str(cp1x, 3, buf) << ',';
      out << Utils::round_str(cp1y, 3, buf) << ',';
      out << Utils::round_str(cp2x, 3, buf) << ',';
      out << Utils::round_str(cp2y, 3, buf);

      break;
    }
    case WPainterPath::Segment::QuadEnd:
      out << ',' << Utils::round_str(s.x(), 3, buf) << ',';
      out << Utils::round_str(s.y(), 3, buf) << ");";
    }
  }
}

void WCanvasPaintDevice::drawPath(const WPainterPath& path)
{
  renderStateChanges();

  std::stringstream tmp;

  drawPlainPath(tmp, path);

  if (painter()->brush().style() != NoBrush) {
    tmp << "ctx.fill();";
  }

  if (painter()->pen().style() != NoPen) {
    tmp << "ctx.stroke();";
  }

  js_ += tmp.str() + '\n';
}

void WCanvasPaintDevice::drawLine(double x1, double y1, double x2, double y2)
{
  renderStateChanges();

  char buf[30];
  std::stringstream tmp;

  tmp << "ctx.beginPath();"
      << "ctx.moveTo(" << Utils::round_str(x1, 3, buf) << ',';
  tmp << Utils::round_str(y1, 3, buf) << ");";
  tmp << "ctx.lineTo(" << Utils::round_str(x2, 3, buf) << ',';
  tmp << Utils::round_str(y2, 3, buf) << ");ctx.stroke();";

  js_ += tmp.str();
}

void WCanvasPaintDevice::drawText(const WRectF& rect,
				  WFlags<AlignmentFlag> flags,
				  const WString& text)
{
  WPointF pos = painter()->combinedTransform().map(rect.topLeft());

  DomElement *e = DomElement::createNew(DomElement_DIV);
  e->setProperty(PropertyStylePosition, "absolute");
  e->setProperty(PropertyStyleTop,
		 boost::lexical_cast<std::string>(pos.y()) + "px");
  e->setProperty(PropertyStyleLeft,
		 boost::lexical_cast<std::string>(pos.x()) + "px");
  e->setProperty(PropertyStyleWidth,
		 boost::lexical_cast<std::string>(rect.width()) + "px");
  e->setProperty(PropertyStyleHeight,
		 boost::lexical_cast<std::string>(rect.height()) + "px");

  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  DomElement *t = e;

  /*
   * HTML tricks to center things vertically -- does not work on IE,
   * (neither does canvas)
   */
  if (verticalAlign != AlignTop) {
    t = DomElement::createNew(DomElement_DIV);

    if (verticalAlign == AlignMiddle) {
      e->setProperty(PropertyStyleDisplay, "table");
      t->setProperty(PropertyStyleDisplay, "table-cell");
      t->setProperty(PropertyStyleVerticalAlign, "middle");
    } else if (verticalAlign == AlignBottom) {
      t->setProperty(PropertyStylePosition, "absolute");
      t->setProperty(PropertyStyleWidth, "100%");
      t->setProperty(PropertyStyleBottom, "0px");
    }

  }

  t->setProperty(PropertyInnerHTML,
		 WWebWidget::escapeText(text, true).toUTF8());

  WFont f = painter()->font();
  f.updateDomElement(*t, false, true);

  t->setProperty(PropertyStyleColor, painter()->pen().color().cssText());

  if (horizontalAlign == AlignRight)
    t->setProperty(PropertyStyleTextAlign, "right");
  else if (horizontalAlign == AlignCenter)
    t->setProperty(PropertyStyleTextAlign, "center");
  else
    t->setProperty(PropertyStyleTextAlign, "left");

  if (t != e)
    e->addChild(t);

  textElements_.push_back(e);
}

void WCanvasPaintDevice::setChanged(WFlags<ChangeFlag> flags)
{
  changeFlags_ |= flags;
}

void WCanvasPaintDevice::renderTransform(std::stringstream& s,
					 const WTransform& t, bool invert)
{
  if (!t.isIdentity()) {
    char buf[30];
    WTransform::TRSRDecomposition d;

    t.decomposeTranslateRotateScaleRotate(d);

    if (!invert) {
      if (fabs(d.dx) > EPSILON || fabs(d.dy) > EPSILON) {
	s << "ctx.translate(" << Utils::round_str(d.dx, 3, buf) << ',';
	s << Utils::round_str(d.dy, 3, buf) << ");";
      }

      if (fabs(d.alpha1) > EPSILON)
	s << "ctx.rotate(" << d.alpha1 << ");";

      if (fabs(d.sx - 1) > EPSILON || fabs(d.sy - 1) > EPSILON) {
	s << "ctx.scale(" << Utils::round_str(d.sx, 3, buf) << ',';
	s << Utils::round_str(d.sy, 3, buf) << ");";
      }

      if (fabs(d.alpha2) > EPSILON)
	s << "ctx.rotate(" << d.alpha2 << ");";
    } else {
      if (fabs(d.alpha2) > EPSILON)
	s << "ctx.rotate(" << -d.alpha2 << ");";

      if (fabs(d.sx - 1) > EPSILON || fabs(d.sy - 1) > EPSILON) {
	s << "ctx.scale(" << Utils::round_str(1/d.sx, 3, buf) << ',';
	s << Utils::round_str(1/d.sy, 3, buf) << ");";
      }

      if (fabs(d.alpha1) > EPSILON)
	s << "ctx.rotate(" << -d.alpha1 << ");";

      if (fabs(d.dx) > EPSILON || fabs(d.dy) > EPSILON) {
	s << "ctx.translate(" << Utils::round_str(-d.dx, 3, buf) << ',';
	s << Utils::round_str(-d.dy, 3, buf) << ");";
      }
    }
  }
}

void WCanvasPaintDevice::renderStateChanges()
{
  if (changeFlags_ == 0)
    return;

  std::stringstream s;

  if (changeFlags_ & (Transform | Clipping)) {
    if (changeFlags_ & Clipping) {
      s << "ctx.restore();ctx.restore();ctx.save();";

      const WTransform& t = painter()->clipPathTransform();

      renderTransform(s, t);
      if (painter()->hasClipping()) {
	drawPlainPath(s, painter()->clipPath());
	s << "ctx.clip();";
      }
      renderTransform(s, t, true);

      s << "ctx.save();";
    } else
      s << "ctx.restore();ctx.save();";

    const WTransform& t = painter()->combinedTransform();
    renderTransform(s, t);

    changeFlags_ |= Pen | Brush;
  }

  if (changeFlags_ & Pen) {
    const WPen& pen = painter()->pen();

    s << "ctx.strokeStyle=\"" + pen.color().cssText(true)
      << "\";ctx.lineWidth="
      << painter()->normalizedPenWidth(pen.width(), true).value()
      << ';';

    switch (pen.capStyle()) {
    case FlatCap:
      s << "ctx.lineCap='butt';";
      break;
    case SquareCap:
      s << "ctx.lineCap='square';";
      break;
    case RoundCap:
      s << "ctx.lineCap='round';";
    }

    switch (pen.joinStyle()) {
    case MiterJoin:
      s << "ctx.lineJoin='miter';";
      break;
    case BevelJoin:
      s << "ctx.lineJoin='bevel';";
      break;
    case RoundJoin:
      s << "ctx.lineJoin='round';";
    }
  }

  if (changeFlags_ & Brush)
    s << "ctx.fillStyle=\"" + painter()->brush().color().cssText(true) + "\";";

  js_ += s.str();

  changeFlags_ = 0;
}

}
