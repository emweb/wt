/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WStackedWidget"

#include "StdWidgetItemImpl.h"

#ifndef WT_DEBUG_JS
#include "js/WStackedWidget.min.js"
#endif

namespace Wt {

WStackedWidget::WStackedWidget(WContainerWidget *parent)
  : WContainerWidget(parent),
    currentIndex_(-1)
{
  WT_DEBUG( setObjectName("WStackedWidget") );

  addStyleClass("Wt-stack");

  javaScriptDefined_ = false;
}

void WStackedWidget::addWidget(WWidget *widget)
{
  WContainerWidget::addWidget(widget);

  if (currentIndex_ == -1)
    currentIndex_ = 0;
  else
    widget->hide();
}

int WStackedWidget::currentIndex() const
{
  return currentIndex_;
}

WWidget *WStackedWidget::currentWidget() const
{
  if (currentIndex() >= 0)
    return widget(currentIndex());
  else
    return 0;
}

void WStackedWidget::insertWidget(int index, WWidget *widget)
{
  WContainerWidget::insertWidget(index, widget);

  if (currentIndex_ == -1)
    currentIndex_ = 0;
  else
    widget->hide();
}

void WStackedWidget::removeChild(WWidget *child)
{
  WContainerWidget::removeChild(child);

  if (currentIndex_ >= count()) {
    currentIndex_ = -1;
    if (count() > 0)
      setCurrentIndex(count() - 1);
  }
}

void WStackedWidget::setTransitionAnimation(const WAnimation& animation,
					    bool autoReverse)
{
  if (loadAnimateJS()) {
    if (!animation.empty())
      addStyleClass("Wt-animated");

    animation_ = animation;
    autoReverseAnimation_ = autoReverse;

    if (isRendered()) {
      setJavaScriptMember("wtAnimateChild",
			  "$('#" + id() + "').data('obj').animateChild");
      setJavaScriptMember("wtAutoReverse",
			  autoReverseAnimation_ ? "true" : "false");
    }
  }
}

void WStackedWidget::setCurrentIndex(int index)
{
  setCurrentIndex(index, animation_, autoReverseAnimation_);
}

void WStackedWidget::setCurrentIndex(int index, const WAnimation& animation,
				     bool autoReverse)
{
  if (!animation.empty() && loadAnimateJS()
      && (isRendered() || !canOptimizeUpdates())) {
    if (canOptimizeUpdates() && index == currentIndex_)
      return;

    WWidget *previous = currentWidget();

    setJavaScriptMember("wtAutoReverse", autoReverse ? "true" : "false");

    if (previous)
      previous->animateHide(animation);
    widget(index)->animateShow(animation);

    currentIndex_ = index;
  } else {
    currentIndex_ = index;

    for (int i = 0; i < count(); ++i)
      if (widget(i)->isHidden() != (currentIndex_ != i))
	widget(i)->setHidden(currentIndex_ != i);

    if (isRendered())
      doJavaScript("$('#" + id() + "').data('obj').setCurrent("
		   + widget(currentIndex_)->jsRef() + ");");
  }
}

bool WStackedWidget::loadAnimateJS()
{
  WApplication *app = WApplication::instance();

  if (app->environment().supportsCss3Animations()) {
    LOAD_JAVASCRIPT(app, "js/WStackedWidget.js",
		    "WStackedWidget.prototype.animateChild", wtjs2);

    return true;
  } else
    return false;
}

void WStackedWidget::defineJavaScript()
{
  if (!javaScriptDefined_) {
    javaScriptDefined_ = true;
    WApplication *app = WApplication::instance();

    LOAD_JAVASCRIPT(app, "js/WStackedWidget.js", "WStackedWidget", wtjs1);

    setJavaScriptMember(" WStackedWidget", "new " WT_CLASS ".WStackedWidget("
			+ app->javaScriptClass() + "," + jsRef() + ");");

    setJavaScriptMember(WT_RESIZE_JS,
			"$('#" + id() + "').data('obj').wtResize");
    setJavaScriptMember(WT_GETPS_JS,
			"$('#" + id() + "').data('obj').wtGetPs");

    if (!animation_.empty()) {
      loadAnimateJS();

      setJavaScriptMember("wtAnimateChild",
			  "$('#" + id() + "').data('obj').animateChild");
      setJavaScriptMember("wtAutoReverse",
			  autoReverseAnimation_ ? "true" : "false");
    }
  }
}

void WStackedWidget::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJavaScript();

  WContainerWidget::render(flags);
}

void WStackedWidget::setCurrentWidget(WWidget *widget)
{
  setCurrentIndex(indexOf(widget));
}

DomElement *WStackedWidget::createDomElement(WApplication *app)
{
  return WContainerWidget::createDomElement(app);
}

void WStackedWidget::getDomChanges(std::vector<DomElement *>& result,
				   WApplication *app)
{
  WContainerWidget::getDomChanges(result, app);
}

}
