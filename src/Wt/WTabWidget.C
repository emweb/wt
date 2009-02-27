/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WTabWidget"
#include "Wt/WMenu"
#include "Wt/WMenuItem"
#include "Wt/WStackedWidget"

namespace Wt {

namespace {

class TabWidgetItem : public WMenuItem
{
public:
  TabWidgetItem(const WString& text, WWidget *contents, LoadPolicy loadPolicy);

protected:
  virtual WWidget *createItemWidget();
  virtual void updateItemWidget(WWidget *itemWidget);
  virtual SignalBase& activateSignal();
};

TabWidgetItem::TabWidgetItem(const WString& text, WWidget *contents,
			     LoadPolicy loadPolicy)
  : WMenuItem(text, contents, loadPolicy)
{ }

void TabWidgetItem::updateItemWidget(WWidget *itemWidget)
{
  WContainerWidget *c = dynamic_cast<WContainerWidget *>(itemWidget);
  WAnchor *a = dynamic_cast<WAnchor *>(c->children()[0]);
  WMenuItem::updateItemWidget(a);
}

WWidget *TabWidgetItem::createItemWidget()
{
  WContainerWidget *c = new WContainerWidget();
  c->setInline(true);
  c->addWidget(WMenuItem::createItemWidget());

  return c;
}

SignalBase& TabWidgetItem::activateSignal()
{
  WContainerWidget *c = dynamic_cast<WContainerWidget *>(itemWidget());

  return dynamic_cast<WInteractWidget *>(c->children()[0])->clicked;
}

}

WTabWidget::WTabWidget(WContainerWidget *parent)
  : WCompositeWidget(parent),
    currentChanged(this)
{
  setImplementation(layout_ = new WContainerWidget());
  layout_->setOverflow(WContainerWidget::OverflowAuto);

  const char *CSS_RULES_NAME = "Wt::WTabWidget";

  WApplication *app = WApplication::instance();
  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    std::string resourcesURL = WApplication::resourcesUrl();

    app->styleSheet().addRule(".Wt-tabs",
			      "background: transparent url("
			      + resourcesURL + "tab_b.gif) "
			      "repeat-x scroll center bottom;"
			      "margin-bottom:4px;"
			      "zoom: 1;"
			      "width:100%", CSS_RULES_NAME);
    app->styleSheet().addRule(".Wt-tabs li",
			      "display: inline;");
    app->styleSheet().addRule(".Wt-tabs ul",
			      "margin: 0px;"
			      "padding-left: 10px;"
			      "list-style-type: none;"
			      "list-style-position: outside;");
    app->styleSheet().addRule(".Wt-tabs span",
			      "background: transparent url("
			      + resourcesURL + "tab_r.gif) "
			      "no-repeat scroll right top;"
			      "border-bottom:1px solid #84B0C7;"
			      "float:left; display:block;"
			      "cursor:pointer;cursor:hand;"
			      "font-size: small; font-weight: bold;");
    app->styleSheet().addRule(".Wt-tabs span.itemselected",
			      "background-position:100% -150px;"
                              );
    app->styleSheet().addRule(".Wt-tabs span span",
			      "background: transparent url("
			      + resourcesURL + "tab_l.gif) "
			      "no-repeat scroll left top;"
			      "border-bottom: 0px;"
			      "white-space: nowrap;"
			      "padding:5px 9px;color:#1A419D;");
    app->styleSheet().addRule(".Wt-tabs span.itemselected span",
			      "background-position:0% -150px;"
                              ); 
  }

  contents_ = new WStackedWidget();
  menu_ = new WMenu(contents_, Horizontal);
  menu_->setRenderAsList(true);

  WContainerWidget *menuDiv = new WContainerWidget();
  menuDiv->setStyleClass("Wt-tabs");
  menuDiv->addWidget(menu_);

  layout_->addWidget(menuDiv);
  layout_->addWidget(contents_);

  menu_->itemSelected.connect(SLOT(this, WTabWidget::onItemSelected));
}

WMenuItem *WTabWidget::addTab(WWidget *child, const WString& label,
			      LoadPolicy loadPolicy)
{
  WMenuItem *result
    = new TabWidgetItem(label, child,
			static_cast<WMenuItem::LoadPolicy>(loadPolicy));

  menu_->addItem(result);

  items_.push_back(TabItem());
  items_.back().enabled = true;
  items_.back().hidden = false;

  return result;
}

void WTabWidget::removeTab(WWidget *child)
{
  WMenuItem *item = menu_->items()[indexOf(child)];
  menu_->removeItem(item);

  item->takeContents();
  delete item;
}

int WTabWidget::count() const
{
  return contents_->count();
}

WWidget *WTabWidget::widget(int index) const
{
  return contents_->widget(index);
}

int WTabWidget::indexOf(WWidget *widget) const
{
  return contents_->indexOf(widget);
}

void WTabWidget::setCurrentIndex(int index)
{
  menu_->select(index);
}

int WTabWidget::currentIndex() const
{
  return menu_->currentIndex();
}

void WTabWidget::setCurrentWidget(WWidget *widget)
{
  setCurrentIndex(contents_->indexOf(widget));
}

WWidget *WTabWidget::currentWidget() const
{
  return contents_->currentWidget();
}

void WTabWidget::setTabEnabled(int index, bool enable)
{
  items_[index].enabled = enable;
}

bool WTabWidget::isTabEnabled(int index) const
{
  return items_[index].enabled;
}

void WTabWidget::setTabHidden(int index, bool hidden)
{
  items_[index].hidden = hidden;
}

bool WTabWidget::isTabHidden(int index) const
{
  return items_[index].hidden;
}

void WTabWidget::setTabText(int index, const WString& label)
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  item->setText(label);
}

const WString& WTabWidget::tabText(int index) const
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  return item->text();
}

void WTabWidget::setTabToolTip(int index, const WString& tip)
{
  items_[index].toolTip = tip;
}

const WString& WTabWidget::tabToolTip(int index) const
{
  return items_[index].toolTip;
}

bool WTabWidget::internalPathEnabled() const
{
  return menu_->internalPathEnabled();
}

void WTabWidget::setInternalPathEnabled()
{
  menu_->setInternalPathEnabled();
}

const std::string& WTabWidget::internalBasePath() const
{
  return menu_->internalBasePath();
}

void WTabWidget::setInternalBasePath(const std::string& path)
{
  menu_->setInternalBasePath(path);
}

void WTabWidget::onItemSelected(WMenuItem *item)
{
  currentChanged.emit(menu_->currentIndex());
}

}
