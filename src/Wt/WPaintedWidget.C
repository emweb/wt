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

WPaintedWidget::WPaintedWidget(WContainerWidget *parent)
  : WInteractWidget(parent),
    preferredMethod_(HtmlCanvas),
    painter_(0),
    needRepaint_(false),
    areaImage_(0)
{
  if (WApplication::instance()) {
    const WEnvironment& env = WApplication::instance()->environment();

    if (env.userAgent().find("Opera") != std::string::npos)
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
  WInteractWidget::resize(width, height);
  setMinimumSize(width, height);

  if (areaImage_)
    areaImage_->resize(width, height);

  update();
}

void WPaintedWidget::update()
{
  needRepaint_ = true;
  WInteractWidget::repaint();
}

bool WPaintedWidget::createPainter()
{
  if (painter_)
    return false;

  const WEnvironment& env = WApplication::instance()->environment();

  /*
   * For IE: no choice. Use VML
   */
  if (env.agentIE()) {
    painter_ = new WWidgetVectorPainter(this, VmlFormat);
    return true;
  }

  /* Otherwise, combined preferred method with actual capabilities */
  Method method;

  if (!env.javaScript())
    method = InlineSvgVml;
  else
    if (env.contentType() != WEnvironment::XHTML1)
      method = HtmlCanvas;
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

  result->setProperty(PropertyStyleOverflowX, "hidden");

  DomElement *canvas = DomElement::createNew(DomElement_DIV);

  if (!app->environment().agentIsSpiderBot())
    canvas->setId('p' + formName());

  WPaintDevice *device = painter_->createPaintDevice();
  paintEvent(device);
  painter_->createContents(canvas, device);
  delete device;

  needRepaint_ = false;

  result->addChild(canvas);

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

void WPaintedWidget::getDomChanges(std::vector<DomElement *>& result,
				   WApplication *app)
{
  DomElement *e = DomElement::getForUpdate(this, DomElement_DIV);
  updateDom(*e, false);
  result.push_back(e);

  bool createNew = createPainter();

  if (needRepaint_) {
    WPaintDevice *device = painter_->createPaintDevice();
    paintEvent(device);
    if (createNew) {
      DomElement *canvas = DomElement::getForUpdate('p' + formName(),
						    DomElement_DIV);
      canvas->removeAllChildren();
      painter_->createContents(canvas, device);
      result.push_back(canvas);
    } else
      painter_->updateContents(result, device);

    delete device;

    needRepaint_ = false;
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
  return areaImage_ ? areaImage_->areas() : std::vector<WAbstractArea *>();
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
    areaImage_->resize(width(), height());
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
    return new WSvgImage(widget_->width(), widget_->height());
  else
    return new WVmlImage(widget_->width(), widget_->height());
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
  DomElement *canvas = DomElement::getForUpdate('p' + widget_->formName(),
						DomElement_DIV);

  /*
   * In fact, we should use another property, since we could be using
   * document.importNode() instead of myImportNode() since the xml does not
   * need to be interpreted as HTML...
   */
  canvas->setProperty(PropertyInnerHTML, vectorDevice->rendered());
  result.push_back(canvas);
}

/*
 * WWidgetCanvasPainter
 */

WWidgetCanvasPainter::WWidgetCanvasPainter(WPaintedWidget *widget)
  : WWidgetPainter(widget)
{ }

WPaintDevice *WWidgetCanvasPainter::createPaintDevice()
{
  return new WCanvasPaintDevice(widget_->width(), widget_->height());
}

void WWidgetCanvasPainter::createContents(DomElement *result,
					  WPaintDevice *device)
{
  std::string wstr
    = boost::lexical_cast<std::string>(widget_->width().value());
  std::string hstr
    = boost::lexical_cast<std::string>(widget_->height().value());

  result->setProperty(PropertyStylePosition, "relative");

  DomElement *canvas = DomElement::createNew(DomElement_CANVAS);
  canvas->setId('c' + widget_->formName());
  canvas->setAttribute("width", wstr);
  canvas->setAttribute("height", hstr);
  result->addChild(canvas);

  DomElement *text = DomElement::createNew(DomElement_DIV);
  text->setId('t' + widget_->formName());
  text->setProperty(PropertyStylePosition, "absolute");
  text->setProperty(PropertyStyleZIndex,
		    boost::lexical_cast<std::string>(widget_->zIndex() + 1));
  text->setProperty(PropertyStyleTop, "0px");
  text->setProperty(PropertyStyleLeft, "0px");

  WCanvasPaintDevice *canvasDevice = dynamic_cast<WCanvasPaintDevice *>(device);

  canvasDevice->render("c" + widget_->formName(), text);

  result->addChild(text);
}

void WWidgetCanvasPainter::updateContents(std::vector<DomElement *>& result,
					  WPaintDevice *device)
{
  WCanvasPaintDevice *canvasDevice = dynamic_cast<WCanvasPaintDevice *>(device);

  DomElement *canvas = DomElement::getForUpdate('c' + widget_->formName(),
						DomElement_CANVAS);
  canvas->setAttribute("width",
		   boost::lexical_cast<std::string>(widget_->width().value()));
  canvas->setAttribute("height",
		   boost::lexical_cast<std::string>(widget_->height().value()));
  result.push_back(canvas);

  DomElement *text = DomElement::getForUpdate('t' + widget_->formName(),
					      DomElement_DIV);
  text->removeAllChildren();

  canvasDevice->render("c" + widget_->formName(), text);

  result.push_back(text);
}

}
