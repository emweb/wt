/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>

#include "Wt/WFileResource"
#include "Wt/WLogger"

namespace Wt {

LOGGER("WFileResource");

WFileResource::WFileResource(WObject *parent)
  : WStreamResource(parent)
{ }

WFileResource::WFileResource(const std::string& fileName, WObject *parent)
  : WStreamResource(parent),
    fileName_(fileName)
{ }

WFileResource::WFileResource(const std::string& mimeType,
			     const std::string& fileName, WObject *parent)
  : WStreamResource(mimeType, parent),
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
  if (!r) {
    LOG_ERROR("Could not open file for reading: " << fileName_);
  }
  handleRequestPiecewise(request, response, r);
}

}
