/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WResourceFavicon.h"

#include "Wt/WFileResource.h"

namespace Wt {

WResourceFavicon::WResourceFavicon(const std::shared_ptr<WResource>& resource)
  : resource_(resource)
{ }

WResourceFavicon::WResourceFavicon(const std::string& filename,
                                   const std::string& mimeType)
  : resource_(std::make_shared<WFileResource>(mimeType, filename))
{ }

void WResourceFavicon::setResource(const std::shared_ptr<WResource>& resource)
{
  resource_ = resource;
}

std::string WResourceFavicon::url() const
{
  if (resource_) {
    return resource_->url();
  }
  return std::string();
}

}