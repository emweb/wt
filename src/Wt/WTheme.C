/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WCssTheme.h"
#include "Wt/WLinkedCssStyleSheet.h"

namespace Wt {

WTheme::WTheme()
{ }

std::string WTheme::resourcesUrl() const
{
  return WApplication::relativeResourcesUrl() + "themes/" + name() + "/";
}

WTheme::~WTheme()
{ }

void WTheme::serveCss(WStringStream& out) const
{
  std::vector<WLinkedCssStyleSheet> sheets = styleSheets();

  for (unsigned i = 0; i < sheets.size(); ++i)
    sheets[i].cssText(out);
}

}
