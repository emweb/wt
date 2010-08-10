/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WIconPair>
#include <Wt/WPanel>
#include <Wt/WTemplate>
#include <Wt/WText>

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
    "${shadow-x1-x2}"
    "${titlebar}"
    "${contents}";

  setImplementation(impl_ = new WTemplate(WString::fromUTF8(TEMPLATE)));
  impl_->setStyleClass("Wt-panel Wt-outset");

  implementStateless(&WPanel::doExpand, &WPanel::undoExpand);
  implementStateless(&WPanel::doCollapse, &WPanel::undoCollapse);

  WContainerWidget *centralArea = new WContainerWidget();
  centralArea->setStyleClass("body");

  impl_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
  impl_->bindWidget("titlebar", 0);
  impl_->bindWidget("contents", centralArea);

  setJavaScriptMember
    (WT_RESIZE_JS,
     "function(self, w, h) {"
     """self.style.height= h + 'px';"
     """var c = self.lastChild;"
     """var t = c.previousSibling;"
     """if (t.className == 'titlebar')"
     ""  "h -= t.offsetHeight;"
     """h -= 8;"
     """if (h > 0) {"
     ""  "c.style.height = h + 'px';"
     // this seems golden, but, JQuery docs say it doesn't work when
     // the panel is indirectly hidden: will this back-fire ?
     ""  "$(c).children().each(function() { "
     ""      "var self = $(this), "
     ""          "padding = self.outerHeight() - self.height();"
     ""      "self.height(h - padding);"
     ""  "});"
     """}"
     "};");
}

void WPanel::setTitle(const WString& title)
{
  setTitleBar(true);
  if (!title_) {
    title_ = new WText();
    titleBarWidget()->insertWidget(titleBarWidget()->count() - 1, title_);
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
    titleBar->setStyleClass("titlebar");

    WBreak *br;
    titleBar->addWidget(br = new WBreak());
    br->setClearSides(Horizontals);
  } else if (!enable && titleBar()) {
    impl_->bindWidget("titlebar", 0);
    title_ = 0;
    collapseIcon_ = 0;
  }
}

void WPanel::setCollapsible(bool on)
{
  if (on && !collapseIcon_) {
    std::string resources = WApplication::resourcesUrl();

    setTitleBar(true);
    collapseIcon_ = new WIconPair(resources + "collapse.gif",
				  resources + "expand.gif");
    collapseIcon_->setInline(false);
    collapseIcon_->setFloatSide(Left);
    titleBarWidget()->insertWidget(0, collapseIcon_);

    collapseIcon_->icon1Clicked().connect(this, &WPanel::doCollapse);
    collapseIcon_->icon1Clicked().connect(this, &WPanel::onCollapse);
    collapseIcon_->icon2Clicked().connect(this, &WPanel::doExpand);
    collapseIcon_->icon2Clicked().connect(this, &WPanel::onExpand);
    collapseIcon_->setState(0);
  } else if (!on && collapseIcon_) {
    delete collapseIcon_;
    collapseIcon_ = 0;
  }
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
  return isCollapsible() && collapseIcon_->state() == 1;
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

void WPanel::doCollapse()
{
  wasCollapsed_ = isCollapsed();
  centralArea()->hide();

  collapsedSS_.emit(true);
}

void WPanel::doExpand()
{
  wasCollapsed_ = isCollapsed();
  centralArea()->show();

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

  if (w)
    centralArea()->addWidget(w);
}

WContainerWidget *WPanel::centralArea() const
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("contents"));
}

}
