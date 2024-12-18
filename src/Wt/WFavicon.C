/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WFavicon.h"

namespace Wt {

WFavicon::WFavicon()
  : isUpdate_(false)
{ }

void WFavicon::update()
{
  isUpdate_ = true;
  doUpdate();
}

void WFavicon::reset()
{
  isUpdate_ = false;
  doReset();
}

void WFavicon::doUpdate()
{ }

void WFavicon::doReset()
{ }


}