/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WViewWidget"

#include "WebSession.h"
#include "DomElement.h"

namespace Wt {

WViewWidget::WViewWidget(WContainerWidget *parent)
  : WWebWidget(parent),
    contents_(0)
{ }

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
  if (needContentsUpdate_ || (flags & RenderFull)) {
    delete contents_; // just to be safe

    WApplication::instance()->setExposeSignals(false);
    contents_ = renderView();
    WApplication::instance()->setExposeSignals(true);

    addChild(contents_);
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
      render(RenderFull);
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
  setIgnoreChildRemoves(true);
  delete contents_;
  contents_ = 0;
  setIgnoreChildRemoves(false);
}

DomElementType WViewWidget::domElementType() const
{
  return isInline() ? DomElement_SPAN : DomElement_DIV;
}

}
