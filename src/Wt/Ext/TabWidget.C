/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "DomElement.h"
#include "Wt/Ext/TabWidget"
#include "Wt/WDefaultLayout"

namespace Wt {
  namespace Ext {

TabWidget::TabWidget(WContainerWidget *parent)
  : Panel(parent),
    currentChanged_(this),
    jCurrentChanged_(this, "tabchange", true),
    currentIndex_(-1)
{
  setLayout(new WDefaultLayout());
}

void TabWidget::removeChild(WWidget *child)
{
  removeTab(indexOf(child));

  Panel::removeChild(child);
}

void TabWidget::addTab(Panel *panel)
{
  panel->setStyleClass("x-hidden");

  layout()->addWidget(panel);
  panels_.push_back(panel);

  if (currentIndex_ == -1)
    currentIndex_ = 0;
}

void TabWidget::removeTab(int index)
{
  if (index != -1) {
    Panel *p = panel(index);
    panels_.erase(panels_.begin() + index);
    layout()->removeWidget(p);
  }
}

void TabWidget::addTab(WWidget *child, const WString& label)
{
  Panel *p = dynamic_cast<Panel *>(child);

  if (!p) {
    p = new Panel();
    p->setAutoScrollBars(true);
    p->setWidget(child);
  }

  p->setTitle(label);
  addTab(p);
}

int TabWidget::count() const
{
  return panels_.size();
}

Panel *TabWidget::panel(int index) const
{
  index = std::min(std::max(0, index), count() - 1);
  return panels_[index];
}

WWidget *TabWidget::widget(int index) const
{
  return panel(index)->widget();
}

void TabWidget::setCurrentIndex(int index)
{
  currentIndex_ = std::min(std::max(0, index), count() - 1);

  if (isRendered())
    addUpdateJS(elVar()
		+ ".setActiveTab(" + panels_[currentIndex_]->elRef() + ");");
}

int TabWidget::currentIndex() const
{
  return currentIndex_;
}

int TabWidget::indexOf(WWidget *w) const
{
  for (unsigned i = 0; i < panels_.size(); ++i)
    if (panels_[i] == w || widget(i) == w)
      return i;

  return -1;
}

void TabWidget::setCurrentWidget(WWidget *widget)
{
  setCurrentIndex(indexOf(widget));
}

WWidget *TabWidget::currentWidget() const
{
  return widget(currentIndex_);
}

void TabWidget::setTabEnabled(int index, bool enable)
{
  index = std::min(std::max(0, index), count() - 1);
  panels_[index]->setEnabled(enable);
}

bool TabWidget::isTabEnabled(int index) const
{
  index = std::min(std::max(0, index), count() - 1);
  return panels_[index]->isEnabled();
}

void TabWidget::setTabHidden(int index, bool hidden)
{
  index = std::min(std::max(0, index), count() - 1);
  panels_[index]->setHidden(hidden);
  if (hidden)
    addUpdateJS(elVar() + ".hideTabStripItem("
		+ boost::lexical_cast<std::string>(index) + ");");
  else
    addUpdateJS(elVar() + ".unhideTabStripItem("
		+ boost::lexical_cast<std::string>(index) + ");");
    
}

bool TabWidget::isTabHidden(int index) const
{
  index = std::min(std::max(0, index), count() - 1);
  return panels_[index]->isHidden();
}

void TabWidget::setTabText(int index, const WString& label)
{
  index = std::min(std::max(0, index), count() - 1);
  panels_[index]->setTitle(label);
}

const WString& TabWidget::tabText(int index) const
{
  index = std::min(std::max(0, index), count() - 1);
  return panels_[index]->title();
}

void TabWidget::setTabToolTip(int index, const WString& tip)
{
  /*
  index = std::min(std::max(0, index), count() - 1);
  panels_[index].toolTip = tip;

  addUpdateJS(elVar() + ".getTab("
	      + boost::lexical_cast<std::string>(index)
	      + ").setTabTooltip("
	      + jsStringLiteral(tip.toUTF8(), '\'') + ");");  
  */
}

const WString TabWidget::tabToolTip(int index) const
{
  /*
  index = std::min(std::max(0, index), count() - 1);
  return panels_[index].toolTip;  
  */
  return "";
}

void TabWidget::refresh()
{
  /*
  for (unsigned i = 0; i < panels_.size(); ++i) {
    TabItem& item = panels_[i];
   
    if (item.text.refresh())
      setTabText(i, item.text);
    if (item.toolTip.refresh())
      setTabToolTip(i, item.toolTip);
  }
  */

  Panel::refresh();
}

void TabWidget::onTabChange(int i)
{
  currentIndex_ = i;
  currentChanged_.emit(currentIndex_);
}

std::string TabWidget::extClassName() const
{
  return "Ext.TabPanel";
}

void TabWidget::updateExt()
{
  Panel::updateExt();
}

std::string TabWidget::createJS(DomElement *inContainer)
{
  if (!jCurrentChanged_.isConnected())
    jCurrentChanged_.connect(this, &TabWidget::onTabChange);

  for (unsigned i = 0; i < panels_.size(); ++i)
    panels_[i]->setTitleBar(false);

  std::stringstream result;
  result << Panel::createJS(inContainer);

  result << elVar()
	 << ".setActiveTab(" << panels_[currentIndex_]->elRef() << ");";

  result << elVar() << ".doLayout();";

  bindEventHandler("tabchange", "tabchangeH", result);

  return result.str();
}

void TabWidget::createConfig(std::ostream& config)
{
  Panel::createConfig(config);

  addWtSignalConfig("tabchangeH", &jCurrentChanged_, jCurrentChanged_.name(),
		    "t,p", "t.items.indexOf(p)", config);
}

  }
}
