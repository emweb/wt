/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WHBoxLayout.h"
#include "Wt/WApplication.h"

namespace Wt {

WHBoxLayout::WHBoxLayout()
  : syncDirection_(false)
{ }

void WHBoxLayout::setDirection (LayoutDirection dir)
{
  syncDirection_ = true;
  WBoxLayout::setDirection (dir);
}

LayoutDirection WHBoxLayout::direction (void)
{

  if (false == syncDirection_)
  {

    syncDirection_ = true;
    WBoxLayout::setDirection (WApplication::instance()->layoutDirection());
  }

  return (WBoxLayout::direction());
}
}
