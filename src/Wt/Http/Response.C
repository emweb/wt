/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Http/Response"
#include "Wt/Http/ResponseContinuation"
#include "Wt/WResource"
#include "WebRequest.h"
#include "WebUtils.h"

namespace Wt {
  namespace Http {

void Response::setStatus(int status)
{
  if (response_)
    response_->setStatus(status);
}

void Response::setContentLength(::uint64_t length)
{
  if (response_)
    response_->setContentLength(length);
}

void Response::setMimeType(const std::string& mimeType)
{
  if (response_)
    response_->setContentType(mimeType);
}

void Response::addHeader(const std::string& name, const std::string& value)
{
  if (response_)
    response_->addHeader(name, value);
}

ResponseContinuation *Response::createContinuation()
{
  if (!continuation_)
    continuation_ = new ResponseContinuation(resource_, response_);
  else
    continuation_->resource_ = resource_;

  return continuation_;
}

ResponseContinuation *Response::continuation() const
{
  if (continuation_ && continuation_->resource_)
    return continuation_;
  else
    return 0;
}

WT_BOSTREAM& Response::out()
{
  if (!headersCommitted_) {
    if (response_ &&
	!continuation_ &&
	(resource_->dispositionType() != WResource::NoDisposition
	 || !resource_->suggestedFileName().empty())) {
      std::string theDisposition;

      switch (resource_->dispositionType()) {
      default:
      case WResource::Inline:
        theDisposition = "inline";
        break;
      case WResource::Attachment:
        theDisposition = "attachment";
        break;
      }

      WString fileName = resource_->suggestedFileName();

      if (!fileName.empty()) {
	if (resource_->dispositionType() == WResource::NoDisposition) {
	  // backward compatibility-ish with older Wt versions
	  theDisposition = "attachment";
	}

	// Browser incompatibility hell: internatianalized filename suggestions
	// First filename is for browsers that don't support RFC 5987
	// Second filename is for browsers that do support RFC 5987
	std::string fileField;
	// We cannot query wApp here, because wApp doesn't exist for
	// static resources.
	bool isIE = response_->userAgent().find("MSIE") != std::string::npos;
	bool isChrome = response_->userAgent().find("Chrome")
	  != std::string::npos;

	if (isIE || isChrome) {
	  // filename="foo-%c3%a4-%e2%82%ac.html"
	  // Note: IE never converts %20 back to space, so avoid escaping
	  // IE wil also not url decode the filename if the file has no ASCII
	  // extension (e.g. .txt)
	  fileField = "filename=\"" +
	    Utils::urlEncode(fileName.toUTF8(), " ") + "\";";
	} else {
	  // Binary UTF-8 sequence: for FF3, Safari, Chrome, Chrome9
	  fileField = "filename=\"" + fileName.toUTF8() + "\";";
	}
	// Next will be picked by RFC 5987 in favour of the
	// one without specified encoding (Chrome9, 
	fileField += Utils::EncodeHttpHeaderField("filename", fileName);
	addHeader("Content-Disposition", theDisposition + ";" + fileField);
      } else {
	addHeader("Content-Disposition", theDisposition);
      }
    }

    headersCommitted_ = true;
  }

  if (out_)
    return *out_;
  else
    return response_->out();
}

Response::Response(WResource *resource, WebResponse *response,
		   ResponseContinuation *continuation)
  : resource_(resource),
    response_(response),
    continuation_(continuation),
    out_(0),
    headersCommitted_(false)
{ }

Response::Response(WResource *resource, WT_BOSTREAM& out)
  : resource_(resource),
    response_(0),
    continuation_(0),
    out_(&out),
    headersCommitted_(false)
{ }

  }
}
