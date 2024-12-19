/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WFavicon.h"

namespace Wt {

WFavicon::WFavicon()
  : isUpdated_(false)
{ }

void WFavicon::update()
{
  isUpdated_ = true;
  doUpdate();
}

void WFavicon::reset()
{
  isUpdated_ = false;
  doReset();
}

void WFavicon::doUpdate()
{ }

void WFavicon::doReset()
{ }


}