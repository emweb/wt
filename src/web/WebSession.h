// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEBSESSION_H_
#define WEBSESSION_H_

#include <mutex>
#include <string>
#include <vector>
#include <deque>

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
#define WT_BOOST_THREADS
#endif

#ifdef WT_BOOST_THREADS
#include <thread>
#include <condition_variable>
#endif

#ifdef WT_TARGET_JAVA
#include <boost/thread.hpp>
#endif // WT_TARGET_JAVA

#include "TimeUtil.h"
#include "WebRenderer.h"
#include "WebRequest.h"
#include "WebController.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"

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
  : public std::enable_shared_from_this<WebSession>
#endif
{
public:
  enum class State {
    JustCreated,
    ExpectLoad,
    Loaded,
    Dead
  };

  WebSession(WebController *controller, const std::string& sessionId,
	     EntryPointType type, const std::string& favicon,
	     const WebRequest *request, WEnvironment *env = nullptr);
  ~WebSession();

#ifdef WT_TARGET_JAVA
  void destruct();
#endif // WT_TARGET_JAVA

  static WebSession *instance();

  bool attachThreadToLockedHandler();

  EntryPointType type() const { return type_; }
  std::string favicon() const { return favicon_; }
  std::string docType() const;

  std::string sessionId() const { return sessionId_; }
  std::string multiSessionId() const { return multiSessionId_; }
  void setMultiSessionId(const std::string &multiSessionId);

  WebController *controller() const { return controller_; }
  WEnvironment&  env() { return *env_; }
  WApplication  *app() { return app_; }
  WebRenderer&   renderer() { return renderer_; }
  bool useUrlRewriting();

  bool debug() const { return debug_; }

  void redirect(const std::string& url);
  std::string getRedirect();

  void setApplication(WApplication *app);

#ifndef WT_TARGET_JAVA
  WLogger& logInstance() const;
  WLogEntry log(const std::string& type) const;
#endif // WT_TARGET_JAVA

  void externalNotify(const WEvent::Impl& e);
  void notify(const WEvent& e);

  void doRecursiveEventLoop();

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
#endif // WT_TARGET_JAVA

  bool dead() { return state_ == State::Dead; }
  State state() const { return state_; }
  void kill();

  bool progressiveBoot() const { return progressiveBoot_; }

  /*
   * URL stuff
   */

  const std::string& applicationName() const { return applicationName_; }
  const std::string applicationUrl() const { return applicationUrl_; }
  const std::string& deploymentPath() const { return deploymentPath_; }

  bool hasSessionIdInUrl() const { return sessionIdInUrl_; }
  void setSessionIdInUrl(bool value) { sessionIdInUrl_ = value; }
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

  enum class BootstrapOption {
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
    enum class LockOption {
      NoLock,
      TryLock,
      TakeLock
    };

    Handler();
    Handler(const std::shared_ptr<WebSession>& session,
	    WebRequest& request, WebResponse& response);
    Handler(const std::shared_ptr<WebSession>& session, LockOption lockOption);
    Handler(WebSession *session);
    ~Handler();

#ifdef WT_TARGET_JAVA
    void release();
#endif // WT_TARGET_JAVA

    static Handler *instance();

    bool haveLock() const;
    void unlock();

    void flushResponse();
    WebResponse *response() { return response_; }
    WebRequest *request() { return request_; }
    WebSession *session() const { return session_; }
    void setRequest(WebRequest *request, WebResponse *response);

    int nextSignal;
    std::vector<unsigned int> signalOrder;

#ifdef WT_THREADED
    std::thread::id lockOwner() const { return lockOwner_; }
    std::unique_lock<std::mutex>& lock() { return lock_; }
#endif

    static void attachThreadToSession
      (const std::shared_ptr<WebSession>& session);
    static Handler *attachThreadToHandler(Handler *handler);

  private:
    void init();

#ifndef WT_TARGET_JAVA
    std::shared_ptr<WebSession> sessionPtr_;
#endif
#ifdef WT_THREADED
    std::unique_lock<std::mutex> lock_;
    std::thread::id lockOwner_;

    Handler(const Handler&);
#endif // WT_THREADED

    Handler *prevHandler_;

    WebSession *session_;

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
  std::mutex& mutex() { return mutex_; }
#ifdef WT_TARGET_JAVA
  static boost::thread_specific_ptr<Handler> threadHandler_;
#endif // WT_TARGET_JAVA
#endif

  void setExpectLoad();
  void setLoaded();

  void generateNewSessionId();
  void queueEvent(const std::shared_ptr<ApplicationEvent>& event);

#ifdef WT_TARGET_JAVA
  void handleWebSocketMessage(Handler& handler);
#endif

#ifndef WT_TARGET_JAVA
  // For use in WTestEnvironment
  void setDocRoot(const std::string &docRoot);
#endif

private:
#ifndef WT_TARGET_JAVA
  void handleWebSocketRequest(Handler& handler);
  static void handleWebSocketMessage(std::weak_ptr<WebSession> session,
				     WebReadEvent event);
  static void webSocketConnect(std::weak_ptr<WebSession> session,
			       WebWriteEvent event);
  static void webSocketReady(std::weak_ptr<WebSession> session,
			     WebWriteEvent event);
#endif

  void checkTimers();
  void hibernate();

#ifdef WT_BOOST_THREADS
  std::mutex mutex_;
  std::mutex eventQueueMutex_;
#endif

  std::deque<std::shared_ptr<ApplicationEvent> > eventQueue_;

  EntryPointType type_;
  std::string favicon_;
  State state_;

  std::string sessionId_, sessionIdCookie_, multiSessionId_;
  bool sessionIdChanged_, sessionIdCookieChanged_, sessionIdInUrl_;

  WebController *controller_;
  WebRenderer renderer_;
  std::string applicationName_;
  std::string bookmarkUrl_, basePath_, absoluteBaseUrl_;
  std::string applicationUrl_, deploymentPath_;
  std::string docRoot_;
  std::string redirect_;
  std::string pagePathInfo_;
  WebResponse *asyncResponse_, *webSocket_, *bootStyleResponse_;
  bool canWriteWebSocket_, webSocketConnected_;
  int pollRequestsIgnored_;
  bool progressiveBoot_;

  WebRequest *deferredRequest_;
  WebResponse *deferredResponse_;
  int deferCount_;

#ifndef WT_TARGET_JAVA
  Time             expire_;
#endif

#ifdef WT_BOOST_THREADS
  std::condition_variable recursiveEvent_, recursiveEventDone_;
#endif
  WEvent::Impl *newRecursiveEvent_;

  /* For synchronous handling */
#ifdef WT_BOOST_THREADS
  std::condition_variable updatesPendingEvent_;
#endif
  bool updatesPending_, triggerUpdate_;

  WEnvironment embeddedEnv_;
  WEnvironment *env_;
  WApplication *app_;
  bool debug_;

  std::vector<Handler *> handlers_;

  Handler *recursiveEventHandler_;

  void pushUpdates();
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

  enum class SignalKind { LearnedStateless = 0,
                          AutoLearnStateless = 1,
                          Dynamic = 2 };
  void processSignal(EventSignalBase *s, const std::string& se,
                     const WebRequest& request, SignalKind kind);

  std::vector<unsigned int> getSignalProcessingOrder(const WEvent& e) const;
  void notifySignal(const WEvent& e);
  void propagateFormValues(const WEvent& e, const std::string& se);

  const std::string *getSignal(const WebRequest& request,
			       const std::string& se) const;

  void init(const WebRequest& request);
  bool start(WebResponse *response);

  std::string sessionQuery() const;
  void flushBootStyleResponse();
  void changeInternalPath(const std::string& path, WebResponse *response);

  void processQueuedEvents(WebSession::Handler& handler);
  std::shared_ptr<ApplicationEvent> popQueuedEvent();

  friend class WebSocketMessage;
  friend class WebRenderer;
  friend class WebSocketSupport;
};

struct WEvent::Impl {
  WebSession::Handler *handler;
  WebResponse *response;
  Function function;
  bool renderOnly;

  Impl(WebSession::Handler *aHandler, bool doRenderOnly = false)
    : handler(aHandler),
      response(nullptr),
      renderOnly(doRenderOnly)
  { }

  Impl(WebSession::Handler *aHandler, const Function& aFunction)
    : handler(aHandler),
      response(nullptr),
      function(aFunction),
      renderOnly(false)
  { }

  Impl(const Impl& other)
    : handler(other.handler),
      response(other.response),
      function(other.function),
      renderOnly(other.renderOnly)
  { }

  Impl(WebResponse *aResponse)
    : handler(nullptr),
      response(aResponse),
      renderOnly(true)
  { }

  Impl()
    : handler(nullptr),
      response(nullptr)
  { }
};

}

#endif // WEBSESSION_H_
