/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <sstream>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "Wt/Http/Request.h"
#include "Wt/Utils.h"
#include "Wt/WEnvironment.h"
#include "Wt/WServer.h"
#include "Wt/WSslInfo.h"

#include "Configuration.h"
#include "WebUtils.h"
#include "WebRequest.h"
#include "Message.h"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

namespace {
  std::stringstream emptyStream;

  inline std::string str(const char *s) {
    return s ? std::string(s) : std::string();
  }

#ifndef WT_TARGET_JAVA
  static std::vector<std::pair<std::string, std::string> > EMPTY_URL_PARAMS;
#endif // WT_TARGET_JAVA
}

namespace Wt {
  namespace Http {

UploadedFile::UploadedFile()
{}
    
UploadedFile::UploadedFile(const std::string& spoolName,
			   const std::string& clientFileName,
			   const std::string& contentType)
{
  fileInfo_.reset(new Impl());

  fileInfo_->spoolFileName = spoolName;
  fileInfo_->clientFileName = clientFileName;
  fileInfo_->contentType = contentType;
  fileInfo_->isStolen = false;
}

void UploadedFile::Impl::cleanup()
{
  if (!isStolen)
    unlink(spoolFileName.c_str());
}

UploadedFile::Impl::~Impl()
{
  cleanup();
}

const std::string& UploadedFile::spoolFileName() const
{
  return fileInfo_->spoolFileName;
}

const std::string& UploadedFile::clientFileName() const
{
  return fileInfo_->clientFileName;
}

const std::string& UploadedFile::contentType() const
{
  return fileInfo_->contentType;
}

void UploadedFile::stealSpoolFile() const
{
  fileInfo_->isStolen = true;
}

Request::ByteRange::ByteRange()
  : firstByte_(0),
    lastByte_(0)
{
}

Request::ByteRange::ByteRange(::uint64_t first, ::uint64_t last)
  : firstByte_(first),
    lastByte_(last)
{ }

Request::ByteRangeSpecifier::ByteRangeSpecifier()
  : satisfiable_(true)
{ }

const std::string *get(const ParameterMap& map, const std::string& name)
{
  ParameterMap::const_iterator i = map.find(name);  

  if (i != map.end())
    return &i->second[0];
  else
    return nullptr;
}

const ParameterValues& Request::getParameterValues(const std::string& name)
  const
{
  ParameterMap::const_iterator i = parameters_.find(name);
  if (i != parameters_.end())
    return i->second;
  else
    return WebRequest::emptyValues_;
}

const std::string *Request::getParameter(const std::string& name) const
{
  const ParameterValues& v = getParameterValues(name);
  if (!Utils::isEmpty(v))
    return &v[0];
  else
    return nullptr;
}

const UploadedFile *Request::getUploadedFile(const std::string& name) const
{
  UploadedFileMap::const_iterator i = files_.find(name);
  if (i != files_.end())
    return &i->second;
  else
    return nullptr;
}

std::string Request::method() const
{
  return request_ ? str(request_->requestMethod()) : str("GET");
}

std::string Request::serverName() const
{
  return request_ ? request_->serverName() : std::string();
}

std::string Request::serverPort() const
{
  return request_ ? request_->serverPort() : std::string();
}

std::string Request::path() const
{
  return request_ ? request_->scriptName() : std::string();
}

std::string Request::pathInfo() const
{
  return request_ ? request_->pathInfo() : std::string();
}

std::string Request::queryString() const
{
  return request_ ? request_->queryString() : std::string();
}

std::string Request::urlScheme() const
{
  return request_ ? request_->urlScheme() : std::string();
}

std::string Request::headerValue(const std::string& field) const
{
  return request_ ? str(request_->headerValue(field.c_str())) : std::string();
}

std::string Request::getCgiValue(const std::string& varName) const
{
  return request_ ? str(request_->envValue(varName.c_str())) : std::string();
}

#ifndef WT_TARGET_JAVA
std::vector<Message::Header> Request::headers() const
{
  return request_->headers();
}
#endif

::int64_t Request::tooLarge() const
{
  return request_ ? request_->postDataExceeded() : 0;
}

std::istream& Request::in() const
{
  if (request_) {
    WebRequest *web = const_cast<WebRequest *>(request_);
    return web->in();
  } else {
    return emptyStream;
  }
}

std::string Request::contentType() const
{
  return request_ ? str(request_->contentType()) : std::string();
}

int Request::contentLength() const
{
  return request_ ? request_->contentLength() : 0;
}

std::string Request::userAgent() const
{
  return request_ ? str(request_->userAgent()) : std::string();
}

std::string Request::clientAddress() const
{
  if (!request_)
    return std::string();

  WServer *server = WServer::instance();
  return request_->clientAddress(server->configuration());
}

WSslInfo *Request::sslInfo() const
{
  if (sslInfo_)
    return sslInfo_.get();
  if (request_) {
    auto server = WServer::instance();
    sslInfo_ = request_->sslInfo(server->configuration());
  }
  return sslInfo_.get();
}

Request::ByteRangeSpecifier Request::getRanges(::int64_t filesize) const
{
  return getRanges(headerValue("Range"), filesize);
}

Request::ByteRangeSpecifier Request::getRanges(const std::string &rangeHdr,
                                               ::int64_t filesize)
{
  Request::ByteRangeSpecifier retval;

  if (filesize == 0) {
    if (rangeHdr.empty()) {
      retval.setSatisfiable(true);
    } else {
      // Don't waste CPU time and simplify code below.
      retval.setSatisfiable(false);
    }
    return retval;
  }

  bool syntaxError = false;
  bool satisfiable = filesize == -1;
  std::vector<std::string> rangeSpecifier;
  boost::split(rangeSpecifier, rangeHdr, boost::is_any_of("="));

  if (rangeSpecifier.size() == 2) {
    boost::trim(rangeSpecifier[0]);
    if (boost::iequals(rangeSpecifier[0], "bytes")) {
      std::vector<std::string> ranges;
      boost::split(ranges, rangeSpecifier[1], boost::is_any_of(","));
      for (std::size_t i = 0; i < ranges.size(); ++i) {
        std::vector<std::string> range;
        boost::split(range, ranges[i], boost::is_any_of("-"));
        if (range.size() == 2) {
          std::string start = range[0];
          std::string end = range[1];

	  boost::trim(start);
	  boost::trim(end);

          uint64_t startInt=0, endInt=0;
          try {
            if (start != "")
              startInt = Utils::stoll(start);
            if (end != "")
              endInt = Utils::stoll(end);
          } catch (std::exception& ) {
            // syntactically invalid
            syntaxError = true;
          }
          if (start == "") {
            // notation -599: return last 599 bytes
            if (filesize != -1 && end != "") {
              if (endInt >= (uint64_t)filesize) {
                endInt = (std::size_t)filesize;
              }
              if (endInt > 0) {
                satisfiable = true;
                retval.push_back
		  (ByteRange
		   (uint64_t(filesize - endInt), std::size_t(filesize - 1)));
              }
              else {
                // Not really specified as syntax error. The paragraph about
                // 'satisfiability' seems to imply that we should simply
                // ignore it.
              }
            } else {
              // syntactically invalid
              syntaxError = true;
            }
          } else {
            if (filesize == -1 || startInt < (uint64_t)filesize) {
              if (end == "") {
                satisfiable = true;
                // notation 599-: returns from byte 599 to eof
                if (filesize == -1)
                  retval.push_back
		    (ByteRange(startInt, std::numeric_limits<uint64_t>::max()));
                else
                  retval.push_back
		    (ByteRange
		     (startInt, uint64_t(filesize - 1)));
              } else {
                if (startInt <= endInt) {
                  satisfiable = true;
                  if (filesize >= 0 && endInt > (uint64_t)filesize)
                    endInt = uint64_t(filesize - 1);
                  retval.push_back(ByteRange(startInt, endInt));
                } else {
                  // syntactically invalid
                  syntaxError = true;
                }
              }
            } else {
              // Not-satisfiable: just skip this range
            }
          }
        } else {
          syntaxError = true;
        }
      }
    } else {
      // only understand 'bytes'
      syntaxError = true;
    }
  } else {
    // Too many equals
    syntaxError = true;
  }
  if (syntaxError) {
    return ByteRangeSpecifier();
  } else {
    retval.setSatisfiable(satisfiable);
    return retval;
  }
}

Request::Request(const WebRequest& request, ResponseContinuation *continuation)
  : request_(&request),
    parameters_(request.getParameterMap()),
    files_(request.uploadedFiles()),
    continuation_(continuation)
{
  if (!continuation) {
    const char *cookie = request_->headerValue("Cookie");
    if (cookie)
      parseCookies(cookie, cookies_);
  }
}

Request::Request(const ParameterMap& parameters, const UploadedFileMap& files)
  : request_(nullptr),
    parameters_(parameters),
    files_(files),
    continuation_(0)
{ }

Request::~Request()
{ }

#ifndef WT_TARGET_JAVA
void Request::parseFormUrlEncoded(const std::string& s,
				  ParameterMap& parameters)
{
  for (std::size_t pos = 0; pos < s.length();) {
    std::size_t next = s.find_first_of("&=", pos);

    if (next == pos && s[next] == '&') {
      // skip empty
      pos = next + 1;
      continue;
    }

    if (next == std::string::npos || s[next] == '&') {
      if (next == std::string::npos)
	next = s.length();
      std::string key = s.substr(pos, next - pos);
      Utils::inplaceUrlDecode(key);
      parameters[key].push_back(std::string());
      pos = next + 1;
    } else {
      std::size_t amp = s.find('&', next + 1);
      if (amp == std::string::npos)
	amp = s.length();

      std::string key = s.substr(pos, next - pos);
      Utils::inplaceUrlDecode(key);

      std::string value = s.substr(next + 1, amp - (next + 1));
      Utils::inplaceUrlDecode(value);

      parameters[key].push_back(value);
      pos = amp + 1;
    }
  }
}

#endif // WT_TARGET_JAVA

void Request::parseCookies(const std::string& cookie,
			   std::map<std::string, std::string>& result)
{
  // in WEnvironment for oink
  WEnvironment::parseCookies(cookie, result);
}

const std::string *Request::getCookieValue(const std::string& cookieName) const
{
  CookieMap::const_iterator i = cookies_.find(cookieName);

  if (i == cookies_.end())
    return nullptr;
  else
    return &i->second;
}

#ifndef WT_TARGET_JAVA
std::string Request::urlParam(const std::string &param) const
{
  if (!request_)
    return "";

  const auto &params = urlParams();
  for (const auto &p : params) {
    if (p.first == param)
      return p.second;
  }
  return "";
}

const std::vector<std::pair<std::string, std::string> >& Request::urlParams() const
{
  return request_ ? request_->urlParams() : EMPTY_URL_PARAMS;
}
#endif // WT_TARGET_JAVA

  }
}
