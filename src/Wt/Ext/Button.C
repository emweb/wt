/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DomElement.h"

#include "Wt/Ext/Button"
#include "Wt/Ext/Panel"
#include "Wt/Ext/Menu"
#include "Wt/Ext/ToolBar"

namespace Wt {
  namespace Ext {

Button::Button(WContainerWidget *parent)
  : AbstractButton(parent),
    panel_(0)
{ }

Button::Button(const WString& text, WContainerWidget *parent)
  : AbstractButton(parent),
    panel_(0)
{
  setText(text);
}

Button::~Button()
{
  if (panel_)
    panel_->removeFooterButton(this);
}

std::string Button::createJS(DomElement *inContainer)
{
  std::string extClassName;

  if (dynamic_cast<ToolBar *>(parent()))
    extClassName = "Ext.Toolbar.Button";
  else
    extClassName = "Ext.Button";

  return createJSHelper(extClassName, inContainer != 0);
}

std::string Button::checkMethodJS() const
{
  return "toggle";
}

std::string Button::checkEventJS() const
{
  return "toggle";
}

std::string Button::checkInitialState() const
{
  return "pressed";
}

void Button::setPanel(Panel *panel)
{
  panel_ = panel;
}

void Button::setDefault(bool how)
{
  if (panel_) {
    if (how) {
      if (!isDefault())
	panel_->setDefaultButton(this);
    } else
      if (isDefault())
	panel_->setDefaultButton(0);
  }
}

bool Button::isDefault() const
{
  if (panel_)
    return panel_->defaultButton() == this;
  else
    return false;
}

}
}
