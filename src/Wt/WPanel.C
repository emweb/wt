/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WIconPair>
#include <Wt/WPanel>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WTheme>

#include "StdWidgetItemImpl.h"

namespace Wt {

WPanel::WPanel(WContainerWidget *parent)
  : WCompositeWidget(parent),
    collapseIcon_(0),
    title_(0),
    centralWidget_(0),
    collapsed_(this),
    expanded_(this),
    collapsedSS_(this),
    expandedSS_(this)
{
  const char *TEMPLATE =
    "${titlebar}"
    "${contents}";

  setImplementation(impl_ = new WTemplate(WString::fromUTF8(TEMPLATE)));

  implementStateless(&WPanel::doExpand, &WPanel::undoExpand);
  implementStateless(&WPanel::doCollapse, &WPanel::undoCollapse);

  WApplication *app = WApplication::instance();

  WContainerWidget *centralArea = new WContainerWidget();
  app->theme()->apply(this, centralArea, PanelBodyRole);

  impl_->bindWidget("titlebar", 0);
  impl_->bindWidget("contents", centralArea);

  setJavaScriptMember
    (WT_RESIZE_JS,
     "function(self, w, h, l) {"
     """var defined = h >= 0;"
     """if (defined) {"
     ""  "var mh = " WT_CLASS ".px(self, 'maxHeight');"
     ""  "if (mh > 0) h = Math.min(h, mh);"
     """}"
     """if (" WT_CLASS ".boxSizing(self)) {"
     ""  "h -= " WT_CLASS ".px(self, 'borderTopWidth') + "
     ""       WT_CLASS ".px(self, 'borderBottomWidth');"
     """}"
     """var c = self.lastChild;"
     """var t = c.previousSibling;"
     """if (t && t.className == 'titlebar')"
     ""  "h -= t.offsetHeight;"
     """h -= 8;" // padding
     """if (defined && h > 0) {"
     ""  "c.lh = l;"
     ""  "c.style.height = h + 'px';"
     // this seems golden, but, JQuery docs say it doesn't work when
     // the panel is indirectly hidden: will this back-fire ?
     ""  "$(c).children().each(function() { "
     ""      "var self = $(this), "
     ""          "padding = self.outerHeight() - self.height();"
     ""      "self.height(h - padding);"
     ""      "this.lh = l;"
     ""  "});"
     """} else {"
     ""  "c.lh = false;"
     ""  "c.style.height = '';"
     ""  "$(c).children().each(function() { "
     ""      "this.style.height = '';"
     ""      "this.lh = false;"
     ""  "});"
     """}"
     "};");

  setJavaScriptMember(WT_GETPS_JS, StdWidgetItemImpl::secondGetPSJS());
}

void WPanel::setTitle(const WString& title)
{
  setTitleBar(true);

  if (!title_) {
    title_ = new WText();
    WApplication *app = WApplication::instance();
    app->theme()->apply(this, title_, PanelTitleRole);
    titleBarWidget()->insertWidget(titleBarWidget()->count(), title_);
  }

  title_->setText(title);
}

WString WPanel::title() const
{
  if (title_)
    return title_->text();
  else
    return WString();
}

bool WPanel::titleBar() const
{
  return titleBarWidget() != 0;
}

WContainerWidget *WPanel::titleBarWidget() const
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("titlebar"));
}

void WPanel::setTitleBar(bool enable)
{
  if (enable && !titleBarWidget()) {
    WContainerWidget *titleBar = new WContainerWidget();
    impl_->bindWidget("titlebar", titleBar);

    WApplication *app = WApplication::instance();
    app->theme()->apply(this, titleBar, PanelTitleBarRole);
  } else if (!enable && titleBar()) {
    impl_->bindWidget("titlebar", 0);
    title_ = 0;
    collapseIcon_ = 0;
  }
}

void WPanel::setCollapsible(bool on)
{
  if (on && !collapseIcon_) {
    std::string resources = WApplication::relativeResourcesUrl();

    setTitleBar(true);
    collapseIcon_ = new WIconPair(resources + "collapse.gif",
				  resources + "expand.gif");
    collapseIcon_->setFloatSide(Left);
    
    WApplication *app = WApplication::instance();
    app->theme()->apply(this, collapseIcon_, PanelCollapseButtonRole);

    titleBarWidget()->insertWidget(0, collapseIcon_);

    collapseIcon_->icon1Clicked().connect(this, &WPanel::doCollapse);
    collapseIcon_->icon1Clicked().connect(this, &WPanel::onCollapse);
    collapseIcon_->icon1Clicked().preventPropagation();
    collapseIcon_->icon2Clicked().connect(this, &WPanel::doExpand);
    collapseIcon_->icon2Clicked().connect(this, &WPanel::onExpand);
    collapseIcon_->icon2Clicked().preventPropagation();
    collapseIcon_->setState(isCollapsed() ? 1 : 0);

    titleBarWidget()->clicked().connect(this, &WPanel::toggleCollapse);

  } else if (!on && collapseIcon_) {
    delete collapseIcon_;
    collapseIcon_ = 0;
  }
}

void WPanel::toggleCollapse()
{
  setCollapsed(!isCollapsed());
}

void WPanel::setCollapsed(bool on)
{
  if (on)
    collapse();
  else
    expand();
}

bool WPanel::isCollapsed() const
{
  return centralArea()->isHidden();
}

void WPanel::collapse()
{
  if (isCollapsible()) {
    collapseIcon_->showIcon2();

    doCollapse();
  }
}

void WPanel::expand()
{
  if (isCollapsible()) {
    collapseIcon_->showIcon1();

    doExpand();
  }
}

void WPanel::setAnimation(const WAnimation& transition)
{
  animation_ = transition;

  if (!animation_.empty())
    addStyleClass("Wt-animated");
}

void WPanel::doCollapse()
{
  wasCollapsed_ = isCollapsed();

  centralArea()->animateHide(animation_);

  collapsedSS_.emit(true);
}

void WPanel::doExpand()
{
  wasCollapsed_ = isCollapsed();

  centralArea()->animateShow(animation_);

  expandedSS_.emit(true);
}

void WPanel::undoCollapse()
{
  if (!wasCollapsed_)
    expand();

  collapsedSS_.emit(false);
}

void WPanel::undoExpand()
{
  if (wasCollapsed_)
    collapse();

  expandedSS_.emit(false);
}

void WPanel::onCollapse()
{
  collapsed_.emit();
}

void WPanel::onExpand()
{
  expanded_.emit();
}

void WPanel::setCentralWidget(WWidget * w)
{
  delete centralWidget_;
  centralWidget_ = w;

  if (w) {
    centralArea()->addWidget(w);
    w->setInline(false);
  }
}

WContainerWidget *WPanel::centralArea() const
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("contents"));
}

}
