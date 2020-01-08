/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WLinkedCssStyleSheet.h"
#include "Wt/WStringStream.h"

namespace Wt {

WLinkedCssStyleSheet::WLinkedCssStyleSheet(const WLink& link,
					   const std::string& media)
  : link_(link),
    media_(media)
{ }

void WLinkedCssStyleSheet::cssText(WStringStream& out) const
{
  WApplication *app = WApplication::instance();
  out << "@import url(\"" << link_.resolveUrl(app) << "\")";

  if (!media_.empty() && media_ != "all")
    out << " " << media_;
  out << ";\n";
}

} // namespace Wt
