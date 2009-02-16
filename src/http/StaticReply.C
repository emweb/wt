/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>

#include "Request.h"
#include "StaticReply.h"
#include "StockReply.h"
#include "MimeTypes.h"

namespace http {
namespace server {

StaticReply::StaticReply(const std::string &full_path,
			 const std::string &extension,
			 const Request& request,
			 const std::string &err_root)
  : Reply(request),
    path_(full_path),
    extension_(extension)
{
  bool stockReply = false;
  bool gzipReply = false;
  std::string modifiedDate, etag;

  if (request.acceptGzipEncoding()) {
    std::string gzipPath = path_ + ".gz";
    stream_.open(gzipPath.c_str(), std::ios::in | std::ios::binary);

    if (stream_) {
      path_ = gzipPath;
      gzipReply = true;
    } else {
      stream_.clear();
      stream_.open(path_.c_str(), std::ios::in | std::ios::binary);
    }
  } else
    stream_.open(path_.c_str(), std::ios::in | std::ios::binary);

  if (!stream_) {
    stockReply = true;
    setRelay(ReplyPtr(new StockReply(request, StockReply::not_found,
				     "", err_root)));
  } else {
    try {
      fileSize_ = boost::filesystem::file_size(path_);
      modifiedDate = computeModifiedDate();
      etag = computeETag();
    } catch (...) {
      fileSize_ = -1;
    }
  }

  if (!stockReply) {
    /*
     * Check if can send a 304 not modified reply
     */
    Request::HeaderMap::const_iterator ims
      = request.headerMap.find("If-Modified-Since");
    Request::HeaderMap::const_iterator inm
      = request.headerMap.find("If-None-Match");

    if ((ims != request.headerMap.end() && ims->second == modifiedDate)
	|| (inm != request.headerMap.end() && inm->second == etag)) {
      stockReply = true;
      setRelay(ReplyPtr(new StockReply(request, StockReply::not_modified)));
    }
  }

  if (!stockReply) {
    /*
     * Add headers for caching, but not for IE since it in fact makes it
     * cache less (images)
     */
    Request::HeaderMap::const_iterator ua=request.headerMap.find("User-Agent");

    if (   ua == request.headerMap.end()
	|| ua->second.find("MSIE") == std::string::npos) {
      addHeader("Cache-Control", "max-age=3600");
      if (!etag.empty())
	addHeader("ETag", etag);

      addHeader("Expires", computeExpires());
    }

    if (!modifiedDate.empty())
      addHeader("Last-Modified", modifiedDate);
  }

  if (gzipReply)
    addHeader("Content-Encoding", "gzip");
}

std::string StaticReply::computeModifiedDate() const
{
  return httpDate(boost::filesystem::last_write_time(path_));
}

std::string StaticReply::computeETag() const
{
  return boost::lexical_cast<std::string>(fileSize_)
    + "-" + computeModifiedDate();
}

std::string StaticReply::computeExpires()
{
  time_t t = time(0);
  t += 3600*24*31;
  return httpDate(t);
}

void StaticReply::consumeRequestBody(Buffer::const_iterator begin,
				     Buffer::const_iterator end,
				     bool endOfRequest)
{
  if (endOfRequest)
    transmitMore();
}

Reply::status_type StaticReply::responseStatus()
{
  return ok;
}

std::string StaticReply::contentType()
{
  return mime_types::extensionToType(extension_);
}

boost::intmax_t StaticReply::contentLength()
{
  return fileSize_;
}

asio::const_buffer StaticReply::nextContentBuffer()
{
  if (request_.method == "HEAD")
    return emptyBuffer;
  else {
    stream_.read(buf_, sizeof(buf_));

    if (stream_.gcount() > 0) {
      return asio::buffer(buf_, stream_.gcount());
    } else
      return emptyBuffer;
  }
}

}
}
