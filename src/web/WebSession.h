// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEBSESSION_H_
#define WEBSESSION_H_

#include <string>
#include <vector>

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#endif // WT_THREADED || WT_TARGET_JAVA

#include "TimeUtil.h"
#include "WebRenderer.h"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLogger"

namespace Wt {

class WebController;
class WebRequest;
class WebResponse;
class WApplication;

/*
 * The WebSession stores the state for one session.
 *
 * It also handles the following tasks:
 *  - propagate events
 *  - determine what needs to be served
 *    (a web page, a resource or a javascript update).
 */
class WT_API WebSession
{
public:
  enum State {
    JustCreated,
    Loaded,
    Dead
  };

  WebSession(WebController *controller, const std::string& sessionId,
	     EntryPointType type, const std::string& favicon,
	     const WebRequest *request, WEnvironment *env = 0);
  ~WebSession();

  static WebSession *instance();

  EntryPointType type() const { return type_; }
  std::string favicon() const { return favicon_; }
  std::string docType() const;

  std::string sessionId() const { return sessionId_; }

  WebController *controller() const { return controller_; }
  WEnvironment&  env() { return *env_; }
  WApplication  *app() { return app_; }
  WebRenderer&   renderer() { return renderer_; }

  bool debug() const { return debug_; }

  void redirect(const std::string& url);
  std::string getRedirect();

  void setApplication(WApplication *app);

  WLogEntry log(const std::string& type);

  void notify(const WEvent& e);
  bool handleRequest(WebRequest& request, WebResponse& response);
  void pushUpdates();

  void doRecursiveEventLoop();

#ifndef WT_TARGET_JAVA
  bool unlockRecursiveEventLoop();
#endif // WT_TARGET_JAVA

  void pushEmitStack(WObject *obj);
  void popEmitStack();
  WObject *emitStackTop();

#ifndef WT_TARGET_JAVA
  const Time& expireTime() const { return expire_; }
  bool shouldDisconnect() const;
#endif // WT_TARGET_JAVA

  bool done() { return state_ == Dead; }
  State state() const { return state_; }

  bool progressiveBoot() const { return progressiveBoot_; }

  /*
   * URL stuff
   */

  const std::string& applicationName() const { return applicationName_; }

  // (http://www.bigapp.com) /myapp/app.wt?wtd=ABCD
  const std::string applicationUrl() const { return applicationUrl_ + sessionQuery(); }

  const std::string& deploymentPath() const { return deploymentPath_; }

  //    (http://www.bigapp.com/myapp/app.wt) ?wtd=ABCD
  // or (http://www.bigapp.com/myapp/) app.wt/path?wtd=ABCD
  std::string mostRelativeUrl(const std::string& internalPath = std::string())
    const;

  std::string appendInternalPath(const std::string& url,
				 const std::string& internalPath) const;

  std::string appendSessionQuery(const std::string& url) const;

  std::string ajaxCanonicalUrl(const WebResponse& request) const;

  enum BootstrapOption {
    ClearInternalPath,
    KeepInternalPath
  };

  std::string bootstrapUrl(const WebResponse& response, BootstrapOption option)
    const;

  // (http://www.bigapp.com/myapp/) app.wt/internal_path
  std::string bookmarkUrl(const std::string& internalPath) const;

  // tries to figure out the current bookmark url (from the app or otherwise)
  std::string bookmarkUrl() const;

  // http://www.bigapp.com:1234/myapp/
  const std::string& absoluteBaseUrl() const { return absoluteBaseUrl_; }

  const std::string& baseUrl() const { return baseUrl_; }

  std::string getCgiValue(const std::string& varName) const;
  std::string getCgiHeader(const std::string& headerName) const;

  class Handler {
  public:
    Handler(WebSession& session, WebRequest& request, WebResponse& response);
    Handler(WebSession& session, bool takeLock);
    ~Handler();

    static Handler *instance();

    WebResponse *response() { return response_; }
    WebRequest  *request() { return request_; }
    WebSession  *session() const { return &session_; }
    void killSession();

  private:
    void init();

    static void attachThreadToSession(WebSession& session);

    bool sessionDead(); // killed or quited()

    void setRequest(WebRequest *request, WebResponse *response);

#ifdef WT_THREADED
    boost::mutex::scoped_lock& lock() { return lock_; }
    boost::mutex::scoped_lock lock_;

    Handler(const Handler&);

    Handler *prevHandler_;
#endif // WT_THREADED

    WebSession&  session_;
    WebRequest  *request_;
    WebResponse *response_;
    bool         killed_;

    friend class WApplication;
    friend class WResource;
    friend class WebSession;
    friend class WFileUploadResource;
  };

#ifdef WT_THREADED
  boost::mutex& mutex() { return mutex_; }
#endif // WT_THREADED

  void setLoaded();

private:
  /*
   * Misc methods
   */

  void checkTimers();
  void hibernate();

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
  boost::mutex mutex_;
  static boost::thread_specific_ptr<Handler> threadHandler_;
#else
  static Handler *threadHandler_;
#endif // WT_TARGET_JAVA

  EntryPointType type_;
  std::string    favicon_;
  State          state_;

  std::string   sessionId_;

  WebController *controller_;
  WebRenderer   renderer_;
  std::string   applicationName_;
  std::string   bookmarkUrl_, baseUrl_, absoluteBaseUrl_;
  std::string   applicationUrl_, deploymentPath_;
  std::string   redirect_;
  WebResponse  *pollResponse_;
  bool          updatesPending_;
  bool          progressiveBoot_;

#ifndef WT_TARGET_JAVA
  Time             expire_;
#ifdef WT_THREADED
  boost::condition recursiveEvent_;
#endif // WT_THREADED
#endif // WT_TARGET_JAVA

  WEnvironment  embeddedEnv_;
  WEnvironment *env_;
  WApplication *app_;
  bool          debug_;

  std::vector<Handler *> handlers_;
  std::vector<WObject *> emitStack_;

  Handler *recursiveEventLoop_;

  WResource *decodeResource(const std::string& resourceId);
  EventSignalBase *decodeSignal(const std::string& signalId);
  EventSignalBase *decodeSignal(const std::string& objectId,
				const std::string& signalName);

  static WObject::FormData getFormData(const WebRequest& request,
				       const std::string& name);

  void render(Handler& handler, WebRenderer::ResponseType responseType);
  void serveError(Handler& handler, const std::string& exception,
		  WebRenderer::ResponseType responseType);
  void serveResponse(Handler& handler, WebRenderer::ResponseType responseType);

  enum SignalKind { LearnedStateless = 0, AutoLearnStateless = 1,
		    Dynamic = 2 };
  void processSignal(EventSignalBase *s, const std::string& se,
		     const WebRequest& request, SignalKind kind);

  std::vector<unsigned int> getSignalProcessingOrder(const WEvent& e);
  void notifySignal(const WEvent& e);
  void propagateFormValues(const WEvent& e, const std::string& se);

  const std::string *getSignal(const WebRequest& request,
			       const std::string& se);

  void setState(State state, int timeout);

  void init(const WebRequest& request);
  bool start();
  void kill();

  void generateNewSessionId();
  std::string sessionQuery() const;
};

/*! \class WEvent
 *  \brief An internal session event.
 *
 * The request controller notifies the application to react to a request
 * using WApplication::notify().
 */
class WT_API WEvent {
private:
  WEvent(WebSession::Handler& aHandler,
	 WebRenderer::ResponseType aResponseType,
	 bool doRenderOnly)
    : handler(aHandler),
      responseType(aResponseType),
      renderOnly(doRenderOnly)
  { }

  WEvent(WebSession::Handler& aHandler,
	 WebRenderer::ResponseType aResponseType)
    : handler(aHandler),
      responseType(aResponseType),
      renderOnly(false)
  { }

  WebSession::Handler& handler;
  WebRenderer::ResponseType responseType;
  bool renderOnly;

  friend class WebSession;
};

}

#endif // WEBSESSION_H_
