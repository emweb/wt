// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSELF_DELETING_RESOURCE_H_
#define WSELF_DELETING_RESOURCE_H_

#include "Wt/WMemoryResource.h"

#include <vector>
#include <string>

namespace Wt {

/*! \class WSelfDeletingResource Wt/WSelfDeletingResource.h Wt/WSelfDeletingResource.h
 *  \brief A resource which removes itself after being served.
 *
 * This resource will automatically unexpose itself after being served.
 *
 * This resource is mainly intended to be used as output for the
 * WResource::botResource() in order to free memory once the bot has
 * fetched the resource.
 */
class WT_API WSelfDeletingResource : public WMemoryResource
{
public:
  /*! \brief Creates a new resource with given mime-type.
   *
   *  You must call setData() before using the resource.
   */
  WSelfDeletingResource(const std::string& mimeType);

#ifndef WT_TARGET_JAVA
  //! Creates a new resource with given mime-type and data
  WSelfDeletingResource(const std::string& mimeType,
                       const std::vector<unsigned char>& data);
#endif // WT_TARGET_JAVA

  //! Destructor.
  ~WSelfDeletingResource();

  void handleRequest(const Http::Request& request,
                     Http::Response& response) override;
};

}

#endif // WSELF_DELETING_RESOURCE_H_
