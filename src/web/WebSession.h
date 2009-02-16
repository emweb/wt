// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEBSESSION_H_
#define WEBSESSION_H_

#include <string>

#ifdef THREADED
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#endif // THREADED

#include "TimeUtil.h"
#include "WebRenderer.h"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLogger"

namespace Wt {

class CgiParser;
class WebRequest;
class WApplication;

/*
 * The WebSession stores the state for one session.
 */
class WT_API WebSession
{
public:
  enum Type { Application, WidgetSet };

  WebSession(const std::string& sessionId, const std::string& sessionPath,
	     Type type, const WebRequest& request);
  ~WebSession();

  Type type() const { return type_; }

  std::string docType() const;

  const std::string& applicationName() const { return applicationName_; }

  // (http://www.bigapp.com) /myapp/app.wt?wtd=ABCD
  const std::string& applicationUrl() const { return applicationUrl_; }

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

  std::string bootstrapUrl(const WebRequest& request, BootstrapOption option)
    const;

  // (http://www.bigapp.com/myapp/) app.wt/internal_path
  std::string bookmarkUrl(const std::string& internalPath) const;

  // tries to figure out the current bookmark url (from the app or otherwise)
  std::string bookmarkUrl() const;

  // http://www.bigapp.com:1234/myapp/
  const std::string& absoluteBaseUrl() const { return absoluteBaseUrl_; }

  const std::string& baseUrl() const { return baseUrl_; }

  WLogEntry log(const std::string& type);

  void setDebug(bool debug);
  bool debug() const { return debug_; }

  enum State {
    JustCreated,
    Bootstrap,
    Loaded,
    Dead };

  bool doingFullPageRender() const;

  State state() const { return state_; }
  void setState(State state, int timeout);
  const Time& expireTime() const { return expire_; }

  bool done() { return state_ == Dead; }

  void init(const CgiParser& cgi, const WebRequest& request);

#ifndef JAVA
  bool start(WApplication::ApplicationCreator createApp);
#endif // JAVA

  void kill();

  void refresh();
  void checkTimers();
  void hibernate();

  std::string   sessionId() const { return sessionId_; }
  WebRenderer&  renderer() { return renderer_; }
  WEnvironment& env()      { return env_; }
  WApplication  *app()     { return app_; }

  void redirect(const std::string& url);
  std::string getRedirect();

  WResource       *decodeResource(const std::string& resourceId);
  EventSignalBase *decodeSignal(const std::string& signalId);
  EventSignalBase *decodeSignal(const std::string& objectId,
				const std::string& signalName);

#ifdef THREADED
  boost::mutex mutex;
#endif // THREADED

  class Handler {
  public:
    Handler(WebSession& session, WebRequest *request = 0);
    ~Handler();

#ifdef THREADED
    void setLock(boost::mutex::scoped_lock *lock);
#endif // THREADED

    void setEventLoop(bool how);

    static void attachThreadToSession(WebSession& session);

    void killSession();
    bool sessionDead(); // killed or quited()
    bool eventLoop() const { return eventLoop_; }

    void setRequest(WebRequest *request);
    WebRequest *request() { return request_; }
    WebSession *session() const { return &session_; }

#ifdef THREADED
    boost::mutex::scoped_lock *lock() { return lock_; }
#endif // THREADED

  private:
    Handler(const Handler&);

#ifdef THREADED
    WebSession *sessionPtr_, **prevSessionPtr_;
    boost::mutex::scoped_lock *lock_;
#endif // THREADED
    WebSession& session_;
    WebRequest *request_;
    bool        eventLoop_;
    bool        killed_;
  };

  static WebSession *instance();

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

private:
#ifdef THREADED
  boost::mutex  stateMutex_;
#endif // THREADED

  Type          type_;
  State         state_;

  std::string   sessionId_;
  std::string   sessionPath_;

  WebRenderer   renderer_;
  std::string   applicationName_, sessionQuery_;
  std::string   bookmarkUrl_, baseUrl_, absoluteBaseUrl_, applicationUrl_;
  std::string   redirect_;
  Time          expire_;

  std::string   initialInternalPath_;
  WEnvironment  env_;
  WApplication *app_;
  bool          debug_;

  std::vector<Handler *> handlers_;
  std::vector<WObject *> emitStack_;

#ifdef THREADED
  static boost::thread_specific_ptr<WebSession *> threadSession_;

  boost::condition recursiveEventDone_;
#else
  static WebSession *threadSession_;
#endif // THREADED

  void pushEmitStack(WObject *obj);
  void popEmitStack();
  WObject *emitStackTop();

  Handler *findEventloopHandler(int index);
  void setEnvRequest(WebRequest *request);

  friend class WApplication;
  friend class SignalBase;
};

}

#endif // WEBSESSION_H_
