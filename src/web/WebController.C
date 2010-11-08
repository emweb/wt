/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>
#include <sstream>

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
#include "Wt/WServer"
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

#ifdef HAVE_RASTER_IMAGE
#include <magick/api.h>
#endif

namespace Wt {

WebController::WebController(Configuration& configuration,
			     WAbstractServer *server, WebStream *stream,
			     std::string singleSessionId)
  : server_(server),
    conf_(configuration),
    stream_(stream),
    singleSessionId_(singleSessionId),
    running_(false),
    shutdown_(false)
#ifdef WT_THREADED
  , socketNotifier_(this),
    threadPool_(conf_.serverType() == Configuration::WtHttpdServer
		? 0 : conf_.numThreads())
#endif // WT_THREADED
{
  CgiParser::init();

#ifdef HAVE_RASTER_IMAGE
  InitializeMagick(0);
#endif
}

WebController::~WebController()
{
#ifdef HAVE_RASTER_IMAGE
  DestroyMagick();
#endif
}

void WebController::forceShutdown()
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  conf_.log("notice") << "Shutdown: stopping sessions.";

  shutdown_ = true;

  for (SessionMap::iterator i = sessions_.begin(); i != sessions_.end();) {
    boost::shared_ptr<WebSession> session = i->second;
    WebSession::Handler handler(session, true);
    session->expire();
    sessions_.erase(i++);
  }

  sessions_.clear();
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
    server_->handleRequest(request);
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
  std::vector<boost::shared_ptr<WebSession> > toKill;

  bool result;
  {
    Time now;

#ifdef WT_THREADED
    boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

    for (SessionMap::iterator i = sessions_.begin(); i != sessions_.end();) {
      boost::shared_ptr<WebSession> session = i->second;

      if (!session->dead()) {
	int diff = session->expireTime() - now;

	if (diff < 1000) {
	  if (session->shouldDisconnect()) {
	    if (session->app()->connected_) {
	      session->app()->connected_ = false;
	      session->log("notice") << "Timeout: disconnected";
	    }
	    ++i;
	  } else {
	    i->second->log("notice") << "Timeout: expiring";
	    WebSession::Handler handler(session, true);
	    session->expire();
	    toKill.push_back(session);
	    sessions_.erase(i++);
	  }
	} else
	  ++i;
      } else
	++i;
    }

    result = !sessions_.empty();
  }

  toKill.clear();

  return result;
}

void WebController::removeSession(const std::string& sessionId)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
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
    threadPool_.schedule
      (boost::bind(&WAbstractServer::handleRequest, server_, request));
  } else
    server_->handleRequest(request);
#else
    server_->handleRequest(request);
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

#ifdef WT_THREADED
WebController::SocketNotifierMap& 
WebController::socketNotifiers(WSocketNotifier::Type type)
{
  switch (type) {
  case WSocketNotifier::Read:
    return socketNotifiersRead_;
  case WSocketNotifier::Write:
    return socketNotifiersWrite_;
  case WSocketNotifier::Exception:
  default: // to avoid return warning
    return socketNotifiersExcept_;
  }
}
#endif // WT_THREADED

void WebController::socketSelected(int descriptor, WSocketNotifier::Type type)
{
#ifdef WT_THREADED
  /*
   * Find notifier, extract session Id
   */
  std::string sessionId;
  {
    boost::recursive_mutex::scoped_lock lock(notifierMutex_);

    SocketNotifierMap &notifiers = socketNotifiers(type);
    SocketNotifierMap::iterator k = notifiers.find(descriptor);

    if (k == notifiers.end()) {
      conf_.log("error") << "WebController::socketSelected(): socket notifier"
	" should have been cancelled?";

      return;
    } else {
      sessionId = k->second->sessionId();
    }
  }

  /*
   * Find session
   */
  boost::shared_ptr<WebSession> session;
  {
    boost::recursive_mutex::scoped_lock lock(mutex_);

    SessionMap::iterator i = sessions_.find(sessionId);

    if (i == sessions_.end() || i->second->dead()) {
      conf_.log("error")
	<< "WebController::socketSelected(): socket notification"
	" for expired session " << sessionId << ". Leaking memory?";

      return;
    } else {
      session = i->second;
    }
  }

  /*
   * Take session lock and notify
   */
  {
    WebSession::Handler handler(session, true);

    if (!session->dead()) {
      WSocketNotifier *notifier = 0;
      {
	boost::recursive_mutex::scoped_lock lock(notifierMutex_);
	SocketNotifierMap &notifiers = socketNotifiers(type);
	SocketNotifierMap::iterator k = notifiers.find(descriptor);	
	if (k != notifiers.end()) {
	  notifier = k->second;
	  notifiers.erase(k);
	}
      }

      session->app()->modifiedWithoutEvent_ = true;
      if (notifier)
	notifier->notify();
      session->app()->modifiedWithoutEvent_ = false;
    }
  }
#endif // WT_THREADED
}

void WebController::addSocketNotifier(WSocketNotifier *notifier)
{
#ifdef WT_THREADED
  {
    boost::recursive_mutex::scoped_lock lock(notifierMutex_);
    socketNotifiers(notifier->type())[notifier->socket()] = notifier;
  }

  switch (notifier->type()) {
  case WSocketNotifier::Read:
    socketNotifier_.addReadSocket(notifier->socket());
    break;
  case WSocketNotifier::Write:
    socketNotifier_.addWriteSocket(notifier->socket());
    break;
  case WSocketNotifier::Exception:
    socketNotifier_.addExceptSocket(notifier->socket());
    break;
  }
#endif // WT_THREADED
}

void WebController::removeSocketNotifier(WSocketNotifier *notifier)
{
#ifdef WT_THREADED
  switch (notifier->type()) {
  case WSocketNotifier::Read:
    socketNotifier_.removeReadSocket(notifier->socket());
    break;
  case WSocketNotifier::Write:
    socketNotifier_.removeWriteSocket(notifier->socket());
    break;
  case WSocketNotifier::Exception:
    socketNotifier_.removeExceptSocket(notifier->socket());
    break;
  }

  boost::recursive_mutex::scoped_lock lock(notifierMutex_);

  SocketNotifierMap &notifiers = socketNotifiers(notifier->type());
  SocketNotifierMap::iterator i = notifiers.find(notifier->socket());
  if (i != notifiers.end())
    notifiers.erase(i);
#endif // WT_THREADED
}

bool WebController::requestDataReceived(WebRequest *request,
					boost::uintmax_t current,
					boost::uintmax_t total)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(uploadProgressUrlsMutex_);
#endif // WT_THREADED

  if (uploadProgressUrls_.find(request->queryString())
      != uploadProgressUrls_.end()) {
#ifdef WT_THREADED
    lock.unlock();
#endif // WT_THREADED

    CgiParser cgi(conf_.maxRequestSize());

    try {
      cgi.parse(*request, false);
    } catch (std::exception& e) {
      conf_.log("error") << "Could not parse request: " << e.what();
      return false;
    }

    const std::string *wtdE = request->getParameter("wtd");
    if (!wtdE)
      return false;

    std::string sessionId = *wtdE;

    boost::shared_ptr<WebSession> session;
    {
#ifdef WT_THREADED
      boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

      SessionMap::iterator i = sessions_.find(sessionId);

      if (i == sessions_.end() || i->second->dead())
	return false;
      else
	session = i->second;
    }

    if (session) {
      WebSession::Handler handler(session, *request, *(WebResponse *)request);

      if (!session->dead() && session->app()) {
	const std::string *requestE = request->getParameter("request");

	WResource *resource = 0;
	if (!requestE && !request->pathInfo().empty())
	  resource
	    = session->app()->decodeExposedResource
	    ("/path/" + request->pathInfo());

	if (!resource) {
	  const std::string *resourceE = request->getParameter("resource");
	  resource = session->app()->decodeExposedResource(*resourceE);
	}

	if (resource) {
	  // FIXME, we should do this within app()->notify()
	  session->app()->modifiedWithoutEvent_ = true;
	  resource->dataReceived().emit(current, total);
	  session->app()->modifiedWithoutEvent_ = false;
	}
      }
    }

    return !request->postDataExceeded();
  }

  return true;
}

void WebController::addUploadProgressUrl(const std::string& url)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(uploadProgressUrlsMutex_);
#endif // WT_THREADED

  uploadProgressUrls_.insert(url.substr(url.find("?") + 1));
}

void WebController::removeUploadProgressUrl(const std::string& url)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(uploadProgressUrlsMutex_);
#endif // WT_THREADED

  uploadProgressUrls_.erase(url.substr(url.find("?") + 1));
}

void WebController::handleRequest(WebRequest *request)
{
  handleAsyncRequest(request);
}

void WebController::handleAsyncRequest(WebRequest *request)
{
  if (!request->entryPoint_)
    request->entryPoint_ = getEntryPoint(request);

  CgiParser cgi(conf_.maxRequestSize());

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

  if (request->entryPoint_->type() == StaticResource) {
    request->entryPoint_->resource()->handle(request, (WebResponse *)request);
    return;
  }

  std::string sessionId;

  /*
   * Get session from request.
   */
  const std::string *wtdE = request->getParameter("wtd");

  if (conf_.sessionTracking() == Configuration::CookiesURL
      && !conf_.reloadIsNewSession())
    sessionId = sessionFromCookie(request->headerValue("Cookie"),
				  request->scriptName(),
				  conf_.sessionIdLength());

  if (sessionId.empty() && wtdE)
    sessionId = *wtdE;

  boost::shared_ptr<WebSession> session;
  {
#ifdef WT_THREADED
    boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

    if (!singleSessionId_.empty() && sessionId != singleSessionId_) {
      if (conf_.persistentSessions()) {
	// This may be because of a race condition in the filesystem:
	// the session file is renamed in generateNewSessionId() but
	// still a request for an old session may have arrived here
	// while this was happening.
	//
	// If it is from the old app, We should be sent a reload signal,
	// this is what will be done by a new session (which does not create
	// an application).
	//
	// If it is another request to take over the persistent session,
	// it should be handled by the persistent session. We can distinguish
	// using the type of the request
	conf_.log("info") 
	  << "Persistent session requested Id: " << sessionId << ", "
	  << "persistent Id: " << singleSessionId_;
	if (sessions_.empty() || request->requestMethod() == "GET")
	  sessionId = singleSessionId_;
      } else
	sessionId = singleSessionId_;
    }

    SessionMap::iterator i = sessions_.find(sessionId);

    if (i == sessions_.end() || i->second->dead()) {
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

	std::string favicon = request->entryPoint_->favicon();
	if (favicon.empty()) {
	  const std::string *confFavicon = conf_.property("favicon");
	  if (confFavicon)
	    favicon = *confFavicon;
	}

	session.reset(new WebSession(this, sessionId,
				     request->entryPoint_->type(),
				     favicon, request));

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
  }

  bool handled = false;
  {
    WebSession::Handler handler(session, *request, *(WebResponse *)request);

    if (!session->dead()) {
      handled = true;
      session->handleRequest(handler);
    }
  }

  if (session->dead())
    removeSession(sessionId);

  session.reset();

  if (!running_)
    expireSessions();

#if defined(WT_THREADED) && defined(NOTHREADPOOL)
  if (running_) {

    boost::thread self;
    boost::mutex::scoped_lock lock(threadsMutex_);

    for (unsigned i = 0; i < threads_.size(); ++i) {
      if (*threads_[i] == self) {
	delete threads_[i];
	threads_.erase(threads_.begin() + i);
	break;
      }
    }
  }
#endif // WT_THREADED && NOTHREADPOOL

  if (!handled)
    handleAsyncRequest(request);
}

WApplication *WebController::doCreateApplication(WebSession *session)
{
  const EntryPoint *ep 
    = WebSession::Handler::instance()->request()->entryPoint_;

  return (*ep->appCallback())(session->env());
}

const EntryPoint *
WebController::getEntryPoint(WebRequest *request)
{
  std::string scriptName = request->scriptName();
  std::string pathInfo = request->pathInfo();

  // Only one default entry point.
  if (conf_.entryPoints().size() == 1
      && conf_.entryPoints()[0].path().empty())
    return &conf_.entryPoints()[0];

  // Multiple entry points.
  // This case probably only happens with built-in http
  for (unsigned i = 0; i < conf_.entryPoints().size(); ++i) {
    const Wt::EntryPoint& ep = conf_.entryPoints()[i];
    if (scriptName==ep.path())
      return &ep;
  }

  // Multiple entry points: also recognized when prefixed with
  // scriptName. For HTTP/ISAPI connectors, we only receive URLs
  // that are subdirs of the scriptname.
  for (unsigned i = 0; i < conf_.entryPoints().size(); ++i) {
    const Wt::EntryPoint& ep = conf_.entryPoints()[i];
    if (boost::starts_with(pathInfo, ep.path())) {
      if (pathInfo.length() > ep.path().length()) {
        char next = pathInfo[ep.path().length()];
        if (next == '/') {
          return &ep;
        }
      } else {
        return &ep;
      }
    }
  }

  conf_.log("error") << "No entry point configured for: '" << scriptName
		     << "', using first entry point ('"
		     << conf_.entryPoints()[0].path() << "'):";

  return &conf_.entryPoints()[0];
}

std::string
WebController::generateNewSessionId(boost::shared_ptr<WebSession> session)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED  

  std::string newSessionId = conf_.generateSessionId();

  if (conf_.serverType() == Configuration::FcgiServer) {
    std::string oldSocketPath = conf_.sessionSocketPath(session->sessionId());
    std::string newSocketPath = conf_.sessionSocketPath(newSessionId);

    rename(oldSocketPath.c_str(), newSocketPath.c_str());
  }

  sessions_[newSessionId] = session;

  SessionMap::iterator i = sessions_.find(session->sessionId());
  sessions_.erase(i);

  if (!singleSessionId_.empty())
    singleSessionId_ = newSessionId;

  return newSessionId;
}

}
