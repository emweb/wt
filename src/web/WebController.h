// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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
#include <Wt/WServer>
#include <Wt/WSocketNotifier>

#include "SocketNotifier.h"

#if defined(WT_THREADED) && !defined(WT_TARGET_JAVA)
#include <boost/thread.hpp>
#endif

namespace Wt {

class Configuration;
class EntryPoint;

class WebRequest;
class WebSession;

class WApplication;
class WServer;

#ifndef WT_CNOR
/*
 * An event to be delivered to a session which is not caused by a web
 * request (or, probably not one for that session).
 */
struct ApplicationEvent {
  ApplicationEvent(const std::string& aSessionId,
		   const boost::function<void ()>& aFunction,
                   const boost::function<void ()>& aFallbackFunction
		     = boost::function<void ()>())
    : sessionId(aSessionId),
      function(aFunction),
      fallbackFunction(aFallbackFunction)
  { }

  std::string sessionId;
  boost::function<void ()> function;
  boost::function<void ()> fallbackFunction;
};
#endif

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

  WApplication *doCreateApplication(WebSession *session);
  Configuration& configuration();

  void addSession(boost::shared_ptr<WebSession> session);
  void removeSession(const std::string& sessionId);

  void newAjaxSession();
  bool limitPlainHtmlSessions();
  WServer *server() { return &server_; }

  std::string computeRedirectHash(const std::string& url);

#ifndef WT_TARGET_JAVA
  WebController(WServer& server,
		const std::string& singleSessionId = std::string(),
		bool autoExpire = true);
  ~WebController();

  int sessionCount() const;

  // Returns whether we should continue receiving data.
  bool requestDataReceived(WebRequest *request, boost::uintmax_t current,
			   boost::uintmax_t total);

  void handleRequest(WebRequest *request);

#ifndef WT_CNOR
  bool handleApplicationEvent(const ApplicationEvent& event);
#endif // WT_CNOR

  bool expireSessions();
  void shutdown();

  static std::string sessionFromCookie(std::string cookies,
				       std::string scriptName,
				       int sessionIdLength);

  typedef std::map<int, WSocketNotifier *> SocketNotifierMap;

  void addSocketNotifier(WSocketNotifier *notifier);
  void removeSocketNotifier(WSocketNotifier *notifier);

  void addUploadProgressUrl(const std::string& url);
  void removeUploadProgressUrl(const std::string& url);

  // returns false if removeSocketNotifier was called while processing
  void socketSelected(int descriptor, WSocketNotifier::Type type);

  std::string switchSession(WebSession *session,
			    const std::string& newSessionId);
  std::string generateNewSessionId(boost::shared_ptr<WebSession> session);

private:
  Configuration& conf_;
  std::string singleSessionId_;
  bool autoExpire_;
  int plainHtmlSessions_, ajaxSessions_;
  std::string redirectSecret_;

#ifdef WT_THREADED
  boost::mutex uploadProgressUrlsMutex_;
#endif // WT_THREADED
  std::set<std::string> uploadProgressUrls_;

  typedef std::map<std::string, boost::shared_ptr<WebSession> > SessionMap;
  SessionMap sessions_;

#ifdef WT_THREADED
  // mutex to protect access to the sessions map and plain/ajax session
  // counts
  boost::recursive_mutex mutex_;

  SocketNotifier socketNotifier_;
  // mutex to protect access to notifier maps. This cannot be protected
  // by mutex_ as this lock is grabbed while the application lock is
  // being held, which would potentially deadlock if we took mutex_.
  boost::recursive_mutex notifierMutex_;
  SocketNotifierMap socketNotifiersRead_;
  SocketNotifierMap socketNotifiersWrite_;
  SocketNotifierMap socketNotifiersExcept_;
  // assumes that you did grab the notifierMutex_
  SocketNotifierMap& socketNotifiers(WSocketNotifier::Type type);
  void socketNotify(int descriptor, WSocketNotifier::Type type);
#endif

  void updateResourceProgress(WebRequest *request,
			      boost::uintmax_t current, boost::uintmax_t total);

  const EntryPoint *getEntryPoint(WebRequest *request);

  static std::string appSessionCookie(std::string url);

#endif // WT_TARGET_JAVA

  WServer& server_;
};

}

#endif // WT_WEB_CONTROLLER_H_
