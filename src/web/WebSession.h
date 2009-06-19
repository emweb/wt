// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEBSESSION_H_
#define WEBSESSION_H_

#include <string>

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
  enum Type { Application, WidgetSet };

  WebSession(WebController *controller, const std::string& sessionId,
	     Type type, const std::string& favicon,
             const WebRequest& request);
  ~WebSession();

  static WebSession *instance();

  Type type() const { return type_; }
  std::string favicon() const { return favicon_; }
  std::string docType() const;

  std::string sessionId() const { return sessionId_; }

  WebController *controller() const { return controller_; }
  WEnvironment&  env() { return env_; }
  WApplication  *app() { return app_; }
  WebRenderer&   renderer() { return renderer_; }

  void redirect(const std::string& url);
  std::string getRedirect();

  void setApplication(WApplication *app);

  WLogEntry log(const std::string& type);

  static void notify(const WEvent& e);
  bool handleRequest(WebRequest& request, WebResponse& response);
  void pushUpdates();

  /*
   * Start a recursive event loop: finishes the request, rendering
   * everything, and suspending the thread until someone calls
   * unlockRecursiveEventLoop();
   */
  void doRecursiveEventLoop(const std::string& javascript);

  /*
   * Immediately returns, but lets the last pending recursive event loop
   * resume and finish the current request, by swapping the request.
   */
  void unlockRecursiveEventLoop();

  void pushEmitStack(WObject *obj);
  void popEmitStack();
  WObject *emitStackTop();

#ifndef WT_TARGET_JAVA
  const Time& expireTime() const { return expire_; }
  bool shouldDisconnect() const;
#endif // WT_TARGET_JAVA

  bool done() { return state_ == Dead; }

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
    Handler(WebSession& session, bool locked = true);
    ~Handler();

    static Handler *instance();

    WebResponse *response() { return response_; }
    WebRequest  *request() { return request_; }
    WebSession  *session() const { return &session_; }
    void killSession();

  private:
    void init();

    void setEventLoop(bool how);

    static void attachThreadToSession(WebSession& session);

    bool sessionDead(); // killed or quited()
    bool eventLoop() const { return eventLoop_; }

    void swapRequest(WebRequest *request, WebResponse *response);

#ifdef WT_THREADED
    boost::mutex::scoped_lock& lock() { return lock_; }
    boost::mutex::scoped_lock lock_;

    Handler(const Handler&);

    Handler *prevHandler_;
#endif // WT_THREADED

    WebSession&  session_;
    WebRequest  *request_;
    WebResponse *response_;
    bool         eventLoop_;
    bool         killed_;

    friend class WApplication;
    friend class WResource;
    friend class WebSession;
    friend class WFileUploadResource;
  };

#ifdef WT_THREADED
  boost::mutex& mutex() { return mutex_; }
#endif // WT_THREADED

private:
  /*
   * Misc methods
   */

  void setDebug(bool debug);
  bool debug() const { return debug_; }

  void checkTimers();
  void hibernate();

  enum State {
    JustCreated,
    Bootstrap,
    Loaded,
    Dead
  };

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
  boost::mutex mutex_;
  static boost::thread_specific_ptr<Handler> threadHandler_;
#else
  static Handler *threadHandler_;
#endif // WT_TARGET_JAVA

  Type          type_;
  std::string   favicon_;
  State         state_;

  std::string   sessionId_;

  WebController *controller_;
  WebRenderer   renderer_;
  std::string   applicationName_;
  std::string   bookmarkUrl_, baseUrl_, absoluteBaseUrl_;
  std::string   applicationUrl_, deploymentPath_;
  std::string   redirect_;
  WebResponse  *pollResponse_;
  bool          updatesPending_;

#ifndef WT_TARGET_JAVA
  Time             expire_;
#ifdef WT_THREADED
  boost::condition recursiveEventDone_;
#endif // WT_THREADED
#endif // WT_TARGET_JAVA

  WEnvironment  env_;
  WApplication *app_;
  bool          debug_;

  std::vector<Handler *> handlers_;
  std::vector<WObject *> emitStack_;

  Handler *findEventloopHandler(int index);

  WResource *decodeResource(const std::string& resourceId);
  EventSignalBase *decodeSignal(const std::string& signalId);
  EventSignalBase *decodeSignal(const std::string& objectId,
				const std::string& signalName);

  static WObject::FormData getFormData(const WebRequest& request,
				       const std::string& name);

  void render(Handler& handler, WebRenderer::ResponseType type);

  enum SignalKind { LearnedStateless = 0, AutoLearnStateless = 1,
		    Dynamic = 2 };
  void processSignal(EventSignalBase *s, const std::string& se,
		     const WebRequest& request, SignalKind kind);

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
 * The request controller notifies the application to react to browser events,
 * or to render itself, using WApplication::notify().
 */
class WT_API WEvent {
public:
  enum EventType { EmitSignal, Refresh, Render, HashChange };

  WebSession::Handler& handler;
  EventType            type;

  WebSession *session() const { return handler.session(); }

  /* For Render type */
  WebRenderer::ResponseType responseType;

  /* For HashChange type */
  std::string hash;

  WEvent(WebSession::Handler& aHandler, EventType aType,
	 WebRenderer::ResponseType aResponseType = WebRenderer::FullResponse)
    : handler(aHandler),
      type(aType),
      responseType(aResponseType)
  { }

  WEvent(WebSession::Handler& aHandler, EventType aType,
	 const std::string& aHash)
    : handler(aHandler),
      type(aType),
      responseType(WebRenderer::FullResponse),
      hash(aHash)
  { }
};

}

#endif // WEBSESSION_H_
