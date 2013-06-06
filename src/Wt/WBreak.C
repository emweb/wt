/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WBreak"

namespace Wt {

WBreak::WBreak(WContainerWidget *parent)
  : WWebWidget(parent)
{ 
  setInline(false);
}

DomElementType WBreak::domElementType() const
{
  return DomElement_BR;
}

}
