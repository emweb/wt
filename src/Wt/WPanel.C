/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WIconPair>
#include <Wt/WPanel>
#include <Wt/WText>

namespace Wt {

WPanel::WPanel(WContainerWidget *parent)
  : WCompositeWidget(parent),
    collapsed(this),
    expanded(this),
    collapsedSS(this),
    expandedSS(this),
    titleBar_(0),
    collapseIcon_(0),
    title_(0),
    centralWidget_(0)
{
  setImplementation(impl_ = new WContainerWidget());

  implementStateless(&WPanel::doExpand, &WPanel::undoExpand);
  implementStateless(&WPanel::doCollapse, &WPanel::undoCollapse);

  impl_->setStyleClass("Wt-panel");

  WContainerWidget *centralArea = new WContainerWidget();
  centralArea->setStyleClass("body");
  impl_->addWidget(centralArea);

  const char *CSS_RULES_NAME = "Wt::WPanel";

  WApplication *app = WApplication::instance();
  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    app->styleSheet().addRule("div.Wt-panel",
			      "border: 3px solid #888888;"
			      "background: #EEEEEE none repeat scroll 0%;",
			      CSS_RULES_NAME);
    app->styleSheet().addRule("div.Wt-panel .titlebar",
			      "background: #888888; color: #FFFFFF;"
			      "padding: 0px 6px 3px;"
			      "height: 18px; font-size: 10pt;");
    app->styleSheet().addRule("div.Wt-panel .body",
			      "background: #FFFFFF;"
			      "padding: 4px 6px 4px;");
  }
}

void WPanel::setTitle(const WString& title)
{
  setTitleBar(true);
  if (!title_)
    title_ = new WText(titleBar_);

  title_->setText(title);
}

WString WPanel::title() const
{
  if (title_)
    return title_->text();
  else
    return WString();
}

void WPanel::setTitleBar(bool enable)
{
  if (enable && !titleBar_) {
    titleBar_ = new WContainerWidget();
    impl_->insertWidget(0, titleBar_);
    titleBar_->setStyleClass("titlebar");
  } else if (!enable && titleBar_) {
    delete titleBar_;
    titleBar_ = 0;
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
    collapseIcon_->setMargin(2, Top);
    titleBar_->insertWidget(0, collapseIcon_);

    collapseIcon_->icon1Clicked.connect(SLOT(this, WPanel::doCollapse));
    collapseIcon_->icon1Clicked.connect(SLOT(this, WPanel::onCollapse));
    collapseIcon_->icon2Clicked.connect(SLOT(this, WPanel::doExpand));
    collapseIcon_->icon2Clicked.connect(SLOT(this, WPanel::onExpand));
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

  collapsedSS.emit(true);
}

void WPanel::doExpand()
{
  wasCollapsed_ = isCollapsed();
  centralArea()->show();

  expandedSS.emit(true);
}

void WPanel::undoCollapse()
{
  if (!wasCollapsed_)
    expand();

  collapsedSS.emit(false);
}

void WPanel::undoExpand()
{
  if (wasCollapsed_)
    collapse();

  expandedSS.emit(false);
}

void WPanel::onCollapse()
{
  collapsed.emit();
}

void WPanel::onExpand()
{
  expanded.emit();
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
  return dynamic_cast<WContainerWidget *>(impl_->children().back());
}

}
