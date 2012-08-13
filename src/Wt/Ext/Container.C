/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/Ext/Container"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WLogger"

#include "Wt/Ext/AccordionLayoutImpl.h"
#include "Wt/Ext/BorderLayoutImpl.h"
#include "Wt/Ext/DefaultLayoutImpl.h"
#include "Wt/Ext/FitLayoutImpl.h"
#include "Wt/Ext/WWidgetItemImpl.h"

#include "Wt/WAccordionLayout"
#include "Wt/WBorderLayout"
#include "Wt/WDefaultLayout"
#include "Wt/WFitLayout"
#include "Wt/WWidgetItem"

#include "DomElement.h"

namespace Wt {

LOGGER("Ext.Container");

  namespace Ext {

Container::Container(WContainerWidget *parent)
  : Component(parent),
    layout_(0),
    widget_(0),
    layoutChanged_(false)
{
  setInline(false);

  setHideWithOffsets();
}

Container::~Container()
{
  delete layout_;
}

WLayout *Container::layout()
{
  if (!layout_)
    setLayout(new WDefaultLayout());

  return layout_;
}

Ext::LayoutImpl *Container::layoutImpl() const
{
  return dynamic_cast<Ext::LayoutImpl *>(layout_->impl());
}

void Container::setLayout(WLayout *layout)
{
  if (layout_) {
    LOG_ERROR("setLayout(): already have a layout.");
    return;
  }

  layout_ = layout;

  WWidget::setLayout(layout);

  layoutImpl()->setContainer(this);
}

void Container::setWidget(WWidget *w)
{
  widget_ = w;
  widget_->setInline(false);
  addOrphan(widget_);
}

void Container::add(WWidget *widget)
{
  addOrphan(widget);
}

void Container::removeChild(WWidget *child)
{
  if (layout_)
    layout_->removeWidget(child);

  if (widget_ == child)
    widget_ = 0;

  Component::removeChild(child);
}

std::string Container::extClassName() const
{
  WContainerWidget* appRoot = WApplication::instance()->root();
  if (parent() == appRoot && appRoot->children().size() == 1)
    return "Ext.Viewport";
  else
    return "Ext.Panel";
}

std::string Container::createJS(DomElement *inContainer)
{
  assert(inContainer);

  if (inWtLayout()) {
    inContainer->setProperty(PropertyStylePosition, "relative");
    inContainer->setProperty(PropertyStyleWidth, "0px");
    inContainer->callMethod("wtResize ="
			    "function(self, w, h){ "
			    + elVar() + ".setSize(w, h);"
			    """if (w == 0) {"
			    """self.style.width='';"
			    """window.onresize();"
			    "}};");
  }

  std::string result;

  if (widget_) {
    std::string s = widget_->styleClass().toUTF8() + " x-hidden";
    DomElement *c = widget_->createSDomElement(WApplication::instance());
    c->setAttribute("class", s);
    inContainer->addChild(c);
  }

  if (layout_)
    layoutImpl()->createComponents(inContainer);

  result += elVar() + "=new " + extClassName() + "(" + configStruct() + ");";

  if (layout_)
    result += elVar() + ".on('afterlayout', function(){"
      + WApplication::instance()->javaScriptClass()
      + "._p_.doAutoJavaScript();});";

  return result;
}

void Container::setSizeConfig(std::ostream& config, WWidget *w)
{
  if (!w->width().isAuto())
    config << ",width:" << w->width().toPixels();

  if (!w->height().isAuto())
    config << ",height:" << w->height().toPixels();

  if (!w->minimumWidth().isAuto())
    config << ",minWidth:" << w->minimumWidth().toPixels();

  if (!w->minimumHeight().isAuto())
    config << ",minHeight:" << w->minimumHeight().toPixels();

  if (!w->maximumWidth().isAuto())
    config << ",maxWidth:" << w->maximumWidth().toPixels();

  if (!w->maximumHeight().isAuto())
    config << ",maxHeight:" << w->maximumHeight().toPixels();
}

void Container::setLayoutChanged()
{
  layoutChanged_ = true;
  repaint();
}

bool Container::applySelfCss() const
{
  return false;
}

bool Container::inWtLayout() const
{
  WWidget *p = parent();

  if (p)
    p = p->parent();

  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(p);

  return wc && wc->layout();
}

void Container::getDomChanges(std::vector<DomElement *>& result,
			      WApplication *app)
{
  if (layoutChanged_) {
    layoutImpl()->getLayoutChanges(id(), result);
    layoutChanged_ = false;
  }
  Component::getDomChanges(result, app);
}

void Container::createConfig(std::ostream& config)
{
  Component::createConfig(config);

  WApplication *app = WApplication::instance();

  WWidget *p = parent();

  if (!dynamic_cast<Container *>(p)
      && extClassName() != "Ext.Viewport" && p != app->domRoot()) {

    config << ",renderTo:'" << id() << "'";

    if (inWtLayout())
      config << ",style:'position:absolute;'";
  } else {
    // because it is relative it sets over the actual interface.
    app->styleSheet().addRule(".Wt-domRoot", "position: static;");
  }

  if (widget_)
    config << ",contentEl:'" << widget_->id() << "',autoShow:true";

  setSizeConfig(config, this);

  if (layout_)
    layoutImpl()->createConfig(config);
}

void Container::addLayoutConfig(Widget *w, std::ostream& config)
{
  if (layout_) {
    WWidgetItem *item = layout_->findWidgetItem(w);
    if (item)
      dynamic_cast<WWidgetItemImpl *>(item->impl())->addConfig(config);
  }
}

WLayoutItemImpl *Container::createLayoutItemImpl(WLayoutItem *item)
{
  {
    WWidgetItem *wi = dynamic_cast<WWidgetItem *>(item);
    if (wi)
      return new WWidgetItemImpl(wi);
  }

  {
    WAccordionLayout *l = dynamic_cast<WAccordionLayout *>(item);
    if (l)
      return new AccordionLayoutImpl(l);
  }

  {
    WBorderLayout *l = dynamic_cast<WBorderLayout *>(item);
    if (l)
      return new BorderLayoutImpl(l);
  }

  {
    WDefaultLayout *l = dynamic_cast<WDefaultLayout *>(item);
    if (l)
      return new DefaultLayoutImpl(l);
  }

  {
    WFitLayout *l = dynamic_cast<WFitLayout *>(item);
    if (l)
      return new FitLayoutImpl(l);
  }

  assert(false);

  return 0;
}
  }
}
