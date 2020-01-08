/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Http/Response.h"
#include "Wt/Http/ResponseContinuation.h"
#include "Wt/WResource.h"
#include "Wt/WStringStream.h"
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
  if (!continuation_) {
    ResponseContinuation *c = new ResponseContinuation(resource_, response_);
    continuation_ = resource_->addContinuation(c);
  } else
    continuation_->resource_ = resource_;

  return continuation_.get();
}

ResponseContinuation *Response::continuation() const
{
  if (continuation_ && continuation_->resource_)
    return continuation_.get();
  else
    return nullptr;
}

WT_BOSTREAM& Response::out()
{
  if (!headersCommitted_) {
    if (response_ &&
	!continuation_ &&
	(resource_->dispositionType() != ContentDisposition::None
	 || !resource_->suggestedFileName().empty())) {
      WStringStream cdp;

      switch (resource_->dispositionType()) {
      default:
      case ContentDisposition::Inline:
        cdp << "inline";
        break;
      case ContentDisposition::Attachment:
        cdp << "attachment";
        break;
      }

      const WString& fileName = resource_->suggestedFileName();

      if (!fileName.empty()) {
	if (resource_->dispositionType() == ContentDisposition::None) {
	  // backward compatibility-ish with older Wt versions
	  cdp.clear();
	  cdp << "attachment";
	}

	// Browser incompatibility hell: internatianalized filename suggestions
	// First filename is for browsers that don't support RFC 5987
	// Second filename is for browsers that do support RFC 5987
	cdp << ';';

	// We cannot query wApp here, because wApp doesn't exist for
	// static resources.
	const char *ua = response_->userAgent();
	bool isIE = ua && strstr(ua, "MSIE") != nullptr;
	bool isChrome = ua && strstr(ua, "Chrome") != nullptr;

	if (isIE || isChrome) {
	  // filename="foo-%c3%a4-%e2%82%ac.html"
	  // Note: IE never converts %20 back to space, so avoid escaping
	  // IE wil also not url decode the filename if the file has no ASCII
	  // extension (e.g. .txt)
	  cdp << "filename=\""
	      << Utils::urlEncode(fileName.toUTF8(), " ")
	      << "\";";
	} else {
	  // Binary UTF-8 sequence: for FF3, Safari, Chrome, Chrome9
	  cdp << "filename=\"" << fileName.toUTF8() << "\";";
	}
	// Next will be picked by RFC 5987 in favour of the
	// one without specified encoding (Chrome9, 
	cdp << Utils::EncodeHttpHeaderField("filename", fileName);
	addHeader("Content-Disposition", cdp.str());
      } else {
	addHeader("Content-Disposition", cdp.str());
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
		   ResponseContinuationPtr continuation)
  : resource_(resource),
    response_(response),
    continuation_(continuation),
    out_(nullptr),
    headersCommitted_(false)
{ }

Response::Response(WResource *resource, WT_BOSTREAM& out)
  : resource_(resource),
    response_(nullptr),
    out_(&out),
    headersCommitted_(false)
{ }

  }
}
