/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>

#include <boost/algorithm/string.hpp>

#ifdef WT_THREADED
#include <chrono>
#endif // WT_THREADED

#include "Wt/Utils.h"
#include "Wt/WApplication.h"
#include "Wt/WEvent.h"
#include "Wt/WRandom.h"
#include "Wt/WResource.h"
#include "Wt/WServer.h"
#include "Wt/WSocketNotifier.h"

#include "Configuration.h"
#include "CgiParser.h"
#include "WebController.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "TimeUtil.h"
#include "WebUtils.h"

#ifdef HAVE_GRAPHICSMAGICK
#include <magick/api.h>
#endif

#ifndef WT_HAVE_POSIX_FILEIO
// boost bug workaround: see WebController constructor
#include <boost/filesystem.hpp>
#endif

#include <algorithm>
#include <csignal>

namespace Wt {

LOGGER("WebController");

WebController::WebController(WServer& server,
			     const std::string& singleSessionId,
			     bool autoExpire)
  : conf_(server.configuration()),
    singleSessionId_(singleSessionId),
    autoExpire_(autoExpire),
    plainHtmlSessions_(0),
    ajaxSessions_(0),
    zombieSessions_(0),
#ifdef WT_THREADED
    socketNotifier_(this),
#endif // WT_THREADED
    server_(server)
{
  CgiParser::init();

#ifndef WT_DEBUG_JS
  WObject::seedId(WRandom::get());
#else
  WObject::seedId(0);
#endif

  redirectSecret_ = WRandom::generateId(32);

#ifdef HAVE_GRAPHICSMAGICK
  InitializeMagick(0);
#endif

#ifndef WT_HAVE_POSIX_FILEIO
  // attempted workaround for:
  // https://svn.boost.org/trac/boost/ticket/6320
  // https://svn.boost.org/trac/boost/ticket/4889
  // https://svn.boost.org/trac/boost/ticket/6690
  // https://svn.boost.org/trac/boost/ticket/6737
  // Invoking the path constructor here should create the global variables
  // in boost.filesystem before the threads are started.
  boost::filesystem::path bugFixFilePath("please-initialize-globals");
#endif

  start();
}

WebController::~WebController()
{
#ifdef HAVE_GRAPHICSMAGICK
  DestroyMagick();
#endif
}

void WebController::start()
{
  running_ = true;
}

void WebController::shutdown()
{
  {
    std::vector<std::shared_ptr<WebSession>> sessionList;

    {
#ifdef WT_THREADED
      std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED

      running_ = false;

      LOG_INFO_S(&server_, "shutdown: stopping " << sessions_.size()
		 << " sessions.");

      for (SessionMap::iterator i = sessions_.begin(); i != sessions_.end();
	   ++i)
	sessionList.push_back(i->second);

      sessions_.clear();

      ajaxSessions_ = 0;
      plainHtmlSessions_ = 0;
    }

    for (unsigned i = 0; i < sessionList.size(); ++i) {
      std::shared_ptr<WebSession> session = sessionList[i];
      WebSession::Handler handler(session, 
				  WebSession::Handler::LockOption::TakeLock);
      session->expire();
    }
  }

#ifdef WT_THREADED
  while (zombieSessions_ > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
#endif
}

void WebController::sessionDeleted()
{
#ifdef WT_THREADED
  std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED
  --zombieSessions_;
}

Configuration& WebController::configuration()
{
  return conf_;
}

int WebController::sessionCount() const
{
  return sessions_.size();
}

std::vector<std::string> WebController::sessions()
{
#ifdef WT_THREADED
  std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif
  std::vector<std::string> sessionIds;
  for (SessionMap::const_iterator i = sessions_.begin(); i != sessions_.end(); ++i) {
    sessionIds.push_back(i->first);
  }
  return sessionIds;
}

bool WebController::expireSessions()
{
  std::vector<std::shared_ptr<WebSession>> toExpire;

  bool result;
  {
    Time now;

#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED

    for (SessionMap::iterator i = sessions_.begin(); i != sessions_.end();) {
      std::shared_ptr<WebSession> session = i->second;

      int diff = session->expireTime() - now;

      if (diff < 1000 && configuration().sessionTimeout() != -1) {
	toExpire.push_back(session);

	if (session->env().ajax())
	  --ajaxSessions_;
	else
	  --plainHtmlSessions_;

	++zombieSessions_;
	sessions_.erase(i++);
      } else
	++i;
    }

    result = !sessions_.empty();
  }

  for (unsigned i = 0; i < toExpire.size(); ++i) {
    std::shared_ptr<WebSession> session = toExpire[i];

    LOG_INFO_S(session, "timeout: expiring");
    WebSession::Handler handler(session,
				WebSession::Handler::LockOption::TakeLock);
    session->expire();
  }

  return result;
}

void WebController::addSession(const std::shared_ptr<WebSession>& session)
{
#ifdef WT_THREADED
  std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED

  sessions_[session->sessionId()] = session;
}

void WebController::removeSession(const std::string& sessionId)
{
#ifdef WT_THREADED
  std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED

  LOG_INFO("Removing session " << sessionId);

  SessionMap::iterator i = sessions_.find(sessionId);
  if (i != sessions_.end()) {
    ++zombieSessions_;
    if (i->second->env().ajax())
      --ajaxSessions_;
    else
      --plainHtmlSessions_;
    sessions_.erase(i);
  }

  if (server_.dedicatedSessionProcess() && sessions_.size() == 0) {
    server_.scheduleStop();
  }
}

std::string WebController::appSessionCookie(const std::string& url)
{
  return Utils::urlEncode(url);
}

std::string WebController::sessionFromCookie(const char * const cookies,
					     const std::string& scriptName,
                                             const int sessionIdLength)
{
  if (!cookies)
    return std::string();

  std::string cookieName = appSessionCookie(scriptName);

  // is_whitespace returns whether a character is whitespace according to RFC 5234 WSP
  auto is_whitespace = [](char c) { return c == ' ' || c == '\t'; };
  auto is_alphanumeric = [](char c) { return (c >= 'A' && c <= 'Z') ||
                                             (c >= 'a' && c <= 'z') ||
                                             (c >= '0' && c <= '9'); };

  const char *start = cookies;
  const char * const end = cookies + strlen(cookies);
  start = std::find_if_not(start, end, is_whitespace); // Skip leading whitespace
  while (start < end) {
    const char *const nextEquals = std::find(start, end, '=');
    if (nextEquals == end)
      return std::string{}; // Cookie header has no equals anymore
    const char *const nextSemicolon = std::find(nextEquals+1, end, ';');
    if (nextSemicolon != end &&
        *(nextSemicolon + 1) != ' ')
      return std::string{}; // Malformed cookie header, no space after semicolon

    assert(nextEquals < nextSemicolon); // Should be guaranteed because nextSemicolon search starts at nextEquals+1
    assert(nextSemicolon <= end); // Should be guaranteed because nextSemicolon search ends at 'end'
    assert(start <= nextEquals); // Should be guaranteed because nextEquals search starts at start

    // othercookie=value; cookiename=cookievalue; lastCookie = value
    // ^- cookies         ^- start  ^           ^- nextSemicolon    ^- end
    //                              \- nextEquals
    // or (last cookie)
    // othercookie=value; cookiename=cookievalue
    // ^- cookies         ^- start  ^           ^- nextSemicolon = end
    //                              \- nextEquals

    if (std::distance(start, nextEquals) == (long)cookieName.size() &&
	std::equal(start, nextEquals, cookieName.c_str())) {
      const char * cookieValueStart = nextEquals+1;
      assert(cookieValueStart <= end); // Because of nextEquals == end check earlier
      // Leave out trailing whitespace
      const char * cookieValueEnd = nextSemicolon == end ? std::find_if(cookieValueStart, end, is_whitespace) : nextSemicolon;

      // Handle cookie value in double quotes
      if (*cookieValueStart == '"') {
        ++cookieValueStart;
        assert(cookieValueEnd - 1 >= cookies); // Should be guaranteed because cookieValueStart >= nextEquals + 1
        if (*(cookieValueEnd - 1) != '"')
          return std::string{}; // Malformed cookie header, unbalanced double quote
        --cookieValueEnd;
      }

      // cookiename=cookievalue;
      //            ^          ^- cookieValueEnd
      //            \- cookieValueStart
      // or (double quotes)
      // cookiename="cookievalue";
      //             ^          ^- cookieValueEnd
      //             \- cookieValueStart

      if (sessionIdLength != std::distance(cookieValueStart, cookieValueEnd))
        return std::string{}; // Session ID cookie length incorrect!
      if (!std::all_of(cookieValueStart, cookieValueEnd, is_alphanumeric))
        return std::string{}; // Session IDs should be alphanumeric!
      return std::string(cookieValueStart, sessionIdLength);
    }

    start = nextSemicolon + 2; // Skip over '; '
  }
  return std::string{};
}

#ifdef WT_THREADED
WebController::SocketNotifierMap&
WebController::socketNotifiers(WSocketNotifier::Type type)
{
  switch (type) {
  case WSocketNotifier::Type::Read:
    return socketNotifiersRead_;
  case WSocketNotifier::Type::Write:
    return socketNotifiersWrite_;
  case WSocketNotifier::Type::Exception:
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
    std::unique_lock<std::recursive_mutex> lock(notifierMutex_);

    SocketNotifierMap &notifiers = socketNotifiers(type);
    SocketNotifierMap::iterator k = notifiers.find(descriptor);

    if (k == notifiers.end()) {
      LOG_ERROR_S(&server_, "socketSelected(): socket notifier should have been "
		  "cancelled?");

      return;
    } else {
      sessionId = k->second->sessionId();
    }
  }

  server_.post(sessionId, std::bind(&WebController::socketNotify,
				      this, descriptor, type));
#endif // WT_THREADED
}

#ifdef WT_THREADED
void WebController::socketNotify(int descriptor, WSocketNotifier::Type type)
{
  WSocketNotifier *notifier = nullptr;
  {
    std::unique_lock<std::recursive_mutex> lock(notifierMutex_);
    SocketNotifierMap &notifiers = socketNotifiers(type);
    SocketNotifierMap::iterator k = notifiers.find(descriptor);	
    if (k != notifiers.end()) {
      notifier = k->second;
      notifiers.erase(k);
    }
  }

  if (notifier)
    notifier->notify();
}
#endif // WT_THREADED

void WebController::addSocketNotifier(WSocketNotifier *notifier)
{
#ifdef WT_THREADED
  {
    std::unique_lock<std::recursive_mutex> lock(notifierMutex_);
    socketNotifiers(notifier->type())[notifier->socket()] = notifier;
  }

  switch (notifier->type()) {
  case WSocketNotifier::Type::Read:
    socketNotifier_.addReadSocket(notifier->socket());
    break;
  case WSocketNotifier::Type::Write:
    socketNotifier_.addWriteSocket(notifier->socket());
    break;
  case WSocketNotifier::Type::Exception:
    socketNotifier_.addExceptSocket(notifier->socket());
    break;
  }
#endif // WT_THREADED
}

void WebController::removeSocketNotifier(WSocketNotifier *notifier)
{
#ifdef WT_THREADED
  switch (notifier->type()) {
  case WSocketNotifier::Type::Read:
    socketNotifier_.removeReadSocket(notifier->socket());
    break;
  case WSocketNotifier::Type::Write:
    socketNotifier_.removeWriteSocket(notifier->socket());
    break;
  case WSocketNotifier::Type::Exception:
    socketNotifier_.removeExceptSocket(notifier->socket());
    break;
  }

  std::unique_lock<std::recursive_mutex> lock(notifierMutex_);

  SocketNotifierMap &notifiers = socketNotifiers(notifier->type());
  SocketNotifierMap::iterator i = notifiers.find(notifier->socket());
  if (i != notifiers.end())
    notifiers.erase(i);
#endif // WT_THREADED
}

bool WebController::requestDataReceived(WebRequest *request,
					std::uintmax_t current,
					std::uintmax_t total)
{
#ifdef WT_THREADED
  std::unique_lock<std::mutex> lock(uploadProgressUrlsMutex_);
#endif // WT_THREADED

  if (!running_)
    return false;

  if (uploadProgressUrls_.find(request->queryString())
      != uploadProgressUrls_.end()) {
#ifdef WT_THREADED
    lock.unlock();
#endif // WT_THREADED

    CgiParser cgi(conf_.maxRequestSize(), conf_.maxFormDataSize());

    try {
      cgi.parse(*request, CgiParser::ReadHeadersOnly);
    } catch (std::exception& e) {
      LOG_ERROR_S(&server_, "could not parse request: " << e.what());
      return false;
    }

    const std::string *wtdE = request->getParameter("wtd");
    if (!wtdE)
      return false;

    std::string sessionId = *wtdE;

    ApplicationEvent event(sessionId,
			   std::bind(&WebController::updateResourceProgress,
				       this, request, current, total));

    if (handleApplicationEvent(event))
      return !request->postDataExceeded();
    else
      return false;
  }

  return true;
}

void WebController::updateResourceProgress(WebRequest *request,
					   std::uintmax_t current,
					   std::uintmax_t total)
{
  WApplication *app = WApplication::instance();

  const std::string *requestE = request->getParameter("request");

  WResource *resource = nullptr;
  if (!requestE && !request->pathInfo().empty())
    resource = app->decodeExposedResource("/path/" + request->pathInfo());

  if (!resource) {
    const std::string *resourceE = request->getParameter("resource");
    resource = app->decodeExposedResource(*resourceE);
  }

  if (resource) {
    ::int64_t dataExceeded = request->postDataExceeded();
    if (dataExceeded)
      resource->dataExceeded().emit(dataExceeded);
    else
      resource->dataReceived().emit(current, total);
  }
}

bool WebController::handleApplicationEvent(const ApplicationEvent& event)
{
  /*
   * This should always be run from within a virgin thread of the
   * thread-pool
   */
  assert(!WebSession::Handler::instance());

  /*
   * Find session (and guard it against deletion)
   */
  std::shared_ptr<WebSession> session;
  {
#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED

    SessionMap::iterator i = sessions_.find(event.sessionId);

    if (i != sessions_.end() && !i->second->dead())
      session = i->second;
  }

  if (!session) {
    if (event.fallbackFunction)
      event.fallbackFunction();
    return false;
  } else
    session->queueEvent(event);

  /*
   * Try to take the session lock now to propagate the event to the
   * application.
   */
  {
    WebSession::Handler handler(session, WebSession::Handler::LockOption::TryLock);
  }

  return true;
}

void WebController::addUploadProgressUrl(const std::string& url)
{
#ifdef WT_THREADED
  std::unique_lock<std::mutex> lock(uploadProgressUrlsMutex_);
#endif // WT_THREADED

  uploadProgressUrls_.insert(url.substr(url.find("?") + 1));
}

void WebController::removeUploadProgressUrl(const std::string& url)
{
#ifdef WT_THREADED
  std::unique_lock<std::mutex> lock(uploadProgressUrlsMutex_);
#endif // WT_THREADED

  uploadProgressUrls_.erase(url.substr(url.find("?") + 1));
}

std::string WebController::computeRedirectHash(const std::string& url)
{
  return Utils::base64Encode(Utils::md5(redirectSecret_ + url));
}

void WebController::handleRequest(WebRequest *request)
{
  if (!running_) {
    request->setStatus(500);
    request->flush();
    return;
  }

  if (!request->entryPoint_) {
    EntryPointMatch match = getEntryPoint(request);
    request->entryPoint_ = match.entryPoint;
    if (!request->entryPoint_) {
      request->setStatus(404);
      request->flush();
      return;
    }
    request->urlParams_ = std::move(match.urlParams);
  }

  CgiParser cgi(conf_.maxRequestSize(), conf_.maxFormDataSize());

  try {
    cgi.parse(*request, conf_.needReadBodyBeforeResponse()
	      ? CgiParser::ReadBodyAnyway
	      : CgiParser::ReadDefault);
  } catch (std::exception& e) {
    LOG_ERROR_S(&server_, "could not parse request: " << e.what());

    request->setContentType("text/html");
    request->out()
      << "<title>Error occurred.</title>"
      << "<h2>Error occurred.</h2>"
         "Error parsing CGI request: " << e.what() << std::endl;

    request->flush(WebResponse::ResponseState::ResponseDone);
    return;
  }

  if (request->entryPoint_->type() == EntryPointType::StaticResource) {
    request
      ->entryPoint_->resource()->handle(request, (WebResponse *)request);
    return;
  }

  const std::string *requestE = request->getParameter("request");
  if (requestE && *requestE == "redirect") {
    const std::string *urlE = request->getParameter("url");
    const std::string *hashE = request->getParameter("hash");

    if (urlE && hashE) {
      if (*hashE != computeRedirectHash(*urlE))
	hashE = nullptr;
    }

    if (urlE && hashE) {
      request->setRedirect(*urlE);
    } else {
      request->setContentType("text/html");
      request->out()
	<< "<title>Error occurred.</title>"
	<< "<h2>Error occurred.</h2><p>Invalid redirect.</p>" << std::endl;
    }

    request->flush(WebResponse::ResponseState::ResponseDone);
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
				  conf_.fullSessionIdLength());

  std::string multiSessionCookie;
  if (conf_.sessionTracking() == Configuration::Combined)
    multiSessionCookie = sessionFromCookie(request->headerValue("Cookie"),
                                           "ms" + request->scriptName(),
                                           conf_.sessionIdLength());

  if (sessionId.empty() && wtdE)
    sessionId = *wtdE;

  std::shared_ptr<WebSession> session;
  {
#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> lock(mutex_);
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
	LOG_INFO_S(&server_, 
		   "persistent session requested Id: " << sessionId << ", "
		   << "persistent Id: " << singleSessionId_);

	if (sessions_.empty() || strcmp(request->requestMethod(), "GET") == 0)
	  sessionId = singleSessionId_;
      } else
	sessionId = singleSessionId_;
    }

    SessionMap::iterator i = sessions_.find(sessionId);

    Configuration::SessionTracking sessionTracking = configuration().sessionTracking();

    if (i == sessions_.end() || i->second->dead() ||
        (sessionTracking == Configuration::Combined &&
	 (multiSessionCookie.empty() || multiSessionCookie != i->second->multiSessionId()))) {
      try {
        if (sessionTracking == Configuration::Combined &&
            i != sessions_.end() && !i->second->dead()) {
          if (!request->headerValue("Cookie")) {
            LOG_ERROR_S(&server_, "Valid session id: " << sessionId << ", but "
                        "no cookie received (expecting multi session cookie)");
            request->setStatus(403);
            request->flush(WebResponse::ResponseState::ResponseDone);
	    return;
	  }
	}

        if (request->isWebSocketRequest()) {
          LOG_INFO_S(&server_, "WebSocket request for non-existing session rejected. "
                               "This is likely because of a browser with an old session "
                               "trying to reconnect (e.g. when the server was restarted)");
          request->setStatus(403);
          request->flush(WebResponse::ResponseState::ResponseDone);
          return;
        }

	if (singleSessionId_.empty()) {
	  do {
	    sessionId = conf_.generateSessionId();
	    if (!conf_.registerSessionId(std::string(), sessionId))
	      sessionId.clear();
	  } while (sessionId.empty());
	}

	std::string favicon = request->entryPoint_->favicon();
	if (favicon.empty())
	  conf_.readConfigurationProperty("favicon", favicon);

	session.reset(new WebSession(this, sessionId,
				     request->entryPoint_->type(),
				     favicon, request));

        if (sessionTracking == Configuration::Combined) {
          if (multiSessionCookie.empty())
            multiSessionCookie = conf_.generateSessionId();
          session->setMultiSessionId(multiSessionCookie);
        }

        if (sessionTracking == Configuration::CookiesURL)
	  request->addHeader("Set-Cookie",
			     appSessionCookie(request->scriptName())
			     + "=" + sessionId + "; Version=1;"
			     + " Path=" + session->env().deploymentPath()
			     + "; httponly;" + (session->env().urlScheme() == "https" ? " secure;" : ""));

	sessions_[sessionId] = session;
	++plainHtmlSessions_;
      } catch (std::exception& e) {
	LOG_ERROR_S(&server_, "could not create new session: " << e.what());
	request->flush(WebResponse::ResponseState::ResponseDone);
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

  if (autoExpire_)
    expireSessions();

  if (!handled)
    handleRequest(request);
}

std::unique_ptr<WApplication> WebController
::doCreateApplication(WebSession *session)
{
  const EntryPoint *ep 
    = WebSession::Handler::instance()->request()->entryPoint_;

  return ep->appCallback()(session->env());
}

EntryPointMatch WebController::getEntryPoint(WebRequest *request)
{
  const std::string& scriptName = request->scriptName();
  const std::string& pathInfo = request->pathInfo();

  return conf_.matchEntryPoint(scriptName, pathInfo, false);
}

std::string
WebController::generateNewSessionId(const std::shared_ptr<WebSession>& session)
{
#ifdef WT_THREADED
  std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED  

  std::string newSessionId;
  do {
    newSessionId = conf_.generateSessionId();
    if (!conf_.registerSessionId(session->sessionId(), newSessionId))
      newSessionId.clear();
  } while (newSessionId.empty());

  sessions_[newSessionId] = session;

  SessionMap::iterator i = sessions_.find(session->sessionId());
  sessions_.erase(i);

  if (!singleSessionId_.empty())
    singleSessionId_ = newSessionId;

  return newSessionId;
}

void WebController::newAjaxSession()
{
#ifdef WT_THREADED
  std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED  

  --plainHtmlSessions_;
  ++ajaxSessions_;
}

bool WebController::limitPlainHtmlSessions()
{
  if (conf_.maxPlainSessionsRatio() > 0) {
#ifdef WT_THREADED
    std::unique_lock<std::recursive_mutex> lock(mutex_);
#endif // WT_THREADED

    if (plainHtmlSessions_ + ajaxSessions_ > 20)
      return plainHtmlSessions_ > conf_.maxPlainSessionsRatio()
	* (ajaxSessions_ + plainHtmlSessions_);
    else
      return false;
  } else
    return false;
}

}
