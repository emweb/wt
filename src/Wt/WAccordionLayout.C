/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAccordionLayout"

namespace Wt {

WAccordionLayout::WAccordionLayout(WWidget *parent)
  : WDefaultLayout()
{ 
  if (parent)
    setLayoutInParent(parent);
}

}
