/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WPopupWidget"

#ifndef WT_DEBUG_JS
#include "js/WPopupWidget.min.js"
#endif

namespace Wt {

WPopupWidget::WPopupWidget(WWidget *impl, WObject *parent)
  : WCompositeWidget(),
    anchorWidget_(0),
    orientation_(Vertical),
    transient_(false),
    autoHideDelay_(0),
    hidden_(this),
    shown_(this),
    jsHidden_(impl, "hidden"),
    jsShown_(impl, "shown")
{
  setImplementation(impl);

  if (parent)
    parent->addChild(this);

  hide();
  setPopup(true);
  setPositionScheme(Absolute);

  // This confuses the close button hide ? XXX
  //WApplication::instance()->globalEscapePressed()
  //  .connect(popup_, &WWidget::hide);
  WInteractWidget *iw = dynamic_cast<WInteractWidget *>(impl);
  if (iw) {
    iw->escapePressed().connect(this, &WWidget::hide);
    iw->clicked().preventPropagation();
  }

  jsHidden_.connect(this, &WWidget::hide);
  jsShown_.connect(this, &WWidget::show);

  WApplication::instance()->domRoot()->addWidget(this);
}

void WPopupWidget::setParent(WObject *p)
{
  if (!p || p == WApplication::instance()->domRoot())
    WObject::setParent(p);
}

void WPopupWidget::setAnchorWidget(WWidget *anchorWidget,
				   Orientation orientation)
{
  anchorWidget_ = anchorWidget;
  orientation_ = orientation;
}

void WPopupWidget::setTransient(bool isTransient, int autoHideDelay)
{
  transient_ = isTransient;
  autoHideDelay_ = autoHideDelay;
}

void WPopupWidget::setHidden(bool hidden, const WAnimation& animation)
{
  if (WWebWidget::canOptimizeUpdates() && hidden == isHidden())
    return;

  WCompositeWidget::setHidden(hidden, animation);

  if (!hidden && anchorWidget_)
    positionAt(anchorWidget_, orientation_);

  if (hidden)
    this->hidden().emit();
  else
    this->shown().emit();

  if (!WWebWidget::canOptimizeUpdates() || isRendered()) {
    if (hidden)
      doJavaScript("var o = jQuery.data(" + jsRef() + ", 'popup');"
		   "if (o) o.hidden();");
    else
      doJavaScript("var o = jQuery.data(" + jsRef() + ", 'popup');"
		   "if (o) o.shown();");
  }
}

void WPopupWidget::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WPopupWidget.js", "WPopupWidget", wtjs1);

  WStringStream jsObj;
  jsObj << "new " WT_CLASS ".WPopupWidget("
	<< app->javaScriptClass() << ',' << jsRef() << ','
	<< transient_ << ',' << autoHideDelay_ << ");";

  setJavaScriptMember(" WPopupWidget", jsObj.str());
}

void WPopupWidget::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJavaScript();

  WCompositeWidget::render(flags);
}

}
