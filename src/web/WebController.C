/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>
#include <sstream>

#ifndef WT_NO_XML
#include <mxml.h>
#endif

#ifdef WT_HAVE_GNU_REGEX
#include <regex.h>
#else
#include <boost/regex.hpp>
#endif // WT_HAVE_GNU_REGEX

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#ifdef WT_THREADED
#include <boost/bind.hpp>
#endif // WT_THREADED

#include "Wt/WApplication"
#include "Wt/WEvent"
#include "Wt/WResource"
#include "Wt/WSocketNotifier"
#include "Wt/WStringUtil"

#include "Configuration.h"
#include "CgiParser.h"
#include "WebController.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WebStream.h"
#include "WtException.h"
#include "TimeUtil.h"
#include "Utils.h"

namespace Wt {

WebController::WebController(Configuration& configuration,
			     WebStream *stream, std::string singleSessionId)
  : conf_(configuration),
    stream_(stream),
    singleSessionId_(singleSessionId),
    running_(false),
    shutdown_(false)
#ifdef WT_THREADED
  , threadPool_(conf_.serverType() == Configuration::WtHttpdServer
		? 0 : conf_.numThreads())
#endif // WT_THREADED
{
  CgiParser::init();

#ifndef WT_NO_XML
  /*
   * mxml errors should not be fatal, and mxml is only used within the
   * scope of an application. Thus, simply log them.
   */
  mxmlSetErrorCallback(WebController::mxml_error_cb);
#endif
}

WebController::~WebController()
{
}

#ifndef WT_NO_XML
void WebController::mxml_error_cb(const char * message)
{
  WApplication *app = wApp;

  if (app)
    app->log("error") << "XML error: " << message;
  else
    std::cerr << "XML error: " << message;
}
#endif // WT_NO_XML

void WebController::forceShutdown()
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // WT_THREADED

  conf_.log("notice") << "Shutdown: stopping sessions.";

  shutdown_ = true;

  while (!sessions_.empty()) {
    SessionMap::iterator i = sessions_.begin();
    WebSession::Handler handler(*i->second, true);
    handler.killSession();
    sessions_.erase(i);
  }
}

Configuration& WebController::configuration()
{
  return conf_;
}

int WebController::sessionCount() const
{
  return sessions_.size();
}

void WebController::run()
{
  running_ = true;

  WebRequest *request = stream_->getNextRequest(10);

  if (request)
    handleRequest(request);
  else
    if (!singleSessionId_.empty()) {
      running_ = false;
      conf_.log("error") << "No initial request ?";
      return;
    }

  for (;;) {
    bool haveMoreSessions = expireSessions();

    if (!haveMoreSessions && !singleSessionId_.empty()) {
      conf_.log("notice") << "Dedicated session process exiting cleanly.";
      break;
    }

    WebRequest *request = stream_->getNextRequest(5);

    if (shutdown_) {
      conf_.log("notice") << "Shared session server exiting cleanly.";
#ifndef WIN32 // Not used in win32 anyway, fastcgi is not available
      sleep(1000);
#endif
      break;
    }

    if (request)
      handleRequestThreaded(request);
  }

  running_ = false;  
}

bool WebController::expireSessions()
{
  Time now;

#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // WT_THREADED

  std::vector<WebSession *> toKill;
  for (SessionMap::iterator i = sessions_.begin(); i != sessions_.end(); ++i) {
    WebSession *session = i->second;

    if (!session->done()) {
      int diff = session->expireTime() - now;

      if (diff < 1000) {
	if (session->shouldDisconnect()) {
	  session->app()->connected_ = false;
	  session->log("notice") << "Timeout: disconnected";
	} else {
	  i->second->log("notice") << "Timeout: expiring";
	  toKill.push_back(i->second);
	}
      }
    }
  }

  for (unsigned i = 0; i < toKill.size(); ++i) {
    WebSession *session = toKill[i];
    sessions_.erase(session->sessionId());
    WebSession::Handler handler(*session, true);
    handler.killSession();
  }

  return !sessions_.empty();
}

void WebController::removeSession(const std::string& sessionId)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // WT_THREADED

  SessionMap::iterator i = sessions_.find(sessionId);
  if (i != sessions_.end())
    sessions_.erase(i);
}

std::string WebController::appSessionCookie(std::string url)
{
  return Utils::urlEncode(url);
}

void WebController::handleRequestThreaded(WebRequest *request)
{
#ifdef WT_THREADED
  if (stream_->multiThreaded()) {
    threadPool_.schedule(boost::bind(&WebController::handleRequest,
				     this, request, (const EntryPoint *)0));
  } else
    handleRequest(request);
#else
  handleRequest(request);
#endif // WT_THREADED
}

std::string WebController::sessionFromCookie(std::string cookies,
					     std::string scriptName,
					     int sessionIdLength)
{
  std::string cookieName = appSessionCookie(scriptName);

#ifndef WT_HAVE_GNU_REGEX
  boost::regex
    cookieSession_e(".*\\Q" + cookieName
		    + "\\E=\"?([a-zA-Z0-9]{"
		    + boost::lexical_cast<std::string>(sessionIdLength)
		    + "})\"?.*");

  boost::smatch what;

  if (boost::regex_match(cookies, what, cookieSession_e))
    return what[1];
  else
    return std::string();
#else
  std::string cookieSession_ep
    = cookieName + "=\"\\?\\([a-zA-Z0-9]\\{"
    + boost::lexical_cast<std::string>(sessionIdLength) + "\\}\\)\"\\?";
  regex_t cookieSession_e;
  regcomp(&cookieSession_e, cookieSession_ep.c_str(), 0);
  regmatch_t pmatch[2];
  int res = regexec(&cookieSession_e, cookies.c_str(), 2, pmatch, 0);
  regfree(&cookieSession_e);

  if (res == 0) {
    return cookies.substr(pmatch[1].rm_so,
			  pmatch[1].rm_eo - pmatch[1].rm_so);
  } else
    return std::string();
#endif
}

bool WebController::socketSelected(int descriptor)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // WT_THREADED

  SocketNotifierMap::iterator k = socketNotifiers_.find(descriptor);

  if (k == socketNotifiers_.end()) {
    conf_.log("error") << "WebController::socketSelected(): socket notifier"
      " should have been cancelled?";

    return false;
  }

  WSocketNotifier *notifier = k->second;

  SessionMap::iterator i = sessions_.find(notifier->sessionId());
  WebSession *session = 0;

  if (i == sessions_.end()) {
    conf_.log("error")
      << "WebController::socketSelected(): socket notification"
       " for expired session " << notifier->sessionId() << ". Leaking memory?";

    return false;
  } else {
    session = i->second;
  }

  {
#ifdef WT_THREADED
    WApplication *app = session->app();

    /*
     * Is correct, but now we are holding the sessionsLock too long: we
     * should in principle make sure the session will not be deleted (like
     * the handler does), and then release the sessionsLock before trying to
     * access the session exclusively.
     */
    WApplication::UpdateLock l = app->getUpdateLock();
    sessionsLock.unlock();
#endif // WT_THREADED

    notifier->notify();
  }

#ifdef WT_THREADED
  sessionsLock.lock();
#endif // WT_THREADED

  if (socketNotifiers_.find(descriptor) == socketNotifiers_.end())
    return true;
  else
    return false;
}

void WebController::handleRequest(WebRequest *request, const EntryPoint *ep)
{
  if (!ep)
    ep = getEntryPoint(request->scriptName());

  CgiParser cgi(conf_.maxRequestSize() * 1024);

  try {
    cgi.parse(*request);
  } catch (std::exception& e) {
    conf_.log("error") << "Could not parse request: " << e.what();

    request->setContentType("text/html");
    request->out()
      << "<title>Error occurred.</title>"
      << "<h2>Error occurred.</h2>"
         "Error parsing CGI request: " << e.what() << std::endl;

    request->flush(WebResponse::ResponseDone);
    return;
  }

  if (ep->type() == StaticResource) {
    ep->resource()->handle(request, (WebResponse *)request);
    return;
  }

  std::string sessionId = singleSessionId_;
  const std::string *wtdE = request->getParameter("wtd");

  if (sessionId.empty()) {
    /*
     * Get session from request.
     */
    if (conf_.sessionTracking() == Configuration::CookiesURL
	&& !conf_.reloadIsNewSession())
      sessionId = sessionFromCookie(request->headerValue("Cookie"),
				    request->scriptName(),
				    conf_.sessionIdLength());

    if (sessionId.empty() && wtdE)
      sessionId = *wtdE;
  }

#ifdef WT_THREADED
  /*
   * -- Begin critical section to handle the session.
   */
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // WT_THREADED

  SessionMap::iterator i = sessions_.find(sessionId);
  WebSession *session = 0;

  if (i == sessions_.end() || i->second->done()) {
    try {
      if (singleSessionId_.empty()) {
	sessionId = conf_.generateSessionId();

	if (conf_.serverType() == Configuration::FcgiServer
	    && conf_.sessionPolicy() == Configuration::SharedProcess) {
	  std::string socketPath = conf_.sessionSocketPath(sessionId);
	  std::ofstream f(socketPath.c_str());
	  f << conf_.pid() << std::endl;
	  f.flush();
	}
      }

      std::string favicon = ep->favicon();
      if (favicon.empty()) {
	const std::string *confFavicon = conf_.property("favicon");
        if (confFavicon)
	  favicon = *confFavicon;
      }

      session = new WebSession(this, sessionId, ep->type(), favicon, request);

      if (configuration().sessionTracking() == Configuration::CookiesURL)
	request->addHeader("Set-Cookie",
			   appSessionCookie(request->scriptName())
			   + "=" + sessionId + "; Version=1;");

      sessions_[sessionId] = session;
    } catch (std::exception& e) {
      configuration().log("error")
	<< "Could not create new session: " << e.what();
      request->flush(WebResponse::ResponseDone);
      return;
    }
  } else {
    session = i->second;
  }

#ifdef WT_THREADED
  sessionsLock.unlock();
#endif // WT_THREADED

  session->handleRequest(*request, *(WebResponse *)request);

  if (!running_)
    expireSessions();

#ifdef WT_THREADED
#ifdef NOTHREADPOOL
  if (running_) {

    boost::thread self;
    boost::mutex::scoped_lock l(threadsMutex_);

    for (unsigned i = 0; i < threads_.size(); ++i) {
      if (*threads_[i] == self) {
	delete threads_[i];
	threads_.erase(threads_.begin() + i);
	break;
      }
    }
  }
#endif
#endif // WT_THREADED
}

WApplication *WebController::doCreateApplication(WebSession *session)
{
  const EntryPoint *ep = getEntryPoint(session->deploymentPath());
  return (*ep->appCallback())(session->env());
}

const EntryPoint *
WebController::getEntryPoint(const std::string& sName)
{
  // Only one default entry point.
  if (conf_.entryPoints().size() == 1
      && conf_.entryPoints()[0].path().empty())
    return &conf_.entryPoints()[0];

  // Multiple entry points.
  for (unsigned i = 0; i < conf_.entryPoints().size(); ++i) {
    const Wt::EntryPoint& ep = conf_.entryPoints()[i];

    if (sName==ep.path())
      return &ep;
  }

  conf_.log("error") << "No entry point configured for: '" << sName
		     << "', using first entry point ('"
		     << conf_.entryPoints()[0].path() << "'):";

  return &conf_.entryPoints()[0];
}

void WebController::addSocketNotifier(WSocketNotifier *notifier)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // WT_THREADED

  socketNotifiers_[notifier->socket()] = notifier;

  stream_->addSocketNotifier(notifier);
}

void WebController::removeSocketNotifier(WSocketNotifier *notifier)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // WT_THREADED

  socketNotifiers_.erase(socketNotifiers_.find(notifier->socket()));

  stream_->removeSocketNotifier(notifier);
}

std::string WebController::generateNewSessionId(WebSession *session)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // WT_THREADED  

  std::string newSessionId = conf_.generateSessionId();

  if (conf_.serverType() == Configuration::FcgiServer) {
    std::string oldSocketPath = conf_.sessionSocketPath(session->sessionId());
    std::string newSocketPath = conf_.sessionSocketPath(newSessionId);

    rename(oldSocketPath.c_str(), newSocketPath.c_str());
  }

  SessionMap::iterator i = sessions_.find(session->sessionId());
  sessions_.erase(i);
  sessions_[newSessionId] = session;

  if (!singleSessionId_.empty())
    singleSessionId_ = newSessionId;

  return newSessionId;
}

}
