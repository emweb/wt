/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractArea.h"
#include "Wt/WApplication.h"
#include "Wt/WCanvasPaintDevice.h"
#include "Wt/WEnvironment.h"
#include "Wt/WImage.h"
#include "Wt/WJavaScriptExposableObject.h"
#include "Wt/WPaintedWidget.h"
#include "Wt/WPainter.h"
#include "Wt/WResource.h"
#include "Wt/WSvgImage.h"
#include "Wt/WVmlImage.h"
#include "WebUtils.h"

#ifdef WT_HAS_WRASTERIMAGE
#include "Wt/WRasterImage.h"
#endif // WT_HAS_WRASTERIMAGE

#include "DomElement.h"

#ifndef WT_DEBUG_JS
#include "js/WPaintedWidget.min.js"
#include "js/WJavaScriptObjectStorage.min.js"
#endif

namespace Wt {

class WWidgetPainter {
public:
  enum class RenderType {
    InlineVml,
    InlineSvg,
    HtmlCanvas,
    PngImage
  };

public:
  virtual ~WWidgetPainter();
  virtual std::unique_ptr<WPaintDevice> createPaintDevice(bool paintUpdate)
  = 0;
  virtual std::unique_ptr<WPaintDevice> getPaintDevice(bool paintUpdate) = 0;
  virtual void createContents(DomElement *element,
			      std::unique_ptr<WPaintDevice> device) = 0;
  virtual void updateContents(std::vector<DomElement *>& result,
			      std::unique_ptr<WPaintDevice> device) = 0;
  virtual RenderType renderType() const = 0;

protected:
  WWidgetPainter(WPaintedWidget *widget);

  WPaintedWidget *widget_;
};

class WWidgetVectorPainter : public WWidgetPainter
{
public:
  WWidgetVectorPainter(WPaintedWidget *widget, RenderType renderType);

  virtual std::unique_ptr<WPaintDevice> createPaintDevice(bool paintUpdate)
    override;
  virtual std::unique_ptr<WPaintDevice> getPaintDevice(bool paintUpdate)
    override;
  virtual void createContents(DomElement *element,
			      std::unique_ptr<WPaintDevice> device) override;
  virtual void updateContents(std::vector<DomElement *>& result,
			      std::unique_ptr<WPaintDevice> device) override;
  virtual RenderType renderType() const override { return renderType_; }

private:
  RenderType renderType_;
};

class WWidgetCanvasPainter : public WWidgetPainter
{
public:
  WWidgetCanvasPainter(WPaintedWidget *widget);
  virtual std::unique_ptr<WPaintDevice> createPaintDevice(bool paintUpdate)
    override;
  virtual std::unique_ptr<WPaintDevice> getPaintDevice(bool paintUpdate)
    override;
  virtual void createContents(DomElement *element,
			      std::unique_ptr<WPaintDevice> device) override;
  virtual void updateContents(std::vector<DomElement *>& result,
			      std::unique_ptr<WPaintDevice> device) override;
  virtual RenderType renderType() const override { 
    return RenderType::HtmlCanvas; 
  }
};

class WWidgetRasterPainter : public WWidgetPainter
{
public:
  WWidgetRasterPainter(WPaintedWidget *widget);
  ~WWidgetRasterPainter();

  virtual std::unique_ptr<WPaintDevice> createPaintDevice(bool paintUpdate)
    override;
  virtual std::unique_ptr<WPaintDevice>getPaintDevice(bool paintUpdate)
    override;
  virtual void createContents(DomElement *element,
			      std::unique_ptr<WPaintDevice> device) override;
  virtual void updateContents(std::vector<DomElement *>& result,
			      std::unique_ptr<WPaintDevice> device) override; 
  virtual RenderType renderType() const override { 
    return RenderType::PngImage; 
  }

private:
  std::unique_ptr<WPaintDevice> device_;
};

WPaintedWidget::WPaintedWidget()
  : preferredMethod_(RenderMethod::HtmlCanvas),
    needRepaint_(false),
    sizeChanged_(false),
    areaImageAdded_(false),
    repaintFlags_(None),
    renderWidth_(0), 
    renderHeight_(0),
    repaintSlot_("function() {"
	"var o=" + this->objJsRef() + ";"
	"if(o){o.repaint();}"
	"}", this),
    jsObjects_(this),
    jsDefined_(false)
{
  if (WApplication::instance()) {
    const WEnvironment& env = WApplication::instance()->environment();

    if (env.agentIsOpera()
	&& env.userAgent().find("Mac OS X") == std::string::npos)
      preferredMethod_ = RenderMethod::InlineSvgVml;
  }

  setInline(false);
}

WPaintedWidget::~WPaintedWidget()
{
  manageWidget(areaImage_, std::unique_ptr<WImage>());
}

void WPaintedWidget::setPreferredMethod(RenderMethod method)
{
  if (preferredMethod_ != method) {
    painter_.reset();
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
  if (dynamic_cast<WWidgetCanvasPainter *>(painter_.get())
      && renderWidth_ != 0 && renderHeight_ != 0)
    update();

  WInteractWidget::enableAjax();
}

void WPaintedWidget::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  if (getMethod() == RenderMethod::HtmlCanvas) {
    LOAD_JAVASCRIPT(app, "js/WPaintedWidget.js", "WPaintedWidget", wtjs10);
    LOAD_JAVASCRIPT(app, "js/WPaintedWidget.js", "gfxUtils", wtjs11);
    if (jsObjects_.size() > 0) {
      setFormObject(true);

      LOAD_JAVASCRIPT(app, "js/WJavaScriptObjectStorage.js", "WJavaScriptObjectStorage", wtjs20);

      jsDefined_ = true;
    } else {
      jsDefined_ = false;
    }
  }
}

void WPaintedWidget::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full) || !jsDefined_) {
    defineJavaScript();
  }

  WInteractWidget::render(flags);
}

void WPaintedWidget::setFormData(const FormData& formData)
{
  Http::ParameterValues parVals = formData.values;
  if (Utils::isEmpty(parVals))
    return;
  if (parVals[0] == "undefined")
    return;

  jsObjects_.assignFromJSON(parVals[0]);
}

RenderMethod WPaintedWidget::getMethod() const
{
  const WEnvironment& env = WApplication::instance()->environment();

  RenderMethod method;

#ifdef WT_HAS_WRASTERIMAGE
  if (preferredMethod_ == RenderMethod::PngImage)
    return RenderMethod::PngImage;
#endif

  if (env.agentIsIElt(9)) {
#ifdef WT_HAS_WRASTERIMAGE
    method = preferredMethod_ == RenderMethod::InlineSvgVml ? 
      RenderMethod::InlineSvgVml : RenderMethod::PngImage;
#else
    method = RenderMethod::InlineSvgVml;
#endif
  } else 
    if (!((env.agentIsChrome() && 
	   static_cast<unsigned int>(env.agent()) >= 
	   static_cast<unsigned int>(UserAgent::Chrome5))
	  || (env.agentIsGecko() && 
	      static_cast<unsigned int>(env.agent()) >= 
	      static_cast<unsigned int>(UserAgent::Firefox4_0)))) {
      // on older browsers, inline svg is only supported in xhtml mode. HTML5
      // also allows inline svg
      // Safari 5 and Opera 11 do not seem to support inline svg in html mode
#ifdef WT_HAS_WRASTERIMAGE
      method = env.javaScript() ? RenderMethod::HtmlCanvas :
	RenderMethod::PngImage;
#else
      method = RenderMethod::HtmlCanvas;
#endif
    } else  {
      if (!env.javaScript()) {
	method = RenderMethod::InlineSvgVml;
      } else {
	/*
	 * For Firefox pre 3.0 on Mac: SVG support is buggy (text filling
	 * is broken).
	 */
	bool oldFirefoxMac =
	  (env.userAgent().find("Firefox/1.5") != std::string::npos
	   || env.userAgent().find("Firefox/2.0") != std::string::npos)
	  && env.userAgent().find("Macintosh") != std::string::npos;

	if (oldFirefoxMac)
	  method = RenderMethod::HtmlCanvas;
	else {
	  // HTML canvas if preferred method is PNG, but there's no
	  // WRasterImage support
	  method = preferredMethod_ == RenderMethod::PngImage 
	    ? RenderMethod::HtmlCanvas : preferredMethod_;
	}

	/*
	 * Nokia 810's default browser does not do SVG but there is no way of
	 * finding that out, ASFAIK.
	 */
	bool nokia810 =
	  (env.userAgent().find("Linux arm") != std::string::npos
	   && env.userAgent().find("Tablet browser") != std::string::npos
	   && env.userAgent().find("Gecko") != std::string::npos);

	if (nokia810)
	  method = RenderMethod::HtmlCanvas;
	else {
	  // HTML canvas if preferred method is PNG, but there's no
	  // WRasterImage support
	  method = preferredMethod_ == RenderMethod::PngImage 
	    ? RenderMethod::HtmlCanvas : preferredMethod_;
	}
      }
    }

  return method;
}

bool WPaintedWidget::createPainter()
{
  if (painter_)
    return false;

  const WEnvironment& env = WApplication::instance()->environment();

  /* Otherwise, combined preferred method with actual capabilities */
  RenderMethod method = getMethod();

  if (method == RenderMethod::InlineSvgVml) {
    if (env.agentIsIElt(9)) {
      painter_.reset
	(new WWidgetVectorPainter(this, WWidgetPainter::RenderType::InlineVml));
    } else {
      painter_.reset
	(new WWidgetVectorPainter(this, WWidgetPainter::RenderType::InlineSvg));
    }
  } else if (method == RenderMethod::PngImage)
    painter_.reset(new WWidgetRasterPainter(this));
  else
    painter_.reset(new WWidgetCanvasPainter(this));

  return true;
}

std::unique_ptr<WPaintDevice> WPaintedWidget::createPaintDevice() const
{
  const_cast<WPaintedWidget *>(this)->createPainter();

  if (painter_)
    return painter_->createPaintDevice(true);
  else
    return nullptr;
}

DomElementType WPaintedWidget::domElementType() const
{
  if (isInline() && WApplication::instance()->environment().agentIsIElt(9))
    return DomElementType::SPAN;
  else  
    return DomElementType::DIV;
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
                        """else "
                        ""  "u.width('auto');"
			"""if (h >= 0) "
			""  "u.height(h);"
                        """else "
                        ""  "u.height('auto');"
			"}");
  }

  createPainter();

  DomElement *result = DomElement::createNew(domElementType());
  setId(result, app);

  DomElement *wrap = result;

  if (width().isAuto() && height().isAuto()) {
    result->setProperty(Property::StylePosition, "relative");

    wrap = DomElement::createNew(DomElementType::DIV);
    wrap->setProperty(Property::StylePosition, "absolute");
    wrap->setProperty(Property::StyleLeft, "0");
    wrap->setProperty(Property::StyleRight, "0");
  }

  DomElement *canvas = DomElement::createNew(DomElementType::DIV);

  if (!app->environment().agentIsSpiderBot())
    canvas->setId('p' + id());

  std::unique_ptr<WPaintDevice> device = painter_->getPaintDevice(false);

  //handle the widget correctly when inline and using VML 
  if (painter_->renderType() == WWidgetPainter::RenderType::InlineVml &&
      isInline()) {
    result->setProperty(Property::Style, "zoom: 1;");
    canvas->setProperty(Property::StyleDisplay, "inline");
    canvas->setProperty(Property::Style, "zoom: 1;");
  }

  if (renderWidth_ != 0 && renderHeight_ != 0) {
    paintEvent(device.get());

#ifdef WT_TARGET_JAVA
    if (device->painter())
      device->painter()->end();
#endif // WT_TARGET_JAVA
  }

  painter_->createContents(canvas, std::move(device));

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
  DomElement *e = DomElement::getForUpdate(this, DomElementType::DIV);
  updateDom(*e, false);
  result.push_back(e);

  bool createdNew = createPainter();

  if (needRepaint_) {
    std::unique_ptr<WPaintDevice> device
      = painter_->getPaintDevice(repaintFlags_.test(PaintFlag::Update) &&
				 !createdNew);

    if (renderWidth_ != 0 && renderHeight_ != 0) {
      paintEvent(device.get());

#ifdef WT_TARGET_JAVA
      if (device->painter())
	device->painter()->end();
#endif // WT_TARGET_JAVA
    }

    if (createdNew) {
      DomElement *canvas
	= DomElement::getForUpdate('p' + id(), DomElementType::DIV);
      canvas->removeAllChildren();
      painter_->createContents(canvas, std::move(device));
      result.push_back(canvas);
    } else {
      painter_->updateContents(result, std::move(device));
    }

    needRepaint_ = false;
    repaintFlags_ = None;
  }
}

void WPaintedWidget::addArea(std::unique_ptr<WAbstractArea> area)
{
  createAreaImage();
  areaImage_->addArea(std::move(area));
}

void WPaintedWidget::insertArea(int index,
				std::unique_ptr<WAbstractArea> area)
{
  createAreaImage();
  areaImage_->insertArea(index, std::move(area));
}

std::unique_ptr<WAbstractArea> WPaintedWidget
::removeArea(WAbstractArea *area)
{
  createAreaImage();
  return areaImage_->removeArea(area);
}

WAbstractArea *WPaintedWidget::area(int index) const
{
  return areaImage_ ? areaImage_->area(index) : nullptr;
}

const std::vector<WAbstractArea *> WPaintedWidget::areas() const
{
  return areaImage_ 
    ? areaImage_->areas()
    : static_cast<const std::vector<WAbstractArea *> >
    (std::vector<WAbstractArea *>());
}

void WPaintedWidget::createAreaImage()
{
  if (!areaImage_) {
    areaImage_.reset(new WImage(wApp->onePixelGifUrl()));
    widgetAdded(areaImage_.get());

    if (positionScheme() == PositionScheme::Static)
      setPositionScheme(PositionScheme::Relative);

    areaImage_->setPositionScheme(PositionScheme::Absolute);
    areaImage_->setOffsets(0, Side::Left | Side::Top);
    areaImage_->setMargin(0, Side::Top);
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

std::unique_ptr<WPaintDevice> WWidgetVectorPainter
::createPaintDevice(bool paintUpdate)
{
  if (renderType_ == RenderType::InlineSvg)
    return std::unique_ptr<WPaintDevice>
      (new WSvgImage(widget_->renderWidth_, widget_->renderHeight_,
		     paintUpdate));
  else
    return std::unique_ptr<WPaintDevice>
      (new WVmlImage(widget_->renderWidth_, widget_->renderHeight_,
		     paintUpdate));
}

std::unique_ptr<WPaintDevice>WWidgetVectorPainter::getPaintDevice(bool paintUpdate)
{
  return createPaintDevice(paintUpdate);
}

void WWidgetVectorPainter
::createContents(DomElement *canvas, std::unique_ptr<WPaintDevice> device)
{
  WVectorImage *vectorDevice = dynamic_cast<WVectorImage *>(device.get());
  canvas->setProperty(Property::InnerHTML, vectorDevice->rendered());
}

void WWidgetVectorPainter
::updateContents(std::vector<DomElement *>& result,
		 std::unique_ptr<WPaintDevice> device)
{
  WVectorImage *vectorDevice = dynamic_cast<WVectorImage *>(device.get());

  if (widget_->repaintFlags_.test(PaintFlag::Update)) {
    DomElement *painter = DomElement::updateGiven
      (WT_CLASS ".getElement('p" + widget_->id()+ "').firstChild",
       DomElementType::DIV);

    painter->setProperty(Property::AddedInnerHTML, vectorDevice->rendered());

    WApplication *app = WApplication::instance();
    if (app->environment().agentIsOpera())
      painter->callMethod("forceRedraw();");

    result.push_back(painter);
  } else {
    DomElement *canvas = DomElement::getForUpdate
      ('p' + widget_->id(), DomElementType::DIV);

    /*
     * In fact, we should use another property, since we could be using
     * document.importNode() instead of myImportNode() since the xml does not
     * need to be interpreted as HTML...
     */
    canvas->setProperty(Property::InnerHTML, vectorDevice->rendered());
    result.push_back(canvas);
  }

  widget_->sizeChanged_ = false;
}

/*
 * WWidgetCanvasPainter
 */

WWidgetCanvasPainter::WWidgetCanvasPainter(WPaintedWidget *widget)
  : WWidgetPainter(widget)
{ }

std::unique_ptr<WPaintDevice> WWidgetCanvasPainter
::createPaintDevice(bool paintUpdate)
{
  return std::unique_ptr<WPaintDevice>
    (new WCanvasPaintDevice(widget_->renderWidth_, widget_->renderHeight_,
			    paintUpdate));
}

std::unique_ptr<WPaintDevice> WWidgetCanvasPainter
::getPaintDevice(bool paintUpdate)
{
  return createPaintDevice(paintUpdate);
}

void WWidgetCanvasPainter
::createContents(DomElement *result, std::unique_ptr<WPaintDevice> device)
{
  std::string wstr = std::to_string(widget_->renderWidth_);
  std::string hstr = std::to_string(widget_->renderHeight_);

  result->setProperty(Property::StylePosition, "relative");
  result->setProperty(Property::StyleOverflowX, "hidden");
  result->setProperty(Property::StyleOverflowY, "hidden");

  DomElement *canvas = DomElement::createNew(DomElementType::CANVAS);
  canvas->setId('c' + widget_->id());
  canvas->setProperty(Property::StyleDisplay, "block");
  canvas->setAttribute("width", wstr);
  canvas->setAttribute("height", hstr);
  result->addChild(canvas);
  widget_->sizeChanged_ = false;

  WCanvasPaintDevice *canvasDevice
    = dynamic_cast<WCanvasPaintDevice *>(device.get());

  DomElement *text = nullptr;
  if (canvasDevice->textMethod() == WCanvasPaintDevice::TextMethod::DomText) {
    text = DomElement::createNew(DomElementType::DIV);
    text->setId('t' + widget_->id());
    text->setProperty(Property::StylePosition, "absolute");
    text->setProperty(Property::StyleZIndex, "1");
    text->setProperty(Property::StyleTop, "0px");
    text->setProperty(Property::StyleLeft, "0px");
  }

  DomElement *el = text ? text : result;
  bool hasJsObjects = widget_->jsObjects_.size() > 0;

  WApplication *app = WApplication::instance();
  {
    WStringStream ss;
    ss << "new " WT_CLASS ".WPaintedWidget("
       << app->javaScriptClass() << "," << widget_->jsRef() << ");";
    el->callJavaScript(ss.str());
  }

  std::string updateAreasJs;
  if (hasJsObjects) {
    WStringStream ss;
    ss << "new " WT_CLASS ".WJavaScriptObjectStorage("
       << app->javaScriptClass() << "," << widget_->jsRef() << ");";
    widget_->jsObjects_.updateJs(ss, true);
    el->callJavaScript(ss.str());
    if (widget_->areaImage_) {
      widget_->areaImage_->setTargetJS(widget_->objJsRef());
      updateAreasJs = widget_->areaImage_->updateAreasJS();
    }
  }

  canvasDevice->render(widget_->objJsRef(), 'c' + widget_->id(), el, updateAreasJs);

  if (text)
    result->addChild(text);
}

void WWidgetCanvasPainter
::updateContents(std::vector<DomElement *>& result,
		 std::unique_ptr<WPaintDevice> device)
{
  WCanvasPaintDevice *canvasDevice
    = dynamic_cast<WCanvasPaintDevice *>(device.get());

  if (widget_->sizeChanged_) {
    DomElement *canvas = DomElement::getForUpdate('c' + widget_->id(),
						  DomElementType::CANVAS);
    canvas->setAttribute("width", std::to_string(widget_->renderWidth_));
    canvas->setAttribute("height", std::to_string(widget_->renderHeight_));
    result.push_back(canvas);

    widget_->sizeChanged_ = false;
  }

  bool domText = canvasDevice->textMethod() == 
    WCanvasPaintDevice::TextMethod::DomText;

  DomElement *el
    = DomElement::getForUpdate(domText ? 't' + widget_->id() : widget_->id(),
			       DomElementType::DIV);
  if (domText)
    el->removeAllChildren();

  bool hasJsObjects = widget_->jsObjects_.size() > 0;

  std::string updateAreasJs;
  if (hasJsObjects) {
    WStringStream ss;
    widget_->jsObjects_.updateJs(ss, false);
    el->callJavaScript(ss.str());
    if (widget_->areaImage_) {
      widget_->areaImage_->setTargetJS(widget_->objJsRef());
      updateAreasJs = widget_->areaImage_->updateAreasJS();
    }
  }

  canvasDevice->render(widget_->objJsRef(), 'c' + widget_->id(), el, updateAreasJs);

  result.push_back(el);
}

/*
 * WWidgetRasterPainter
 */

WWidgetRasterPainter::WWidgetRasterPainter(WPaintedWidget *widget)
  : WWidgetPainter(widget)
{ }

WWidgetRasterPainter::~WWidgetRasterPainter()
{ }

std::unique_ptr<WPaintDevice> WWidgetRasterPainter
::createPaintDevice(bool paintUpdate)
{
#ifdef WT_HAS_WRASTERIMAGE
  return std::unique_ptr<WPaintDevice>
    (new WRasterImage("png", widget_->renderWidth_, widget_->renderHeight_));
#else
  throw WException("Wt was built without WRasterImage (graphicsmagick, skia or Direct2D)");
#endif
}

std::unique_ptr<WPaintDevice> WWidgetRasterPainter
::getPaintDevice(bool paintUpdate)
{
  if (!device_ || widget_->sizeChanged_) {
#ifdef WT_HAS_WRASTERIMAGE
    device_ = createPaintDevice(paintUpdate);
#else
    throw WException("Wt was built without WRasterImage (graphicsmagick, skia or Direct2D)");
#endif
  }

#ifdef WT_HAS_WRASTERIMAGE
  if (!paintUpdate)
    (dynamic_cast<WRasterImage *>(device_.get()))->clear();
#endif

  return std::move(device_);
}

void WWidgetRasterPainter::createContents
  (DomElement *result, std::unique_ptr<WPaintDevice> device)
{
  std::string wstr = std::to_string(widget_->renderWidth_);
  std::string hstr = std::to_string(widget_->renderHeight_);

  DomElement *img = DomElement::createNew(DomElementType::IMG);
  img->setId('i' + widget_->id());
  img->setAttribute("width", wstr);
  img->setAttribute("height", hstr);
  img->setAttribute("class", "unselectable");
  img->setAttribute("unselectable", "on");
  img->setAttribute("onselectstart", "return false;");
  img->setAttribute("onmousedown", "return false;");

  WResource *resource = dynamic_cast<WResource *>(device.get());
  img->setAttribute("src", resource->generateUrl());

  result->addChild(img);

  device_ = std::move(device);
}

void WWidgetRasterPainter
::updateContents(std::vector<DomElement *>& result,
		 std::unique_ptr<WPaintDevice> device)
{
  WResource *resource = dynamic_cast<WResource *>(device.get());

  DomElement *img
    = DomElement::getForUpdate('i' + widget_->id(), DomElementType::IMG);

  if (widget_->sizeChanged_) {
    img->setAttribute("width", std::to_string(widget_->renderWidth_));
    img->setAttribute("height", std::to_string(widget_->renderHeight_));
    widget_->sizeChanged_ = false;
  }

  img->setAttribute("src", resource->generateUrl());

  result.push_back(img);

  device_ = std::move(device);
}

WJavaScriptHandle<WTransform> WPaintedWidget::createJSTransform()
{
  return jsObjects_.addObject(new WTransform());
}

WJavaScriptHandle<WBrush> WPaintedWidget::createJSBrush()
{
  return jsObjects_.addObject(new WBrush());
}

WJavaScriptHandle<WPen> WPaintedWidget::createJSPen()
{
  return jsObjects_.addObject(new WPen());
}

WJavaScriptHandle<WPainterPath> WPaintedWidget::createJSPainterPath()
{
  return jsObjects_.addObject(new WPainterPath());
}

WJavaScriptHandle<WRectF> WPaintedWidget::createJSRect()
{
  return jsObjects_.addObject(new WRectF(0,0,0,0));
}

WJavaScriptHandle<WPointF> WPaintedWidget::createJSPoint()
{
  return jsObjects_.addObject(new WPointF(0,0));
}

std::string WPaintedWidget::objJsRef() const
{
  return jsRef() + ".wtObj";
}

}
