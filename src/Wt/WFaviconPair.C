/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WFaviconPair.h"

#include "Wt/WFileResource.h"

namespace Wt {

WFaviconPair::WFaviconPair(std::unique_ptr<WFavicon> defaultFavicon,
                           std::unique_ptr<WFavicon> updatedFavicon)
  : defaultFavicon_(std::move(defaultFavicon)),
    updatedFavicon_(std::move(updatedFavicon))
{ }

void WFaviconPair::setDefaultFavicon(std::unique_ptr<WFavicon> defaultFavicon)
{
  defaultFavicon_ = std::move(defaultFavicon);
}

void WFaviconPair::setUpdatedFavicon(std::unique_ptr<WFavicon> updatedFavicon)
{
  updatedFavicon_ = std::move(updatedFavicon);
}

std::string WFaviconPair::url() const
{
  if (currentFavicon()) {
    return currentFavicon()->url();
  }
  return std::string();
}

}