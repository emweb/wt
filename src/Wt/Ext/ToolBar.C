/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/ToolBar"
#include "Wt/WLogger"
#include "WebUtils.h"

#include <iostream>

namespace Wt {

LOGGER("Ext.ToolBar");

  namespace Ext {

class ToolBarSeparator : public Widget
{
public:
  ToolBarSeparator()
    : Widget()
  { }

private:
  virtual std::string createJS(DomElement *inContainer)
  {
    return elVar() + "=new Ext.Toolbar.Separator();";
  }
};

class ToolBarStretch : public Widget
{
public:
  ToolBarStretch()
    : Widget()
  { }

private:
  virtual std::string createJS(DomElement *inContainer)
  {
    return elVar() + "=new Ext.Toolbar.Fill();";
  }
};


ToolBar::ToolBar(WContainerWidget *parent)
  : Widget(parent)
{ }

Button *ToolBar::addButton(const WString& text)
{
  return addButton(std::string(), text);
}

Button *ToolBar::addButton(const std::string& iconPath, const WString& text)
{
  return addButton(iconPath, text, 0);
}

Button *ToolBar::addButton(const WString& text, Menu *menu)
{
  return addButton(std::string(), text, menu);
}

Button *ToolBar::addButton(const std::string& iconPath, const WString& text,
			   Menu *menu)
{
  Button *button = new Button(text);
  button->setIcon(iconPath);
  button->setMenu(menu);

  add(button);

  return button;
}

void ToolBar::add(Button *button)
{
  add(dynamic_cast<WWidget *>(button));
}

void ToolBar::add(WWidget *item)
{
  items_.push_back(item);
  addOrphan(item);

  Widget::renderExtAdd(item);
}

void ToolBar::insert(int index, WWidget *item)
{
  if (isRendered())
    LOG_ERROR("insert(): can only insert plain widgets before initial "
	      "rendering");

  items_.insert(items_.begin() + index, item);
  addOrphan(item);
}

void ToolBar::insert(int index, Button *button)
{
  items_.insert(items_.begin() + index, button);
  addOrphan(button);

  if (!isRendered())
    return;

  std::stringstream js;
  std::string var = button->createExtElement(js, 0);
  js << elVar() << ".insertButton(" << index << "," << var << ");";
  addUpdateJS(js.str());
}

void ToolBar::addSeparator()
{
  add(new ToolBarSeparator());
}

void ToolBar::addStretch()
{
  add(new ToolBarStretch());
}

void ToolBar::removeChild(WWidget *child)
{
  Widget::removeChild(child);

  if (Utils::erase(items_, child)) {
    // FIXME: no ext method to remove a button??
  }
}

std::string ToolBar::createJS(DomElement *inContainer)
{
  std::stringstream result;
  std::string refs = createMixed(items_, result);

  result << elVar() << "=new Ext.Toolbar([" << refs << "]);";

  if (inContainer) {
    result << elVar() << ".render('" << id() << "');";
    jsAfterPanelRendered(result);
  }

  return result.str();
}

void ToolBar::jsAfterPanelRendered(std::stringstream&)
{ }

}
}
