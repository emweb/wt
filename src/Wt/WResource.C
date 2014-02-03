/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <string>

#include "Wt/WResource"
#include "Wt/WApplication"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "WebController.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WebUtils.h"

#ifdef WT_THREADED
#include <boost/thread/recursive_mutex.hpp>
#endif // WT_THREADED

namespace Wt {

LOGGER("WResource");

WResource::WResource(WObject* parent)
  : WObject(parent),
    dataChanged_(this),
    beingDeleted_(false),
    trackUploadProgress_(false),
    dispositionType_(NoDisposition)
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

  WApplication *app = WApplication::instance();
  if (app) {
    app->removeExposedResource(this);
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

void WResource::haveMoreData()
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(*mutex_);
#endif // WT_THREADED

  std::vector<Http::ResponseContinuation *> cs = continuations_;

  for (unsigned i = 0; i < cs.size(); ++i)
    if (cs[i]->isWaitingForMoreData())
      cs[i]->doContinue(WriteCompleted);
}

void WResource::doContinue(Http::ResponseContinuation *continuation)
{
  WebResponse *webResponse = continuation->response();
  WebRequest *webRequest = webResponse;

  try {
    handle(webRequest, webResponse, continuation);
  } catch (std::exception& e) {
    LOG_ERROR("exception while handling resource continuation: " << e.what());
  } catch (...) {
    LOG_ERROR("exception while handling resource continuation");
  }
}

void WResource::handle(WebRequest *webRequest, WebResponse *webResponse,
		       Http::ResponseContinuation *continuation)
{
  bool retakeLock = false;
  WebSession::Handler *handler = WebSession::Handler::instance();

  bool dynamic = handler || continuation;

  {
#ifdef WT_THREADED
    boost::shared_ptr<boost::recursive_mutex> mutex = mutex_;
    boost::recursive_mutex::scoped_lock lock(*mutex, boost::defer_lock);

    if (dynamic) {
      lock.lock();
      if (beingDeleted_)
	return;

      // when we are handling a continuation, we do not have the session
      // lock, unless we are being called from haveMoreData(), but then it's
      // fine I guess ?
      if (!continuation) {
	if (handler->haveLock() && 
	    handler->lockOwner() == boost::this_thread::get_id()) {
	  retakeLock = true;
	  handler->lock().unlock();
	}
      }
    }
#endif // WT_THREADED

    if (continuation)
      continuation->resource_ = 0;

    Http::Request request(*webRequest, continuation);
    Http::Response response(this, webResponse, continuation);

    if (!continuation)
      response.setStatus(200);

    handleRequest(request, response);

    if (!response.continuation_ || !response.continuation_->resource_) {
      if (response.continuation_) {
	Utils::erase(continuations_, response.continuation_);
	delete response.continuation_;
      }

      response.out(); // trigger committing the headers if still necessary

      webResponse->flush(WebResponse::ResponseDone);
    } else {
      if (response.continuation_->isWaitingForMoreData()) {
	webResponse->flush
	  (WebResponse::ResponseFlush,
	   boost::bind(&Http::ResponseContinuation::flagReadyToContinue,
		       response.continuation_, _1));
      } else
	webResponse->flush
	  (WebResponse::ResponseFlush,
	   boost::bind(&Http::ResponseContinuation::doContinue,
		       response.continuation_, _1));
    }
  }

  if (retakeLock) {
#ifdef WT_THREADED
    if (!handler->haveLock())
      handler->lock().lock();
#endif // WT_THREADED
  }
}

void WResource::suggestFileName(const WString& name,
                                DispositionType dispositionType)
{
  suggestedFileName_ = name;
  dispositionType_ = dispositionType;

  currentUrl_.clear();
}

void WResource::setInternalPath(const std::string& path)
{
  internalPath_ = path;

  currentUrl_.clear();
}

void WResource::setDispositionType(DispositionType dispositionType)
{
  dispositionType_ = dispositionType;
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
    currentUrl_ = app->addExposedResource(this);
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
