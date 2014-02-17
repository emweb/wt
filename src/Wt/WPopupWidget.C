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
    fakeParent_(0),
    anchorWidget_(0),
    orientation_(Vertical),
    transient_(false),
    autoHideDelay_(0),
    deleteWhenHidden_(false),
    hidden_(this),
    shown_(this),
    jsHidden_(impl, "hidden"),
    jsShown_(impl, "shown")
{
  setImplementation(impl);

  if (parent)
    parent->addChild(this);

  WApplication::instance()->addGlobalWidget(this);

  hide();
  setPopup(true);
  setPositionScheme(Absolute);

  jsHidden_.connect(this, &WWidget::hide);
  jsShown_.connect(this, &WWidget::show);
}

WPopupWidget::~WPopupWidget()
{
  if (fakeParent_)
    fakeParent_->WObject::removeChild(this);

  WApplication::instance()->removeGlobalWidget(this);
}

void WPopupWidget::setParent(WObject *p)
{
  /*
   * We will only register the dom root as parent, since this
   * is required for rendering.
   */
  if (!p || p == WApplication::instance()->domRoot()) {
    if (!p)
      fakeParent_ = 0;
    WObject::setParent(p);
  } else if (p)
    fakeParent_ = p;
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
  if (isRendered()) {
    WStringStream ss;
    ss << "jQuery.data(" << jsRef() << ", 'popup').setTransient("
       << transient_ << ',' << autoHideDelay_ << ");";
    doJavaScript(ss.str());
  }
}

void WPopupWidget::setDeleteWhenHidden(bool enable)
{
  deleteWhenHidden_ = enable;
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

  if (!WWebWidget::canOptimizeUpdates() && hidden && deleteWhenHidden_)
    delete this;
}

void WPopupWidget::defineJS()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WPopupWidget.js", "WPopupWidget", wtjs1);

  WStringStream jsObj;
  jsObj << "new " WT_CLASS ".WPopupWidget("
	<< app->javaScriptClass() << ',' << jsRef() << ','
	<< transient_ << ',' << autoHideDelay_ << ','
	<< !isHidden() << ");";

  setJavaScriptMember(" WPopupWidget", jsObj.str());
}

void WPopupWidget::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJS();

  WCompositeWidget::render(flags);
}

}
