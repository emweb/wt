// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRESOURCE_H_
#define WRESOURCE_H_

#include <Wt/WObject.h>
#include <Wt/WGlobal.h>
#include <Wt/WSignal.h>
#include <Wt/WString.h>

#include <iostream>
#include <condition_variable>
#ifdef WT_THREADED
#include <mutex>
#endif // WT_THREADED

namespace Wt {

  class WebRequest;
  class WebResponse;
  class WebSession;

  namespace Http {
    class Request;
    class Response;
    class ResponseContinuation;

    typedef std::shared_ptr<ResponseContinuation> ResponseContinuationPtr;
  }

/*! \brief Values for the disposition type in the Content-Disposition header
 */
enum class ContentDisposition {
  None,       //!< Do not specify a disposition type
  Attachment, //!< Open with a helper application or show 'Save As' dialog
  Inline      //!< View with a browser plugin
};

/*! \class WResource Wt/WResource.h Wt/WResource.h
 *  \brief An object which can be rendered in the HTTP protocol.
 *
 * <h3>Usage</h3>
 *
 * Besides the main page, other objects may be rendered as additional
 * resources, for example documents or images. Classes such as WAnchor
 * or WImage can use a resource instead of a URL to provide their
 * contents. Whenever the resource has changed, you should call the
 * setChanged() method. setChanged() will make sure that the browser will
 * use a new version of the resource by generating a new URL, and emits the
 * dataChanged() signal to make those that refer to the resource aware
 * that they should update their references to the new URL.
 *
 * You can help the browser to start a suitable helper application to
 * handle the downloaded resource, or suggest to the user a suitable
 * filename for saving the resource, by setting an appropriate file
 * name using suggestFileName().
 *
 * To serve resources that you create on the fly, you need to specialize
 * this class and implement handleRequest().
 * 
 * \if cpp
 * Example for a custom resource implementation:
 * \code
 * class MyResource : public Wt::WResource
 * {
 * public:
 *   MyResource()
 *   {
 *     suggestFileName("data.txt");
 *   }
 *
 *   ~MyResource() {
 *     beingDeleted(); // see "Concurrency issues" below.
 *   }
 *
 *   void handleRequest(const Wt::Http::Request& request,
 *                      Wt::Http::Response& response) {
 *     response.setMimeType("plain/text");
 *     response.out() << "I am a text file." << std::endl;
 *   }
 * };
 * \endcode
 * \endif
 *
 * <h3>Concurrency issues</h3>
 *
 * Because of the nature of the web, a resource may be requested one
 * time or multiple times at the discretion of the browser, and
 * therefore your resource should in general not have any side-effects
 * except for what is needed to render its own contents. Unlike event
 * notifications to a %Wt application, resource requests are not
 * serialized, but are handled concurrently. You need to grab the
 * application lock if you want to access or modify other widget state
 * from within the resource. You can enable takesUpdateLock() to let
 * %WResource do that for you. When deleting a resource, any pending
 * request is cancelled first. For this mechanism to work you need to
 * specialize the destructor and call beingDeleted(). This method may
 * safely be called multiple times (i.e. from within each destructor
 * in the hierachy).
 *
 * <h3>Continuations for asynchronous I/O</h3>
 *
 * With respect to I/O, the current strategy is to cache the whole
 * response first in a buffer and use async I/O to push the data to
 * the client, in order to free the thread while waiting for I/O on a
 * possibly slow link. You do not necessarily have to provide all
 * output at once, instead you can obtain a Http::ResponseContinuation
 * object for a response, construct the response piecewise. A new
 * request() will be made to continue the response.
 *
 * Example for a custom resource implementation using continuations:
 * \code
class MyResource : public Wt::WResource
{
public:
  MyResource(int iterations)
    : iterations_(iterations)
  {
    suggestFileName("data.txt");
  }

  ~MyResource() {
    beingDeleted();
  }

  void handleRequest(const Wt::Http::Request& request,
                     Wt::Http::Response& response) {
    // see if this request is for a continuation:
    Wt::Http::ResponseContinuation *continuation = request.continuation();

    // calculate the current start
    int iteration = continuation ? Wt::cpp17::any_cast<int>(continuation->data()) : 0;
    if (iteration == 0)
      response.setMimeType("plain/text");

    int last = std::min(iterations_, iteration + 100);
    for (int i = iteration; i < last; ++i)
      response.out() << "Data item " << i << std::endl;

    // if we have not yet finished, create a continuation to serve more
    if (last < iterations_) {
      continuation = response.createContinuation();
      // remember what to do next
      continuation->setData(last);
    }
  }

private:
  int iterations_;
};
 * \endcode
 *
 * <h3>Global and private resources</h3>
 *
 * By default, a resource is private to an application: access to it
 * is protected by same secret session Id that protects any other
 * access to the application.
 *
 * You can also deploy static resources, which are global, using
 * WServer::addResource().
 *
 * <h3>Monitoring upload progress</h3>
 *
 * A resource may also handle the uploading of files (in fact,
 * WFileUpload uses a WResource to do exactly that) or transmission of
 * other large bodies of data being POST'ed or PUT to the resource
 * URL. For these requests, it may be convenient to enable upload
 * progress monitoring using setUploadProgress(), which allows you to
 * be notified of data being received.
 *
 * \sa WAnchor, WImage
 */
class WT_API WResource : public WObject
{
public:
  /*! \brief Creates a new resource.
   */
  WResource();

  /*! \brief Destroys the resource.
   *
   * When specializing a resource, you MUST call beingDeleted() from
   * within the specialized destructor, in order to stop any further
   * requests to the resource, unless it is guaranteed that no requests
   * will arrive for this resource when it is being deleted, e.g. if
   * the server is already stopped.
   */
  ~WResource();

  /*! \brief Suggests a filename to the user for the data streamed by this
   *         resource.
   *
   * For resources, intended to be downloaded by the user, suggest a
   * name used for saving. The filename extension may also help the
   * browser to identify the correct program for opening the resource.
   *
   * The disposition type determines if the resource is intended to
   * be opened by a plugin in the browser (Inline), or to be saved to disk
   * (Attachment). NoDisposition is not a valid Content-Disposition when a
   * filename is suggested; this will be rendered as Attachment.
   *
   * \sa setDispositionType().
   */
  void suggestFileName(const Wt::WString &name,
                       ContentDisposition disposition
		         = ContentDisposition::Attachment);

  /*! \brief Returns the suggested file name.
   *
   * \sa suggestFileName();
   */
  const Wt::WString& suggestedFileName() const { return suggestedFileName_; }

  /*! \brief Configures the Content-Disposition header
   *
   * The Content-Disposition header allows to instruct the browser that a
   * resource should be shown inline or as attachment. This function enables
   * you to set this property.
   *
   * This is often used in combination with suggestFilename(). The
   * Content-Disposition must not be None when a filename is given;
   * if this case is encountered, None will be rendered as Attachment.
   *
   * \sa suggestFilename().
   */
  void setDispositionType(ContentDisposition cd);

  /*! \brief Returns the currently configured content disposition
   *
   * \sa setDispositionType()
   */
  ContentDisposition dispositionType() const { return dispositionType_; }

  /*! \brief Generates a new URL for this resource and emits the changed signal
   *
   * This does not work when the resource is deployed at an internal path using
   * setInternalPath().
   */
  void setChanged();

  /*! \brief Sets an internal path for this resource.
   *
   * Using this method you can deploy the resource at a fixed path. Unless
   * you deploy using cookies for session tracking (not recommended), a
   * session identifier will be appended to the path.
   *
   * You should use internal paths that are different from internal paths
   * handled by your application (WApplication::setInternalPath()), if you
   * do not want a conflict between these two when the browser does not use
   * AJAX (and thus url fragments for its internal paths).
   *
   * The default value is empty. By default the URL for a resource is
   * unspecified and a URL will be generated by the library. 
   *
   * The internal path for a static resource is the deployment path
   * specified using WServer::addResource().
   */
  void setInternalPath(const std::string& path);

  /*! \brief Returns the internal path.
   *
   * \sa setInternalPath().
   */
  std::string internalPath() const { return internalPath_; }

  /*! \brief Generates an URL for this resource.
   *
   * Generates a new url that refers to this resource. The url is
   * unique to assure that it is not cached by the web browser, and
   * can thus be used to refer to a new "version" of the resource,
   * which can be indicated by emitting the dataChanged() signal.
   *
   * The old urls are not invalidated by calling this method.
   */
  const std::string& generateUrl();

  /*! \brief Returns the current URL for this resource.
   *
   * Returns the url that references this resource.
   */
  const std::string& url() const;

  /*! \brief %Signal emitted when the data presented in this resource
   *         has changed.
   *
   * Widgets that reference the resource (such as anchors and images) will
   * make sure the new data is rendered.
   *
   * It is better to call setChanged() than to emit this signal. setChanged
   * generates a new URL for this resource to avoid caching problems and then
   * emits this signal.
   */
  Signal<>& dataChanged() { return dataChanged_; }

  /*! \brief Indicate interest in upload progress.
   *
   * When supported, you can track upload progress using this
   * signal. While data is being received, and before handleRequest()
   * is called, progress information is indicated using
   * dataReceived().
   *
   * We envision that in the future support will depend on a
   * combination of browser and connector. Currently only the wthttp
   * connector provides support for this across all AJAX browsers. In
   * the future, we are likely to implement this also using JavaScript
   * File API features making it independent of connectors.
   *
   * The default value is \c false.
   */
  void setUploadProgress(bool enabled);

  /*! \brief %Signal emitted when data has been received for this resource.
   *
   * When this signal is emitted, you have the update lock to modify
   * the application. Because there is however no corresponding
   * request from the browser, any update to the user interface is not
   * immediately reflected in the client. To update the client
   * interface, you need to use a WTimer or enable \link
   * WApplication::enableUpdates() server-push\endlink.
   *
   * The first argument is the number of bytes received so far,
   * and the second argument is the total number of bytes.
   *
   * \sa setUploadProgress()
   */
  Signal< ::uint64_t, ::uint64_t>& dataReceived() { return dataReceived_; }

  Signal< ::uint64_t >& dataExceeded() { return dataExceeded_; }

  /*! \brief Stream the resource to a stream.
   *
   * This is a convenience method to serialize to a stream (for
   * example a file stream).
   */
  void write(WT_BOSTREAM& out,
	     const Http::ParameterMap& parameters = Http::ParameterMap(),
	     const Http::UploadedFileMap& files = Http::UploadedFileMap());

  /*! \brief Handles a request.
   *
   * Reimplement this method so that a proper response is generated
   * for the given request. From the \p request object you can
   * access request parameters and whether the request is a
   * continuation request. In the \p response object, you should
   * set the mime type and stream the output data.
   *
   * A request may also concern a continuation, indicated in
   * Http::Request::continuation(), in which case the next part for a
   * previously created continuation should be served.
   *
   * While handling a request, which may happen at any time together
   * with event handling, the library makes sure that the resource is
   * not being concurrently deleted, but multiple requests may happend
   * simultaneously for a single resource.
   */
  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) = 0;

  /*! \brief Handles a continued request being aborted.
   *
   * This function is only applicable to a request for which a
   * response continuation was created, and indicates that the client
   * has been closed (or the resource is being deleted) before the
   * response was completed. This function may be reimplemented so
   * that you can close any resources associated with the response
   * continuation.
   *
   * The base implementation is empty.
   *
   * Note that because this function could be called from within the
   * destructor, you should not forget to call beingDeleted() from the
   * specialized destructor of your resource.
   */
  virtual void handleAbort(const Http::Request& request);

  /*! \brief Indicate that more data is available.
   *
   * In some occasions, data may be requested for a resource which is
   * currently not yet available. Then you can suspend the response
   * using a continuation which you let wait for more data.
   *
   * Using this method you can indicate that more data is
   * available. This will resume all responses that are currently
   * waiting for more data.
   *
   * If no responses are currently waiting for data, then this method
   * has no effect.
   *
   * \sa ResponseContinuation::waitForMoreData()
   */
  void haveMoreData();

  /*! \brief Set whether this resource takes the %WApplication's update lock
   *
   * By default, %WResource does not keep the WApplication's update lock,
   * so handleRequest() is done outside of the %WApplication's event loop.
   * This makes sure that resources don't block the event loop, and multiple
   * requests to a %WResource can be handled concurrently.
   *
   * However, if necessary you can either manually grab the update lock,
   * (see WApplication::UpdateLock) or enable this option.
   *
   * \note This is not applicable to static resources, since static resources
   * do not have a %WApplication associated with them.
   */
  void setTakesUpdateLock(bool enabled);

  /*! \brief Returns whether this resources takes the %WApplication's update lock
   *
   * \sa setTakesUpdateLock()
   */
  bool takesUpdateLock() const { return takesUpdateLock_; }

protected:
  /*! \brief Prepares the resource for deletion.
   *
   * When specializing a resource, you MUST call beingDeleted() from
   * within the specialized destructor, in order to stop any further
   * requests to the resource.
   */
  void beingDeleted();

private:
  struct UseLock {
    UseLock();
    ~UseLock();

    bool use(WResource *resource);

  private:
    WResource *resource_;
  };

#ifdef WT_THREADED
  std::shared_ptr<std::recursive_mutex> mutex_;
  bool beingDeleted_;
  int useCount_;
  std::condition_variable_any useDone_;
#endif

  Signal<> dataChanged_;
  Signal< ::uint64_t, ::uint64_t > dataReceived_;
  Signal< ::uint64_t > dataExceeded_;

  bool trackUploadProgress_;
  bool takesUpdateLock_;

  std::vector<Http::ResponseContinuationPtr> continuations_;

  void removeContinuation(Http::ResponseContinuationPtr continuation);
  Http::ResponseContinuationPtr addContinuation(Http::ResponseContinuation *c);
  void doContinue(Http::ResponseContinuationPtr continuation);
  void handle(WebRequest *webRequest, WebResponse *webResponse,
	      Http::ResponseContinuationPtr continuation
	        = Http::ResponseContinuationPtr());

  Wt::WString suggestedFileName_;
  ContentDisposition dispositionType_;
  std::string currentUrl_;
  std::string internalPath_;

  WApplication *app_; // associated app (for non-static resources)

  friend class Http::ResponseContinuation;
  friend class Http::Response;
  friend class WebSession;
  friend class WebController;
  friend class Configuration;
};

}

#endif // WRESOURCE_H_
