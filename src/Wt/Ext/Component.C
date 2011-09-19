/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/Component"

namespace Wt {
  namespace Ext {

Component::Component(WContainerWidget *parent)
  : Widget(parent)
{ }

void Component::setEnabled(bool enabled)
{
  setDisabled(!enabled);
}

void Component::enable()
{
  setDisabled(false);
}

void Component::disable()
{
  setDisabled(true);
}

void Component::propagateSetEnabled(bool enabled)
{
  if (isRendered())
    addUpdateJS(elVar()
		+ ".setDisabled(" + (enabled ? "false" : "true") + ");");

  Widget::propagateSetEnabled(enabled);
}

void Component::createConfig(std::ostream& config)
{ 
  Widget::createConfig(config);

  if (!isEnabled())
    config << ",disabled:true";
}

  }
}
