/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WScrollArea"
#include "Wt/WScrollBar"
#include "DomElement.h"

namespace Wt {

WScrollArea::WScrollArea(WContainerWidget *parent)
  : WWebWidget(parent),
    widget_(0),
    widgetChanged_(false),
    scrollBarChanged_(false),
    horizontalScrollBarPolicy_(ScrollBarAsNeeded),
    verticalScrollBarPolicy_(ScrollBarAsNeeded),
    scrollBarPolicyChanged_(false)
{
  setInline(false);

  horizontalScrollBar_ = new WScrollBar(this, Horizontal);
  verticalScrollBar_ = new WScrollBar(this, Vertical);
}

WScrollArea::~WScrollArea()
{
  delete horizontalScrollBar_;
  delete verticalScrollBar_;
}

void WScrollArea::scrollBarChanged()
{
  scrollBarChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WScrollArea::setWidget(WWidget *widget)
{
  delete widget_;

  widget_ = widget;
  widgetChanged_ = true;
  repaint(RepaintInnerHtml);

  if (widget)
    widget->setParentWidget(this);
}

WWidget *WScrollArea::takeWidget()
{
  WWidget *result = widget_;
  widget_ = 0;

  setWidget(0);

  if (result)
    result->setParentWidget(0);

  return result;
}

void WScrollArea::setHorizontalScrollBarPolicy(ScrollBarPolicy policy)
{
  horizontalScrollBarPolicy_ = policy;
  scrollBarPolicyChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WScrollArea::setVerticalScrollBarPolicy(ScrollBarPolicy policy)
{
  verticalScrollBarPolicy_ = policy;
  scrollBarPolicyChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WScrollArea::setScrollBarPolicy(ScrollBarPolicy policy)
{
  horizontalScrollBarPolicy_ = verticalScrollBarPolicy_ = policy;
  scrollBarPolicyChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WScrollArea::updateDom(DomElement& element, bool all)
{
  if (all)
    if (isInLayout() && WApplication::instance()->environment().ajax()) {
      setJavaScriptMember("wtResize",
			  "function(s, w, h) {"
			  "s.style.width=w+'px';"
			  "s.style.height=h+'px';"
			  "}");
    }

  if (widgetChanged_ || all) {
    if (widget_)
      element.addChild(widget_->webWidget()
		       ->createDomElement(WApplication::instance()));

    widgetChanged_ = false;
  }

  if (scrollBarChanged_ || all) {
    if ((horizontalScrollBar_->tiesChanged_)
	|| (verticalScrollBar_->tiesChanged_)) {
      horizontalScrollBar_->tiesChanged_ = true;
      verticalScrollBar_->tiesChanged_ = true;
    }
    horizontalScrollBar_->updateDom(element, all);
    verticalScrollBar_->updateDom(element, all);

    scrollBarChanged_ = false;
  }

  if (scrollBarPolicyChanged_ || all) {
    switch (horizontalScrollBarPolicy_) {
    case ScrollBarAsNeeded:
      element.setProperty(Wt::PropertyStyleOverflowX, "auto");
      break;
    case ScrollBarAlwaysOff:
      element.setProperty(Wt::PropertyStyleOverflowX, "hidden");
      break;
    case ScrollBarAlwaysOn:
      element.setProperty(Wt::PropertyStyleOverflowX, "scroll");
      break;
    }

    switch (verticalScrollBarPolicy_) {
    case ScrollBarAsNeeded:
      element.setProperty(Wt::PropertyStyleOverflowY, "auto");
      break;
    case ScrollBarAlwaysOff:
      element.setProperty(Wt::PropertyStyleOverflowY, "hidden");
      break;
    case ScrollBarAlwaysOn:
      element.setProperty(Wt::PropertyStyleOverflowY, "scroll");
      break;
    }

    scrollBarPolicyChanged_ = false;
  }    

  WWebWidget::updateDom(element, all);
}

DomElementType WScrollArea::domElementType() const
{
  return DomElement_DIV;
}

}
