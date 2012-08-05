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
#define WT_BOOST_THREADS
#endif

#ifdef WT_BOOST_THREADS
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#endif

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

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
#ifndef WT_TARGET_JAVA
  : public boost::enable_shared_from_this<WebSession>
#endif
{
public:
  enum State {
    JustCreated,
    ExpectLoad,
    Loaded,
    Dead
  };

  WebSession(WebController *controller, const std::string& sessionId,
	     EntryPointType type, const std::string& favicon,
	     const WebRequest *request, WEnvironment *env = 0);
  ~WebSession();

  static WebSession *instance();

  bool attachThreadToLockedHandler();

  EntryPointType type() const { return type_; }
  std::string favicon() const { return favicon_; }
  std::string docType() const;

  std::string sessionId() const { return sessionId_; }

  WebController *controller() const { return controller_; }
  WEnvironment&  env() { return *env_; }
  WApplication  *app() { return app_; }
  WebRenderer&   renderer() { return renderer_; }
  bool useUrlRewriting() { return useUrlRewriting_; }

  bool debug() const { return debug_; }

  void redirect(const std::string& url);
  std::string getRedirect();

  void setApplication(WApplication *app);

#ifndef WT_TARGET_JAVA
  WLogEntry log(const std::string& type) const;
#endif // WT_TARGET_JAVA

  void notify(const WEvent& e);
  void pushUpdates();

  void doRecursiveEventLoop();
  bool bootStyleResponse() const { return !noBootStyleResponse_; }

  void deferRendering();
  void resumeRendering();
  void setTriggerUpdate(bool needTrigger);

  void expire();
  bool unlockRecursiveEventLoop();

  void pushEmitStack(WObject *obj);
  void popEmitStack();
  WObject *emitStackTop();

#ifndef WT_TARGET_JAVA
  const Time& expireTime() const { return expire_; }
  bool shouldDisconnect() const;
#endif // WT_TARGET_JAVA

  bool dead() { return state_ == Dead; }
  State state() const { return state_; }
  void kill();

  bool progressiveBoot() const { return progressiveBoot_; }

  /*
   * URL stuff
   */

  const std::string& applicationName() const { return applicationName_; }
  const std::string applicationUrl() const
    { return applicationUrl_ + sessionQuery(); }
  const std::string& deploymentPath() const { return deploymentPath_; }

  bool hasSessionIdInUrl() const { return sessionIdInUrl_; }
  bool useUglyInternalPaths() const;
  void setPagePathInfo(const std::string& path);
  std::string pagePathInfo() const { return pagePathInfo_; }

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

  std::string fixRelativeUrl(const std::string& url) const;
  std::string makeAbsoluteUrl(const std::string& url) const;

  // (http://www.bigapp.com/myapp/) app.wt/internal_path
  std::string bookmarkUrl(const std::string& internalPath) const;

  // tries to figure out the current bookmark url (from the app or otherwise)
  std::string bookmarkUrl() const;

  std::string getCgiValue(const std::string& varName) const;
  std::string getCgiHeader(const std::string& headerName) const;

  EventType getEventType(const WEvent& event) const;
  void setState(State state, int timeout);

  class WT_API Handler {
  public:
    Handler();
    Handler(boost::shared_ptr<WebSession> session,
	    WebRequest& request, WebResponse& response);
    Handler(boost::shared_ptr<WebSession> session, bool takeLock);
    Handler(WebSession *session);
    ~Handler();

#ifdef WT_TARGET_JAVA
    void release();
#endif // WT_TARGET_JAVA

    static Handler *instance();

    bool haveLock() const;

    WebResponse *response() { return response_; }
    WebRequest *request() { return request_; }
    WebSession *session() const { return session_; }
    void setRequest(WebRequest *request, WebResponse *response);

    int nextSignal;
    std::vector<unsigned int> signalOrder;

#ifdef WT_THREADED
    boost::mutex::scoped_lock& lock() { return lock_; }
#endif

    static void attachThreadToSession(boost::shared_ptr<WebSession> session);
    static Handler *attachThreadToHandler(Handler *handler);

  private:
    void init();

#ifdef WT_THREADED
    boost::mutex::scoped_lock lock_;

    Handler(const Handler&);
#endif // WT_THREADED

    Handler *prevHandler_;

    WebSession *session_;
#ifndef WT_TARGET_JAVA
    boost::shared_ptr<WebSession> sessionPtr_;
#endif

    WebRequest *request_;
    WebResponse *response_;
    bool killed_;

    friend class WApplication;
    friend class WResource;
    friend class WebSession;
    friend class WFileUploadResource;
  };

  void handleRequest(Handler& handler);

#ifdef WT_BOOST_THREADS
  boost::mutex& mutex() { return mutex_; }
#endif

  void setExpectLoad();
  void setLoaded();

  void generateNewSessionId();

#if defined(WT_THREADED)
  /*
   * SyncLocks
   */
  struct SyncLocks {
    boost::mutex state_;
    boost::condition unlock_;
    int lastId_;
    int lockedId_;
  } syncLocks_;
#endif // WT_THREADED

private:
  void handleWebSocketRequest(Handler& handler);
  static void handleWebSocketMessage(boost::weak_ptr<WebSession> session);
  static void webSocketReady(boost::weak_ptr<WebSession> session);

  void checkTimers();
  void hibernate();

#ifdef WT_BOOST_THREADS
  boost::mutex mutex_;
  static boost::thread_specific_ptr<Handler> threadHandler_;
#else
  static Handler *threadHandler_;
#endif

  EntryPointType type_;
  std::string favicon_;
  State state_;

  bool useUrlRewriting_;
  std::string sessionId_, sessionIdCookie_;
  bool sessionIdChanged_, sessionIdCookieChanged_, sessionIdInUrl_;

  WebController *controller_;
  WebRenderer renderer_;
  std::string applicationName_;
  std::string bookmarkUrl_, basePath_, absoluteBaseUrl_;
  std::string applicationUrl_, deploymentPath_;
  std::string redirect_;
  std::string pagePathInfo_;
  WebResponse *asyncResponse_, *bootStyleResponse_;
  bool canWriteAsyncResponse_, noBootStyleResponse_;
  int pollRequestsIgnored_;
  bool progressiveBoot_;

  WebRequest *deferredRequest_;
  WebResponse *deferredResponse_;
  int deferCount_;

#ifndef WT_TARGET_JAVA
  Time             expire_;
#endif

#ifdef WT_BOOST_THREADS
  boost::condition recursiveEvent_;
#endif
  bool             newRecursiveEvent_;

  /* For synchronous handling */
#ifdef WT_BOOST_THREADS
  boost::condition updatesPendingEvent_;
#endif
  bool             updatesPending_, triggerUpdate_;

  WEnvironment  embeddedEnv_;
  WEnvironment *env_;
  WApplication *app_;
  bool          debug_;

  std::vector<Handler *> handlers_;
  std::vector<WObject *> emitStack_;

  Handler *recursiveEventLoop_;

  WResource *decodeResource(const std::string& resourceId);
  EventSignalBase *decodeSignal(const std::string& signalId,
				bool checkExposed) const;
  EventSignalBase *decodeSignal(const std::string& objectId,
				const std::string& signalName,
				bool checkExposed) const;

  static WObject::FormData getFormData(const WebRequest& request,
				       const std::string& name);

  void render(Handler& handler);
  void serveError(int status, Handler& handler, const std::string& exception);
  void serveResponse(Handler& handler);

  enum SignalKind { LearnedStateless = 0, AutoLearnStateless = 1,
		    Dynamic = 2 };
  void processSignal(EventSignalBase *s, const std::string& se,
		     const WebRequest& request, SignalKind kind);

  std::vector<unsigned int> getSignalProcessingOrder(const WEvent& e) const;
  void notifySignal(const WEvent& e);
  void propagateFormValues(const WEvent& e, const std::string& se);

  const std::string *getSignal(const WebRequest& request,
			       const std::string& se) const;

  void init(const WebRequest& request);
  bool start();

  std::string sessionQuery() const;
  void flushBootStyleResponse();

  friend class WebSocketMessage;
  friend class WebRenderer;
};

struct WEvent::Impl {
  WebSession::Handler *handler;
#ifndef WT_CNOR
  boost::function<void ()> function;
#endif
  bool renderOnly;

  Impl(WebSession::Handler *aHandler, bool doRenderOnly = false)
    : handler(aHandler),
      renderOnly(doRenderOnly)
  { }

#ifndef WT_CNOR
  Impl(WebSession::Handler *aHandler, const boost::function<void ()>& aFunction)
    : handler(aHandler),
      function(aFunction),
      renderOnly(false)
  { }
#endif

  Impl()
    : handler(0)
  { }
};

}

#endif // WEBSESSION_H_
