/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WPushButton.h>
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

  std::unique_ptr<WContainerWidget> centralArea(new WContainerWidget());

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
     // the panel is indirectly hidden: will this back-fire ?
     ""  "c.querySelectorAll(':scope > *').forEach(function(self) { "
     ""      "let padding = self.getBoundingClientRect().height - " WT_CLASS ".px(self, 'height');"
     ""      "self.style.height = (h - padding) + 'px';"
     ""      "self.lh = true;"
     ""  "});"
     """} else {"
     ""  "c.style.height = '';"
     ""  "c.lh = false;"
     ""  "for (const child of c.children) {"
     ""      "child.style.height = '';"
     ""      "child.lh = false;"
     ""  "}"
     """}"
     "};");

  setJavaScriptMember(WT_GETPS_JS, StdWidgetItemImpl::secondGetPSJS());
}

void WPanel::setTitle(const WString& title)
{
  setTitleBar(true);

  if (!title_) {
    title_ = titleBarWidget()->addWidget(std::make_unique<WText>());
  }

  auto text = dynamic_cast<WText*>(title_);
  auto button = dynamic_cast<WPushButton*>(title_);
  if (text) {
    text->setText(title);
  } else if (button) {
    button->setText(title);
  }

  auto app = WApplication::instance();
  app->theme()->apply(this, title_, PanelTitle);
  app->theme()->apply(this, titleBarWidget(), PanelTitleBar);
}

WString WPanel::title() const
{
  auto text = dynamic_cast<WText*>(title_);
  auto button = dynamic_cast<WPushButton*>(title_);
  if (text) {
    return text->text();
  } else if (button) {
    return button->text();
  } else {
    return WString();
  }
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
  if (enable && !titleBar()) {
    impl_->bindWidget("titlebar", std::make_unique<WContainerWidget>());
  } else if (!enable && titleBar()) {
    if (isCollapsible()) {
      setCollapsible(false);
    }
    impl_->bindEmpty("titlebar");
    title_ = nullptr;
  }
}

void WPanel::setCollapsible(bool on)
{
  toggleStyleClass("Wt-collapsible", on);

  if (on && !isCollapsible()) {
    std::string resources = WApplication::relativeResourcesUrl();

    setTitleBar(true);

    auto app = WApplication::instance();

    auto icon = std::make_unique<WIconPair>(resources + "collapse.gif",
                                            resources + "expand.gif");

    collapseIcon_ = icon.get();
    if (app->theme()->panelCollapseIconSide() == Side::Left) {
      titleBarWidget()->insertWidget(0, std::move(icon));
    } else {
      titleBarWidget()->addWidget(std::move(icon));
    }

    collapseIcon_->icon1Clicked().connect(this, &WPanel::doCollapse);
    collapseIcon_->icon1Clicked().connect(this, &WPanel::onCollapse);
    collapseIcon_->icon1Clicked().preventPropagation();
    collapseIcon_->icon2Clicked().connect(this, &WPanel::doExpand);
    collapseIcon_->icon2Clicked().connect(this, &WPanel::onExpand);
    collapseIcon_->icon2Clicked().preventPropagation();
    collapseIcon_->setState(isCollapsed() ? 1 : 0);

    app->theme()->apply(this, collapseIcon_, PanelCollapseButton);

    if (app->environment().ajax()) {
      // Prevent the title bar from turning into a <button> when JS is not available.
      // We can click the collapse icon instead.
      titleBarWidget()->clicked().connect(this, &WPanel::toggleCollapse);
    }
  } else if (!on && isCollapsible()) {
    if (isCollapsed()) {
      setCollapsed(false);
    }

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

  addStyleClass("Wt-collapsed", true);
  centralArea()->animateHide(animation_);

  collapsedSS_.emit(true);
}

void WPanel::doExpand()
{
  wasCollapsed_ = isCollapsed();

  removeStyleClass("Wt-collapsed", true);
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

    auto app = WApplication::instance();
    app->theme()->apply(this, centralArea(), PanelBody);
    app->theme()->apply(this, centralWidget_, PanelBodyContent);
  }
}

WContainerWidget *WPanel::centralArea() const
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("contents"));
}

void WPanel::enableAjax()
{
  WCompositeWidget::enableAjax();

  if (isCollapsible()) {
    titleBarWidget()->clicked().connect(this, &WPanel::toggleCollapse);
  }
}

}
