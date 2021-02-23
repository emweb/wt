/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WCanvasPaintDevice.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WFontMetrics.h"
#include "Wt/WPainter.h"
#include "Wt/WPainterPath.h"
#include "Wt/WRectF.h"
#include "Wt/WStringStream.h"
#include "Wt/WWebWidget.h"

#include "DomElement.h"
#include "WebUtils.h"
#include "ServerSideFontMetrics.h"

#include <cmath>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
  static const double EPSILON = 1E-5;
}

namespace Wt {

namespace {
  double adjust360(double d) {
    if (d > 360.0)
      return 360.0;
    else if (d < -360.0)
      return -360.0;
    else
      return d;
  }

  double adjustPositive360(double d) {
    const double result = std::fmod(d, 360.0);
    if (result < 0)
      return result + 360.0;
    else
      return result;
  }

  bool fequal(double d1, double d2) {
    return std::fabs(d1 - d2) < 1E-5;
  }

  std::string defineGradient(const Wt::WGradient& gradient,
			     std::stringstream& js) {
    std::string jsRef = "grad";
    if (gradient.style() == Wt::GradientStyle::Linear) {
      const WLineF& gradVec = gradient.linearGradientVector();
      js << "var " << jsRef << " = ctx.createLinearGradient("
	  << gradVec.x1() << ", " << gradVec.y1() << ", "
	  << gradVec.x2() << ", " << gradVec.y2()
	  << ");";
    } else if (gradient.style() == Wt::GradientStyle::Radial) {
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
				       bool paintUpdate)
  : width_(width),
    height_(height),
    painter_(nullptr),
    paintUpdate_(paintUpdate),
    currentClippingEnabled_(false),
    fontMetrics_(nullptr)
{
  textMethod_ = TextMethod::Html5Text;

  WApplication *app = WApplication::instance();

  if (app) {
    if (app->environment().agentIsChrome()) {
      if (static_cast<unsigned int>(app->environment().agent()) <= 
	  static_cast<unsigned int>(UserAgent::Chrome2))
	textMethod_ = TextMethod::DomText;
    } else if (app->environment().agentIsGecko()) {
      if (static_cast<unsigned int>(app->environment().agent()) <
	  static_cast<unsigned int>(UserAgent::Firefox3_0))
	textMethod_ = TextMethod::DomText;
      else if (static_cast<unsigned int>(app->environment().agent()) 
	       < static_cast<unsigned int>(UserAgent::Firefox3_5))
	textMethod_ = TextMethod::MozText;
    } else if (app->environment().agentIsSafari()) {
      if (app->environment().agent() == UserAgent::Safari3)
	textMethod_ = TextMethod::DomText;
    }
  }
}

WCanvasPaintDevice::~WCanvasPaintDevice()
{
  delete fontMetrics_;
}

WFlags<PaintDeviceFeatureFlag> WCanvasPaintDevice::features() const
{
  if (ServerSideFontMetrics::available())
    return PaintDeviceFeatureFlag::FontMetrics;
  else
    return None;
}

void WCanvasPaintDevice::render(const std::string& paintedWidgetJsRef,
                                const std::string& canvasId,
                                DomElement *text,
                                const std::string& updateAreasJs)
{
  std::string canvasVar = WT_CLASS ".getElement('" + canvasId + "')";
  std::string paintedWidgetObjRef = paintedWidgetJsRef + ".wtObj";

  WStringStream tmp;

  tmp << ";" // Extra ; to make sure that JavaScript interprets the next thing as
             // a separate statement (for when ; was forgotten in another doJavaScript call)
         "(function(){";
  tmp << "var pF=function(){";

  tmp <<
    "if(" << canvasVar << ".getContext){";

  if (!images_.empty()) {
    tmp << "var images=" << paintedWidgetObjRef << ".images;";
  }
  tmp << "var ctx=" << canvasVar << ".getContext('2d');";
  // Older browsers don't have setLineDash
  tmp << "if (!ctx.setLineDash) {ctx.setLineDash = function(a){};}";

  if (!paintUpdate_) {
    tmp << "ctx.clearRect(0,0,"
	<< width().value() << "," << height().value() << ");";
  }

  lastTransformWasIdentity_ = true;

  tmp << "ctx.save();" << js_.str()
      << "ctx.restore();";

  tmp << "}";

  tmp << updateAreasJs;

  tmp << "};";

  if (!paintUpdate_) {
    tmp << paintedWidgetObjRef << ".repaint=pF;";
    tmp << "pF=function(){"
        << paintedWidgetObjRef << ".repaint();"
        << "};";
  }

  tmp << "var o=" << paintedWidgetObjRef << ";";
  if (!paintUpdate_)
    tmp << "o.cancelPreloaders();";
  tmp << "if(" << canvasVar << ".getContext){";
  tmp << "var l=new ";
  tmp << wApp->javaScriptClass() << "._p_.ImagePreloader([";

  for (unsigned i = 0; i < images_.size(); ++i) {
    if (i != 0)
      tmp << ',';
    tmp << '\'' << images_[i] << '\'';
  }

  tmp << "],function(images){"
           "if (!" << paintedWidgetJsRef << ")"
             "return;"
           "this.done = true;"
           "var o=" << paintedWidgetObjRef << ";"
           "if(o.imagePreloaders.length===0||"
             "this===o.imagePreloaders[0]){"
             "o.images=images;"
             "pF();"
             "o.imagePreloaders.shift();"
           "}else{"
             "while(o.imagePreloaders.length>0&&"
                   "o.imagePreloaders[0].done){"
               "o.imagePreloaders[0].callback(o.imagePreloaders[0].images);"
             "}"
           "}"
         "});"
	 "if(!l.done)"
	   "o.imagePreloaders.push(l);"
       "}"
       "})();";

  text->callJavaScript(tmp.str());

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
  currentPen_.setCapStyle(PenCapStyle::Flat);
  currentShadow_ = WShadow();
  currentFont_ = WFont();

  changeFlags_ = PainterChangeFlag::Transform | 
    PainterChangeFlag::Pen | 
    PainterChangeFlag::Brush | 
    PainterChangeFlag::Shadow | 
    PainterChangeFlag::Font;
}

void WCanvasPaintDevice::done()
{ }

void WCanvasPaintDevice::drawArc(const WRectF& rect, double startAngle,
				 double spanAngle)
{
  if (rect.width() < EPSILON || rect.height() < EPSILON)
    return;

  renderStateChanges(true);

  const double rStartAngle = WTransform::degreesToRadians(adjustPositive360(-startAngle));
  double rEndAngle;
  if (spanAngle >= 360.0 || spanAngle <= -360.0) {
    rEndAngle = rStartAngle - 2.0 * M_PI * (spanAngle > 0 ? 1.0 : -1.0);
  } else {
    rEndAngle = WTransform::degreesToRadians(adjustPositive360(-startAngle - adjust360(spanAngle)));
  }
  const bool anticlockwise = spanAngle > 0;

  double sx, sy, r, lw;
  if (rect.width() > rect.height()) {
    sx = 1;
    sy = std::max(0.005, rect.height() / rect.width());
    r = rect.width()/2;
  } else if (rect.width() < rect.height()) {
    sx = std::max(0.005, rect.width() / rect.height());
    sy = 1;
    r = rect.height()/2;
  } else {
    sx = 1;
    sy = 1;
    r = rect.width() / 2;
  }

  const WPen& pen = painter()->pen();
  if (pen.style() != PenStyle::None)
    lw = painter()->normalizedPenWidth(pen.width(), true).value()
      * 1 / std::min(sx, sy);
  else
    lw = 0;

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
  js_ << ',' << Utils::round_js_str(rStartAngle, 6, buf);
  js_ << ',' << Utils::round_js_str(rEndAngle, 6, buf) << ',';
  js_ << (anticlockwise ? "true" : "false") << ");";

  // restore comes before fill and stroke, otherwise the gradient will use 
  // this temporary coordinate system
  js_ << "ctx.restore();";

  if (painter_->brush().style() != BrushStyle::None) {
    js_ << "ctx.fill();";
  }

  if (painter_->pen().style() != PenStyle::None) {
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

  js_ << WT_CLASS ".gfxUtils.drawImage("
         "ctx,"
         "images[" << imageIndex << "],"
      << Wt::WWebWidget::jsStringLiteral(imgUri) << ','
      << sourceRect.jsRef() << ','
      << rect.jsRef() << ");";
}

void WCanvasPaintDevice::drawPlainPath(std::stringstream& out,
				       const WPainterPath& path)
{
  char buf[30];

  out << "ctx.beginPath();";

  const std::vector<WPainterPath::Segment>& segments = path.segments();

  if (segments.size() > 0
      && segments[0].type() != MoveTo)
    out << "ctx.moveTo(0,0);";

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    switch (s.type()) {
    case MoveTo:
      out << "ctx.moveTo(" << Utils::round_js_str(s.x() + pathTranslation_.x(),
						  3, buf);
      out << ',' << Utils::round_js_str(s.y() + pathTranslation_.y(),
				     3, buf) << ");";
      break;
    case LineTo:
      out << "ctx.lineTo(" << Utils::round_js_str(s.x() + pathTranslation_.x(),
						  3, buf);
      out << ',' << Utils::round_js_str(s.y() + pathTranslation_.y(),
					3, buf) << ");";
      break;
    case CubicC1:
      out << "ctx.bezierCurveTo("
	  << Utils::round_js_str(s.x() + pathTranslation_.x(), 3, buf);
      out << ',' << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf);
      break;
    case CubicC2:
      out << ',' << Utils::round_js_str(s.x() + pathTranslation_.x(), 3, buf)
	  << ',';
      out << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf);
      break;
    case CubicEnd:
      out << ',' << Utils::round_js_str(s.x() + pathTranslation_.x(), 3, buf)
	  << ',';
      out << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf) << ");";
      break;
    case ArcC:
      out << "ctx.arc(" << Utils::round_js_str(s.x() + pathTranslation_.x(), 3,
					       buf) << ',';
      out << Utils::round_js_str(s.y() + pathTranslation_.y(), 3, buf);
      break;
    case ArcR:
      out << ',' << Utils::round_js_str(std::max(0.0, s.x()), 3, buf);
      break;
    case ArcAngleSweep:
      {
        const double startAngle = s.x();
        const double spanAngle = s.y();
        const double rStartAngle = WTransform::degreesToRadians(adjustPositive360(-startAngle));
        double rEndAngle;
        if (spanAngle >= 360.0 || spanAngle <= -360.0) {
          rEndAngle = rStartAngle - 2.0 * M_PI * (spanAngle > 0 ? 1.0 : -1.0);
        } else {
          rEndAngle = WTransform::degreesToRadians(adjustPositive360(-startAngle - adjust360(spanAngle)));
        }
        const bool anticlockwise = spanAngle > 0;

        out << ',' << Utils::round_js_str(rStartAngle, 6, buf);
        out << ',' << Utils::round_js_str(rEndAngle, 6, buf);
        out << ',' << (anticlockwise ? "true" : "false") << ");";
      }
      break;
    case QuadC: {
      const double cpx = s.x();
      const double cpy = s.y();
      out << "ctx.quadraticCurveTo("
          << Utils::round_js_str(cpx + pathTranslation_.x(), 3, buf) << ',';
      out << Utils::round_js_str(cpy + pathTranslation_.y(), 3, buf);

      break;
    }
    case QuadEnd:
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
  if (rectangle.isJavaScriptBound()) {
    renderStateChanges(true);
    js_ << WT_CLASS << ".gfxUtils.drawRect(ctx," << rectangle.jsRef() << ","
	<< (currentNoBrush_ ? "false" : "true") << ","
	<< (currentNoPen_ ? "false" : "true") << ");";
  } else {
    drawPath(rectangle.toPath());
  }
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
  if (textFlag == TextFlag::WordWrap)
    throw WException("WCanvasPaintDevice::drawText() "
		     "WordWrap is not supported");

  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  if (textMethod_ != TextMethod::DomText) {
    renderStateChanges(true);
  }

  switch (textMethod_) {
  case TextMethod::Html5Text: 
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
  case TextMethod::MozText:
    {
      std::string x;

      switch (horizontalAlign) {
      case AlignmentFlag::Left:
	x = std::to_string(rect.left());
	break;
      case AlignmentFlag::Right:
	x = std::to_string(rect.right())
	  + " - ctx.mozMeasureText(" + text.jsStringLiteral() + ")";
	break;
      case AlignmentFlag::Center:
	x = std::to_string(rect.center().x())
	  + " - ctx.mozMeasureText(" + text.jsStringLiteral() + ")/2";
	break;
      default:
	break;
      }

      double fontSize;
      switch (painter()->font().size()) {
      case FontSize::FixedSize:
	fontSize = painter()->font().sizeLength().toPixels();
	break;
      default:
	fontSize = 16;
      }

      double y = 0;
      switch (verticalAlign) {
      case AlignmentFlag::Top:
	y = rect.top() + fontSize * 0.75; break;
      case AlignmentFlag::Middle:
	y = rect.center().y() + fontSize * 0.25; break;
      case AlignmentFlag::Bottom:
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
  case TextMethod::DomText:
    {
      WPointF pos = painter()->combinedTransform().map(rect.topLeft());

      DomElement *e = DomElement::createNew(DomElementType::DIV);
      e->setProperty(Property::StylePosition, "absolute");
      e->setProperty(Property::StyleTop,
		     std::to_string(pos.y()) + "px");
      e->setProperty(Property::StyleLeft,
		     std::to_string(pos.x()) + "px");
      e->setProperty(Property::StyleWidth,
		     std::to_string(rect.width()) + "px");
      e->setProperty(Property::StyleHeight,
		     std::to_string(rect.height()) + "px");

      DomElement *t = e;

      /*
       * HTML tricks to center things vertically -- does not work on IE,
       * (neither does canvas)
       */
      if (verticalAlign != AlignmentFlag::Top) {
	t = DomElement::createNew(DomElementType::DIV);

	if (verticalAlign == AlignmentFlag::Middle) {
	  e->setProperty(Property::StyleDisplay, "table");
	  t->setProperty(Property::StyleDisplay, "table-cell");
	  t->setProperty(Property::StyleVerticalAlign, "middle");
	} else if (verticalAlign == AlignmentFlag::Bottom) {
	  t->setProperty(Property::StylePosition, "absolute");
	  t->setProperty(Property::StyleWidth, "100%");
	  t->setProperty(Property::StyleBottom, "0px");
	}
      }

      t->setProperty(Property::InnerHTML,
		     WWebWidget::escapeText(text, true).toUTF8());

      WFont f = painter()->font();
      f.updateDomElement(*t, false, true);

      t->setProperty(Property::StyleColor, painter()->pen().color().cssText());

      if (horizontalAlign == AlignmentFlag::Right)
	t->setProperty(Property::StyleTextAlign, "right");
      else if (horizontalAlign == AlignmentFlag::Center)
	t->setProperty(Property::StyleTextAlign, "center");
      else
	t->setProperty(Property::StyleTextAlign, "left");

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

void WCanvasPaintDevice::setChanged(WFlags<PainterChangeFlag> flags)
{
  changeFlags_ |= flags;
}

void WCanvasPaintDevice::renderTransform(std::stringstream& s,
					 const WTransform& t)
{
  if (!(t.isIdentity() && lastTransformWasIdentity_)) {
    s << "ctx.wtTransform=" << t.jsRef() << ';';
    s << "ctx.setTransform.apply(ctx, ctx.wtTransform);";
  }
  lastTransformWasIdentity_ = t.isIdentity();
}

void WCanvasPaintDevice::renderStateChanges(bool resetPathTranslation)
{
  if (resetPathTranslation) {
    if (!fequal(pathTranslation_.x(), 0) ||
	!fequal(pathTranslation_.y(), 0))
      changeFlags_ |= PainterChangeFlag::Transform;
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
    changeFlags_.test(PainterChangeFlag::Brush) &&
    (currentBrush_ != painter()->brush()) &&
    (slowFirefox || painter()->brush().style() != BrushStyle::None);

  bool penChanged =
    changeFlags_.test(PainterChangeFlag::Pen) &&
    (currentPen_ != painter()->pen()) &&
    (slowFirefox || painter()->pen().style() != PenStyle::None);

  bool penColorChanged = 
    penChanged && 
    (painter()->pen().isJavaScriptBound() ||
     currentPen_.color() != painter()->pen().color() ||
     currentPen_.gradient() != painter()->pen().gradient());

  bool shadowChanged =
    changeFlags_.test(PainterChangeFlag::Shadow)
    && (currentShadow_ != painter()->shadow());

  bool fontChanged = changeFlags_.test(PainterChangeFlag::Font)
    && (currentFont_ != painter()->font());

  bool clippingChanged = changeFlags_.test(PainterChangeFlag::Clipping)
    && (currentClippingEnabled_ != painter()->hasClipping() ||
	 currentClipPath_ != painter()->clipPath() ||
	 currentClipTransform_ != painter()->clipPathTransform());

  changeFlags_.clear(PainterChangeFlag::Clipping);

  if (changeFlags_.test(PainterChangeFlag::Transform) || clippingChanged) {
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
    } else if (changeFlags_.test(PainterChangeFlag::Transform)) {
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

         changeFlags_ = None;

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

  currentNoPen_ = painter()->pen().style() == PenStyle::None;
  currentNoBrush_ = painter()->brush().style() == BrushStyle::None;

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

    char buf[30];
    double lw = painter()->normalizedPenWidth(painter()->pen().width(), true).value();

    switch (painter()->pen().style()) {
    case PenStyle::SolidLine:
      js_ << "ctx.setLineDash([]);";
      break;
    case PenStyle::DashLine:
      js_ << "ctx.setLineDash([";
      js_ << Utils::round_js_str(lw * 4.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 2.0, 3, buf);
      js_ << "]);";
      break;
    case PenStyle::DotLine:
      js_ << "ctx.setLineDash([";
      js_ << Utils::round_js_str(lw * 1.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 2.0, 3, buf);
      js_ << "]);";
      break;
    case PenStyle::DashDotLine:
      js_ << "ctx.setLineDash([";
      js_ << Utils::round_js_str(lw * 4.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 2.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 1.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 2.0, 3, buf);
      js_ << "]);";
      break;
    case PenStyle::DashDotDotLine:
      js_ << "ctx.setLineDash([";
      js_ << Utils::round_js_str(lw * 4.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 2.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 1.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 2.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 1.0, 3, buf) << ',';
      js_ << Utils::round_js_str(lw * 2.0, 3, buf);
      js_ << "]);";
      break;
    case PenStyle::None:
      break;
    }

    js_ << "ctx.lineWidth="
        << Utils::round_js_str(lw, 3, buf)
	<< ';';

    if (currentPen_.capStyle() != painter()->pen().capStyle())
      switch (painter()->pen().capStyle()) {
      case PenCapStyle::Flat:
	js_ << "ctx.lineCap='butt';";
	break;
      case PenCapStyle::Square:
	js_ << "ctx.lineCap='square';";
	break;
      case PenCapStyle::Round:
	js_ << "ctx.lineCap='round';";
      }

    if (currentPen_.joinStyle() != painter()->pen().joinStyle())
      switch (painter()->pen().joinStyle()) {
      case PenJoinStyle::Miter:
	js_ << "ctx.lineJoin='miter';";
	break;
      case PenJoinStyle::Bevel:
	js_ << "ctx.lineJoin='bevel';";
	break;
      case PenJoinStyle::Round:
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
	js_ << "ctx.fillStyle=" WT_CLASS ".gfxUtils.css_text(" 
	    << currentBrush_.jsRef() << ".color);";
      } else {
	js_ << "ctx.fillStyle=" 
	    << WWebWidget::jsStringLiteral(currentBrush_.color().cssText(true))
	    << ";";
      }
    }
  }

  if (shadowChanged) {
    currentShadow_ = painter_->shadow();

    double offsetX = currentShadow_.offsetX();
    double offsetY = currentShadow_.offsetY();
    double blur = currentShadow_.blur();

    char buf[30];
    js_ << "ctx.shadowOffsetX=" << Utils::round_js_str(offsetX, 3, buf) << ';';
    js_ << "ctx.shadowOffsetY=" << Utils::round_js_str(offsetY, 3, buf) << ';';
    js_ << "ctx.shadowBlur=" << Utils::round_js_str(blur, 3, buf) << ';'
	<< "ctx.shadowColor=" 
	<< WWebWidget::jsStringLiteral(currentShadow_.color().cssText(true))
	<< ";";
  }

  if (fontChanged) {
    currentFont_ = painter_->font();

    switch (textMethod_) {
    case TextMethod::Html5Text: 
      js_ << "ctx.font="
	  << WWebWidget::jsStringLiteral(painter()->font().cssText()) << ";";
      break;
    case TextMethod::MozText:
      js_ << "ctx.mozTextStyle = "
	  << WWebWidget::jsStringLiteral(painter()->font().cssText()) << ";";
      break;
    case TextMethod::DomText:
      break;
    }
  }

  changeFlags_ = None;
}

}
