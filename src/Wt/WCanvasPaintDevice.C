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
    currentClippingEnabled_(false),
    fontMetrics_(0)
{ 
  textMethod_ = Html5Text;

  WApplication *app = WApplication::instance();

  if (app) {
    if (app->environment().agentIsChrome()) {
      if (app->environment().agent() <= WEnvironment::Chrome2)
	textMethod_ = DomText;
    } else if (app->environment().agentIsGecko()) {
      if (app->environment().agent() < WEnvironment::Firefox3_0)
	textMethod_ = DomText;
      else if (app->environment().agent() < WEnvironment::Firefox3_5)
	textMethod_ = MozText;
    } else if (app->environment().agentIsSafari()) {
      if (app->environment().agent() == WEnvironment::Safari3)
	textMethod_ = DomText;
    }
  }
}

WCanvasPaintDevice::~WCanvasPaintDevice()
{
  delete fontMetrics_;
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

  recordedJs_.str("");
  std::stringstream &tmp = recordedJs_;

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

  lastTransformWasIdentity_ = true;

  tmp << "ctx.save();" << js_.str()
      << "ctx.restore();}";

  if (!images_.empty()) {
    tmp << ");";
  }

  tmp << "}";

  for (unsigned i = 0; i < textElements_.size(); ++i)
    text->addChild(textElements_[i]);
}

void WCanvasPaintDevice::renderPaintCommands(std::stringstream& js_target,
					     const std::string& canvasElement)
{
  js_target << "var ctx=" << canvasElement << ".getContext('2d');";
  js_target << "if (!ctx.setLineDash) {ctx.setLineDash = function(a){};}";
  js_target << "ctx.save();" << js_.str()
	    << "ctx.restore();";
}

void WCanvasPaintDevice::init()
{
  lastTransformWasIdentity_ = true;
  currentBrush_ = WBrush();
  currentNoBrush_ = false;
  currentPen_ = WPen();
  currentNoPen_ = false;
  currentPen_.setCapStyle(FlatCap);
  currentShadow_ = WShadow();
  currentFont_ = WFont();

  changeFlags_ = Transform | Pen | Brush | Shadow | Font;
}

void WCanvasPaintDevice::done()
{ }

void WCanvasPaintDevice::drawArc(const WRectF& rect, double startAngle,
				 double spanAngle)
{
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

  out << "ctx.beginPath();";

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
      out << ',' << Utils::round_js_str(std::max(0.0, s.x()), 3, buf);
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
      const double cpx = s.x();
      const double cpy = s.y();
      out << "ctx.quadraticCurveTo("
          << Utils::round_js_str(cpx + pathTranslation_.x(), 3, buf) << ',';
      out << Utils::round_js_str(cpy + pathTranslation_.y(), 3, buf);

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
  if (!currentNoBrush_)
    js_ << "ctx.fill();";

  if (!currentNoPen_)
    js_ << "ctx.stroke();";

  js_ << '\n';
}

void WCanvasPaintDevice::drawPath(const WPainterPath& path)
{
  if (path.isJavaScriptBound()) {
    renderStateChanges(true);
    js_ << WT_CLASS ".gfxUtils.drawPath(ctx," << path.jsRef() << ","
      << (currentNoBrush_ ? "false" : "true") << ","
      << (currentNoPen_ ? "false" : "true") << ");";
  } else {
    renderStateChanges(false);
    drawPlainPath(js_, path);
    finishPath();
  }
}

void WCanvasPaintDevice::drawStencilAlongPath(const WPainterPath &stencil,
					      const WPainterPath &path,
					      bool softClipping)
{
  renderStateChanges(true);
  js_ << WT_CLASS << ".gfxUtils.drawStencilAlongPath(ctx,"
      << stencil.jsRef() << "," << path.jsRef() << ","
      << (currentNoBrush_ ? "false" : "true") << ","
      << (currentNoPen_ ? "false" : "true") << ","
      << (softClipping ? "true" : "false") << ");";
}

void WCanvasPaintDevice::drawRect(const WRectF& rectangle)
{
  renderStateChanges(true);
  js_ << WT_CLASS << ".gfxUtils.drawRect(ctx," << rectangle.jsRef() << ","
    << (currentNoBrush_ ? "false" : "true") << ","
    << (currentNoPen_ ? "false" : "true") << ");";
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
				  const WString& text,
				  const WPointF *clipPoint)
{
  if (textFlag == TextWordWrap)
    throw WException("WCanvasPaintDevice::drawText() "
		     "TextWordWrap is not supported");

  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  if (textMethod_ != DomText) {
    renderStateChanges(true);
  }

  switch (textMethod_) {
  case Html5Text: 
    {
      js_ << WT_CLASS ".gfxUtils.drawText(ctx,"
	  << rect.jsRef() << ',' << flags.value() << ','
	  << text.jsStringLiteral();
      if (clipPoint && painter()) {
	js_ << ',' << painter()->worldTransform().map(*clipPoint).jsRef();
      }
      js_ << ");";
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
      if (currentPen_.isJavaScriptBound()) {
	js_ << "ctx.fillStyle=" WT_CLASS ".gfxUtils.css_text(" << currentPen_.jsRef() << ".color);";
      } else if (currentBrush_.color() != currentPen_.color() || currentBrush_.isJavaScriptBound())
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

void WCanvasPaintDevice::drawTextOnPath(const WRectF &rect,
					WFlags<AlignmentFlag> alignmentFlags,
					const std::vector<WString> &text,
					const WTransform &transform,
					const WPainterPath &path,
					double angle, double lineHeight,
					bool softClipping)
{
  renderStateChanges(true);
  js_ << WT_CLASS ".gfxUtils.drawTextOnPath(ctx,[";
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (i != 0)
      js_ << ',';
    js_ << text[i].jsStringLiteral();
  }
  js_ << "],";
  js_ << rect.jsRef() << ',';
  js_ << transform.jsRef() << ',';
  js_ << path.jsRef() << ',';
  char buf[30];
  js_ << Utils::round_js_str(angle, 3, buf) << ',';
  js_ << Utils::round_js_str(lineHeight, 3, buf) << ',';
  js_ << alignmentFlags.value() << ',';
  js_ << (softClipping ? "true" : "false") << ");";
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
					 const WTransform& t)
{
  if (t.isJavaScriptBound()) {
    s << "ctx.setTransform.apply(ctx, " << t.jsRef() << ");";
  } else if (!(t.isIdentity() && lastTransformWasIdentity_)) {
    char buf[30];
    s << "ctx.setTransform(" << Utils::round_js_str(t.m11(), 3, buf) << ",";
    s			  << Utils::round_js_str(t.m12(), 3, buf) << ",";
    s			  << Utils::round_js_str(t.m21(), 3, buf) << ",";
    s			  << Utils::round_js_str(t.m22(), 3, buf) << ",";
    s			  << Utils::round_js_str(t.m31(), 3, buf) << ",";
    s                     << Utils::round_js_str(t.m32(), 3, buf) << ");";
  }
  lastTransformWasIdentity_ = t.isIdentity();
}

void WCanvasPaintDevice::renderStateChanges(bool resetPathTranslation)
{
  if (resetPathTranslation) {
    if (!fequal(pathTranslation_.x(), 0) ||
	!fequal(pathTranslation_.y(), 0))
      changeFlags_ |= Transform;
  }

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
    (painter()->pen().isJavaScriptBound() ||
     currentPen_.color() != painter()->pen().color() ||
     currentPen_.gradient() != painter()->pen().gradient());

  bool shadowChanged =
    (changeFlags_ & Shadow)
    && (currentShadow_ != painter()->shadow());

  bool fontChanged = (changeFlags_ & Font)
    && (currentFont_ != painter()->font());

  bool clippingChanged = (changeFlags_ & Clipping)
    && (currentClippingEnabled_ != painter()->hasClipping() ||
	 currentClipPath_ != painter()->clipPath() ||
	 currentClipTransform_ != painter()->clipPathTransform());

  changeFlags_.clear(Clipping);

  if (changeFlags_ & Transform || clippingChanged) {
    bool resetTransform = false;

    if (clippingChanged) {
      js_ << "ctx.restore();ctx.save();";

      lastTransformWasIdentity_ = true;
      pathTranslation_.setX(0);
      pathTranslation_.setY(0);

      const WTransform& t = painter()->clipPathTransform();
      const WPainterPath &p = painter()->clipPath();
      if (!p.isEmpty()) {
	js_ << WT_CLASS << ".gfxUtils.setClipPath(ctx," << p.jsRef() << "," << t.jsRef() << ","
	    << (painter()->hasClipping() ? "true" : "false") << ");";
      } else {
	js_ << WT_CLASS << ".gfxUtils.removeClipPath(ctx);";
      }
      currentClipTransform_ = t;
      currentClipPath_ = p;

      penChanged = true;
      penColorChanged = true;
      brushChanged = true;
      shadowChanged = true;
      fontChanged = true;
      init();
      resetTransform = true;

      currentClippingEnabled_ = painter()->hasClipping();
    } else if (changeFlags_ & Transform) {
      WTransform f = painter()->combinedTransform();

      resetTransform = currentTransform_ != f ||
	  ((!fequal(pathTranslation_.x(), 0) ||
	    !fequal(pathTranslation_.y(), 0)) && resetPathTranslation);

      if (!painter()->brush().gradient().isEmpty() ||
	  !painter()->pen().gradient().isEmpty()) {
	resetTransform = true;
      } else if (!resetPathTranslation
		 && !currentTransform_.isJavaScriptBound()
		 && !f.isJavaScriptBound()
		 && fequal(f.m11(), currentTransform_.m11())
		 && fequal(f.m12(), currentTransform_.m12())
		 && fequal(f.m21(), currentTransform_.m21())
		 && fequal(f.m22(), currentTransform_.m22())) {
	/*
	 * Invert scale/rotate to compute the delta needed
	 * before applying these transformations to get the
	 * same as the global translation.
	 *
	 * (This is an optimization that prevents the rendering from
	 * involving many ctx.transform calls, when only moves
	 * are performed.)
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
    }

    if (resetTransform) {
      currentTransform_ = painter()->combinedTransform();
      renderTransform(js_, currentTransform_);
      pathTranslation_.setX(0);
      pathTranslation_.setY(0);
    }
  }

  currentNoPen_ = painter()->pen().style() == NoPen;
  currentNoBrush_ = painter()->brush().style() == NoBrush;

  if (penChanged) {
    if (penColorChanged) {
      // prevent infinite recursion by applying new color to a temporary pen
      PenCapStyle capStyle = currentPen_.capStyle();
      PenJoinStyle joinStyle = currentPen_.joinStyle();
      WPen tmpPen;
      tmpPen.setCapStyle(capStyle);
      tmpPen.setJoinStyle(joinStyle);
      tmpPen.setColor(painter()->pen().color());
      tmpPen.setGradient(painter()->pen().gradient());
      currentPen_ = tmpPen;
      if (!painter()->pen().gradient().isEmpty()) {
	std::string gradientName = defineGradient(painter()->pen().gradient(),
						  js_);
	js_ << "ctx.strokeStyle=" << gradientName << ";";
	renderStateChanges(true);
      } else {
	if (painter()->pen().isJavaScriptBound()) {
	  js_ << "ctx.strokeStyle=" WT_CLASS ".gfxUtils.css_text(" << painter()->pen().jsRef() << ".color);";
	} else {
	  js_ << "ctx.strokeStyle="
	      << WWebWidget::jsStringLiteral
	    (painter()->pen().color().cssText(true))
	      << ";";
	}
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
      if (currentBrush_.isJavaScriptBound()) {
	js_ << "ctx.fillStyle=" WT_CLASS ".gfxUtils.css_text(" << currentBrush_.jsRef() << ".color);";
      } else {
	js_ << "ctx.fillStyle=" 
	    << WWebWidget::jsStringLiteral(currentBrush_.color().cssText(true))
	    << ";";
      }
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
