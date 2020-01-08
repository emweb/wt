/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WViewWidget.h"

#include "WebSession.h"
#include "DomElement.h"

namespace Wt {

WViewWidget::WViewWidget()
{ }

WViewWidget::~WViewWidget()
{
  manageWidget(contents_, std::unique_ptr<WWidget>());
}

void WViewWidget::load()
{
  update();

  WWebWidget::load();
}

void WViewWidget::update()
{
  needContentsUpdate_ = true;
  if (isRendered())
    scheduleRender();
}

void WViewWidget::refresh()
{
  if (!contents_)
    update();
}

void WViewWidget::render(WFlags<RenderFlag> flags)
{
  if (needContentsUpdate_ || (flags.test(RenderFlag::Full))) {
    WApplication::instance()->setExposeSignals(false);
    contents_ = renderView();
    widgetAdded(contents_.get());
    WApplication::instance()->setExposeSignals(true);

    contents_->render(flags); // it may affect isInline(), e.g. WText
    setInline(contents_->isInline());

    needContentsUpdate_ = false;
  }

  WWebWidget::render(flags);
}

void WViewWidget::updateDom(DomElement& element, bool all)
{
  WApplication *app = WApplication::instance();
  
  if (!app->session()->renderer().preLearning()) {
    if (all && !contents_) {
      needContentsUpdate_ = true;
      render(RenderFlag::Full);
    }

    if (contents_) {
      bool savedVisibleOnly = app->session()->renderer().visibleOnly();

      WApplication::instance()->session()->renderer().setVisibleOnly(false);

      DomElement *e = contents_->createSDomElement(WApplication::instance());

      if (!all)
	element.setWasEmpty(true); // removes previous content
      element.addChild(e);

      WApplication::instance()->session()->renderer()
	.setVisibleOnly(savedVisibleOnly);

      needContentsUpdate_ = false;
    }
  }

  WWebWidget::updateDom(element, all);
}

void WViewWidget::propagateRenderOk(bool deep)
{
  needContentsUpdate_ = false;

  WWebWidget::propagateRenderOk(deep);
}

void WViewWidget::doneRerender()
{
  if(contents_){
    widgetRemoved(contents_.get(), false);
    contents_.reset();
  }
}

DomElementType WViewWidget::domElementType() const
{
  return isInline() ? DomElementType::SPAN : DomElementType::DIV;
}

}
