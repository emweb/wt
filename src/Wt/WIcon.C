/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WIcon.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WCssDecorationStyle.h"
#include "Wt/WFont.h"

#include "WebUtils.h"
#include "DomElement.h"

namespace Wt {

WIcon::WIcon()
  : iconChanged_(false)
{ }

WIcon::WIcon(const std::string& name)
  : iconChanged_(false)
{
  setName(name);
}

void WIcon::setName(const std::string& name)
{
  if (name_ != name) {
    name_ = name;
    iconChanged_ = true;
    repaint();

    if (!name_.empty())
      loadIconFont();
  }
}

void WIcon::setSize(double factor)
{
  decorationStyle().font().setSize(WLength(factor, LengthUnit::FontEm));
}

double WIcon::size() const
{
  const WFont& f = decorationStyle().font();
  if (f.sizeLength().unit() == LengthUnit::FontEm)
    return f.sizeLength().value();
  else
    return 1;
}

void WIcon::updateDom(DomElement& element, bool all)
{
  if (iconChanged_ || all) {
    std::string sc;
    if (!all)
      sc = styleClass().toUTF8();

    if (!name_.empty())
      sc = Utils::addWord(sc, "fa fa-" + name_);

    element.setProperty(Property::Class, sc);

    iconChanged_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

DomElementType WIcon::domElementType() const
{
  return DomElementType::I;
}

void WIcon::propagateRenderOk(bool deep)
{
  iconChanged_ = false;

  WInteractWidget::propagateRenderOk(deep);
}

void WIcon::loadIconFont()
{
  WApplication *app = WApplication::instance();

  std::string fontDir = WApplication::relativeResourcesUrl()
    + "font-awesome/";
  
  app->useStyleSheet(fontDir + "css/font-awesome.min.css");
}

}
