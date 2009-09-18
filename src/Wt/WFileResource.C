/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>

#include "Wt/WFileResource"

#include "Wt/Http/Request"
#include "Wt/Http/Response"
#include "Wt/Http/ResponseContinuation"

namespace Wt {

WFileResource::WFileResource(const std::string& mimeType,
			     const std::string& fileName, WObject *parent)
  : WResource(parent),
    mimeType_(mimeType),
    fileName_(fileName),
    bufferSize_(8192)
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

void WFileResource::setMimeType(const std::string& mimeType)
{
  mimeType_ = mimeType;
  setChanged();
}

void WFileResource::setBufferSize(int bufferSize)
{
  bufferSize_ = bufferSize;
}

void WFileResource::handleRequest(const Http::Request& request,
				  Http::Response& response)
{
  Http::ResponseContinuation *continuation = request.continuation();
  int startByte = continuation ? boost::any_cast<int>(continuation->data()) : 0;

  std::ifstream r(fileName_.c_str(), std::ios::in | std::ios::binary);

  if (startByte == 0)
    response.setMimeType(mimeType_);
  else
    r.seekg(startByte);

  char *buf = new char[bufferSize_];
  r.read(buf, bufferSize_);
  response.out().write(buf, r.gcount());
  delete[] buf;

  if (r.good()) {
    continuation = response.createContinuation();
    continuation->setData(startByte + bufferSize_);
  }
}

}
