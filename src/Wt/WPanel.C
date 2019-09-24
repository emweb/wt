/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WIconPair.h>
#include <Wt/WPanel.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTheme.h>

#include "StdWidgetItemImpl.h"

namespace Wt {

WPanel::WPanel()
  : collapseIcon_(nullptr),
    title_(nullptr),
    centralWidget_(nullptr)
{
  const char *TEMPLATE =
    "${titlebar}"
    "${contents}";

  impl_ = new WTemplate(WString::fromUTF8(TEMPLATE));
  setImplementation(std::unique_ptr<WWidget>(impl_));

  implementStateless(&WPanel::doExpand, &WPanel::undoExpand);
  implementStateless(&WPanel::doCollapse, &WPanel::undoCollapse);

  WApplication *app = WApplication::instance();

  std::unique_ptr<WContainerWidget> centralArea(new WContainerWidget());
  app->theme()->apply(this, centralArea.get(), PanelBody);

  impl_->bindEmpty("titlebar");
  impl_->bindWidget("contents", std::move(centralArea));

  setJavaScriptMember
    (WT_RESIZE_JS,
     "function(self, w, h, s) {"
     """var hdefined = h >= 0;"
     """if (hdefined) {"
     ""  "var mh = " WT_CLASS ".px(self, 'maxHeight');"
     ""  "if (mh > 0) h = Math.min(h, mh);"
     """}"
     """if (" WT_CLASS ".boxSizing(self)) {"
     ""  "h -= " WT_CLASS ".px(self, 'borderTopWidth') + "
     ""       WT_CLASS ".px(self, 'borderBottomWidth');"
     """}"
     """var c = self.lastChild;"
     """var t = c.previousSibling;"
     """if (t)"
     ""  "h -= t.offsetHeight;"
     """h -= 8;" // padding
     """if (hdefined && h > 0) {"
     ""  "c.lh = true;"
     ""  "c.style.height = h + 'px';"
     // this seems golden, but, JQuery docs say it doesn't work when
     // the panel is indirectly hidden: will this back-fire ?
     ""  "$(c).children().each(function() { "
     ""      "var self = $(this), "
     ""          "padding = self.outerHeight() - self.height();"
     ""      "self.height(h - padding);"
     ""      "this.lh = true;"
     ""  "});"
     """} else {"
     ""  "c.style.height = '';"
     ""  "c.lh = false;"
     ""  "$(c).children().each(function() { "
     ""    "this.style.height = '';"
     ""    "this.lh = false;"
     ""  "});"
     """}"
     "};");

  setJavaScriptMember(WT_GETPS_JS, StdWidgetItemImpl::secondGetPSJS());
}

void WPanel::setTitle(const WString& title)
{
  setTitleBar(true);

  if (!title_) {
    title_ = titleBarWidget()->addWidget(cpp14::make_unique<WText>());
    WApplication *app = WApplication::instance();
    app->theme()->apply(this, title_, PanelTitle);
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
  return titleBarWidget() != nullptr;
}

WContainerWidget *WPanel::titleBarWidget() const
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("titlebar"));
}

void WPanel::setTitleBar(bool enable)
{
  if (enable && !titleBarWidget()) {
    auto titleBar = impl_->bindWidget("titlebar", cpp14::make_unique<WContainerWidget>());

    WApplication *app = WApplication::instance();
    app->theme()->apply(this, titleBar, PanelTitleBar);
  } else if (!enable && titleBar()) {
    impl_->bindEmpty("titlebar");
    title_ = nullptr;
    collapseIcon_ = nullptr;
  }
}

void WPanel::setCollapsible(bool on)
{
  if (on && !collapseIcon_) {
    std::string resources = WApplication::relativeResourcesUrl();

    setTitleBar(true);
    std::unique_ptr<WIconPair> icon
      (collapseIcon_ = new WIconPair(resources + "collapse.gif",
				     resources + "expand.gif"));
    collapseIcon_->setFloatSide(Side::Left);
    
    WApplication *app = WApplication::instance();
    app->theme()->apply(this, collapseIcon_, PanelCollapseButton);
    titleBarWidget()->insertWidget(0, std::move(icon));

    collapseIcon_->icon1Clicked().connect(this, &WPanel::doCollapse);
    collapseIcon_->icon1Clicked().connect(this, &WPanel::onCollapse);
    collapseIcon_->icon1Clicked().preventPropagation();
    collapseIcon_->icon2Clicked().connect(this, &WPanel::doExpand);
    collapseIcon_->icon2Clicked().connect(this, &WPanel::onExpand);
    collapseIcon_->icon2Clicked().preventPropagation();
    collapseIcon_->setState(isCollapsed() ? 1 : 0);

    titleBarWidget()->clicked().connect(this, &WPanel::toggleCollapse);

  } else if (!on && collapseIcon_) {
    titleBarWidget()->removeWidget(collapseIcon_);
    collapseIcon_ = nullptr;
  }
}

void WPanel::toggleCollapse()
{
  setCollapsed(!isCollapsed());

  if(isCollapsed())
    collapsed_.emit();
  else
    expanded_.emit();
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

void WPanel::setCentralWidget(std::unique_ptr<WWidget> w)
{
  if (centralWidget_) {
    centralArea()->removeWidget(centralWidget_);
    centralWidget_ = nullptr;
  }
    
  if (w) {
    centralWidget_ = w.get();
    centralWidget_->setInline(false);
    centralArea()->addWidget(std::move(w));
  }
}

WContainerWidget *WPanel::centralArea() const
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("contents"));
}

}
