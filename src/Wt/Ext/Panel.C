/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "DomElement.h"
#include "Wt/Ext/Panel"
#include "Wt/Ext/ToolBar"
#include "WebUtils.h"

namespace Wt {
  namespace Ext {

Panel::Panel(WContainerWidget *parent)
  : Container(parent),
    collapsed_(this, "collapsed", true),
    expanded_(this, "expanded", true),
    titleBar_(false),
    animate_(false),
    autoScrollBars_(false),
    border_(true),
    isCollapsed_(false),
    collapsible_(false),
    split_(false),
    topToolBar_(0),
    bottomToolBar_(0),
    defaultButton_(0)
{ 
  collapsed_.connect(this, &Panel::onCollapse);
  expanded_.connect(this, &Panel::onExpand);
}

Panel::~Panel()
{
  while (!footerButtons_.empty())
    removeFooterButton(footerButtons_.back());
}

void Panel::removeChild(WWidget *child)
{
  if (child == topToolBar_) {
    topToolBar_ = 0;
    if (isRendered())
      addUpdateJS(elVar() + ".getTopToolbar().hide();");
  }

  if (child == bottomToolBar_) {
    bottomToolBar_ = 0;
    if (isRendered())
      addUpdateJS(elVar() + ".getBottomToolbar().hide();");
  }

  Container::removeChild(child);
}

void Panel::setTopToolBar(ToolBar *toolBar)
{
  delete topToolBar_;
  topToolBar_ = toolBar;

  if (topToolBar_)
    addOrphan(topToolBar_);
}

void Panel::setBottomToolBar(ToolBar *toolBar)
{
  delete bottomToolBar_;
  bottomToolBar_ = toolBar;

  if (bottomToolBar_)
    addOrphan(bottomToolBar_);
}

void Panel::addFooterButton(Button *button)
{
  footerButtons_.push_back(button);
  button->setPanel(this);
  addOrphan(button);
}

void Panel::removeFooterButton(Button *button)
{
  if (button == defaultButton_)
    defaultButton_ = 0;

  if (Utils::erase(footerButtons_, button))
    button->setPanel(0);
}

void Panel::setDefaultButton(Button *button)
{
  defaultButton_ = button;
}

void Panel::setTitleBar(bool enable)
{
  titleBar_ = enable;
}

void Panel::setTitle(const WString& title)
{
  title_ = title;
  addUpdateJS(elVar() + ".setTitle(" + title_.jsStringLiteral() + ");");
  setTitleBar(true);
}

void Panel::setAnimate(bool on)
{ 
  animate_ = on;
}

void Panel::setAutoScrollBars(bool on)
{ 
  autoScrollBars_ = on;
}

void Panel::setBorder(bool on)
{ 
  border_ = on;
}

void Panel::setCollapsed(bool on)
{ 
  if (isCollapsed_ != on) {
    isCollapsed_ = on;
    addUpdateJS(elVar() + "." + (on ? "collapse()" : "expand()") + ";");
  }
}

void Panel::collapse()
{
  setCollapsed(true);
}

void Panel::expand()
{
  setCollapsed(false);
}

void Panel::setCollapsible(bool on)
{ 
  collapsible_ = on;
}

void Panel::setResizable(bool on)
{ 
  split_ = on;
}

void Panel::onCollapse()
{
  isCollapsed_ = true;
}

void Panel::onExpand()
{
  isCollapsed_ = false;
}

void Panel::refresh()
{
  if (title_.refresh())
    setTitle(title_);

  Widget::refresh();
}

void Panel::updateExt()
{
  updateWtSignal(&collapsed_, collapsed_.name(), "", "");
  updateWtSignal(&expanded_, expanded_.name(), "", "");

  Container::updateExt();
}

std::string Panel::createJS(DomElement *inContainer)
{
  std::stringstream result;

  if (topToolBar_)
    topToolBar_->createExtElement(result, 0);
  if (bottomToolBar_)
    bottomToolBar_->createExtElement(result, 0);

  for (unsigned i = 0; i < footerButtons_.size(); ++i)
    footerButtons_[i]->createExtElement(result, 0);

  result << Container::createJS(inContainer);

  if (collapsible_) {
    bindEventHandler("collapse", "collapseH", result);
    bindEventHandler("expand", "expandH", result);
  }

  if (topToolBar_)
    topToolBar_->jsAfterPanelRendered(result);
  if (bottomToolBar_)
    bottomToolBar_->jsAfterPanelRendered(result);

  return result.str();
}

void Panel::createConfig(std::ostream& config)
{
  Container::createConfig(config);

  if (animate_ != false)        config << ",animate:true";
  if (autoScrollBars_ != false) config << ",autoScroll:true";
  if (border_ != true)          config << ",border:false";
  if (isCollapsed_ != false)    config << ",collapsed:true";
  if (collapsible_ != false)    config << ",collapsible:true";
  if (split_ != false)          config << ",split:true";
  //config << ",hideMode:'offsets'";

  if (!title_.empty()) {
    config << ",title:" << title_.jsStringLiteral();
    if (titleBar_ != true)      config << ",header:false";
  } else
    if (titleBar_ != false)     config << ",header:true";

  if (topToolBar_)
    config << ",tbar:" << topToolBar_->elVar();
  if (bottomToolBar_)
    config << ",bbar:" << bottomToolBar_->elVar();

  if (!footerButtons_.empty()) {
    config << ",buttons:[";

    for (unsigned i = 0; i < footerButtons_.size(); ++i) {
      if (i != 0)
	config << ',';
      config << footerButtons_[i]->elRef();
    }

    config << ']';
  }

  addWtSignalConfig("collapseH", &collapsed_, collapsed_.name(),
		    "", "", config);
  addWtSignalConfig("expandH", &expanded_, expanded_.name(),
		    "", "", config);

}

  }
}
