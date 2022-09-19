/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WIconPair.h>
#include <Wt/WPanel.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTheme.h>
#include <Wt/WBootstrap5Theme.h>

#include "StdWidgetItemImpl.h"

namespace Wt {

WPanel::WPanel()
  : collapseIcon_(nullptr),
    title_(nullptr),
    centralWidget_(nullptr),
    isCollapsible_(false)
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
  if (enable && !titleBarWidget()) {
    impl_->bindWidget("titlebar", std::make_unique<WContainerWidget>());
  } else if (!enable && titleBar()) {
    impl_->bindEmpty("titlebar");
    title_ = nullptr;
    collapseIcon_ = nullptr;
  }
}

void WPanel::setCollapsible(bool on)
{
  auto app = WApplication::instance();
  auto bs5Theme = std::dynamic_pointer_cast<WBootstrap5Theme>(app->theme());

  if (!bs5Theme) {
    if (on && !isCollapsible()) {
      isCollapsible_ = on;

      std::string resources = WApplication::relativeResourcesUrl();

      setTitleBar(true);
      std::unique_ptr<WIconPair> icon
        (collapseIcon_ = new WIconPair(resources + "collapse.gif",
                                       resources + "expand.gif"));
      collapseIcon_->setFloatSide(Side::Left);

      titleBarWidget()->insertWidget(0, std::move(icon));

      collapseIcon_->icon1Clicked().connect(this, &WPanel::doCollapse);
      collapseIcon_->icon1Clicked().connect(this, &WPanel::onCollapse);
      collapseIcon_->icon1Clicked().preventPropagation();
      collapseIcon_->icon2Clicked().connect(this, &WPanel::doExpand);
      collapseIcon_->icon2Clicked().connect(this, &WPanel::onExpand);
      collapseIcon_->icon2Clicked().preventPropagation();
      collapseIcon_->setState(isCollapsed() ? 1 : 0);

      titleBarWidget()->clicked().connect(this, &WPanel::toggleCollapse);

      app->theme()->apply(this, collapseIcon_, PanelCollapseButton);
    } else if (!on && collapseIcon_) {
      isCollapsible_ = on;

      titleBarWidget()->removeWidget(collapseIcon_);
      collapseIcon_ = nullptr;
    }
  } else if (on && !isCollapsible()) {
    isCollapsible_ = on;

    setTitleBar(true);

    if (title_) {
      auto currentText = dynamic_cast<WText*>(title_)->text();
      titleBarWidget()->removeWidget(title_);
      title_ = titleBarWidget()->addWidget(std::make_unique<WPushButton>());
      dynamic_cast<WPushButton*>(title_)->setText(currentText);
      app->theme()->apply(this, title_, PanelCollapseButton);
      app->theme()->apply(this, titleBarWidget(), PanelTitleBar);
      app->theme()->setDataTarget(title_, centralArea());
    }
  } else {
    isCollapsible_ = on;
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
  // With Bootstrap 5 a WPanel can be collapsed
  // by user interaction, but there is no collapse
  // icon, and collapse() has no effect
  if (isCollapsible() && collapseIcon_) {
    collapseIcon_->showIcon2();

    doCollapse();
  }
}

void WPanel::expand()
{
  // With Bootstrap 5 a WPanel can be collapsed
  // by user interaction, but there is no collapse
  // icon, and expand() has no effect
  if (isCollapsible() && collapseIcon_) {
    collapseIcon_->showIcon1();

    doExpand();
  }
}

void WPanel::setAnimation(const WAnimation& transition)
{
  Wt::WBootstrap5Theme *bs5Theme = dynamic_cast<Wt::WBootstrap5Theme *>(wApp->theme().get());
  if (bs5Theme)
    return;
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

    auto app = WApplication::instance();
    app->theme()->apply(this, centralArea(), PanelBody);
    app->theme()->apply(this, centralWidget_, PanelBodyContent);
  }
}

WContainerWidget *WPanel::centralArea() const
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("contents"));
}

}
