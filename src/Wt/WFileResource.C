/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>

#include "Wt/WFileResource.h"

namespace Wt {

WFileResource::WFileResource()
{ }

WFileResource::WFileResource(const std::string& fileName)
  : fileName_(fileName)
{ }

WFileResource::WFileResource(const std::string& mimeType,
			     const std::string& fileName)
  : WStreamResource(mimeType),
    fileName_(fileName)
{ }

WFileResource::~WFileResource()
{
  beingDeleted();
}

void WFileResource::setFileName(const std::string& fileName)
{
  fileName_ = fileName;
  setChanged();
}

void WFileResource::handleRequest(const Http::Request& request,
				  Http::Response& response)
{
  std::ifstream r(fileName_.c_str(), std::ios::in | std::ios::binary);
  handleRequestPiecewise(request, response, r);
}

}
