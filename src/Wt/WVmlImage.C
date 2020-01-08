/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/*
 * The rendering of VML on high DPI displays on IE6 and IE7 is not fully
 * supported. That is because IE scales entire websites up on such
 * displays, but somebody forgot about VML. For more information about
 * this scaling, google for strings such as UseHR (the registry key
 * to enable/disable this scaling), screen.deviceXDPI, screen.logicalXDPI.
 *
 * At the time, there is only a handful computers who have this setting
 * enabled, and the rest of the internet does not appear to care about
 * them too much (e.g. google maps renders wrong on such displays). We do
 * our best, but some artefacts cannot be compensated for.
 *
 * Known limitations:
 * - mirrored text will be drawn as upside-down text
 * - rotated text will not show up on the exact correct position. The longer
 *   the string, the wronger the position
 *
 * We blame this on a problematic amount of bugs in the VML rendering of IE.
 *
 */

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WFontMetrics.h"
#include "Wt/WLogger.h"
#include "Wt/WPainter.h"
#include "Wt/WPainterPath.h"
#include "Wt/WRectF.h"
#include "Wt/WStringStream.h"
#include "Wt/WVmlImage.h"

#include "EscapeOStream.h"
#include "WebUtils.h"
#include "ServerSideFontMetrics.h"

#include <cmath>

namespace {
  const int Z = 10;

  int myzround(double a, bool doScale = true) {
    Wt::WApplication *app = Wt::WApplication::instance();
    double dpiScale = doScale ? app->environment().dpiScale() : 1.0;
    return static_cast<int>(dpiScale * ( (Z * a) - Z/2 + 0.5 ));
  }  

  bool fequal(double d1, double d2) {
    return std::fabs(d1 - d2) < 1E-5;
  }

  double norm(const Wt::WPointF& p) {
    return std::sqrt(p.x() * p.x() + p.y() * p.y());
  }
}

/*
 * Optimization possibilities:
 * - like SVG, process transform completely to allow continuing an existing
 *   path (not just translation)
 * - use style classes to avoid the excessive repeated style information
 */

namespace Wt {

LOGGER("WVmlImage");

WVmlImage::WVmlImage(const WLength& width, const WLength& height,
		     bool paintUpdate)
  : width_(width),
    height_(height),
    painter_(nullptr),
    paintUpdate_(paintUpdate),
    clippingChanged_(false),
    fontMetrics_(nullptr)
{ }

WVmlImage::~WVmlImage()
{
  delete fontMetrics_;
}

WFlags<PaintDeviceFeatureFlag> WVmlImage::features() const
{
  if (ServerSideFontMetrics::available())
    return PaintDeviceFeatureFlag::FontMetrics;
  else
    return None;
}

void WVmlImage::init()
{ 
  currentBrush_ = painter()->brush();
  currentPen_ = painter()->pen();
  currentShadow_ = painter()->shadow();
  penBrushShadowChanged_ = true;

  startClip(WRectF(0, 0, width().value(), height().value()));
}

void WVmlImage::done()
{
  finishPaths();
  stopClip();
}

void WVmlImage::setChanged(WFlags<PainterChangeFlag> flags)
{
  if (!(flags & (PainterChangeFlag::Pen | 
	       PainterChangeFlag::Brush | 
	       PainterChangeFlag::Shadow)).empty())
    penBrushShadowChanged_ = true;

  if (flags.test(PainterChangeFlag::Clipping))
    clippingChanged_ = true;
}

void WVmlImage::drawArc(const WRectF& rect, double startAngle, double spanAngle)
{
  painter()->save();

  painter()->translate(rect.center().x(), rect.center().y());
  painter()->scale(1., rect.height() / rect.width());

  WPainterPath path;
  path.arcMoveTo(0, 0, rect.width()/2., startAngle);
  path.arcTo(0, 0, rect.width()/2., startAngle, spanAngle);
  painter()->drawPath(path);

  painter()->restore();
}

void WVmlImage::drawImage(const WRectF& rect, const std::string& imageUri,
			  int imgWidth, int imgHeight, const WRectF& sourceRect)
{
  finishPaths();
  processClipping();

  WApplication *app = WApplication::instance();
  std::string imgUri;
  if (app)
    imgUri = app->resolveRelativeUrl(imageUri);

  WTransform t = painter()->combinedTransform();
  WPointF tl = t.map(rect.topLeft());

  rendered_ << "<v:group style=\"width:" << Z * width().value() << "px;height:"
	    << Z * height().value() << "px;";

  double cx = 1, cy = 1;

  if (t.m11() != 1.0 || t.m22() != 1.0 || t.m12() != 0.0 || t.m21() != 0.0) {
    cx = width().value() / rect.width();
    cy = height().value() / rect.height();

    // FIXME: figure out padding ?

    rendered_ << "filter:progid:DXImageTransform.Microsoft.Matrix(M11='"
	      << t.m11() / cx << "',M12='" << t.m21() / cy << "',M21='"
	      << t.m12() / cx << "',M22='" << t.m22() / cy << "',Dx='"
	      << tl.x() << "',Dy='" << tl.y() << "',sizingmethod='clip');";
  } else
    rendered_ << "top:" << Z * tl.y() << "px;left:" << Z * tl.x() << "px;";

  rendered_ << "\"><v:image src=\"" << imgUri
	    << "\" style=\"width:" << Z * rect.width() * cx
	    << "px;height:" << Z * rect.height() * cy
	    << "px\" cropleft=\"" << sourceRect.x() / imgWidth
	    << "\" croptop=\"" << sourceRect.y() / imgHeight
	    << "\" cropright=\"" << (imgWidth - sourceRect.right())/imgWidth
	    << "\" cropbottom=\"" << (imgHeight - sourceRect.bottom())/imgHeight
	    << "\"/></v:group>";
}

void WVmlImage::drawRect(const WRectF& rectangle)
{
  drawPath(rectangle.toPath());
}

void WVmlImage::drawPath(const WPainterPath& path)
{
  if (path.isEmpty())
    return;

  if (penBrushShadowChanged_)
    if ((currentPen_ != painter()->pen())
	|| (currentBrush_ != painter()->brush())
	|| (currentShadow_ != painter()->shadow()))
      finishPaths();

  if (clippingChanged_) {
    if (!activePaths_.empty())
      finishPaths();
    processClipping();
  }

  WTransform transform = painter()->combinedTransform();
  WRectF bbox = transform.map(path.controlPointRect());

  int thisPath = -1;
  if (!activePaths_.empty())
    for (unsigned i = 0; i < activePaths_.size(); ++i) {
      if (!activePaths_[i].bbox.intersects(bbox)) {
	thisPath = i;
	break;
      }
    }

  if (activePaths_.empty()) {
    currentPen_       = painter()->pen();
    currentBrush_     = painter()->brush();
    currentShadow_    = painter()->shadow();
    penBrushShadowChanged_ = false;
  }

  WStringStream tmp;

  const std::vector<WPainterPath::Segment>& segments = path.segments();

  if (thisPath == -1) {
    tmp << "<v:shape style=\"width:" << (int)(Z * currentRect_.width())
	<< "px;height:" << (int)(Z * currentRect_.height())
	<< "px;\" path=\"m0,0l0,0";

    activePaths_.push_back(ActivePath());
    thisPath = activePaths_.size() - 1;
  }

#ifdef DEBUG_BBOX
  tmp << "m" << myzround(bbox.left()) << "," << myzround(bbox.top())
      << "l" << myzround(bbox.right()) << "," << myzround(bbox.bottom());

  if (!activePaths_[thisPath].bbox.isEmpty()) {
    const WRectF& bbox = activePaths_[thisPath].bbox;

    tmp << "m" << myzround(bbox.left()) << "," << myzround(bbox.top())
	<< "l" << myzround(bbox.right()) << "," << myzround(bbox.bottom());
  }
#endif // DEBUG_BBOX

  if (segments.size() > 0
      && segments[0].type() != MoveTo)
    tmp << "m0,0";

  for (unsigned i = 0; i < segments.size(); ++i) {
    const WPainterPath::Segment s = segments[i];

    /*
     * a move as last operation is (wrongly?) rendered as a stroke...
     * but this is common after a closeSubPath()
     */
    if ((i == segments.size() - 1)
	&& (s.type() == MoveTo))
      break;

    double x = s.x();
    double y = s.y();

    if (s.type() == ArcC) {
      double cx = segments[i].x();
      double cy = segments[i].y();
      double rx = segments[i+1].x();
      double ry = segments[i+1].y();
      double theta1 = -WTransform::degreesToRadians(segments[i+2].x());
      double deltaTheta = -WTransform::degreesToRadians(segments[i+2].y());
      i += 2;

      WPointF c = transform.map(WPointF(cx, cy));

      WPointF p1(rx * std::cos(theta1) + cx,
		 ry * std::sin(theta1) + cy);
      WPointF p2(rx * std::cos(theta1 + deltaTheta) + cx,
		 ry * std::sin(theta1 + deltaTheta) + cy);

      // XXX: VML can only have ellipses with axes parallel to the X/Y
      //      axis. So this will fail if there is a rotation + unequal
      //      scale in X/Y direction.
      rx *= norm(WPointF(transform.m11(), transform.m12()));
      ry *= norm(WPointF(transform.m21(), transform.m22()));

      WPointF a(c.x() - rx, c.y() - ry);
      WPointF b(c.x() + rx, c.y() + ry);

      p1 = transform.map(p1);
      p2 = transform.map(p2);

      if (deltaTheta < 0)
	tmp << "at";
      else
	tmp << "wa";
      tmp <<        myzround(a.x()) << "," << myzround(a.y())
	  << "," << myzround(b.x()) << "," << myzround(b.y())
	  << "," << myzround(p1.x()) << "," << myzround(p1.y())
	  << "," << myzround(p2.x()) << "," << myzround(p2.y());
    } else {
      switch (s.type()) {
      case MoveTo:
	tmp << "m";
	break;
      case LineTo:
	tmp << "l";
	break;
      case CubicC1:
	tmp << "c";
	break;
      case CubicC2:
      case CubicEnd:
	tmp << ",";
	break;
      case QuadC: {
	/*
	 * VML's quadratic bezier don't seem to work as advertized ?
	 */
	WPointF current = path.positionAtSegment(i);
	const double cpx = s.x();
	const double cpy = s.y();
	const double xend = segments[i+1].x();
	const double yend = segments[i+1].y();
      
	const double cp1x = current.x() + 2.0/3.0*(cpx - current.x());
	const double cp1y = current.y() + 2.0/3.0*(cpy - current.y());
	const double cp2x = cp1x + (xend - current.x())/3.0;
	const double cp2y = cp1y + (yend - current.y())/3.0;

	WPointF cp1(cp1x, cp1y);
        cp1 = transform.map(cp1);

	tmp << "c" << myzround(cp1.x()) << "," << myzround(cp1.y()) << ",";
	x = cp2x;
	y = cp2y;

	break;
      }
      case QuadEnd:
	tmp << ",";
	break;
      default:
	assert(false);
      }

      WPointF p(x, y);
      p = transform.map(p);

      tmp << myzround(p.x()) << "," << myzround(p.y());
    }
  }

  activePaths_[thisPath].path += tmp.str();
  activePaths_[thisPath].bbox = activePaths_[thisPath].bbox.united(bbox);
}

std::string WVmlImage::createShadowFilter() const
{
  char buf[30];

  WStringStream filter;

  double r = std::sqrt(2 * currentShadow_.blur());
  filter << "left: " << myzround(currentShadow_.offsetX() - r/2 - 1) << "px;";
  filter << "top: " << myzround(currentShadow_.offsetY() - r/2 - 1) 
	 << "px;z-index:-10;";
  filter << "filter:progid:DXImageTransform.Microsoft.Blur(makeShadow=1,";
  filter << "pixelradius="
	 << Utils::round_css_str(r, 2, buf);
  filter << ",shadowOpacity="
	 << Utils::round_css_str(currentShadow_.color().alpha()/255., 2, buf)
	 << ");";

  return filter.str();
}

void WVmlImage::finishPaths()
{
  for (unsigned i = 0; i < activePaths_.size(); ++i) {
    /*
     * High quality shadows are created by duplicating the path and
     * blurring it using a filter
     */
    if (!(painter()->renderHints() & RenderHint::LowQualityShadows)
	&& !currentShadow_.none()) {
      const std::string& path = activePaths_[i].path;
      std::size_t pos = path.find("style=\"") + 7;

      rendered_ << path.substr(0, pos)
		<< createShadowFilter()
		<< path.substr(pos)
		<< "e\">"
		<< strokeElement(currentPen_)
		<< fillElement(currentBrush_)
		<< "</v:shape>";
    }
    rendered_ << activePaths_[i].path
	      << "e\">"
	      << strokeElement(currentPen_)
	      << fillElement(currentBrush_)
	      << shadowElement(currentShadow_)
	      << "</v:shape>";
  }

  activePaths_.clear();
}

void WVmlImage::drawLine(double x1, double y1, double x2, double y2)
{
  WPainterPath path;
  path.moveTo(x1, y1);
  path.lineTo(x2, y2);

  WBrush oldBrush = painter()->brush();
  painter()->setBrush(WBrush());
  drawPath(path);
  painter()->setBrush(oldBrush);
}

void WVmlImage::drawText(const WRectF& rect, 
			 WFlags<AlignmentFlag> flags, TextFlag textFlag,
			 const WString& text, const WPointF *clipPoint)
{
  if (textFlag == TextFlag::WordWrap)
    throw WException("WVmlImage::drawText(): TextFlag::WordWrap is not supported");

  if (clipPoint && painter() && !painter()->clipPath().isEmpty()) {
    if (!painter()->clipPathTransform().map(painter()->clipPath())
	  .isPointInPath(painter()->worldTransform().map(*clipPoint)))
      return;
  }

  finishPaths();

  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

#ifdef TEXT_DIVS
  DomElement *e = DomElement::createNew(DomElement::DIV);

  WPointF pos = painter()->combinedTransform().map(rect.topLeft());

  /*
   * HTML tricks to center things vertically in IE
   */
  e->setProperty(Property::StylePosition, "absolute");
  e->setProperty(Property::StyleTop, std::to_string(pos.y()) + "px");
  e->setProperty(Property::StyleLeft, std::to_string(pos.x()) + "px");
  e->setProperty(Property::StyleWidth, std::to_string(rect.width()) + "px");
  e->setProperty(Property::StyleHeight, std::to_string(rect.height()) + "px");

  DomElement *t = e;
  DomElement *i = 0;

  if (verticalAlign != AlignmentFlag::Top) {
    t = DomElement::createNew(DomElement::DIV);

    if (verticalAlign == AlignmentFlag::Middle) {      
      i = DomElement::createNew(DomElement::DIV);
      i->setProperty(Property::StylePosition, "absolute");
      i->setProperty(Property::StyleTop, "50%");

      t->setProperty(Property::StylePosition, "relative");
      t->setProperty(Property::StyleTop, "-50%");      
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

  if (horizontalAlign == AlignmentFlag::Right) {
    t->setProperty(Property::StyleTextAlign, "right");
    if (i)
      i->setProperty(Property::StyleRight, "0px");
  } else if (horizontalAlign == AlignmentFlag::Center)
    t->setProperty(Property::StyleTextAlign, "center");

  if (i) {
    i->addChild(t);
    e->addChild(i);
  } else if (t != e)
    e->addChild(t);

  {
    EscapeOStream os(rendered_);
    DomElement::TimeoutList notused;
    e->asHTML(os, notused);
  }

  delete e;

#else

  double fontSize = painter()->font().sizeLength().toPixels();

  double y = rect.center().y();
  switch (verticalAlign) {
  case AlignmentFlag::Top:
    y = rect.top() + fontSize * 0.55; break;
  case AlignmentFlag::Middle:
    y = rect.center().y(); break;
  case AlignmentFlag::Bottom:
    y = rect.bottom() - fontSize * 0.45 ; break;
  default:
    break;
  }

  EscapeOStream render;
  render << "<v:shape style=\"width:" << (int)(Z * currentRect_.width())
	 << "px;height:" << (int)(Z * currentRect_.height())
	 << "px;\"><v:path textpathok=\"True\" v=\"m"
	 << myzround(rect.left(), false) << ',' << myzround(y, false) << 'l'
	 << myzround(rect.right(), false) << ',' << myzround(y, false)
	 << "m0,0l0,0e\"/><v:fill on=\"True\" "
	 << colorAttributes(painter()->pen().color())
	 << "/><v:stroke on=\"False\"/>"
	 << skewElement(painter()->combinedTransform())
	 << "<v:textpath on=\"True\" string=\"";

  render.pushEscape(EscapeOStream::HtmlAttribute);
  render << text.toUTF8();
  render.popEscape();

  render << "\" style=\"v-text-align:";

  switch (horizontalAlign) {
  case AlignmentFlag::Left:
    render << "left"; break;
  case AlignmentFlag::Center:
    render << "center"; break;
  case AlignmentFlag::Right:
    render << "right"; break;
  default:
    break;
  }

  Wt::WApplication *app = Wt::WApplication::instance();
  Wt::WFont textFont(painter()->font());
  textFont.setSize(textFont.sizeLength() * app->environment().dpiScale());

  std::string cssFont = textFont.cssText(false);
  std::size_t i = cssFont.find(',');
  if (i != std::string::npos) {
    cssFont = cssFont.substr(0, i);
    std::cerr << cssFont << std::endl;
  }

  render << ";" << cssFont << "\"/></v:shape>";

  if (!(painter()->renderHints() & RenderHint::LowQualityShadows)
      && !currentShadow_.none()) {
    std::string result = render.str();
    std::size_t pos = result.find("style=\"") + 7;
    rendered_ << result.substr(0, pos)
	      << createShadowFilter()
	      << result.substr(pos);
  }

  rendered_ << render.str();
#endif
}

WTextItem WVmlImage::measureText(const WString& text, double maxWidth,
				 bool wordWrap)
{
  if (!fontMetrics_)
    fontMetrics_ = new ServerSideFontMetrics();

  return fontMetrics_->measureText(painter()->font(), text, maxWidth, wordWrap);
}

WFontMetrics WVmlImage::fontMetrics()
{
  if (!fontMetrics_)
    fontMetrics_ = new ServerSideFontMetrics();

  return fontMetrics_->fontMetrics(painter()->font());
}

std::string WVmlImage::quote(const std::string& s)
{
  return '"' + s + '"';
}

std::string WVmlImage::quote(double d)
{
  char buf[30];
  return quote(Utils::round_js_str(d, 5, buf));
}

std::string WVmlImage::colorAttributes(const WColor& color)
{
  std::string result = " color=" + quote(color.cssText());

  if (color.alpha() != 255)
    result += " opacity=" + quote(color.alpha() / 255.);

  return result;
}

std::string WVmlImage::skewElement(const WTransform& t) const
{
  if (!t.isIdentity()) {
    char buf[30];
    WStringStream s;

    s << "<v:skew on=\"true\" matrix=\""
      << Utils::round_js_str(t.m11(), 5, buf) << ',';
    s << Utils::round_js_str(t.m21(), 5, buf) << ',';
    s << Utils::round_js_str(t.m12(), 5, buf) << ',';
    s << Utils::round_js_str(t.m22(), 5, buf)
      << ",0,0\""
      " origin=\"-0.5 -0.5\""
      " offset=\"";
    s << Utils::round_js_str(t.dx() + std::fabs(t.m11()) * 0.5, 5, buf)
      << "px,";
    s << Utils::round_js_str(t.dy() + std::fabs(t.m22()) * 0.5, 5, buf)
      << "px\"/>";

    /*
     * Note adding negative t.m11() and t.m22() seems to correct a
     * slight discrepancy in the paint example -- not sure if it is 
     * a good general rule though !
     * The vertical correction is another weird thing ?
     */

    return s.str();
  } else
    return std::string();
}

std::string WVmlImage::shadowElement(const WShadow& shadow) const
{
  if (!(painter()->renderHints() & RenderHint::LowQualityShadows))
    return std::string();

  char buf[30];

  if (!shadow.none()) {
    WStringStream result;

    result << "<v:shadow on=\"true\" offset=\""
	   << Utils::round_js_str(shadow.offsetX(), 3, buf) << "px,";
    result << Utils::round_js_str(shadow.offsetY(), 3, buf) << "px\" "
	   << colorAttributes(shadow.color()) << "/>";

    return result.str();
  } else
    return std::string();
}

std::string WVmlImage::fillElement(const WBrush& brush) const
{
  if (brush.style() != BrushStyle::None) {
    return "<v:fill " + colorAttributes(brush.color()) + "/>";
  } else
    return "<v:fill on=\"false\" />";
}

std::string WVmlImage::strokeElement(const WPen& pen) const
{
  if (pen.style() != PenStyle::None) {
    std::string result;

    result = "<v:stroke " + colorAttributes(pen.color());

    switch (pen.capStyle()) {
    case PenCapStyle::Flat:
      result += " endcap=\"flat\"";
      break;
    case PenCapStyle::Square:
      result += " endcap=\"square\"";
      break;
    case PenCapStyle::Round:
      break;
    }

    switch (pen.joinStyle()) {
    case PenJoinStyle::Miter:
      result += " joinstyle=\"miter\"";
      break;
    case PenJoinStyle::Bevel:
      result += " joinstyle=\"bevel\"";
      break;
    case PenJoinStyle::Round:
      break;
    }

    switch (pen.style()) {
    case PenStyle::None:
      break;
    case PenStyle::SolidLine:
      break;
    case PenStyle::DashLine:
      result += " dashstyle=\"dash\"";
      break;
    case PenStyle::DotLine:
      result += " dashstyle=\"dot\"";
      break;
    case PenStyle::DashDotLine:
      result += " dashstyle=\"dashdot\"";
      break;
    case PenStyle::DashDotDotLine:
      result += " dashstyle=\"2 2 0 2 0 2\"";
      break;
    }

    WLength w = painter()->normalizedPenWidth(pen.width(), false);
    if (w != WLength(1))
      result += " weight=" + quote(w.cssText());

    return result + "/>";
  } else
    return "<v:stroke on=\"false\" />";
}

void WVmlImage::processClipping()
{
  if (clippingChanged_) {
    /*
     * We can only deal with rectangles.
     */
    if (painter()->hasClipping()) {
      WRectF rect(0, 0, 0, 0);
      if (painter()->clipPath().asRect(rect)) {
	WTransform t = painter()->clipPathTransform();

	WPointF tl = t.map(rect.topLeft());
	WPointF tr = t.map(rect.topRight());
	WPointF bl = t.map(rect.bottomLeft());
	WPointF br = t.map(rect.bottomRight());

	double tlx = 0, tly = 0, brx = 0, bry = 0;
	bool ok = false;
	if (fequal(tl.y(), tr.y())) {
	  tlx = std::min(tl.x(), tr.x());
	  brx = std::max(tl.x(), tr.x());
	  tly = std::min(tl.y(), bl.y());
	  bry = std::max(tl.y(), br.y());

	  ok = true;
	} else if (fequal(tl.x(), tr.x())) {
	  tlx = std::min(tl.x(), bl.x());
	  brx = std::max(tl.x(), bl.x());
	  tly = std::min(tl.y(), tr.y());
	  bry = std::max(tl.y(), tr.y());

	  ok = true;
	}

	if (ok) {
	  stopClip();
	  startClip(WRectF(tlx, tly, brx - tlx, bry - tly));
	} else
	  LOG_WARN("VML only supports rectangle clipping "
		   << "with rectangles aligned to the window");
      } else
	LOG_WARN("VML only supports rectangle clipping");
    } else {
      stopClip();
      startClip(WRectF(0, 0, width().value(), height().value()));
    }

    clippingChanged_ = false;
  }
}

std::string WVmlImage::rendered()
{

  if (paintUpdate_)
    return rendered_.str();
  else {
    WStringStream s;
    s << "<div style=\"position:relative;width:"
      << width().cssText() << ";height:" << height().cssText()
      << ";overflow:hidden;\">"
      << rendered_.str()
      << "</div>";
    return s.str();
  }
}

void WVmlImage::startClip(const WRectF& rect)
{
  rendered_ << "<div style=\"position:absolute;left:"
	    << rect.left() << "px;top:" << rect.top() << "px;width:"
	    << rect.width() << "px;height:" << rect.height() 
	    << "px;overflow:hidden;\""
	    << " onselectstart=\"return false;\">"
	    << "<v:group style=\"position:absolute;left:0px;top:0px;width:"
	    << rect.width() << "px;height:" 
	    << rect.height() << "px\" coordorigin=\""
	    << 0.5 * rect.left() * Z 
	    << "," << 0.5 * rect.top() * Z << "\" coordsize=\""
	    << rect.width() * Z << "," << rect.height() * Z << "\">";

  currentRect_ = rect;
}

void WVmlImage::stopClip()
{
  rendered_ << "</v:group></div>";
}

}
