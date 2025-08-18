/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSelfDeletingResource.h"
#include "Wt/Http/Response.h"

#include "Wt/WServer.h"

namespace Wt {

WSelfDeletingResource::WSelfDeletingResource(const std::string& mimeType)
  : WMemoryResource(mimeType)
{
  setAllowAutoRemoval(true);
}

#ifndef WT_TARGET_JAVA
WSelfDeletingResource::WSelfDeletingResource(const std::string& mimeType,
                                             const std::vector<unsigned char>& data)
  : WMemoryResource(mimeType, data)
{
  setAllowAutoRemoval(true);
}
#endif // WT_TARGET_JAVA

WSelfDeletingResource::~WSelfDeletingResource()
{
  beingDeleted();
}

void WSelfDeletingResource::handleRequest(const Http::Request& request,
                                          Http::Response& response)
{
  WMemoryResource::handleRequest(request, response);

  WApplication *app = WApplication::instance();
  if (app) {
    app->removeExposedResource(this);
  } else {
    WServer::instance()->removeResource(this);
  }
}

} // namespace Wt
