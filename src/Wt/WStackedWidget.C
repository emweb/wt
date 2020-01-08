/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WStackedWidget.h"

#include "StdWidgetItemImpl.h"

#ifndef WT_DEBUG_JS
#include "js/WStackedWidget.min.js"
#endif

namespace Wt {

WStackedWidget::WStackedWidget()
  : autoReverseAnimation_(false),
    currentIndex_(-1),
    widgetsAdded_(false),
    javaScriptDefined_(false),
    loadAnimateJS_(false)
{
  setOverflow(Overflow::Hidden);
  addStyleClass("Wt-stack");
}

void WStackedWidget::addWidget(std::unique_ptr<WWidget> widget)
{
  WContainerWidget::addWidget(std::move(widget));

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
    return nullptr;
}

void WStackedWidget::insertWidget(int index, std::unique_ptr<WWidget> widget)
{
  WContainerWidget::insertWidget(index, std::move(widget));

  if (currentIndex_ == -1)
    currentIndex_ = 0;

  widgetsAdded_ = true;
}

std::unique_ptr<WWidget> WStackedWidget::removeWidget(WWidget *widget)
{
  auto result = WContainerWidget::removeWidget(widget);

  if (currentIndex_ >= count()) {
    if (count() > 0)
      setCurrentIndex(count() - 1);
    else
      currentIndex_ = -1;
  }

  return result;
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
      doJavaScript(jsRef() + ".wtObj.adjustScroll("
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
      doJavaScript(jsRef() + ".wtObj.setCurrent("
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
			  jsRef() + ".wtObj.animateChild");
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
			jsRef() + ".wtObj.wtResize");
    setJavaScriptMember(WT_GETPS_JS,
			jsRef() + ".wtObj.wtGetPs");

    if (loadAnimateJS_) {
      loadAnimateJS_ = false;
      loadAnimateJS();
    }
  }
}

void WStackedWidget::render(WFlags<RenderFlag> flags)
{
  if (widgetsAdded_ || flags.test(RenderFlag::Full)) {
    for (int i = 0; i < count(); ++i)
      if (!canOptimizeUpdates() || (widget(i)->isHidden() != (currentIndex_ != i)))
	widget(i)->setHidden(currentIndex_ != i);
    widgetsAdded_ = false;
  }

  if (flags.test(RenderFlag::Full)) {
    defineJavaScript();
    if (currentIndex_ >= 0 && isRendered() && javaScriptDefined_)
      doJavaScript(jsRef() + ".wtObj.setCurrent("
		   + widget(currentIndex_)->jsRef() + ");");
  }

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
