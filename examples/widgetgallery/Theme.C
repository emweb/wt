/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "Theme.h"

#include <Wt/WLink.h>

Theme::Theme(const std::string &name)
  : name_(name)
{}

std::vector<Wt::WLinkedCssStyleSheet> Theme::styleSheets() const
{
  return {
    Wt::WLinkedCssStyleSheet(Wt::WLink("style/" + name_ + ".css"))
  };
}
