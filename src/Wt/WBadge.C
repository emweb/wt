/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBadge.h"

#include "Wt/WApplication.h"
#include "Wt/WTheme.h"

namespace Wt {

WBadge::WBadge()
{ }

WBadge::WBadge(const WString& text)
  : WText(text)
{ }

WBadge::WBadge(const WString& text, TextFormat textFormat)
  : WText(text, textFormat)
{ }

void WBadge::updateDom(DomElement& element, bool all)
{
  WApplication *app = WApplication::instance();

  if (all) {
    app->theme()->apply(this, element, Badge);
  }

  WText::updateDom(element, all);
}

}