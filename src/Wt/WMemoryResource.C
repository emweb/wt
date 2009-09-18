/*
 * Copyright (C) 2007 Wim Dumon, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WMemoryResource"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

namespace Wt {

WMemoryResource::WMemoryResource(WObject *parent)
  : WResource(parent)
{ }

WMemoryResource::WMemoryResource(const std::string& mimeType,
				 WObject *parent)
  : WResource(parent),
    mimeType_(mimeType)
{ }

WMemoryResource::WMemoryResource(const std::string& mimeType,
				 const std::vector<unsigned char> &data,
				 WObject *parent)
  : WResource(parent),
    mimeType_(mimeType),
    data_(data)
{ }

WMemoryResource::~WMemoryResource()
{
  beingDeleted();
}

void WMemoryResource::setMimeType(const std::string& mimeType)
{
  mimeType_ = mimeType;
  setChanged();
}

void WMemoryResource::setData(const std::vector<unsigned char>& data)
{
  data_ = data;
  setChanged();
}

void WMemoryResource::setData(const unsigned char *data, int count)
{
  data_.clear();
  data_.insert(data_.end(), data, data + count);

  setChanged();
}

void WMemoryResource::handleRequest(const Http::Request& request,
				    Http::Response& response)
{
  response.setMimeType(mimeType_);

  for (unsigned int i = 0; i < data_.size(); ++i)
    response.out().put(data_[i]);
}

}
