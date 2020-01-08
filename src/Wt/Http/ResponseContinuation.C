/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Http/ResponseContinuation.h"
#include "Wt/WLogger.h"
#include "Wt/WResource.h"

#include "WebRequest.h"

#ifdef WT_THREADED
#include <thread>
#endif

namespace Wt {

LOGGER("Http::ResponseContinuation");

  namespace Http {

void ResponseContinuation::setData(const cpp17::any& data)
{
  data_ = data;
}

void ResponseContinuation::haveMoreData()
{
  WResource::UseLock useLock;
  WResource *resource = nullptr;

  {
#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> lock(*mutex_);
#endif // WT_THREADED

    if (!useLock.use(resource_))
      return;

    if (waiting_) {
      waiting_ = false;
      if (readyToContinue_) {
        readyToContinue_ = false;
	resource = resource_;
	resource_ = nullptr;
      }
    }
  }

  if (resource)
    resource->doContinue(shared_from_this());
}

void ResponseContinuation::readyToContinue(WebWriteEvent event)
{
  if (event == WebWriteEvent::Error) {
    LOG_ERROR("WebWriteEvent::Error");
    cancel(false);
    return;
  }

  WResource::UseLock useLock;
  WResource *resource = nullptr;

  {
#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> lock(*mutex_);
#endif // WT_THREADED

    if (!useLock.use(resource_))
      return;

    readyToContinue_ = true;

    if (!waiting_) {
      readyToContinue_ = false;
      resource = resource_;
      resource_ = nullptr;
    } else {
      response_->detectDisconnect
	(std::bind(&Http::ResponseContinuation::handleDisconnect,
		   shared_from_this()));
    }
  }

  if (resource)
    resource->doContinue(shared_from_this());
}

ResponseContinuation::ResponseContinuation(WResource *resource,
					   WebResponse *response)
  : 
#ifdef WT_THREADED
    mutex_(resource->mutex_),
#endif
    resource_(resource),
    response_(response),
    waiting_(false),
    readyToContinue_(false)
{ }

void ResponseContinuation::cancel(bool resourceIsBeingDeleted)
{
  WResource::UseLock useLock;
  WResource *resource = nullptr;

  {
#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> lock(*mutex_);
#endif // WT_THREADED

    if (resourceIsBeingDeleted) {
      if (!resource_)
	return;
    } else if (!useLock.use(resource_))
      return;

    resource = resource_;
    resource_ = nullptr;
  }

  if (resource) {
    Http::Request request(*response_, this);
    resource->handleAbort(request);
    resource->removeContinuation(shared_from_this());
    response_->flush(WebResponse::ResponseState::ResponseDone);
  }
}

void ResponseContinuation::handleDisconnect()
{
  WResource::UseLock useLock;
  WResource *resource = nullptr;

  {
#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> lock(*mutex_);
#endif // WT_THREADED

    if (!resource_)
      return;

    resource = resource_;
    resource_ = nullptr;
  }

  if (resource) {
    Http::Request request(*response_, this);
    resource->handleAbort(request);
    resource->removeContinuation(shared_from_this());
    response_->flush(WebResponse::ResponseState::ResponseDone);
  }
}

void ResponseContinuation::waitForMoreData()
{
  waiting_ = true;
}

ResponseContinuation::~ResponseContinuation()
{ }

  }
}
