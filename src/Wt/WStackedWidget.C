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
    currentIndex_(-1),
    widgetsAdded_(false),
    javaScriptDefined_(false),
    loadAnimateJS_(false)
{
  addStyleClass("Wt-stack");
}

void WStackedWidget::addWidget(WWidget *widget)
{
  WContainerWidget::addWidget(widget);

  if (currentIndex_ == -1)
    currentIndex_ = 0;

  widgetsAdded_ = true;
}

int WStackedWidget::currentIndex() const
{
  return currentIndex_;
}

WWidget *WStackedWidget::currentWidget() const
{
  if (currentIndex_ >= 0 && currentIndex_ < count())
    return widget(currentIndex_);
  else
    return 0;
}

void WStackedWidget::insertWidget(int index, WWidget *widget)
{
  WContainerWidget::insertWidget(index, widget);

  if (currentIndex_ == -1)
    currentIndex_ = 0;

  widgetsAdded_ = true;
}

void WStackedWidget::removeChild(WWidget *child)
{
  WContainerWidget::removeChild(child);

  if (currentIndex_ >= count()) {
    if (count() > 0)
      setCurrentIndex(count() - 1);
    else
      currentIndex_ = -1;
  }
}

void WStackedWidget::setTransitionAnimation(const WAnimation& animation,
					    bool autoReverse)
{
  if (WApplication::instance()->environment().supportsCss3Animations()) {
    if (!animation.empty())
      addStyleClass("Wt-animated");

    animation_ = animation;
    autoReverseAnimation_ = autoReverse;

    loadAnimateJS();
  }
}

void WStackedWidget::setCurrentIndex(int index)
{
  setCurrentIndex(index, animation_, autoReverseAnimation_);
}

void WStackedWidget::setCurrentIndex(int index, const WAnimation& animation,
				     bool autoReverse)
{
  if (!animation.empty() && 
      WApplication::instance()->environment().supportsCss3Animations() &&
      ((isRendered() && javaScriptDefined_) || !canOptimizeUpdates())) {
    if (canOptimizeUpdates() && index == currentIndex_)
      return;

    loadAnimateJS();

    WWidget *previous = currentWidget();

    if (previous)
      doJavaScript("$('#" + id() + "').data('obj').adjustScroll("
		   + previous->jsRef() + ");");

    setJavaScriptMember("wtAutoReverse", autoReverse ? "true" : "false");

    if (previous)
      previous->animateHide(animation);

    widget(index)->animateShow(animation);

    currentIndex_ = index;
  } else {
    currentIndex_ = index;

    for (int i = 0; i < count(); ++i)
      if (!canOptimizeUpdates() || (widget(i)->isHidden() != (currentIndex_ != i)))
	widget(i)->setHidden(currentIndex_ != i);

    if (currentIndex_ >= 0 && isRendered() && javaScriptDefined_)
      doJavaScript("$('#" + id() + "').data('obj').setCurrent("
		   + widget(currentIndex_)->jsRef() + ");");
  }
}

void WStackedWidget::loadAnimateJS()
{
  if (!loadAnimateJS_) {
    loadAnimateJS_ = true;
    if (javaScriptDefined_) {
      LOAD_JAVASCRIPT(WApplication::instance(), "js/WStackedWidget.js",
		      "WStackedWidget.prototype.animateChild", wtjs2);
      setJavaScriptMember("wtAnimateChild",
			  "$('#" + id() + "').data('obj').animateChild");
      setJavaScriptMember("wtAutoReverse",
			  autoReverseAnimation_ ? "true" : "false");
    }
  }
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

    if (loadAnimateJS_) {
      loadAnimateJS_ = false;
      loadAnimateJS();
    }
  }
}

void WStackedWidget::render(WFlags<RenderFlag> flags)
{
  if (widgetsAdded_ || (flags & RenderFull)) {
    for (int i = 0; i < count(); ++i)
      if (!canOptimizeUpdates() || (widget(i)->isHidden() != (currentIndex_ != i)))
	widget(i)->setHidden(currentIndex_ != i);
    widgetsAdded_ = false;
  }

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
