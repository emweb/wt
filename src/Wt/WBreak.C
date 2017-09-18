/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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
