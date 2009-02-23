// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEBCONTROLLER_H_
#define WEBCONTROLLER_H_

#include <string>
#include <vector>

#ifdef THREADED
#include <boost/thread.hpp>
#include "threadpool/threadpool.hpp"
#endif

#include "Wt/WEnvironment"

#include "WebSession.h"
#include "Configuration.h"

namespace Wt {

class CgiParser;
class WebRequest;
class WebStream;
class Configuration;

class WApplication;
class WWidget;
class WObject;
class WResource;
class WSocketNotifier;
class WStatelessSlot;
class WWebWidget;

/*
 * The controller is a singleton class
 *
 * It either listens for incoming requests from a webstream, using run(),
 * or it may be used to handle an incoming request, using handleRequest().
 * In the latter case, sessions will only expire with a delay -- at the
 * next request. Seems harmless to me.
 *
 * There is a method forceShutDown() to quit the controller.
 *
 * It has the following tasks:
 *  - decode the request
 *  - propagate events
 *  - determine what needs to be served
 *    (a web page, a resource or a javascript update).
 *  - handle session life-cycle: create new sessions, delete quit()ed
 *    sessions, expire sessions on timeout
 *  - manage concurrency
 */
class WT_API WebController
{
public:
  /*
   * Construct the WebController and let it read requests from the given
   * streams.
   */
  WebController(Configuration& configuration, WebStream& stream,
		std::string singleSessionId = std::string());

  ~WebController();

  void run();
  int sessionCount() const;

#ifndef JAVA
  void handleRequest(WebRequest *request, const EntryPoint *entryPoint = 0);
#endif // JAVA

  void forceShutdown();

  static Configuration& conf();

  static std::string appSessionCookie(std::string url);
  static std::string sessionFromCookie(std::string cookies,
				       std::string scriptName,
				       int sessionIdLength);

  static WebController *instance() { return instance_; }

  void render(WebSession::Handler& handler,
	      CgiParser *cgi, WebRenderer::ResponseType type);

  void notify(const WEvent& e);

  typedef std::map<int, WSocketNotifier *> SocketNotifierMap;

  void addSocketNotifier(WSocketNotifier *notifier);
  void removeSocketNotifier(WSocketNotifier *notifier);

  // returns false if removeSocketNotifier was called while processing
  bool socketSelected(int descriptor);

  const SocketNotifierMap& socketNotifiers() const { return socketNotifiers_; }

private:
  Configuration& conf_;
  WebStream&     stream_;
  std::string    singleSessionId_;
  bool           running_;

  typedef std::map<std::string, WebSession *> SessionMap;
  SessionMap sessions_;

  SocketNotifierMap socketNotifiers_;

  bool shutdown_;

#ifdef THREADED
  // mutex to protect access to the sessions map.
  boost::recursive_mutex mutex_;

  boost::threadpool::pool threadPool_;
#endif

  bool expireSessions(std::vector<WebSession *>& toKill);
  void removeSession(WebSession *session);
  void handleRequestThreaded(WebRequest *request);

#ifndef JAVA
  const EntryPoint *getEntryPoint(WebRequest *request);
#endif // JAVA

  void checkTimers();

  enum SignalKind { LearnedStateless = 0, AutoLearnStateless = 1,
		    Dynamic = 2 };
  void processSignal(EventSignalBase *s, const std::string& se,
		     CgiParser& cgi, WebSession *session,
		     SignalKind kind);

  void notifySignal(const WEvent& e);
  void propagateFormValues(const WEvent& e);

  static void mxml_error_cb(const char *message);

  static WebController *instance_;

  CgiEntry *getSignal(const CgiParser& cgi, const std::string& se);
};

extern void WebStreamAddSocketNotifier(WSocketNotifier *notifier);
extern void WebStreamRemoveSocketNotifier(WSocketNotifier *notifier);

}

#endif // WEBCONTROLLER_H_
