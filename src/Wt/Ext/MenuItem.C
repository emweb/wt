/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/MenuItem"

namespace Wt {
  namespace Ext {

MenuItem::MenuItem(const WString& text)
  : AbstractButton()
{ 
  setText(text);
}

MenuItem::MenuItem(const std::string& iconPath, const WString& text)
  : AbstractButton()
{ 
  setText(text);
  setIcon(iconPath);
}

std::string MenuItem::createJS(DomElement *inContainer)
{
  assert(!inContainer);

  if (isCheckable())
    return createJSHelper("Ext.menu.CheckItem");
  else
    return createJSHelper("Ext.menu.Item");
}

std::string MenuItem::checkMethodJS() const
{
  return "setChecked";
}

std::string MenuItem::checkEventJS() const
{
  return "checkchange";
}

std::string MenuItem::checkInitialState() const
{
  return "checked";
}

}
}
