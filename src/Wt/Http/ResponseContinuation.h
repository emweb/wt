// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HTTP_RESPONSE_CONTINUATION_H_
#define HTTP_RESPONSE_CONTINUATION_H_

#include <Wt/WGlobal.h>
#include <Wt/WAny.h>

#include <mutex>

namespace Wt {

  class WResource;
  class WebResponse;
  class WebSession;

  namespace Http {

    class Response;

/*! \class ResponseContinuation Wt/Http/ResponseContinuation.h Wt/Http/ResponseContinuation.h
 *  \brief A resource response continuation object.
 *
 * A response continuation object is used to keep track of a response
 * which is to be continued.
 *
 * You may associate data with the object using setData().
 *
 * A continuation is used to resume sending more data later for this
 * response. There are two possible reasons for this:
 * - the entire response is quite big and you may want to read and send
 *   it in smaller chunks. This avoids unbounded memory usage since the
 *   I/O layer buffers the response first in memory to send it then out
 *   to a possibly slow client using async I/O.
 * - you may not have any more data available, currently, but expect more
 *   data later. In that case you can call waitForMoreData() and later call
 *   WResource::haveMoreData() when more data is available.
 *
 * \sa Response::createContinuation(), Request::continuation()
 *
 * \ingroup http
 */
class WT_API ResponseContinuation 
#ifndef WT_TARGET_JAVA
  : public std::enable_shared_from_this<ResponseContinuation>
#endif
{
public:
  ~ResponseContinuation();

  /*! \brief Set data associated with the continuation.
   *
   * You could do this to keep track of the state of sending the data
   * for a WResource.
   */
  void setData(const cpp17::any& data);

  /*! \brief Return data associated with the continuation.
   *
   * \sa setData()
   */
  cpp17::any data() { return data_; }

  /*! \brief Return the resource.
   */
  WResource *resource() const { return resource_; }

  /*! \brief Wait for more data.
   *
   * This suspends the handling of this request until more data is
   * available, indicated with a call to haveMoreData(), or to a
   * resource globally using WResource::haveMoreData().
   */
  void waitForMoreData();

  /*! \brief Indicates that we have more data.
   *
   * This will allow the response to be resumed with a new call to
   * WResource::handleRequest().
   */
  void haveMoreData();

  /*! \brief Returns whether this continuation is waiting for data.
   *
   * \sa waitForMoreData()
   */
  bool isWaitingForMoreData() const { return waiting_; }

private:
#ifdef WT_THREADED
  std::shared_ptr<std::recursive_mutex> mutex_;
#endif

  WResource *resource_;
  WebResponse *response_;
  cpp17::any data_;
  bool waiting_, readyToContinue_;

  ResponseContinuation(WResource *resource, WebResponse *response);
  ResponseContinuation(const ResponseContinuation&);

  void cancel(bool resourceIsBeingDeleted);
  void readyToContinue(WebWriteEvent writeResult);
  void handleDisconnect();

  WebResponse *response() { return response_; }

  friend class Wt::WResource;
  friend class Wt::WebSession;
  friend class Response;
};

typedef std::shared_ptr<ResponseContinuation> ResponseContinuationPtr;

  }
}

#endif // HTTP_RESPONSE_CONTINUATION_H_
