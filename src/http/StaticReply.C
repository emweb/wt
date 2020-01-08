/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#include <boost/spirit/include/classic_core.hpp>

#include "Configuration.h"
#include "StaticReply.h"
#include "Request.h"
#include "StockReply.h"
#include "MimeTypes.h"

#include "FileUtils.h"

#include "Wt/WLogger.h"

using namespace BOOST_SPIRIT_CLASSIC_NS;

namespace Wt {
  LOGGER("wthttp");
}

namespace {

static bool openStream(std::ifstream &stream, std::string &path, bool acceptGzip) {
  bool gzipReply = false;
  if (acceptGzip) {
    std::string gzipPath = path + ".gz";
    stream.open(gzipPath.c_str(), std::ios::in | std::ios::binary);

    if (stream) {
      path = gzipPath;
      gzipReply = true;
    } else {
      stream.clear();
      stream.open(path.c_str(), std::ios::in | std::ios::binary);
    }
  } else {
    stream.open(path.c_str(), std::ios::in | std::ios::binary);
  }
  return gzipReply;
}

}

namespace http {
namespace server {

StaticReply::StaticReply(Request& request, const Configuration& config)
  : Reply(request, config)
{
  reset(0);
}

void StaticReply::reset(const Wt::EntryPoint *ep)
{
  Reply::reset(ep);

  stream_.close();
  stream_.clear();

  hasRange_ = false;

  std::string request_path = request_.request_path;

  // Request path for a static file must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos) {
    setRelay(ReplyPtr(new StockReply(request_, StockReply::not_found,
				     "", configuration())));
    return;
  }

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (request_path[request_path.size() - 1] == '/')
    request_path += "index.html";

  // Determine the file extension.
  std::size_t last_slash_pos = request_path.find_last_of("/");
  std::size_t last_dot_pos = request_path.find_last_of(".");

  if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
    extension_ = request_path.substr(last_dot_pos + 1);
  else
    extension_.clear();

  path_ = configuration().docRoot() + request_path;

  bool gzipReply = false;
  std::string modifiedDate, etag;

  parseRangeHeader();

  // Do not consider .gz files if we will respond with a range, as we cannot
  // stream partial data from a .gz file
  bool acceptGzip = request_.acceptGzipEncoding() && !hasRange_;
  gzipReply = openStream(stream_, path_, acceptGzip);

  // Try fallback resources folder if not found
  if (!stream_ && !configuration().resourcesDir().empty() &&
      boost::starts_with(request_path, "/resources/")) {
    path_ = configuration().resourcesDir() + request_path.substr(sizeof("/resources") - 1);
    gzipReply = openStream(stream_, path_, acceptGzip);
  }

  if (!stream_) {
    setRelay(ReplyPtr(new StockReply(request_, StockReply::not_found,
				     "", configuration())));
    return;
  } else {
    try {
      fileSize_ = Wt::FileUtils::size(path_);
      modifiedDate = computeModifiedDate();
      etag = computeETag();
    } catch (...) {
      fileSize_ = -1;
    }
  }

  // Can't specify zero-length Content-Range headers. But for zero-length
  // files, we just ignore the Range header and send the full file instead of
  // a 416 Requested Range Not Satisfiable error
  if (fileSize_ == 0)
    hasRange_ = false;

  if (hasRange_) {
    stream_.seekg((std::streamoff)rangeBegin_, std::ios_base::cur);
    std::streamoff curpos = stream_.tellg();
    if (curpos != rangeBegin_) {
      // Won't be able to send even a single byte -> error 416
      ReplyPtr sr(new StockReply
		  (request_, StockReply::requested_range_not_satisfiable,
		   "", configuration()));
      if (fileSize_ != -1) {
        // 416 SHOULD include a Content-Range with byte-range-resp-spec * and
        // instance-length set to current lenght
        sr->addHeader("Content-Range", "bytes */" + std::to_string(fileSize_));
      }
      setRelay(sr);
      stream_.close();
      return;
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

      LOG_INFO("sending: " << contentRange.str());

      addHeader("Content-Range", contentRange.str());
    }
  }

  /*
   * Check if can send a 304 not modified reply
   */
  const Request::Header *ims = request_.getHeader("If-Modified-Since");
  const Request::Header *inm = request_.getHeader("If-None-Match");

  if ((ims && ims->value == modifiedDate) || (inm && inm->value == etag)) {
    setRelay(ReplyPtr(new StockReply(request_, StockReply::not_modified,
				     configuration())));
    stream_.close();
    return;
  }

  /*
   * Add headers for caching, but not for IE since it in fact makes it
   * cache less (images)
   */
  const Request::Header *ua = request_.getHeader("User-Agent");

  if (!ua || !ua->value.contains("MSIE")) {
    addHeader("Cache-Control", "max-age=3600");
    if (!etag.empty())
      addHeader("ETag", etag);

    addHeader("Expires", computeExpires());
  } else {
    // We experienced problems with some swf files if they are cached in IE.
    // Therefore, don't cache swf files on IE.
    if (boost::iequals(extension_, "swf"))
      addHeader("Cache-Control", "no-cache");
  }

  if (!modifiedDate.empty())
    addHeader("Last-Modified", modifiedDate);
 
  if (gzipReply)
    addHeader("Content-Encoding", "gzip");

  if (hasRange_)
    setStatus(partial_content);
  else
    setStatus(ok);
}

std::string StaticReply::computeModifiedDate() const
{
  return httpDate(Wt::FileUtils::lastWriteTime(path_));
}

std::string StaticReply::computeETag() const
{
  return std::to_string(fileSize_) + "-" + computeModifiedDate();
}

std::string StaticReply::computeExpires()
{
  time_t t = time(0);
  t += 3600*24*31;
  return httpDate(t);
}

bool StaticReply::consumeData(const char *begin,
			      const char *end,
			      Request::State state)
{
  if (state != Request::Partial)
    send();
  return true;
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

void StaticReply::writeDone(bool success)
{
  if (relay()) {
    relay()->writeDone(success);
    return;
  }

  if (success && stream_.is_open())
    send();
}

bool StaticReply::nextContentBuffers(std::vector<asio::const_buffer>& result)
{
  if (request_.method != "HEAD") {
    boost::uintmax_t rangeRemainder = (std::numeric_limits< ::int64_t>::max)();

    if (hasRange_)
      rangeRemainder = rangeEnd_ - stream_.tellg() + 1;

    stream_.read(buf_, (std::streamsize)
		 (std::min<boost::uintmax_t>)(rangeRemainder, sizeof(buf_)));

    if (stream_.gcount() > 0) {
      result.push_back(asio::buffer(buf_, stream_.gcount()));
      return false;
    } else {
      stream_.close();
      return true;
    }
  } else {
    stream_.close();
    return true;
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
  const Request::Header *range = request_.getHeader("Range");

  hasRange_ = false;
  rangeBegin_ = (std::numeric_limits< ::int64_t>::max)();
  rangeEnd_ = (std::numeric_limits< ::int64_t>::max)();
  if (range) {
    std::string rangeHeader = range->value.str();

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
