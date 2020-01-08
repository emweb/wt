/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WPopupWidget.h"

#ifndef WT_DEBUG_JS
#include "js/WPopupWidget.min.js"
#endif

namespace Wt {

WPopupWidget::WPopupWidget(std::unique_ptr<WWidget> impl)
  : anchorWidget_(nullptr),
    orientation_(Orientation::Vertical),
    transient_(false),
    autoHideDelay_(0),
    jsHidden_(impl.get(), "hidden"),
    jsShown_(impl.get(), "shown")
{
  setImplementation(std::move(impl));

  WApplication::instance()->addGlobalWidget(this);

  hide();
  setPopup(true);
  setPositionScheme(PositionScheme::Absolute);

  jsHidden_.connect(this, &WWidget::hide);
  jsShown_.connect(this, &WWidget::show);

  WApplication::instance()->internalPathChanged().connect(this, &WPopupWidget::onPathChange);
}

WPopupWidget::~WPopupWidget()
{
  WApplication::instance()->removeGlobalWidget(this);
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
    ss << jsRef() << ".wtPopup.setTransient("
       << transient_ << ',' << autoHideDelay_ << ");";
    doJavaScript(ss.str());
  }
}

void WPopupWidget::onPathChange()
{
  hide();
}

void WPopupWidget::setHidden(bool hidden, const WAnimation& animation)
{
  if (WWebWidget::canOptimizeUpdates() && hidden == isHidden())
    return;

  WCompositeWidget::setHidden(hidden, animation);

  if (!hidden && anchorWidget_)
    positionAt(anchorWidget_.get(), orientation_);

  if (hidden)
    this->hidden().emit();
  else
    this->shown().emit();

  if (!WWebWidget::canOptimizeUpdates() || isRendered()) {
    if (hidden)
      doJavaScript("var o = " + jsRef() + ";"
		   "if (o && o.wtPopup) o.wtPopup.hidden();");
    else
      doJavaScript("var o = " + jsRef() + ";"
		   "if (o && o.wtPopup) o.wtPopup.shown();");
  }
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
  if (flags.test(RenderFlag::Full))
    defineJS();

  WCompositeWidget::render(flags);
}

}
