/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdexcept>

#include "Wt/Http/Request"
#include "WebRequest.h"

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

UploadedFile::Impl::~Impl()
{
  if (!isStolen)
    unlink(spoolFileName.c_str());
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

int Request::tooLarge() const
{
  if (request_)
    return request_->postDataExceeded();
  else
    return 0;
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
