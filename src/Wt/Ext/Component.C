/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/Component"

namespace Wt {
  namespace Ext {

Component::Component(WContainerWidget *parent)
  : Widget(parent),
    enabled_(true)
{
  implementStateless(&Component::enable, &Component::undoEnable);
  implementStateless(&Component::disable, &Component::undoEnable);
}

void Component::enable()
{
  wasEnabled_ = enabled_;
  setEnabled(true);
}

void Component::disable()
{
  wasEnabled_ = enabled_;
  setEnabled(false);
}

void Component::undoEnable()
{
  setEnabled(wasEnabled_);
}

void Component::setEnabled(bool how)
{
  enabled_ = how;

  if (isRendered())
    addUpdateJS(elVar() + ".setDisabled(" + (how ? "false" : "true") + ");");
}

void Component::createConfig(std::ostream& config)
{ 
  Widget::createConfig(config);

  if (!enabled_)
    config << ",disabled:true";

}

  }
}
