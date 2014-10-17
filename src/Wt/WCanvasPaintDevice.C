/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WCanvasPaintDevice"
#include "Wt/WEnvironment"
#include "Wt/WFontMetrics"
#include "Wt/WPainter"
#include "Wt/WPainterPath"
#include "Wt/WRectF"
#include "Wt/WWebWidget"

#include "DomElement.h"
#include "WebUtils.h"
#include "ServerSideFontMetrics.h"

#include <cmath>
#include <sstream>
#include <boost/lexical_cast.hpp>

namespace {
  static const double EPSILON = 1E-5;
}

namespace Wt {

namespace {
  WPointF normalizedDegreesToRadians(double angle, double sweep) {
    angle = 360 - angle;
    int i = (int)angle / 360;
    angle -= (i * 360);

    double r1 = WTransform::degreesToRadians(angle);

    if (std::fabs(sweep - 360) < 0.01)
      sweep = 359.9;
    else if (std::fabs(sweep + 360) < 0.01)
      sweep = -359.9;

    double a2 = angle - sweep;

    double r2 = WTransform::degreesToRadians(a2);

    return WPointF(r1, r2);
  }

  bool fequal(double d1, double d2) {
    return std::fabs(d1 - d2) < 1E-5;
  }

  std::string defineGradient(const Wt::WGradient& gradient,
			     std::stringstream& js) {
    std::string jsRef = "grad";
    if (gradient.style() == Wt::LinearGradient) {
      const WLineF& gradVec = gradient.linearGradientVector();
      js << "var " << jsRef << " = ctx.createLinearGradient("
	  << gradVec.x1() << ", " << gradVec.y1() << ", "
	  << gradVec.x2() << ", " << gradVec.y2()
	  << ");";
    } else if (gradient.style() == Wt::RadialGradient) {
      js << "var " << jsRef << " = ctx.createRadialGradient("
	  << gradient.radialFocalPoint().x() << ", "
	  << gradient.radialFocalPoint().y() << ","
	  << "0, "
	  << gradient.radialCenterPoint().x() << ", "
	  << gradient.radialCenterPoint().y() << ", "
	  << gradient.radialRadius() << ");";
    }
    for (unsigned i=0; i<gradient.colorstops().size(); i++) {
      js << jsRef << ".addColorStop(" 
	 << gradient.colorstops()[i].position() << ","
	 << WWebWidget::jsStringLiteral
	(gradient.colorstops()[i].color().cssText(true))
	 << ");";
    }

    return jsRef;
  }
}

WCanvasPaintDevice::WCanvasPaintDevice(const WLength& width,
				       const WLength& height,
				       WObject *parent,
				       bool paintUpdate)
  : WObject(parent),
    width_(width),
    height_(height),
    painter_(0),
    paintUpdate_(paintUpdate),
    busyWithPath_(false),
    fontMetrics_(0)
{ 
  textMethod_ = DomText;

  WApplication *app = WApplication::instance();

  if (app) {
    if (app->environment().agentIsIE()) {
      textMethod_ = Html5Text;
    } else if (app->environment().agentIsChrome()) {
      if (app->environment().agent() >= WEnvironment::Chrome2
	  && !app->environment().agentIsMobileWebKit())
	textMethod_ = Html5Text;
    } else if (app->environment().agentIsGecko()) {
      if (app->environment().agent() >= WEnvironment::Firefox3_5)
	textMethod_ = Html5Text;
      else if (app->environment().agent() >= WEnvironment::Firefox3_0)
	textMethod_ = MozText;
    } else if (app->environment().agentIsSafari()) {
      if (app->environment().agent() >= WEnvironment::Safari4)
	textMethod_ = Html5Text;
    }
  }
}

WFlags<WPaintDevice::FeatureFlag> WCanvasPaintDevice::features() const
{
  if (ServerSideFontMetrics::available())
    return HasFontMetrics;
  else
    return 0;
}

void WCanvasPaintDevice::render(const std::string& canvasId,
				DomElement *text)
{
  std::string canvasVar = WT_CLASS ".getElement('" + canvasId + "')";

  std::stringstream tmp;

  tmp <<
    "if(" << canvasVar << ".getContext){";

  if (!images_.empty()) {
    tmp << "new Wt._p_.ImagePreloader([";

    for (unsigned i = 0; i < images_.size(); ++i) {
      if (i != 0)
	tmp << ',';
      tmp << '\'' << images_[i] << '\'';
    }

    tmp <<
      "],function(images)";
  }

  tmp << "{var ctx=" << canvasVar << ".getContext('2d');";
  // Older browsers don't have setLineDash
  tmp << "if (!ctx.setLineDash) {ctx.setLineDash = function(a){};}";

  if (!paintUpdate_) {
    tmp << "ctx.clearRect(0,0,"
	<< width().value() << "," << height().value() << ");";
  }

  tmp << "ctx.save();ctx.save();" << js_.str() 
      << "ctx.restore();ctx.restore();}";

  if (!images_.empty()) {
    tmp << ");";
  }

  tmp << "}";

  text->callJavaScript(tmp.str());

  for (unsigned i = 0; i < textElements_.size(); ++i)
    text->addChild(textElements_[i]);
}

void WCanvasPaintDevice::renderPaintCommands(std::stringstream& js_target,
					     const std::string& canvasElement)
{
  js_target << "var ctx=" << canvasElement << ".getContext('2d');";
  js_target << "if (!ctx.setLineDash) {ctx.setLineDash = function(a){};}";
  js_target << "ctx.save();ctx.save();" << js_.str() 
	    << "ctx.restore();ctx.restore();";
}

void WCanvasPaintDevice::init()
{
  currentBrush_ = WBrush();
  currentNoBrush_ = false;
  currentPen_ = WPen();
  currentNoPen_ = false;
  currentPen_.setCapStyle(FlatCap);
  currentShadow_ = WShadow();
  currentFont_ = WFont();
  currentTextVAlign_ = currentTextHAlign_ = AlignSuper;

  changeFlags_ = Transform | Pen | Brush | Shadow | Font;
}

void WCanvasPaintDevice::done()
{
  finishPath();
}

void WCanvasPaintDevice::drawArc(const WRectF& rect, double startAngle,
				 double spanAngle)
{
  finishPath();
  
  if (rect.width() < EPSILON || rect.height() < EPSILON)
    return;

  renderStateChanges(true);

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

  char buf[30];

  js_ << "ctx.save();"
      << "ctx.translate(" << Utils::round_js_str(rect.center().x(), 3, buf);
  js_ << "," << Utils::round_js_str(rect.center().y(), 3, buf);
  js_ << ");"
      << "ctx.scale(" << Utils::round_js_str(sx, 3, buf);
  js_ << "," << Utils::round_js_str(sy, 3, buf) << ");";
  js_ << "ctx.lineWidth = " << Utils::round_js_str(lw, 3, buf) << ";"
      << "ctx.beginPath();";
  js_ << "ctx.arc(0,0," << Utils::round_js_str(r, 3, buf);
  js_ << ',' << Utils::round_js_str(ra.x(), 3, buf);
  js_ << "," << Utils::round_js_str(ra.y(), 3, buf) << ",true);";

  // restore comes before fill and stroke, otherwise the gradient will use 
  // this temporary coordinate system
  js_ << "ctx.restore();";

  if (painter_->brush().style() != NoBrush) {
    js_ << "ctx.fill();";
  }

  if (painter_->pen().style() != NoPen) {
    js_ << "ctx.stroke();";
  }
}

int WCanvasPaintDevice::createImage(const std::string& imgUri)
{
  images_.push_back(imgUri);
  return images_.size() - 1;
}

void WCanvasPaintDevice::drawImage(const WRectF& rect,
				   const std::string& imageUri,
				   int imgWidth, int imgHeight,
				   const WRectF& sourceRect)
{
  finishPath();

  renderStateChanges(true);

  WApplication *app = WApplication::instance();
  std::string imgUri;
  if (app)
    imgUri = app->resolveRelativeUrl(imageUri);

  int imageIndex = createImage(imgUri);

  char buf[30];
  js_ << "ctx.drawImage(images[" << imageIndex
      << "]," << Utils::round_js_str(sourceRect.x(), 3, buf);
  js_ << ',' << Utils::round_js_str(sourceRect.y(), 3, buf);
  js_ << ',' << Utils::round_js_str(sourceRect.width(), 3, buf);
  js_ << ',' << Utils::round_js_str(sourceRect.height(), 3, buf);
  js_ << ',' << Utils::round_js_str(rect.x(), 3, buf);
  js_ << ',' << Utils::round_js_str(rect.y(), 3, buf);
  js_ << ',' << Utils::round_js_str(rect.width(), 3, buf);
  js_ << ',' << Utils::round_js_str(rect.height(), 3, buf) << ");";
}

void WCanvasPaintDevice::drawPlainPath(std::stringstream& out,
				       const WPainterPath& path)
{
  char buf[30];

  if (!busyWithPath_) {
    out << "ctx.beginPath();";
    busyWithPath_ = true;
  }

  const std::vector<WPainterPath::Segment>& segments = path.segments();

  if (segments.size() > 0
      && segments[0].type() != WPainterPath::Segment::MoveTo)
    out << "ctx.moveTo(0,0);";

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    switch (s.type()) {
    case WPainterPath::Segment::MoveTo:
      out << "ctx.moveTo(" << Utils::round_js_str(s.x() + pathTranslation_.x(),
						  3, buf);
      out << ',' << Utils::round_js_str(s.y() + pathTranslation_.y(),
				     3, buf) << ");";
      break;
    case WPainterPath::Segment::LineTo:
      out << "ctx.lineTo(" << Utils::round_js_str(s.x() + pathTranslation_.x(),
						  3, buf);
      out << ',' << Utils::round_js_str(s.y() + pathTranslation_.y(),
					3, buf) << ");";
      break;
    case WPainterPath::Segment::CubicC1:
      out << "ctx.bezierCurveTo("
	  << Utils::round_js_str(s.x() + pathTranslation_.x(), 3, buf);
      out << ',' << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf);
      break;
    case WPainterPath::Segment::CubicC2:
      out << ',' << Utils::round_js_str(s.x() + pathTranslation_.x(), 3, buf)
	  << ',';
      out << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf);
      break;
    case WPainterPath::Segment::CubicEnd:
      out << ',' << Utils::round_js_str(s.x() + pathTranslation_.x(), 3, buf)
	  << ',';
      out << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf) << ");";
      break;
    case WPainterPath::Segment::ArcC:
      out << "ctx.arc(" << Utils::round_js_str(s.x() + pathTranslation_.x(), 3,
					       buf) << ',';
      out << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf);
      break;
    case WPainterPath::Segment::ArcR:
      out << ',' << Utils::round_js_str(s.x(), 3, buf);
      break;
    case WPainterPath::Segment::ArcAngleSweep:
      {
	WPointF r = normalizedDegreesToRadians(s.x(), s.y());

	out << ',' << Utils::round_js_str(r.x(), 3, buf);
	out << ',' << Utils::round_js_str(r.y(), 3, buf);
	out << ',' << (s.y() > 0 ? "true" : "false") << ");";
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
	  << Utils::round_js_str(cp1x + pathTranslation_.x(), 3, buf) << ',';
      out << Utils::round_js_str(cp1y + pathTranslation_.y(), 3, buf) << ',';
      out << Utils::round_js_str(cp2x + pathTranslation_.x(), 3, buf) << ',';
      out << Utils::round_js_str(cp2y + pathTranslation_.y(), 3, buf);

      break;
    }
    case WPainterPath::Segment::QuadEnd:
      out << ','
	  << Utils::round_js_str(s.x() + pathTranslation_.x(), 3, buf) << ',';
      out << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf) << ");";
    }
  }
}

void WCanvasPaintDevice::finishPath()
{
  if (busyWithPath_) {
    if (!currentNoBrush_)
      js_ << "ctx.fill();";

    if (!currentNoPen_)
      js_ << "ctx.stroke();";

    js_ << '\n';

    busyWithPath_ = false;
  }
}

void WCanvasPaintDevice::drawPath(const WPainterPath& path)
{
  renderStateChanges(false);

  drawPlainPath(js_, path);
  if (painter_->brush().color().alpha() != 255 ||
      painter_->pen().color().alpha() != 255)
    finishPath();
}

void WCanvasPaintDevice::drawLine(double x1, double y1, double x2, double y2)
{
  WPainterPath path;
  path.moveTo(x1, y1);
  path.lineTo(x2, y2);
  drawPath(path);
}

void WCanvasPaintDevice::drawText(const WRectF& rect,
				  WFlags<AlignmentFlag> flags,
				  TextFlag textFlag,
				  const WString& text)
{
  if (textFlag == TextWordWrap)
    throw WException("WCanvasPaintDevice::drawText() "
		     "TextWordWrap is not supported");

  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  if (textMethod_ != DomText) {
    finishPath();
    renderStateChanges(true);
  }

  switch (textMethod_) {
  case Html5Text: 
    {
      double x = 0, y = 0;
      
      if (horizontalAlign != currentTextHAlign_) {
	js_ << "ctx.textAlign='";
	switch (horizontalAlign) {
	case AlignLeft: js_ << "left"; break;
	case AlignRight: js_ << "right"; break;
	case AlignCenter: js_ << "center"; break;
	default: break;
	}
	js_ << "';";
	currentTextHAlign_ = horizontalAlign;
      }

      switch (horizontalAlign) {
      case AlignLeft: x = rect.left(); break;
      case AlignRight: x = rect.right(); break;
      case AlignCenter: x = rect.center().x(); break;
      default: break;
      }

      if (verticalAlign != currentTextVAlign_) {
	js_ << "ctx.textBaseline='";
	switch (verticalAlign) {
	case AlignTop: js_ << "top"; break;
	case AlignBottom: js_ << "bottom"; break;
	case AlignMiddle: js_ << "middle"; break;
	default: break;
	}
	js_ << "';";
	currentTextVAlign_ = verticalAlign;
      }

      switch (verticalAlign) {
      case AlignTop: y = rect.top(); break;
      case AlignBottom: y = rect.bottom(); break;
      case AlignMiddle: y = rect.center().y(); break;
      default: break;
      }

      if (currentBrush_.color() != currentPen_.color())
	js_ << "ctx.fillStyle="
	    << WWebWidget::jsStringLiteral(currentPen_.color().cssText(true))
	    << ";";

      char buf[30];

      js_ << "ctx.fillText(" << text.jsStringLiteral()
	  << ',' << Utils::round_js_str(x, 3, buf) << ',';
      js_ << Utils::round_js_str(y, 3, buf) << ");";

      if (currentBrush_.color() != currentPen_.color())
	js_ << "ctx.fillStyle="
	    << WWebWidget::jsStringLiteral(currentBrush_.color().cssText(true))
	    << ";";
    }
    break;
  case MozText:
    {
      std::string x;

      switch (horizontalAlign) {
      case AlignLeft:
	x = boost::lexical_cast<std::string>(rect.left());
	break;
      case AlignRight:
	x = boost::lexical_cast<std::string>(rect.right())
	  + " - ctx.mozMeasureText(" + text.jsStringLiteral() + ")";
	break;
      case AlignCenter:
	x = boost::lexical_cast<std::string>(rect.center().x())
	  + " - ctx.mozMeasureText(" + text.jsStringLiteral() + ")/2";
	break;
      default:
	break;
      }

      double fontSize;
      switch (painter()->font().size()) {
      case WFont::FixedSize:
	fontSize = painter()->font().fixedSize().toPixels();
	break;
      default:
	fontSize = 16;
      }

      double y = 0;
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

      js_ << "ctx.save();";
      js_ << "ctx.translate(" << x << ", " << y << ");";
      if (currentBrush_.color() != currentPen_.color())
	js_ << "ctx.fillStyle="
	    << WWebWidget::jsStringLiteral(currentPen_.color().cssText(true))
	    << ";";
      js_ << "ctx.mozDrawText(" << text.jsStringLiteral() << ");";
      js_ << "ctx.restore();";
    }
    break;
  case DomText:
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
  }
}

WTextItem WCanvasPaintDevice::measureText(const WString& text, double maxWidth,
					  bool wordWrap)
{
  if (!fontMetrics_)
    fontMetrics_ = new ServerSideFontMetrics();

  return fontMetrics_->measureText(painter()->font(), text, maxWidth, wordWrap);
}

WFontMetrics WCanvasPaintDevice::fontMetrics()
{
  if (!fontMetrics_)
    fontMetrics_ = new ServerSideFontMetrics();

  return fontMetrics_->fontMetrics(painter()->font());
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
      if (std::fabs(d.dx) > EPSILON || std::fabs(d.dy) > EPSILON) {
	s << "ctx.translate(" << Utils::round_js_str(d.dx, 3, buf) << ',';
	s << Utils::round_js_str(d.dy, 3, buf) << ");";
      }

      if (std::fabs(d.alpha1) > EPSILON)
	s << "ctx.rotate(" << d.alpha1 << ");";

      if (std::fabs(d.sx - 1) > EPSILON || std::fabs(d.sy - 1) > EPSILON) {
	s << "ctx.scale(" << Utils::round_js_str(d.sx, 3, buf) << ',';
	s << Utils::round_js_str(d.sy, 3, buf) << ");";
      }

      if (std::fabs(d.alpha2) > EPSILON)
	s << "ctx.rotate(" << d.alpha2 << ");";
    } else {
      if (std::fabs(d.alpha2) > EPSILON)
	s << "ctx.rotate(" << -d.alpha2 << ");";

      if (std::fabs(d.sx - 1) > EPSILON || std::fabs(d.sy - 1) > EPSILON) {
	s << "ctx.scale(" << Utils::round_js_str(1/d.sx, 3, buf) << ',';
	s << Utils::round_js_str(1/d.sy, 3, buf) << ");";
      }

      if (std::fabs(d.alpha1) > EPSILON)
	s << "ctx.rotate(" << -d.alpha1 << ");";

      if (std::fabs(d.dx) > EPSILON || std::fabs(d.dy) > EPSILON) {
	s << "ctx.translate(" << Utils::round_js_str(-d.dx, 3, buf) << ',';
	s << Utils::round_js_str(-d.dy, 3, buf) << ");";
      }
    }
  }
}

void WCanvasPaintDevice::renderStateChanges(bool resetPathTranslation)
{
  if (resetPathTranslation && 
      (!fequal(pathTranslation_.x(), 0) ||
       !fequal(pathTranslation_.y(), 0)))
    changeFlags_ |= Transform;

  if (!changeFlags_)
    return;

  /*
   * For unclear reasons, Firefox on Linux reacts badly against very
   * long painter paths (e.g. when drawing markers in charts) and by
   * more prematurely rendering pen/brush changes we avoid that.
   */
  WApplication *app = WApplication::instance();
  bool slowFirefox = app && app->environment().agentIsGecko();
  if (slowFirefox &&
      app->environment().userAgent().find("Linux") == std::string::npos)
    slowFirefox = false;

  bool brushChanged =
    (changeFlags_ & Brush) &&
    (currentBrush_ != painter()->brush()) &&
    (slowFirefox || painter()->brush().style() != NoBrush);

  bool penChanged =
    (changeFlags_ & Pen) &&
    (currentPen_ != painter()->pen()) &&
    (slowFirefox || painter()->pen().style() != NoPen);

  bool penColorChanged = 
    penChanged && 
    (currentPen_.color() != painter()->pen().color() ||
     currentPen_.gradient() != painter()->pen().gradient());

  bool shadowChanged =
    (changeFlags_ & Shadow)
    && (currentShadow_ != painter()->shadow());

  bool fontChanged = (changeFlags_ & Font)
    && (currentFont_ != painter()->font());

  if (changeFlags_ & (Transform | Clipping)) {
    bool resetTransform = false;

    if (changeFlags_ & Clipping) {
      finishPath();

      js_ << "ctx.restore();ctx.restore();ctx.save();";

      const WTransform& t = painter()->clipPathTransform();

      renderTransform(js_, t);
      if (painter()->hasClipping()) {
	pathTranslation_.setX(0);
	pathTranslation_.setY(0);

	drawPlainPath(js_, painter()->clipPath());
	js_ << "ctx.clip();";
	busyWithPath_ = false;
      }
      renderTransform(js_, t, true);

      js_ << "ctx.save();";

      resetTransform = true;
    } else if (changeFlags_ & Transform) {
      WTransform f = painter()->combinedTransform();

      resetTransform = currentTransform_ != f;

      if (busyWithPath_) {
	if (!painter()->brush().gradient().isEmpty() ||
	    !painter()->pen().gradient().isEmpty()) {
	  resetTransform = true;
	} else if (fequal(f.m11(), currentTransform_.m11())
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

	  resetTransform = false;
	}
      } else if (!resetTransform) {
	if (!fequal(pathTranslation_.x(), 0)
	    || !fequal(pathTranslation_.y(), 0))
	  resetTransform = true;
      }

      if (resetTransform) {
	finishPath();
	js_ << "ctx.restore();ctx.save();";
      }
    }

    if (resetTransform) {
      currentTransform_ = painter()->combinedTransform();
      renderTransform(js_, currentTransform_);
      pathTranslation_.setX(0);
      pathTranslation_.setY(0);
      penChanged = true;
      penColorChanged = true;
      brushChanged = true;
      shadowChanged = true;
      fontChanged = true;
      currentTextHAlign_ = currentTextVAlign_ = AlignSuper;
      init();
    }
  }

  bool newNoPen = painter()->pen().style() == NoPen;
  bool newNoBrush = painter()->brush().style() == NoBrush;

  if (penChanged || brushChanged || shadowChanged ||
      newNoBrush != currentNoBrush_ ||
      newNoPen != currentNoPen_)
    finishPath();

  currentNoPen_ = painter()->pen().style() == NoPen;
  currentNoBrush_ = painter()->brush().style() == NoBrush;

  if (penChanged) {
    if (penColorChanged) {
      // prevent infinite recursion by applying new color to old pen
      currentPen_.setColor(painter()->pen().color());
      currentPen_.setGradient(painter()->pen().gradient());
      if (!painter()->pen().gradient().isEmpty()) {
	std::string gradientName = defineGradient(painter()->pen().gradient(),
						  js_);
	js_ << "ctx.strokeStyle=" << gradientName << ";";
	renderStateChanges(true);
      } else {
	js_ << "ctx.strokeStyle="
	    << WWebWidget::jsStringLiteral
	  (painter()->pen().color().cssText(true))
	    << ";";
      }
    }

    switch (painter()->pen().style()) {
    case SolidLine:
      js_ << "ctx.setLineDash([]);";
      break;
    case DashLine:
      js_ << "ctx.setLineDash([4,2]);";
      break;
    case DotLine:
      js_ << "ctx.setLineDash([1,2]);";
      break;
    case DashDotLine:
      js_ << "ctx.setLineDash([4,2,1,2]);";
      break;
    case DashDotDotLine:
      js_ << "ctx.setLineDash([4,2,1,2,1,2]);";
      break;
    case NoPen:
      break;
    }

    js_ << "ctx.lineWidth="
	<< painter()->normalizedPenWidth(painter()->pen().width(), true).value()
	<< ';';

    if (currentPen_.capStyle() != painter()->pen().capStyle())
      switch (painter()->pen().capStyle()) {
      case FlatCap:
	js_ << "ctx.lineCap='butt';";
	break;
      case SquareCap:
	js_ << "ctx.lineCap='square';";
	break;
      case RoundCap:
	js_ << "ctx.lineCap='round';";
      }

    if (currentPen_.joinStyle() != painter()->pen().joinStyle())
      switch (painter()->pen().joinStyle()) {
      case MiterJoin:
	js_ << "ctx.lineJoin='miter';";
	break;
      case BevelJoin:
	js_ << "ctx.lineJoin='bevel';";
	break;
      case RoundJoin:
	js_ << "ctx.lineJoin='round';";
      }

    currentPen_ = painter()->pen();
  }

  if (brushChanged) {
    currentBrush_ = painter_->brush();
    if (!currentBrush_.gradient().isEmpty()) {
      std::string gradientName = defineGradient(currentBrush_.gradient(), js_);
      js_ << "ctx.fillStyle=" << gradientName << ";";
      renderStateChanges(true);
    } else {
      js_ << "ctx.fillStyle=" 
	  << WWebWidget::jsStringLiteral(currentBrush_.color().cssText(true))
	  << ";";
    }
  }

  if (shadowChanged) {
    currentShadow_ = painter_->shadow();

    js_ << "ctx.shadowOffsetX=" << currentShadow_.offsetX() << ';'
	<< "ctx.shadowOffsetY=" << currentShadow_.offsetY() << ';'
	<< "ctx.shadowBlur=" << currentShadow_.blur() << ';'
	<< "ctx.shadowColor=" 
	<< WWebWidget::jsStringLiteral(currentShadow_.color().cssText(true))
	<< ";";
  }

  if (fontChanged) {
    currentFont_ = painter_->font();

    switch (textMethod_) {
    case Html5Text: 
      js_ << "ctx.font="
	  << WWebWidget::jsStringLiteral(painter()->font().cssText()) << ";";
      break;
    case MozText:
      js_ << "ctx.mozTextStyle = "
	  << WWebWidget::jsStringLiteral(painter()->font().cssText()) << ";";
      break;
    case DomText:
      break;
    }
  }

  changeFlags_ = 0;
}

}
