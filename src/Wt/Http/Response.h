// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include <string>
#include <Wt/WGlobal.h>
#include <Wt/Http/ResponseContinuation.h>
#include <ostream>

namespace Wt {

  class WResource;
  class WebSession;

  namespace Http {

/*! \class Response Wt/Http/Response.h Wt/Http/Response.h
 *  \brief A resource response.
 *
 * This class defines the HTTP response for a WResource request.
 *
 * More specifically you can:
 * - set the content mime type using setMimeType()
 * - add HTTP headers using addHeader()
 * - stream content into out()
 *
 * You may chose to provide only a partial response. In that case, use
 * createContinuation() to create a continuation object to which you
 * can annotate information for the next request to process the
 * response further.
 *
 * \sa WResource::handleRequest(), Request
 *
 * \ingroup http
 */
class WT_API Response
{
public:
  /*! \brief Sets the response status.
   *
   * Unless a overriden, 200 OK will be assumed.
   */
  void setStatus(int status);

  /*! \brief Sets the content length
   *
   * If content length is known, use this method to set it. File downloads
   * will see progress bars. If not set, Wt will use chunked transfers.
   *
   * Always use this method instead of setting the Content-Length header
   * with addHeader().
   *
   * Headers may be added only before setting the content mime-type
   * (setMimeType()), and before streaming any data to the out()
   * stream.
   */
   void setContentLength(::uint64_t length);

  /*! \brief Set the content mime type.
   *
   * The content mimetype is used by the browser to correctly interpret
   * the resource.
   */
  void setMimeType(const std::string& mimeType);

  /*! \brief Add an HTTP header.
   *
   * Headers may be added only before setting the content mime-type
   * (setMimeType()), and before streaming any data to the out()
   * stream.
   */
  void addHeader(const std::string& name, const std::string& value);

  /*! \brief Create a continuation object for this response.
   *
   * A continuation is used to resume sending more data later for this
   * response. There are two possible reasons for this:
   * - the entire response is quite big and you may want to read and send
   *   it in smaller chunks to avoid memory consumption problems since the
   *   I/O layer buffers the response first in memory to send it then out
   *   to a possibly slow client using async I/O.
   * - you may not have any more data available, currently, but expect more
   *   data later. In that case you can call
   *   ResponseContinuation::waitForMoreData() and later call
   *   WResource::haveMoreData() when more data is available.
   *
   * A new call to handleRequest() will be made to retrieve more
   * data.
   *
   * \sa continuation()
   */
  ResponseContinuation *createContinuation();

  /*! \brief Return the continuation, if one was created for this response.
   *
   * Returns the continuation that was previously created using
   * createContinuation(), or \c nullptr if no continuation was created yet.
   *
   * \sa createContinuation()
   */
  ResponseContinuation *continuation() const;

  /*! \brief Returns the stream for getting the response output.
   */
  std::ostream& out();

  WT_BOSTREAM& bout() { return out(); }

private:
  WResource *resource_;
  WebResponse *response_;
  ResponseContinuationPtr continuation_;
  WT_BOSTREAM *out_;
  bool headersCommitted_;

  Response(WResource *resource, WebResponse *response,
	   ResponseContinuationPtr continuation);
  Response(WResource *resource, WT_BOSTREAM& out);

  friend class Wt::WResource;
  friend class Wt::WebSession;
};

  }
}

#endif // HTTP_RESPONSE_H_
