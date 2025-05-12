/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WCssTheme.h"
#include "Wt/WLinkedCssStyleSheet.h"
#include "Wt/WLogger.h"
#include "Wt/WWidget.h"

namespace Wt {
LOGGER("WTheme");

WTheme::WTheme()
{ }

std::string WTheme::resourcesUrl() const
{
  return WApplication::relativeResourcesUrl() + "themes/" + name() + "/";
}

WTheme::~WTheme()
{ }

void WTheme::init(WT_MAYBE_UNUSED WApplication* app) const
{ }

void WTheme::apply(WWidget *widget, WWidget *child, int widgetRole) const
{
  // Always apply functional styling.
  applyFunctionalStyling(widget, child, widgetRole);

  if (widget->isThemeStyleEnabled()) {
    applyOptionalStyling(widget, child, widgetRole);
  }
}

void WTheme::serveCss(WStringStream& out) const
{
  std::vector<WLinkedCssStyleSheet> sheets = styleSheets();

  for (unsigned i = 0; i < sheets.size(); ++i)
    sheets[i].cssText(out);
}

Side WTheme::panelCollapseIconSide() const
{
  return Side::Left;
}

void WTheme::loadValidationStyling(WT_MAYBE_UNUSED WApplication* app) const
{
  LOG_WARN("loadValidationStyling(): Using the default (empty) call. Override it if you make use of custom validation (using DOM.validate() or DOM.wtValdiate()).");
}
}
