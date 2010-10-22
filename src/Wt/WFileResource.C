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

#include <boost/lexical_cast.hpp>

namespace Wt {

WFileResource::WFileResource(WObject *parent)
  : WResource(parent),
    mimeType_("text/plain"),
    bufferSize_(8192)
{ }

WFileResource::WFileResource(const std::string& fileName, WObject *parent)
  : WResource(parent),
    mimeType_("text/plain"),
    fileName_(fileName),
    bufferSize_(8192)
{ }

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
  ::uint64_t startByte = continuation ?
      boost::any_cast< ::uint64_t >(continuation->data()) : 0;

  std::ifstream r(fileName_.c_str(), std::ios::in | std::ios::binary);

  if (startByte == 0) {
    /*
     * Initial request (not a continuation)
     */
    if (!r) {
      response.setStatus(404);
      return;
    }

    /*
     * See if we should return a range.
     */
    r.seekg(0, std::ios::end);
    std::ifstream::pos_type fsize = r.tellg();
    r.seekg(0, std::ios::beg);

    Http::Request::ByteRangeSpecifier ranges = request.getRanges(fsize);

    if (!ranges.isSatisfiable()) {
      response.setStatus(416); // Requested range not satisfiable
      response.addHeader("Content-Range",
          "bytes */" + boost::lexical_cast<std::string>(fsize));
      return;
    }

    if (ranges.size() == 1) {
      response.setStatus(206);
      startByte = ranges[0].firstByte();
      beyondLastByte_ = ranges[0].lastByte() + 1;

      std::stringstream contentRange;
      contentRange << "bytes " << startByte << "-"
		   << beyondLastByte_ - 1 << "/" << fsize;
      response.addHeader("Content-Range", contentRange.str());
      response.setContentLength(beyondLastByte_ - startByte);
    } else {
      response.setContentLength(fsize);
      beyondLastByte_ = (::uint64_t)(fsize);
    }

    response.setMimeType(mimeType_);
  }

  r.seekg(static_cast<std::streamoff>(startByte));

  char *buf = new char[bufferSize_];
  int bytesToRead = bufferSize_;
  if (startByte + bytesToRead > beyondLastByte_) {
    bytesToRead = (int)(beyondLastByte_ - startByte);
  }
  r.read(buf, bytesToRead);
  response.out().write(buf, r.gcount());
  delete[] buf;

  if (r.good() && startByte + bytesToRead < beyondLastByte_) {
    continuation = response.createContinuation();
    continuation->setData(::uint64_t(startByte + bufferSize_));
  }
}

}
