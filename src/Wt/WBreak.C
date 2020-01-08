/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WBreak.h"

namespace Wt {

WBreak::WBreak()
{ 
  setInline(false);
}

DomElementType WBreak::domElementType() const
{
  return DomElementType::BR;
}

}
