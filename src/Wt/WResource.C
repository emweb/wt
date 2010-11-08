/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <string>
#include <boost/lexical_cast.hpp>

#include "Wt/WResource"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "WebController.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WtRandom.h"
#include "WtException.h"
#include "Utils.h"

#ifdef WT_THREADED
#include <boost/thread/recursive_mutex.hpp>
#endif // WT_THREADED

namespace Wt {

WResource::WResource(WObject* parent)
  : WObject(parent),
    dataChanged_(this),
    beingDeleted_(false),
    trackUploadProgress_(false)
{ 
#ifdef WT_THREADED
  mutex_.reset(new boost::recursive_mutex());
#endif // WT_THREADED
}

void WResource::beingDeleted()
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(*mutex_);
  beingDeleted_ = true;
#endif // WT_THREADED
}

WResource::~WResource()
{
  beingDeleted();

  for (unsigned i = 0; i < continuations_.size(); ++i) {
    continuations_[i]->stop();
    delete continuations_[i];
  }

  if (wApp) {
    wApp->removeExposedResource(this);
    if (trackUploadProgress_)
      WebSession::instance()->controller()->removeUploadProgressUrl(url());
  }
}

void WResource::setUploadProgress(bool enabled)
{
  if (trackUploadProgress_ != enabled) {
    trackUploadProgress_ = enabled;

    WebController *c = WebSession::instance()->controller();
    if (enabled)
      c->addUploadProgressUrl(url());
    else
      c->removeUploadProgressUrl(url());
  }
}

void WResource::doContinue(Http::ResponseContinuation *continuation)
{
  WebResponse *webResponse = continuation->response();
  WebRequest *webRequest = webResponse;

  try {
    handle(webRequest, webResponse, continuation);
  } catch (std::exception& e) {
    std::cerr << "Exception while handling resource continuation: "
	      << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Exception while handling resource continuation." << std::endl;
  }
}

void WResource::handle(WebRequest *webRequest, WebResponse *webResponse,
		       Http::ResponseContinuation *continuation)
{
#ifdef WT_THREADED
  boost::shared_ptr<boost::recursive_mutex> mutex = mutex_;
  boost::recursive_mutex::scoped_lock lock(*mutex);

  if (beingDeleted_)
    return;

  // when we are handling a continuation, we do not have the session
  // lock
  if (!continuation) {
    WebSession::Handler *h = WebSession::Handler::instance();
    if (h && h->lock().owns_lock())
      h->lock().unlock();
  }
#endif // WT_THREADED

  if (continuation)
    continuation->resource_ = 0;

  Http::Request  request(*webRequest, continuation);
  Http::Response response(this, webResponse, continuation);

  if (!continuation && !suggestedFileName_.empty())
    response.addHeader("Content-Disposition",
		       "attachment;filename=" + suggestedFileName_);

  handleRequest(request, response);

  if (!response.continuation_ || !response.continuation_->resource_) {
    if (response.continuation_) {
      Utils::erase(continuations_, response.continuation_);
      delete response.continuation_;
    }
    webResponse->flush(WebResponse::ResponseDone);
  } else
    webResponse->flush(WebResponse::ResponseFlush,
		       boost::bind(&Http::ResponseContinuation::doContinue,
				   response.continuation_));
}

void WResource::suggestFileName(const std::string& name)
{
  suggestedFileName_ = name;

  generateUrl();
}

void WResource::setInternalPath(const std::string& path)
{
  internalPath_ = path;

  generateUrl();
}

void WResource::setChanged()
{
  generateUrl();

  dataChanged_.emit();
}

const std::string& WResource::url() const
{
  if (currentUrl_.empty())
    (const_cast<WResource *>(this))->generateUrl();

  return currentUrl_;
}

const std::string& WResource::generateUrl()
{
  WApplication *app = WApplication::instance();

  if (app) {
    WebController *c = 0;
    if (trackUploadProgress_)
      c = WebSession::instance()->controller();

    if (c && !currentUrl_.empty())
      c->removeUploadProgressUrl(currentUrl_);
    currentUrl_ = app->addExposedResource(this, internalPath_);
    if (c)
      c->addUploadProgressUrl(currentUrl_);    
  } else
    currentUrl_ = internalPath_;

  return currentUrl_;
}

void WResource::write(WT_BOSTREAM& out,
		      const Http::ParameterMap& parameters,
		      const Http::UploadedFileMap& files)
{
  Http::Request  request(parameters, files);
  Http::Response response(this, out);

  handleRequest(request, response);

  // While the resource indicates more data to be sent, get it too.
  while (response.continuation_	&& response.continuation_->resource_) {
    response.continuation_->resource_ = 0;
    request.continuation_ = response.continuation_;

    handleRequest(request, response);
  }

  delete response.continuation_;
}

}
