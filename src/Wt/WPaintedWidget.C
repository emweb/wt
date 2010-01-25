/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WCanvasPaintDevice"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WPainter"
#include "Wt/WPaintedWidget"
#include "Wt/WSvgImage"
#include "Wt/WVmlImage"

#include "DomElement.h"

namespace Wt {

class WWidgetPainter {
public:
  virtual ~WWidgetPainter();
  virtual WPaintDevice *createPaintDevice() = 0;
  virtual void createContents(DomElement *element, WPaintDevice *device) = 0;
  virtual void updateContents(std::vector<DomElement *>& result,
			      WPaintDevice *device) = 0;

protected:
  WWidgetPainter(WPaintedWidget *widget);

  WPaintedWidget *widget_;
};

enum VectorFormat {
  SvgFormat,
  VmlFormat
};

class WWidgetVectorPainter : public WWidgetPainter
{
public:
  WWidgetVectorPainter(WPaintedWidget *widget, VectorFormat format);
  virtual WPaintDevice *createPaintDevice();
  virtual void createContents(DomElement *element, WPaintDevice *device);
  virtual void updateContents(std::vector<DomElement *>& result,
			      WPaintDevice *device); 

private:
  VectorFormat format_;
};

class WWidgetCanvasPainter : public WWidgetPainter
{
public:
  WWidgetCanvasPainter(WPaintedWidget *widget);
  virtual WPaintDevice *createPaintDevice();
  virtual void createContents(DomElement *element, WPaintDevice *device);
  virtual void updateContents(std::vector<DomElement *>& result,
			      WPaintDevice *device); 
};

#ifdef WT_TARGET_JAVA
class WWidgetRasterPainter : public WWidgetPainter
{
public:
  WWidgetRasterPainter(WPaintedWidget *widget);
  virtual WPaintDevice *createPaintDevice();
  virtual void createContents(DomElement *element, WPaintDevice *device);
  virtual void updateContents(std::vector<DomElement *>& result,
			      WPaintDevice *device); 
};
#endif // WT_TARGET_JAVA

WPaintedWidget::WPaintedWidget(WContainerWidget *parent)
  : WInteractWidget(parent),
    preferredMethod_(HtmlCanvas),
    painter_(0),
    needRepaint_(false),
    sizeChanged_(false),
    repaintFlags_(0),
    areaImage_(0),
    renderWidth_(0), renderHeight_(0),
    resized_(this, "resized")
{
  if (WApplication::instance()) {
    const WEnvironment& env = WApplication::instance()->environment();

    if (env.userAgent().find("Opera") != std::string::npos)
      preferredMethod_ = InlineSvgVml;
  }

  setInline(false);

  resized_.connect(SLOT(this, WPaintedWidget::onResize));

  setJavaScriptMember
    ("wtResize",
     "function(self, w, h) {"
     ""  "if (!self.wtWidth || self.wtWidth!=w "
     ""      "|| !self.wtHeight || self.wtHeight!=h) {"
     ""    "self.wtWidth=w; self.wtHeight=h;"
     ""    "self.style.height=h + 'px';"
     + resized_.createCall("w", "h") +
     ""  "}"
     "};");
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
    setJavaScriptMember("wtResize", "");
    resizeCanvas(static_cast<int>(width.toPixels()),
		 static_cast<int>(height.toPixels()));
  }

  WInteractWidget::resize(width, height);
}

void WPaintedWidget::resizeCanvas(int width, int height)
{
  renderWidth_ = width;
  renderHeight_ = height;

  if (areaImage_)
    areaImage_->resize(renderWidth_, renderHeight_);

  sizeChanged_ = true;
  update();
}

void WPaintedWidget::onResize(int width, int height)
{
  resize(WLength::Auto, WLength::Auto);

  resizeCanvas(width, height-5);
}

void WPaintedWidget::update(WFlags<PaintFlag> flags)
{
  needRepaint_ = true;
  repaintFlags_ |= flags;

  WInteractWidget::repaint();
}

void WPaintedWidget::enableAjax()
{
  if (dynamic_cast<WWidgetCanvasPainter *>(painter_)
      && renderWidth_ != 0 && renderHeight_ != 0)
    update();

  WInteractWidget::enableAjax();
}

bool WPaintedWidget::createPainter()
{
  if (painter_)
    return false;

#ifdef WT_TARGET_JAVA
  if (preferredMethod_ == PngImage) {
    painter_ = new WWidgetRasterPainter(this);
    return true;
  }
#endif // WT_TARGET_JAVA

  const WEnvironment& env = WApplication::instance()->environment();

  /*
   * For IE: no choice. Use VML
   */
  if (env.agentIsIE()) {
    painter_ = new WWidgetVectorPainter(this, VmlFormat);
    return true;
  }

  /* Otherwise, combined preferred method with actual capabilities */
  Method method;

  if (env.contentType() != WEnvironment::XHTML1)
    method = HtmlCanvas;
  else 
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

  if (method == InlineSvgVml)
    painter_ = new WWidgetVectorPainter(this, SvgFormat);
  else
    painter_ = new WWidgetCanvasPainter(this);

  return true;
}

DomElementType WPaintedWidget::domElementType() const
{
  return DomElement_DIV;
}

DomElement *WPaintedWidget::createDomElement(WApplication *app)
{
  createPainter();

  DomElement *result = DomElement::createNew(DomElement_DIV);
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

  WPaintDevice *device = painter_->createPaintDevice();

  if (renderWidth_ != 0 && renderHeight_ != 0) {
    paintEvent(device);
#ifdef WT_TARGET_JAVA
    if (device->painter())
      device->painter()->end();
#endif // WT_TARGET_JAVA
  }

  painter_->createContents(canvas, device);
  delete device;

  needRepaint_ = false;

  wrap->addChild(canvas);
  if (wrap != result)
    result->addChild(wrap);

  updateDom(*result, true);

  return result;
}

void WPaintedWidget::updateDom(DomElement& element, bool all)
{
  if (all && areaImage_)
    element.addChild(((WWebWidget *)areaImage_)
		     ->createDomElement(WApplication::instance()));

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

  bool createNew = createPainter();

  if (needRepaint_) {
    WPaintDevice *device = painter_->createPaintDevice();
    if (!createNew)
      device->setPaintFlags(repaintFlags_ & PaintUpdate);
    paintEvent(device);
#ifdef WT_TARGET_JAVA
  if (device->painter())
    device->painter()->end();
#endif // WT_TARGET_JAVA
    if (createNew) {
      DomElement *canvas = DomElement::getForUpdate('p' + id(), DomElement_DIV);
      canvas->removeAllChildren();
      painter_->createContents(canvas, device);
      result.push_back(canvas);
    } else {
      painter_->updateContents(result, device);
    }

    delete device;

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
    areaImage_->setParent(this);

    if (positionScheme() == Static)
      setPositionScheme(Relative);
    areaImage_->setPositionScheme(Absolute);
    areaImage_->setOffsets(0, Left | Top);
    areaImage_->setMargin(0, Top);
    areaImage_->resize(renderWidth_, renderHeight_);
    areaImage_->setPopup(true);
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
					   VectorFormat format)
  : WWidgetPainter(widget),
    format_(format)
{ }

WPaintDevice *WWidgetVectorPainter::createPaintDevice()
{
  if (format_ == SvgFormat)
    return new WSvgImage(widget_->renderWidth_, widget_->renderHeight_);
  else
    return new WVmlImage(widget_->renderWidth_, widget_->renderHeight_);
}

void WWidgetVectorPainter::createContents(DomElement *canvas,
					  WPaintDevice *device)
{
  WVectorImage *vectorDevice = dynamic_cast<WVectorImage *>(device);
  canvas->setProperty(PropertyInnerHTML, vectorDevice->rendered());
}

void WWidgetVectorPainter::updateContents(std::vector<DomElement *>& result,
					  WPaintDevice *device)
{
  WVectorImage *vectorDevice = dynamic_cast<WVectorImage *>(device);

  if (device->paintFlags() & PaintUpdate) {
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
}

/*
 * WWidgetCanvasPainter
 */

WWidgetCanvasPainter::WWidgetCanvasPainter(WPaintedWidget *widget)
  : WWidgetPainter(widget)
{ }

WPaintDevice *WWidgetCanvasPainter::createPaintDevice()
{
  return new WCanvasPaintDevice(widget_->renderWidth_, widget_->renderHeight_);
}

void WWidgetCanvasPainter::createContents(DomElement *result,
					  WPaintDevice *device)
{
  std::string wstr = boost::lexical_cast<std::string>(widget_->renderWidth_);
  std::string hstr = boost::lexical_cast<std::string>(widget_->renderHeight_);

  result->setProperty(PropertyStylePosition, "relative");
  result->setProperty(PropertyStyleOverflowX, "hidden");

  DomElement *canvas = DomElement::createNew(DomElement_CANVAS);
  canvas->setId('c' + widget_->id());
  canvas->setAttribute("width", wstr);
  canvas->setAttribute("height", hstr);
  result->addChild(canvas);

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
}

#ifdef WT_TARGET_JAVA

class WRasterPaintDevice : public WPaintDevice, WResource
{
  enum Format { PngFormat };

  WRasterPaintDevice(Format format, WLength width, WLength height) { };
};

/*
 * WWidgetRasterPainter
 */

WWidgetRasterPainter::WWidgetRasterPainter(WPaintedWidget *widget)
  : WWidgetPainter(widget)
{ }

WPaintDevice *WWidgetRasterPainter::createPaintDevice()
{
  return new WRasterPaintDevice(WRasterPaintDevice::PngFormat,
				widget_->renderWidth_, widget_->renderHeight_);
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

  WRasterPaintDevice *rasterDevice = dynamic_cast<WRasterPaintDevice *>(device);

  img->setAttribute("src", rasterDevice->generateUrl());

  result->addChild(img);
}

void WWidgetRasterPainter::updateContents(std::vector<DomElement *>& result,
					  WPaintDevice *device)
{
  WRasterPaintDevice *rasterDevice = dynamic_cast<WRasterPaintDevice *>(device);

  DomElement *img
    = DomElement::getForUpdate('i' + widget_->id(), DomElement_IMG);

  if (widget_->sizeChanged_) {
    img->setAttribute("width",
		      boost::lexical_cast<std::string>(widget_->renderWidth_));
    img->setAttribute("height",
		      boost::lexical_cast<std::string>(widget_->renderHeight_));
    widget_->sizeChanged_ = false;
  }

  img->setAttribute("src", rasterDevice->generateUrl());

  result.push_back(img);
}

#endif // WT_TARGET_JAVA

}
