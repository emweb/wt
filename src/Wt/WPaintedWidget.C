/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WCanvasPaintDevice"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WPaintedWidget"
#include "Wt/WPainter"
#include "Wt/WResource"
#include "Wt/WSvgImage"
#include "Wt/WVmlImage"

#ifdef WT_HAS_WRASTERIMAGE
#include "Wt/WRasterImage"
#endif // WT_HAS_WRASTERIMAGE

#include "DomElement.h"

namespace Wt {

class WWidgetPainter {
public:
  enum RenderType {
    InlineVml,
    InlineSvg,
    HtmlCanvas,
    PngImage
  };

public:
  virtual ~WWidgetPainter();
  virtual WPaintDevice *createPaintDevice(bool paintUpdate) = 0;
  virtual WPaintDevice *getPaintDevice(bool paintUpdate) = 0;
  virtual void createContents(DomElement *element, WPaintDevice *device) = 0;
  virtual void updateContents(std::vector<DomElement *>& result,
			      WPaintDevice *device) = 0;
  virtual RenderType renderType() const = 0;

protected:
  WWidgetPainter(WPaintedWidget *widget);

  WPaintedWidget *widget_;
};

class WWidgetVectorPainter : public WWidgetPainter
{
public:
  WWidgetVectorPainter(WPaintedWidget *widget, RenderType renderType);
  virtual WVectorImage *createPaintDevice(bool paintUpdate);
  virtual WPaintDevice *getPaintDevice(bool paintUpdate);
  virtual void createContents(DomElement *element, WPaintDevice *device);
  virtual void updateContents(std::vector<DomElement *>& result,
			      WPaintDevice *device);
  virtual RenderType renderType() const { return renderType_; }

private:
  RenderType renderType_;
};

class WWidgetCanvasPainter : public WWidgetPainter
{
public:
  WWidgetCanvasPainter(WPaintedWidget *widget);
  virtual WCanvasPaintDevice *createPaintDevice(bool paintUpdate);
  virtual WPaintDevice *getPaintDevice(bool paintUpdate);
  virtual void createContents(DomElement *element, WPaintDevice *device);
  virtual void updateContents(std::vector<DomElement *>& result,
			      WPaintDevice *device); 
  virtual RenderType renderType() const { return HtmlCanvas; }
};

class WWidgetRasterPainter : public WWidgetPainter
{
public:
  WWidgetRasterPainter(WPaintedWidget *widget);
  ~WWidgetRasterPainter();
  virtual
#ifdef WT_HAS_WRASTERIMAGE
  WRasterImage
#else
  WPaintDevice
#endif
 *createPaintDevice(bool paintUpdate);
  virtual WPaintDevice *getPaintDevice(bool paintUpdate);
  virtual void createContents(DomElement *element, WPaintDevice *device);
  virtual void updateContents(std::vector<DomElement *>& result,
			      WPaintDevice *device); 
  virtual RenderType renderType() const { return PngImage; }

private:
  WRasterImage *device_;
};

WPaintedWidget::WPaintedWidget(WContainerWidget *parent)
  : WInteractWidget(parent),
    preferredMethod_(HtmlCanvas),
    painter_(0),
    needRepaint_(false),
    sizeChanged_(false),
    areaImageAdded_(false),
    repaintFlags_(0),
    areaImage_(0),
    renderWidth_(0), renderHeight_(0)
{
  if (WApplication::instance()) {
    const WEnvironment& env = WApplication::instance()->environment();

    if (env.agentIsOpera()
	&& env.userAgent().find("Mac OS X") == std::string::npos)
      preferredMethod_ = InlineSvgVml;
  }

  setInline(false);
}

WPaintedWidget::~WPaintedWidget()
{
  delete painter_;
  delete areaImage_;
}

void WPaintedWidget::setPreferredMethod(Method method)
{
  if (preferredMethod_ != method) {
    delete painter_;
    painter_ = 0;
    preferredMethod_ = method;
  }
}

void WPaintedWidget::resize(const WLength& width, const WLength& height)
{
  if (!width.isAuto() && !height.isAuto()) {
    setLayoutSizeAware(false);
    resizeCanvas(static_cast<int>(width.toPixels()),
		 static_cast<int>(height.toPixels()));
  }

  WInteractWidget::resize(width, height);
}

void WPaintedWidget::resizeCanvas(int width, int height)
{
  if (renderWidth_ == width && renderHeight_ == height)
    return;

  renderWidth_ = width;
  renderHeight_ = height;

  if (areaImage_)
    areaImage_->resize(renderWidth_, renderHeight_);

  sizeChanged_ = true;
  update();
}

void WPaintedWidget::layoutSizeChanged(int width, int height)
{
  // WInteractWidget::resize(width, height);

  resizeCanvas(width, height);
}

void WPaintedWidget::update(WFlags<PaintFlag> flags)
{
  needRepaint_ = true;
  repaintFlags_ |= flags;
  repaint();
}

void WPaintedWidget::enableAjax()
{
  if (dynamic_cast<WWidgetCanvasPainter *>(painter_)
      && renderWidth_ != 0 && renderHeight_ != 0)
    update();

  WInteractWidget::enableAjax();
}

WPaintedWidget::Method WPaintedWidget::getMethod() const
{
  const WEnvironment& env = WApplication::instance()->environment();

  Method method;

  if (!((env.agentIsChrome() && env.agent() >= WEnvironment::Chrome5)
        || (env.agentIsIE() && env.agent() >= WEnvironment::IE9)
        || (env.agentIsGecko() && env.agent() >= WEnvironment::Firefox4_0))) {
    // on older browsers, inline svg is only supported in xhtml mode. HTML5
    // also allows inline svg
    // Safari 5 and Opera 11 do not seem to support inline svg in html mode
#ifdef WT_HAS_WRASTERIMAGE
    method = env.javaScript() ? HtmlCanvas : PngImage;
#else
    method = HtmlCanvas;
#endif
  } else 
    if (!env.javaScript())
      method = InlineSvgVml;
    else {
      /*
       * For Firefox pre 3.0 on Mac: SVG support is buggy (text filling
       * is broken).
       */
      bool oldFirefoxMac =
	(env.userAgent().find("Firefox/1.5") != std::string::npos
	 || env.userAgent().find("Firefox/2.0") != std::string::npos)
	&& env.userAgent().find("Macintosh") != std::string::npos;

      if (oldFirefoxMac)
	method = HtmlCanvas;
      else
	method = preferredMethod_;

      /*
       * Nokia 810's default browser does not do SVG but there is no way of
       * finding that out, ASFAIK.
       */
      bool nokia810 =
	(env.userAgent().find("Linux arm") != std::string::npos
	 && env.userAgent().find("Tablet browser") != std::string::npos
	 && env.userAgent().find("Gecko") != std::string::npos);

      if (nokia810)
	method = HtmlCanvas;
      else
	method = preferredMethod_;
    }

  return method;
}

bool WPaintedWidget::createPainter()
{
  if (painter_)
    return false;

  if (preferredMethod_ == PngImage) {
    painter_ = new WWidgetRasterPainter(this);
    return true;
  }

  const WEnvironment& env = WApplication::instance()->environment();

  /*
   * For IE < 9: no choice. Use VML
   */
  if (env.agentIsIElt(9)) {
    painter_ = new WWidgetVectorPainter(this, WWidgetPainter::InlineVml);
    return true;
  }

  /* Otherwise, combined preferred method with actual capabilities */
  Method method = getMethod();

  if (method == InlineSvgVml)
    painter_ = new WWidgetVectorPainter(this, WWidgetPainter::InlineSvg);
  else if (method == PngImage)
    painter_ = new WWidgetRasterPainter(this);
  else
    painter_ = new WWidgetCanvasPainter(this);

  return true;
}

WPaintDevice *WPaintedWidget::createPaintDevice() const
{
  const_cast<WPaintedWidget *>(this)->createPainter();

  if (painter_)
    return painter_->createPaintDevice(true);
  else
    return 0;
}

DomElementType WPaintedWidget::domElementType() const
{
  if (isInline() && WApplication::instance()->environment().agentIsIElt(9))
    return DomElement_SPAN;
  else  
    return DomElement_DIV;
}

DomElement *WPaintedWidget::createDomElement(WApplication *app)
{
  if (isInLayout()) {
    setLayoutSizeAware(true);
    setJavaScriptMember(WT_RESIZE_JS,
			"function(self, w, h) {"
			"""var u = $(self).find('canvas, img');"
			"""if (w >= 0) "
			""  "u.width(w);"
			"""if (h >= 0) "
			""  "u.height(h);"
			"}");
  }

  createPainter();

  DomElement *result = DomElement::createNew(domElementType());
  setId(result, app);

  DomElement *wrap = result;

  if (width().isAuto() && height().isAuto()) {
    result->setProperty(PropertyStylePosition, "relative");

    wrap = DomElement::createNew(DomElement_DIV);
    wrap->setProperty(PropertyStylePosition, "absolute");
    wrap->setProperty(PropertyStyleLeft, "0");
    wrap->setProperty(PropertyStyleRight, "0");
  }

  DomElement *canvas = DomElement::createNew(DomElement_DIV);

  if (!app->environment().agentIsSpiderBot())
    canvas->setId('p' + id());

  WPaintDevice *device = painter_->getPaintDevice(false);

  //handle the widget correctly when inline and using VML 
  if (painter_->renderType() == WWidgetPainter::InlineVml && isInline()) {
    result->setProperty(PropertyStyle, "zoom: 1;");
    canvas->setProperty(PropertyStyleDisplay, "inline");
    canvas->setProperty(PropertyStyle, "zoom: 1;");
  }

  if (renderWidth_ != 0 && renderHeight_ != 0) {
    paintEvent(device);

#ifdef WT_TARGET_JAVA
    if (device->painter())
      device->painter()->end();
#endif // WT_TARGET_JAVA
  }

  painter_->createContents(canvas, device);

  needRepaint_ = false;

  wrap->addChild(canvas);
  if (wrap != result)
    result->addChild(wrap);

  updateDom(*result, true);

  return result;
}

void WPaintedWidget::updateDom(DomElement& element, bool all)
{
  if ((all && areaImage_) || areaImageAdded_) {
    element.addChild(areaImage_->createSDomElement(WApplication::instance()));
    areaImageAdded_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

void WPaintedWidget::propagateRenderOk(bool deep)
{
  needRepaint_ = false;

  WInteractWidget::propagateRenderOk(deep);
}

void WPaintedWidget::getDomChanges(std::vector<DomElement *>& result,
				   WApplication *app)
{
  DomElement *e = DomElement::getForUpdate(this, DomElement_DIV);
  updateDom(*e, false);
  result.push_back(e);

  bool createdNew = createPainter();

  if (needRepaint_) {
    WPaintDevice *device = painter_->getPaintDevice
      ((repaintFlags_ & PaintUpdate) && !createdNew);

    if (renderWidth_ != 0 && renderHeight_ != 0) {
      paintEvent(device);

#ifdef WT_TARGET_JAVA
      if (device->painter())
	device->painter()->end();
#endif // WT_TARGET_JAVA
    }

    if (createdNew) {
      DomElement *canvas = DomElement::getForUpdate('p' + id(), DomElement_DIV);
      canvas->removeAllChildren();
      painter_->createContents(canvas, device);
      result.push_back(canvas);
    } else {
      painter_->updateContents(result, device);
    }

    needRepaint_ = false;
    repaintFlags_ = 0;
  }
}

void WPaintedWidget::addArea(WAbstractArea *area)
{
  createAreaImage();
  areaImage_->addArea(area);
}

void WPaintedWidget::insertArea(int index, WAbstractArea *area)
{
  createAreaImage();
  areaImage_->insertArea(index, area);
}

void WPaintedWidget::removeArea(WAbstractArea *area)
{
  createAreaImage();
  areaImage_->removeArea(area);
}

WAbstractArea *WPaintedWidget::area(int index) const
{
  return areaImage_ ? areaImage_->area(index) : 0;
}

const std::vector<WAbstractArea *> WPaintedWidget::areas() const
{
  return areaImage_ ? areaImage_->areas()
    : static_cast<const std::vector<WAbstractArea *> >(std::vector<WAbstractArea *>());
}

void WPaintedWidget::createAreaImage()
{
  if (!areaImage_) {
    areaImage_ = new WImage(wApp->onePixelGifUrl());
    areaImage_->setParentWidget(this);

    if (positionScheme() == Static)
      setPositionScheme(Relative);
    areaImage_->setPositionScheme(Absolute);
    areaImage_->setOffsets(0, Left | Top);
    areaImage_->setMargin(0, Top);
    areaImage_->resize(renderWidth_, renderHeight_);

    areaImageAdded_ = true;
  }
}

/*
 * WWidgetPainter
 */

WWidgetPainter::WWidgetPainter(WPaintedWidget *widget)
  : widget_(widget)
{ }

WWidgetPainter::~WWidgetPainter()
{ }

/*
 * WWidgetVectorPainter
 */

WWidgetVectorPainter::WWidgetVectorPainter(WPaintedWidget *widget,
					   RenderType renderType)
  : WWidgetPainter(widget),
    renderType_(renderType)
{ }

WVectorImage *WWidgetVectorPainter::createPaintDevice(bool paintUpdate)
{
  if (renderType_ == InlineSvg)
    return new WSvgImage(widget_->renderWidth_, widget_->renderHeight_, 0,
			 paintUpdate);
  else
    return new WVmlImage(widget_->renderWidth_, widget_->renderHeight_,
			 paintUpdate);
}

WPaintDevice *WWidgetVectorPainter::getPaintDevice(bool paintUpdate)
{
  return createPaintDevice(paintUpdate);
}

void WWidgetVectorPainter::createContents(DomElement *canvas,
					  WPaintDevice *device)
{
  WVectorImage *vectorDevice = dynamic_cast<WVectorImage *>(device);
  canvas->setProperty(PropertyInnerHTML, vectorDevice->rendered());
  delete device;
}

void WWidgetVectorPainter::updateContents(std::vector<DomElement *>& result,
					  WPaintDevice *device)
{
  WVectorImage *vectorDevice = dynamic_cast<WVectorImage *>(device);

  if (widget_->repaintFlags_ & PaintUpdate) {
    DomElement *painter = DomElement::updateGiven
      (WT_CLASS ".getElement('p" + widget_->id()+ "').firstChild",
       DomElement_DIV);

    painter->setProperty(PropertyAddedInnerHTML, vectorDevice->rendered());

    WApplication *app = WApplication::instance();
    if (app->environment().agentIsOpera())
      painter->callMethod("forceRedraw();");

    result.push_back(painter);
  } else {
    DomElement *canvas = DomElement::getForUpdate
      ('p' + widget_->id(), DomElement_DIV);

    /*
     * In fact, we should use another property, since we could be using
     * document.importNode() instead of myImportNode() since the xml does not
     * need to be interpreted as HTML...
     */
    canvas->setProperty(PropertyInnerHTML, vectorDevice->rendered());
    result.push_back(canvas);
  }

  widget_->sizeChanged_ = false;

  delete device;
}

/*
 * WWidgetCanvasPainter
 */

WWidgetCanvasPainter::WWidgetCanvasPainter(WPaintedWidget *widget)
  : WWidgetPainter(widget)
{ }

WCanvasPaintDevice *WWidgetCanvasPainter::createPaintDevice(bool paintUpdate)
{
  return new WCanvasPaintDevice(widget_->renderWidth_, widget_->renderHeight_,
				0, paintUpdate);
}

WPaintDevice *WWidgetCanvasPainter::getPaintDevice(bool paintUpdate)
{
  return createPaintDevice(paintUpdate);
}

void WWidgetCanvasPainter::createContents(DomElement *result,
					  WPaintDevice *device)
{
  std::string wstr = boost::lexical_cast<std::string>(widget_->renderWidth_);
  std::string hstr = boost::lexical_cast<std::string>(widget_->renderHeight_);

  result->setProperty(PropertyStylePosition, "relative");
  result->setProperty(PropertyStyleOverflowX, "hidden");
  result->setProperty(PropertyStyleOverflowY, "hidden");

  DomElement *canvas = DomElement::createNew(DomElement_CANVAS);
  canvas->setId('c' + widget_->id());
  canvas->setProperty(PropertyStyleDisplay, "block");
  canvas->setAttribute("width", wstr);
  canvas->setAttribute("height", hstr);
  result->addChild(canvas);
  widget_->sizeChanged_ = false;

  WCanvasPaintDevice *canvasDevice = dynamic_cast<WCanvasPaintDevice *>(device);

  DomElement *text = 0;
  if (canvasDevice->textMethod() == WCanvasPaintDevice::DomText) {
    text = DomElement::createNew(DomElement_DIV);
    text->setId('t' + widget_->id());
    text->setProperty(PropertyStylePosition, "absolute");
    text->setProperty(PropertyStyleZIndex, "1");
    text->setProperty(PropertyStyleTop, "0px");
    text->setProperty(PropertyStyleLeft, "0px");
  }

  canvasDevice->render("c" + widget_->id(), text ? text : result);

  if (text)
    result->addChild(text);

  delete device;
}

void WWidgetCanvasPainter::updateContents(std::vector<DomElement *>& result,
					  WPaintDevice *device)
{
  WCanvasPaintDevice *canvasDevice = dynamic_cast<WCanvasPaintDevice *>(device);

  if (widget_->sizeChanged_) {
    DomElement *canvas = DomElement::getForUpdate('c' + widget_->id(),
						  DomElement_CANVAS);
    canvas->setAttribute("width",
		 boost::lexical_cast<std::string>(widget_->renderWidth_));
    canvas->setAttribute("height",
		 boost::lexical_cast<std::string>(widget_->renderHeight_));
    result.push_back(canvas);

    widget_->sizeChanged_ = false;
  }

  bool domText = canvasDevice->textMethod() == WCanvasPaintDevice::DomText;

  DomElement *el
    = DomElement::getForUpdate(domText ? 't' + widget_->id() : widget_->id(),
			       DomElement_DIV);
  if (domText)
    el->removeAllChildren();

  canvasDevice->render('c' + widget_->id(), el);

  result.push_back(el);

  delete device;
}

/*
 * WWidgetRasterPainter
 */

WWidgetRasterPainter::WWidgetRasterPainter(WPaintedWidget *widget)
  : WWidgetPainter(widget),
    device_(0)
{ }

WWidgetRasterPainter::~WWidgetRasterPainter()
{
#ifdef WT_HAS_WRASTERIMAGE
  delete device_;
#endif
}

#ifdef WT_HAS_WRASTERIMAGE
WRasterImage
#else
WPaintDevice
#endif
*WWidgetRasterPainter::createPaintDevice(bool paintUpdate)
{
#ifdef WT_HAS_WRASTERIMAGE
  return new WRasterImage("png", widget_->renderWidth_, widget_->renderHeight_);
#else
  throw WException("Wt was built without WRasterImage (graphicsmagick)");
#endif
}

WPaintDevice *WWidgetRasterPainter::getPaintDevice(bool paintUpdate)
{
  if (!device_ || widget_->sizeChanged_) {
#ifdef WT_HAS_WRASTERIMAGE
    delete device_;
    device_ = createPaintDevice(paintUpdate);
#else
    throw WException("Wt was built without WRasterImage (graphicsmagick)");
#endif
  }

#ifdef WT_HAS_WRASTERIMAGE
  if (!paintUpdate)
    device_->clear();

  return device_;
#else
  return 0;
#endif
}

void WWidgetRasterPainter::createContents(DomElement *result,
					  WPaintDevice *device)
{
  std::string wstr = boost::lexical_cast<std::string>(widget_->renderWidth_);
  std::string hstr = boost::lexical_cast<std::string>(widget_->renderHeight_);

  DomElement *img = DomElement::createNew(DomElement_IMG);
  img->setId('i' + widget_->id());
  img->setAttribute("width", wstr);
  img->setAttribute("height", hstr);
  img->setAttribute("class", "unselectable");
  img->setAttribute("unselectable", "on");
  img->setAttribute("onselectstart", "return false;");
  img->setAttribute("onmousedown", "return false;");

  WResource *resource = dynamic_cast<WResource *>(device);
  img->setAttribute("src", resource->generateUrl());

  result->addChild(img);
}

void WWidgetRasterPainter::updateContents(std::vector<DomElement *>& result,
					  WPaintDevice *device)
{
  WResource *resource = dynamic_cast<WResource *>(device);

  DomElement *img
    = DomElement::getForUpdate('i' + widget_->id(), DomElement_IMG);

  if (widget_->sizeChanged_) {
    img->setAttribute("width",
		      boost::lexical_cast<std::string>(widget_->renderWidth_));
    img->setAttribute("height",
		      boost::lexical_cast<std::string>(widget_->renderHeight_));
    widget_->sizeChanged_ = false;
  }

  img->setAttribute("src", resource->generateUrl());

  result.push_back(img);
}

}
