/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WStreamResource.h"

#include "Wt/Http/Request.h"
#include "Wt/Http/Response.h"

#include <boost/scoped_array.hpp>

namespace Wt {

WStreamResource::WStreamResource()
  : mimeType_("text/plain"),
    bufferSize_(8192)
{ }

WStreamResource::WStreamResource(const std::string& mimeType)
  : mimeType_(mimeType),
    bufferSize_(8192)
{ }

WStreamResource::~WStreamResource()
{
  beingDeleted();
}

void WStreamResource::setMimeType(const std::string& mimeType)
{
  mimeType_ = mimeType;
  setChanged();
}

void WStreamResource::setBufferSize(int bufferSize)
{
  bufferSize_ = bufferSize;
}

void WStreamResource::handleRequestPiecewise(const Http::Request& request,
                                             Http::Response& response,
                                             std::istream& input)
{
  typedef std::pair<::uint64_t, std::streamsize> State;

  ::uint64_t startByte = 0;
  std::streamsize beyondLastByte;
  Http::ResponseContinuation *continuation = request.continuation();

  if (continuation) {
    State state = cpp17::any_cast<State>(continuation->data());
    startByte = state.first;
    beyondLastByte = state.second;
  }

  if (startByte == 0) {
    /*
     * Initial request (not a continuation)
     */
    if (!input) {
      response.setStatus(404);
      return;
    } else
      response.setStatus(200);

    /*
     * See if we should return a range.
     */
    input.seekg(0, std::ios::end);
    std::istream::pos_type isize = input.tellg();
    input.seekg(0, std::ios::beg);

    Http::Request::ByteRangeSpecifier ranges = request.getRanges(isize);

    if (!ranges.isSatisfiable()) {
      std::ostringstream contentRange;
      contentRange << "bytes */" << isize;
      response.setStatus(416); // Requested range not satisfiable
      response.addHeader("Content-Range", contentRange.str());
      return;
    }

    if (ranges.size() == 1) {
      response.setStatus(206);
      startByte = ranges[0].firstByte();
      beyondLastByte = std::streamsize(ranges[0].lastByte() + 1);

      std::ostringstream contentRange;
      contentRange << "bytes " << startByte << "-"
                   << beyondLastByte - 1 << "/" << isize;
      response.addHeader("Content-Range", contentRange.str());
      response.setContentLength(::uint64_t(beyondLastByte) - startByte);
    } else {
      beyondLastByte = std::streamsize(isize);
      response.setContentLength(::uint64_t(beyondLastByte));
    }

    response.setMimeType(mimeType_);
  }

  input.seekg(static_cast<std::istream::pos_type>(startByte));

  // According to 27.6.1.3, paragraph 1 of ISO/IEC 14882:2003(E),
  // each unformatted input function may throw an exception.
  boost::scoped_array<char> buf(new char[bufferSize_]);
  std::streamsize sbufferSize = std::streamsize(bufferSize_);

  std::streamsize restSize = beyondLastByte - std::streamsize(startByte);
  std::streamsize pieceSize =  sbufferSize > restSize ? restSize : sbufferSize;

  input.read(buf.get(), pieceSize);
  std::streamsize actualPieceSize = input.gcount();
  response.out().write(buf.get(), actualPieceSize);

  if (input.good() && actualPieceSize < restSize) {
    continuation = response.createContinuation();
    continuation->setData(State(startByte + ::uint64_t(actualPieceSize), beyondLastByte));
  }
}

}
