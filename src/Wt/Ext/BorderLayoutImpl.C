/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/BorderLayoutImpl.h"
#include "Wt/WBorderLayout"

namespace Wt {
  namespace Ext {

BorderLayoutImpl::BorderLayoutImpl(WBorderLayout *layout)
  : LayoutImpl(layout)
{ }

void BorderLayoutImpl::createConfig(std::ostream& config)
{
  config << ",layout:'border'";

  LayoutImpl::createConfig(config);
}

void BorderLayoutImpl::addLayoutConfig(LayoutItemImpl *item,
				       std::ostream& config)
{
  const char *regionStr[] = { "north", "east", "south", "west", "center" };

  WBorderLayout *l = dynamic_cast<WBorderLayout *>(layout());
  WBorderLayout::Position p = l->position(item->layoutItem());

  config << ",region:'" << regionStr[p] << '\'';
}

  }
}
