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
  : useDefaultStyle_(true)
{ }

WBadge::WBadge(const WString& text)
  : WText(text),
    useDefaultStyle_(true)
{ }

WBadge::WBadge(const WString& text, TextFormat textFormat)
  : WText(text, textFormat),
    useDefaultStyle_(true)
{ }

void WBadge::updateDom(DomElement& element, bool all)
{
  WApplication *app = WApplication::instance();

  if (all || flags_.test(BIT_USE_DEFAULT_STYLE_CHANGED)) {
    app->theme()->apply(this, element, Badge);
    flags_.reset(BIT_USE_DEFAULT_STYLE_CHANGED);
  }

  WText::updateDom(element, all);
}

void WBadge::setUseDefaultStyle(bool use)
{
  useDefaultStyle_ = use;
  flags_.set(BIT_USE_DEFAULT_STYLE_CHANGED);
}

}