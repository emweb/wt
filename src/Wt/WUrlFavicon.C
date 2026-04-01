/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WUrlFavicon.h"

namespace Wt {

WUrlFavicon::WUrlFavicon(const std::string& url)
  : url_(url)
{ }

void WUrlFavicon::setUrl(const std::string& url)
{
  url_ = url;
}

}