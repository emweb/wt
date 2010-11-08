/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/spirit/include/classic_core.hpp>

#include "Request.h"
#include "StaticReply.h"
#include "StockReply.h"
#include "MimeTypes.h"


using namespace BOOST_SPIRIT_CLASSIC_NS;

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

  parseRangeHeader();

  // Do not consider .gz files if we will respond with a range, as we cannot
  // stream partial data from a .gz file
  if (request.acceptGzipEncoding() && !hasRange_) {
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

  // Can't specify zero-length Content-Range headers. But for zero-length
  // files, we just ignore the Range header and send the full file instead of
  // a 416 Requested Range Not Satisfiable error
  if (stockReply || (fileSize_ == 0))
    hasRange_ = false;

  if ((!stockReply) && hasRange_) {
    stream_.seekg((std::streamoff)rangeBegin_, std::ios_base::cur);
    std::streamoff curpos = stream_.tellg();
    if (curpos != rangeBegin_) {
      // Won't be able to send even a single byte -> error 416
      stockReply = true;
      ReplyPtr sr(new StockReply(request,
        StockReply::requested_range_not_satisfiable, "", err_root));
      if (fileSize_ != -1) {
        // 416 SHOULD include a Content-Range with byte-range-resp-spec * and
        // instance-length set to current lenght
        sr->addHeader("Content-Range",
          "bytes */" + boost::lexical_cast<std::string>(fileSize_));
      }
      setRelay(sr);
    } else {
      ::int64_t last = rangeEnd_;
      if (fileSize_ != -1 && last >= fileSize_) {
        last = fileSize_ - 1;
      }
      std::stringstream contentRange;
      // Note: if fileSize is unknown, we're not sure we'll be able to
      // transmit the requested range (i.e. when the file is not large enough
      // to satisfy the request). Wt wil report that it understood the request,
      // and close the link prematurely if it can't provide the requested bytes
      contentRange << "bytes " << rangeBegin_ << "-" << last << "/";
      if (fileSize_ == -1) {
        contentRange << "*";
      } else {
        contentRange << fileSize_;
      }
      addHeader("Content-Range", contentRange.str());
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
    } else {
      // We experienced problems with some swf files if they are cached in IE.
      // Therefore, don't cache swf files on IE.
      if (boost::iequals(extension_, "swf")) {
        addHeader("Cache-Control", "no-cache");
      }
    }

    if (!modifiedDate.empty())
      addHeader("Last-Modified", modifiedDate);
  }
 
  if ((!stockReply) && gzipReply)
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

void StaticReply::consumeData(Buffer::const_iterator begin,
			      Buffer::const_iterator end,
			      Request::State state)
{
  if (state != Request::Partial)
    send();
}

Reply::status_type StaticReply::responseStatus()
{
  if (hasRange_) {
    return partial_content;
  } else {
    return ok;
  }
}

std::string StaticReply::contentType()
{
  return mime_types::extensionToType(extension_);
}

::int64_t StaticReply::contentLength()
{
  if (hasRange_) {
    if (fileSize_ == -1) {
      return -1;
    }
    if (rangeBegin_ >= fileSize_) {
      return 0;
    }
    if (rangeEnd_ < fileSize_) {
      return rangeEnd_ - rangeBegin_ + 1;
    } else {
      return fileSize_ - rangeBegin_;
    }
  } else {
    return fileSize_;
  }
}

asio::const_buffer StaticReply::nextContentBuffer()
{
  if (request_.method == "HEAD")
    return emptyBuffer;
  else {
    boost::uintmax_t rangeRemainder
      = (std::numeric_limits< ::int64_t>::max)();
    if (hasRange_)
      rangeRemainder = rangeEnd_ - stream_.tellg() + 1;
    stream_.read(buf_,
                 (std::streamsize)(std::min<boost::uintmax_t>)(rangeRemainder,
                                                               sizeof(buf_)));

    if (stream_.gcount() > 0) {
      return asio::buffer(buf_, stream_.gcount());
    } else
      return emptyBuffer;
  }
}

void StaticReply::parseRangeHeader()
{
  // Wt only support these types of ranges for now:
  // Range: bytes=0-
  // Range: bytes=10-
  // Range: bytes=250-499
  // NOT SUPPORTED: multiple ranges, and the suffix-byte-range-spec:
  // Range: bytes=10-20,30-40
  // Range: bytes=-500 // 'last 500 bytes'
  Request::HeaderMap::const_iterator range
    = request_.headerMap.find("Range");

  hasRange_ = false;
  rangeBegin_ = (std::numeric_limits< ::int64_t>::max)();
  rangeEnd_ = (std::numeric_limits< ::int64_t>::max)();
  if (range != request_.headerMap.end()) {
    std::string rangeHeader = range->second;
    uint_parser< ::int64_t> const uint_max_p = uint_parser< ::int64_t>();
    hasRange_ = parse(rangeHeader.c_str(),
      str_p("bytes") >> ch_p('=') >>
      (uint_max_p[assign_a(rangeBegin_)] >>
      ch_p('-') >>
      !uint_max_p[assign_a(rangeEnd_)]),
      space_p).full;
    if (hasRange_) {
      // Validation of the Range header
      if (rangeBegin_ > rangeEnd_)
        hasRange_ = false;
    }
  }
}

}
}
