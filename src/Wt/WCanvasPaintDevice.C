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

#include <math.h>
#include <sstream>
#include <boost/lexical_cast.hpp>

namespace Wt {

namespace {

  static const double EPSILON=1E-5;

  void normalizedDegreesToRadians(double angle, double sweep,
				  double& r1, double& r2) {
    angle = 360 - angle;
    int i = (int)angle / 360;
    angle -= (i * 360);

    r1 = WTransform::degreesToRadians(angle);

    if (fabs(sweep - 360) < 0.01)
      sweep = 359.9;
    else if (fabs(sweep + 360) < 0.01)
      sweep = -359.9;

    double a2 = angle - sweep;

    // the code below broke rendering on chrome, seems to be not needed
    // for other browsers ?
    //i = (int)a2 / 360;
    //a2 -= (i * 360);

    r2 = WTransform::degreesToRadians(a2);
  }

  double myround(double d)
  {
    return static_cast<int>(d * 100) / 100.;
  }
}

WCanvasPaintDevice::WCanvasPaintDevice(const WLength& width,
				       const WLength& height,
				       WObject *parent)
  : WPaintDevice(width, height),
    WObject(parent)
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

  double a1, a2;
  normalizedDegreesToRadians(startAngle, spanAngle, a1, a2);

  double sx, sy, r, lw;
  if (rect.width() > rect.height()) {
    sx = 1;
    sy = std::max(0.005, rect.height() / rect.width());
    r = rect.width()/2;
  } else {
    sx = std::max(0.005, rect.width() / rect.height());
    sy = 1;
    lw = normalizedPenWidth(painter()->pen().width(), true).value()
      * 1 / std::min(sx, sy);
    r = rect.height()/2;
  }

  const WPen& pen = painter()->pen();
  if (pen.style() != NoPen)
    lw = normalizedPenWidth(pen.width(), true).value() * 1 / std::min(sx, sy);
  else
    lw = 0;
  r = std::max(0.005, r - lw/2);

  tmp << "ctx.save();"
      << "ctx.translate(" << rect.center().x() << "," << rect.center().y()
      << ");"
      << "ctx.scale(" << sx << "," << sy << ");"
      << "ctx.lineWidth = " << lw << ";"
      << "ctx.beginPath();"
      << "ctx.arc(0,0," << r << ',' << a1 << "," << a2 << ",true);";

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

  std::stringstream tmp;
  tmp << "ctx.drawImage(images[" << imageIndex
      << "]," << myround(sourceRect.x())
      << "," << myround(sourceRect.y())
      << "," << myround(sourceRect.width())
      << "," << myround(sourceRect.height())
      << "," << myround(rect.x())
      << "," << myround(rect.y())
      << "," << myround(rect.width())
      << "," << myround(rect.height()) << ");";

  js_ += tmp.str();
}

void WCanvasPaintDevice::drawPlainPath(std::ostream& out,
				       const WPainterPath& path)
{
  const std::vector<WPainterPath::Segment>& segments = path.segments();

  out << "ctx.beginPath();";

  if (segments.size() > 0
      && segments[0].type() != WPainterPath::Segment::MoveTo)
    out << "ctx.moveTo(0,0);";

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    switch (s.type()) {
    case WPainterPath::Segment::MoveTo:
      out << "ctx.moveTo(" << myround(s.x()) << "," << myround(s.y()) << ");";
      break;
    case WPainterPath::Segment::LineTo:
      out << "ctx.lineTo(" << myround(s.x()) << "," << myround(s.y()) << ");";
      break;
    case WPainterPath::Segment::CubicC1:
      out << "ctx.bezierCurveTo(" << myround(s.x()) << "," << myround(s.y());
      break;
    case WPainterPath::Segment::CubicC2:
      out << "," << myround(s.x()) << "," << myround(s.y());
      break;
    case WPainterPath::Segment::CubicEnd:
      out << "," << myround(s.x()) << "," << myround(s.y()) << ");";
      break;
    case WPainterPath::Segment::ArcC:
      out << "ctx.arc(" << myround(s.x()) << "," << myround(s.y());
      break;
    case WPainterPath::Segment::ArcR:
      out << "," << myround(s.x());
      break;
    case WPainterPath::Segment::ArcAngleSweep:
      {
	double a1, a2;

	normalizedDegreesToRadians(s.x(), s.y(), a1, a2);

	out << "," << a1 << "," << a2 << "," << (s.y() > 0 ? "true" : "false")
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
      out << "ctx.bezierCurveTo("
	  <<        myround(cp1x) << "," << myround(cp1y)
	  << "," << myround(cp2x) << "," << myround(cp2y);
   
      break;
    }
    case WPainterPath::Segment::QuadEnd:
      out << "," << myround(s.x()) << "," << myround(s.y()) << ");";
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

  js_ += tmp.str();
}

void WCanvasPaintDevice::drawLine(double x1, double y1, double x2, double y2)
{
  renderStateChanges();

  std::stringstream tmp;

  tmp << "ctx.beginPath();"
      << "ctx.moveTo(" << myround(x1) << "," << myround(y1) << ");"
      << "ctx.lineTo(" << myround(x2) << "," << myround(y2) << ");"
      << "ctx.stroke();";

  js_ += tmp.str();
}

void WCanvasPaintDevice::drawText(const WRectF& rect, int flags,
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

  int horizontalAlign = flags & 0xF;
  int verticalAlign = flags & 0xF0;

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
		 WWebWidget::escapeText(text.value(), true).toUTF8());

  WFont f = painter()->font();
  f.updateDomElement(*t, false, true);

  t->setProperty(PropertyStyleColor, painter()->pen().color().cssText());

  if (horizontalAlign == AlignRight)
    t->setProperty(PropertyStyleTextAlign, "right");
  else if (horizontalAlign == AlignCenter)
    t->setProperty(PropertyStyleTextAlign, "center");

  if (t != e)
    e->addChild(t);

  textElements_.push_back(e);
}

void WCanvasPaintDevice::setChanged(int flags)
{
  changeFlags_ |= flags;
}

void WCanvasPaintDevice::renderTransform(std::ostream& s, const WTransform& t,
					 bool invert)
{
  if (!t.isIdentity()) {
    double dx, dy, alpha1, sx, sy, alpha2;

    t.decomposeTranslateRotateScaleRotate(dx, dy, alpha1, sx, sy, alpha2);

    if (!invert) {
      if (fabs(dx) > EPSILON || fabs(dy) > EPSILON)
	s << "ctx.translate(" << myround(dx) << ',' << myround(dy) << ");";

      if (fabs(alpha1) > EPSILON)
	s << "ctx.rotate(" << alpha1 << ");";

      if (fabs(sx - 1) > EPSILON || fabs(sy - 1) > EPSILON)
	s << "ctx.scale(" << sx << ',' << sy << ");";

      if (fabs(alpha2) > EPSILON)
	s << "ctx.rotate(" << alpha2 << ");";
    } else {
      if (fabs(alpha2) > EPSILON)
	s << "ctx.rotate(" << -alpha2 << ");";

      if (fabs(sx - 1) > EPSILON || fabs(sy - 1) > EPSILON)
	s << "ctx.scale(" << 1/sx << ',' << 1/sy << ");";

      if (fabs(alpha1) > EPSILON)
	s << "ctx.rotate(" << -alpha1 << ");";

      if (fabs(dx) > EPSILON || fabs(dy) > EPSILON)
	s << "ctx.translate(" << myround(-dx) << ',' << myround(-dy) << ");";
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
      << "\";ctx.lineWidth=" << normalizedPenWidth(pen.width(), true).value()
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
