/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdexcept>
#include <sstream>

#include "Wt/Http/Request"
#include "WebRequest.h"

namespace {
  std::stringstream emptyStream;
}

namespace Wt {
  namespace Http {

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
  if (!v.empty())
    return &v[0];
  else
    return 0;
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

int Request::tooLarge() const
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
  return request_ ? request_->contentType() : std::string();
}

int Request::contentLength() const
{
  return request_ ? request_->contentLength() : 0;
}

std::string Request::userAgent() const
{
  return request_ ? request_->userAgent() : std::string();
}

std::string Request::clientAddress() const
{
  return request_ ? request_->remoteAddr() : std::string();
}

Request::Request(const WebRequest& request, ResponseContinuation *continuation)
  : request_(&request),
    parameters_(request.getParameterMap()),
    files_(request.uploadedFiles()),
    continuation_(continuation)
{ }

Request::Request(const ParameterMap& parameters, const UploadedFileMap& files)
  : request_(0),
    parameters_(parameters),
    files_(files),
    continuation_(0)
{ }

  }
}
