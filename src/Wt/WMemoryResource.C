/*
 * Copyright (C) Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WMemoryResource.h"
#include "Wt/Http/Response.h"

namespace Wt {

WMemoryResource::WMemoryResource()
{ 
  create();
}

WMemoryResource::WMemoryResource(const std::string& mimeType)
  : mimeType_(mimeType),
    data_(new std::vector<unsigned char>())
{ 
  create();
}

WMemoryResource::WMemoryResource(const std::string& mimeType,
				 const std::vector<unsigned char> &data)
  : mimeType_(mimeType),
    data_(new std::vector<unsigned char>(data))
{ 
  create();
}

void WMemoryResource::create()
{
#ifdef WT_THREADED
  dataMutex_.reset(new std::mutex());
#endif // WT_THREADED
}

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
  {
#ifdef WT_THREADED
    std::unique_lock<std::mutex> lock(*dataMutex_);
#endif // WT_THREADED

    data_.reset(new std::vector<unsigned char>(data));
  }

  setChanged();
}

void WMemoryResource::setData(const unsigned char *data, int count)
{
  {
#ifdef WT_THREADED
    std::unique_lock<std::mutex> l(*dataMutex_);
#endif
    data_.reset(new std::vector<unsigned char>(data, data + count));
  }

  setChanged();
}

const std::vector<unsigned char> WMemoryResource::data() const
{
  DataPtr data;

  {
#ifdef WT_THREADED
    std::unique_lock<std::mutex> l(*dataMutex_);
#endif
    data = data_;
  }

  if (!data)
    return std::vector<unsigned char>();
  else
    return *data;
}

void WMemoryResource::handleRequest(const Http::Request& request,
				    Http::Response& response)
{
  DataPtr data;
  {
#ifdef WT_THREADED
    std::unique_lock<std::mutex> l(*dataMutex_);
#endif
    data = data_;
  }

  if (!data)
    return;

  response.setMimeType(mimeType_);

  for (unsigned int i = 0; i < (*data).size(); ++i)
    response.out().put((*data)[i]);
}

}
