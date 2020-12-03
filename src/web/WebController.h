// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WEB_CONTROLLER_H_
#define WT_WEB_CONTROLLER_H_

#include <string>
#include <vector>
#include <set>
#include <map>

#include <Wt/WDllDefs.h>
#include <Wt/WServer.h>
#include <Wt/WSocketNotifier.h>

#include "EntryPoint.h"
#include "SocketNotifier.h"

#if defined(WT_THREADED) && !defined(WT_TARGET_JAVA)
#include <thread>
#include <mutex>
#endif

namespace http {
  namespace server {
	class ProxyReply;
  }
}

namespace Wt {

class Configuration;
class EntryPoint;

class WebRequest;
class WebSession;

class WApplication;
class WServer;

#ifdef WT_CNOR
class Runnable {
public:
  void run();
};

typedef Runnable *Function;

#define WT_CALL_FUNCTION(f) (f)->run();

#else
typedef std::function<void ()>  Function;
#define WT_CALL_FUNCTION(f) (f)();
#endif

/*
 * An event to be delivered to a session which is not caused by a web
 * request (or, probably not one for that session).
 */
struct ApplicationEvent {
  ApplicationEvent(const std::string& aSessionId,
		   const Function& aFunction,
                   const Function& aFallbackFunction = Function())
    : sessionId(aSessionId),
      function(aFunction),
      fallbackFunction(aFallbackFunction)
  { }

  std::string sessionId;
  Function function;
  Function fallbackFunction;
};

/*
 * The controller handle incoming request, in handleRequest().
 * Optionally, it will expire session on each incoming request. Seems
 * harmless to me (but causes confusion to others).
 *
 * There is a method shutdown() to quit the controller.
 *
 * It has the following tasks:
 *  - handle session life-cycle: create new sessions, delete quit()ed
 *    sessions, expire sessions on timeout
 *  - handles web requests and application events
 */
class WT_API WebController
#ifdef WT_TARGET_JAVA
  : public WServer
#endif // WT_TARGET_JAVA
{
public:
  static bool isAsyncSupported() { return true; }

  std::unique_ptr<WApplication> doCreateApplication(WebSession *session);
  Configuration& configuration();

  void addSession(const std::shared_ptr<WebSession>& session);
  void removeSession(const std::string& sessionId);
  void sessionDeleted();

  void newAjaxSession();
  bool limitPlainHtmlSessions();
  WServer *server() { return &server_; }

  std::string computeRedirectHash(const std::string& url);

#ifdef WT_TARGET_JAVA
  int getIdForWebSocket(); 
  std::string getContextPath();
#else // WT_TARGET_JAVA
  WebController(WServer& server,
		const std::string& singleSessionId = std::string(),
		bool autoExpire = true);
  ~WebController();

  int sessionCount() const;

  // Returns whether we should continue receiving data.
  bool requestDataReceived(WebRequest *request, std::uintmax_t current,
			   std::uintmax_t total);

  void handleRequest(WebRequest *request);

#ifndef WT_CNOR
  bool handleApplicationEvent(const std::shared_ptr<ApplicationEvent>& event);
#endif // WT_CNOR

  std::vector<std::string> sessions();
  bool expireSessions();
  void start();
  void shutdown();

  static std::string sessionFromCookie(const char * const cookies,
				       const std::string& scriptName,
                                       const int sessionIdLength);

  typedef std::map<int, WSocketNotifier *> SocketNotifierMap;

  void addSocketNotifier(WSocketNotifier *notifier);
  void removeSocketNotifier(WSocketNotifier *notifier);

  void addUploadProgressUrl(const std::string& url);
  void removeUploadProgressUrl(const std::string& url);

  // returns false if removeSocketNotifier was called while processing
  void socketSelected(int descriptor, WSocketNotifier::Type type);

  std::string switchSession(WebSession *session,
			    const std::string& newSessionId);
  std::string generateNewSessionId(const std::shared_ptr<WebSession>& session);

private:
  Configuration& conf_;
  std::string singleSessionId_;
  bool autoExpire_;
  int plainHtmlSessions_, ajaxSessions_;
  volatile int zombieSessions_;
  std::string redirectSecret_;
  bool running_;

#ifdef WT_THREADED
  std::mutex uploadProgressUrlsMutex_;
#endif // WT_THREADED
  std::set<std::string> uploadProgressUrls_;

  typedef std::map<std::string, std::shared_ptr<WebSession> > SessionMap;
  SessionMap sessions_;

#ifdef WT_THREADED
  // mutex to protect access to the sessions map and plain/ajax session
  // counts
  std::recursive_mutex mutex_;

  SocketNotifier socketNotifier_;
  // mutex to protect access to notifier maps. This cannot be protected
  // by mutex_ as this lock is grabbed while the application lock is
  // being held, which would potentially deadlock if we took mutex_.
  std::recursive_mutex notifierMutex_;
  SocketNotifierMap socketNotifiersRead_;
  SocketNotifierMap socketNotifiersWrite_;
  SocketNotifierMap socketNotifiersExcept_;
  // assumes that you did grab the notifierMutex_
  SocketNotifierMap& socketNotifiers(WSocketNotifier::Type type);
  void socketNotify(int descriptor, WSocketNotifier::Type type);
#endif

  struct UpdateResourceProgressParams {
      std::string requestParam;
      std::string resourceParam;
      ::int64_t postDataExceeded;
      std::string pathInfo;
      std::uintmax_t current;
      std::uintmax_t total;
  };
  void updateResourceProgress(const UpdateResourceProgressParams &params);

  EntryPointMatch getEntryPoint(WebRequest *request);

  static std::string appSessionCookie(const std::string& url);

#endif // WT_TARGET_JAVA

  WServer& server_;

  friend class http::server::ProxyReply;
  friend class WEnvironment;
};

}

#endif // WT_WEB_CONTROLLER_H_
