/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/Menu"
#include "WebUtils.h"

namespace Wt {
  namespace Ext {

class MenuSeparator : public Widget
{
public:
  MenuSeparator()
    : Widget()
  { }

private:
  virtual std::string createJS(DomElement *inContainer)
  {
    return elVar() + "=new Ext.menu.Separator();";
  }
};

Menu::Menu()
  : Widget()
{ }

MenuItem *Menu::addItem(const WString& text)
{
  return addItem(std::string(), text);
}

MenuItem *Menu::addItem(const std::string& iconPath, const WString& text)
{
  return addMenu(iconPath, text, 0);
}

MenuItem *Menu::addMenu(const WString& text, Menu *menu)
{
  return addMenu(std::string(), text, menu);
}

MenuItem *Menu::addMenu(const std::string& iconPath, const WString& text,
			Menu *menu)
{
  MenuItem *item = new MenuItem(text);
  item->setIcon(iconPath);
  item->setMenu(menu);

  add(item);

  return item;
}

void Menu::add(MenuItem *item)
{
  add(dynamic_cast<WWidget *>(item));
}

void Menu::add(WWidget *item)
{
  items_.push_back(item);
  addOrphan(item);

  Widget::renderExtAdd(item);
}

void Menu::addSeparator()
{
  add(new MenuSeparator());
}

void Menu::removeChild(WWidget *child)
{
  Widget::removeChild(child);

  if (Utils::erase(items_, child)) {
    // FIXME: no ext method to remove a menu item ??
  }
}

std::string Menu::createJS(DomElement *inContainer)
{
  assert(!inContainer);
  std::stringstream result;
  std::string refs = createMixed(items_, result);

  result << elVar() << "=new Ext.menu.Menu([" << refs << "]);";

  return result.str();
}

}
}
