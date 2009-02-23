/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <sstream>
#include <mxml.h>

#ifdef HAVE_GNU_REGEX
#include <regex.h>
#else
#include <boost/regex.hpp>
#endif // HAVE_GNU_REGEX

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#ifdef THREADED
#include <boost/bind.hpp>
#endif // THREADED

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

class WEvent {
public:
  enum EventType { EmitSignal, Refresh, Render, HashChange,
		   PropagateFormValues
  };

  WebSession::Handler& handler;
  CgiParser&           cgi;
  EventType            type;

  /* For Render type */
  WebRenderer::ResponseType responseType;

  /* For HashChange type */
  std::string hash;

  WEvent(WebSession::Handler& aHandler, CgiParser& aCgi, EventType aType,
	 WebRenderer::ResponseType aResponseType = WebRenderer::FullResponse)
    : handler(aHandler),
      cgi(aCgi),
      type(aType),
      responseType(aResponseType)
  { }

  WEvent(WebSession::Handler& aHandler, CgiParser& aCgi, EventType aType,
	 const std::string& aHash)
    : handler(aHandler),
      cgi(aCgi),
      type(aType),
      responseType(WebRenderer::FullResponse),
      hash(aHash)
  { }
};

WebController *WebController::instance_ = 0;

WebController::WebController(Configuration& configuration,
			     WebStream& stream, std::string singleSessionId)
  : conf_(configuration),
    stream_(stream),
    singleSessionId_(singleSessionId),
    running_(false),
    shutdown_(false)
#ifdef THREADED
  , threadPool_(conf_.serverType() == Configuration::WtHttpdServer
		? 0 : conf_.numThreads())
#endif // THREADED
{
  instance_ = this;
  CgiParser::init();

  /*
   * mxml errors should not be fatal, and mxml is only used within the
   * scope of an application. Thus, simply log them.
   */
  mxmlSetErrorCallback(WebController::mxml_error_cb);
}

WebController::~WebController()
{
  instance_ = 0;
}

void WebController::mxml_error_cb(const char * message)
{
  WApplication *app = wApp;

  if (app)
    app->log("error") << "XML error: " << message;
  else if (WebController::instance())
    WebController::conf().log("error") << "XML error: " << message;
}

void WebController::forceShutdown()
{
#ifdef THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // THREADED

  conf_.log("notice") << "Shutdown: stopping sessions.";

  shutdown_ = true;

  while (!sessions_.empty()) {
    SessionMap::iterator i = sessions_.begin();
    WebSession::Handler handler(*i->second);
    handler.killSession();
    sessions_.erase(i);
  }
}

Configuration& WebController::conf()
{
  return instance_->conf_;
}

int WebController::sessionCount() const
{
  return sessions_.size();
}

void WebController::run()
{
  running_ = true;

  WebRequest *request = stream_.getNextRequest(10);

  if (request)
    handleRequest(request);
  else
    if (!singleSessionId_.empty()) {
      running_ = false;
      conf_.log("error") << "No initial request ?";
      return;
    }

  for (;;) {
    std::vector<WebSession *> sessionsToKill;
    bool haveMoreSessions = expireSessions(sessionsToKill);

    for (unsigned i = 0; i < sessionsToKill.size(); ++i) {
      WebSession::Handler handler(*sessionsToKill[i]);
      handler.killSession();
    }

    if (!haveMoreSessions && !singleSessionId_.empty()) {
      conf_.log("notice") << "Dedicated session process exiting cleanly.";
      break;
    }

    WebRequest *request = stream_.getNextRequest(120);

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

bool WebController::expireSessions(std::vector<WebSession *>& toKill)
{
  Time now;

#ifdef THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // THREADED

  std::vector<SessionMap::iterator> toErase;
  for (SessionMap::iterator i = sessions_.begin(); i != sessions_.end(); ++i) {
    int diff = i->second->expireTime() - now;

    if (diff < 1000) {
      i->second->log("notice") << "Timeout: expiring";
      toKill.push_back(i->second);
      toErase.push_back(i);
    }
  }

  for (unsigned i = 0; i < toErase.size(); ++i)
    sessions_.erase(toErase[i]);

  return !sessions_.empty();
}

void WebController::removeSession(WebSession *session)
{
#ifdef THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // THREADED

  SessionMap::iterator i = sessions_.find(session->sessionId());
  if (i != sessions_.end())
    sessions_.erase(i);
}

std::string WebController::appSessionCookie(std::string url)
{
  Wt::Utils::replace(url, '(', "#40");
  Wt::Utils::replace(url, ')', "#41");
  Wt::Utils::replace(url, '<', "#60");
  Wt::Utils::replace(url, '>', "#62");
  Wt::Utils::replace(url, '@', "#64");
  Wt::Utils::replace(url, ',', "#44");
  Wt::Utils::replace(url, ';', "#59");
  Wt::Utils::replace(url, '\\', "#92");
  Wt::Utils::replace(url, '"', "#34");
  Wt::Utils::replace(url, '/', "#47");
  Wt::Utils::replace(url, '[', "#91");
  Wt::Utils::replace(url, ']', "#93");
  Wt::Utils::replace(url, '?', "#63");
  Wt::Utils::replace(url, '=', "#61");
  Wt::Utils::replace(url, '{', "#123");
  Wt::Utils::replace(url, '}', "#125");
  Wt::Utils::replace(url, ' ', "#32");

  return url;
}

void WebController::handleRequestThreaded(WebRequest *request)
{
#ifdef THREADED
  if (stream_.multiThreaded()) {
    threadPool_.schedule(boost::bind(&WebController::handleRequest,
				     this, request, (const EntryPoint *)0));
  } else
    handleRequest(request);
#else
  handleRequest(request);
#endif // THREADED  
}

std::string WebController::sessionFromCookie(std::string cookies,
					     std::string scriptName,
					     int sessionIdLength)
{
  std::string cookieName = appSessionCookie(scriptName);

#ifndef HAVE_GNU_REGEX
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
#ifdef THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // THREADED

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
#ifdef THREADED
    WApplication *app = session->app();

    /*
     * Is correct, but now we are holding the sessionsLock too long: we
     * should in principle make sure the session will not be deleted (like
     * the handler does), and then release the sessionsLock before trying to
     * access the session exclusively.
     */
    WApplication::UpdateLock l = app->getUpdateLock();
    sessionsLock.unlock();
#endif // THREADED

    notifier->notify();
  }

#ifdef THREADED
  sessionsLock.lock();
#endif // THREADED

  if (socketNotifiers_.find(descriptor) == socketNotifiers_.end())
    return true;
  else
    return false;
}

void WebController::handleRequest(WebRequest *request, const EntryPoint *ep)
{
  std::vector<WebSession *> sessionsToKill;

  if (!running_)
    expireSessions(sessionsToKill);

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

    request->flush();
    return;
  }

  std::string sessionId = singleSessionId_;
  CgiEntry *wtdEntry = cgi.getEntry("wtd");

  if (sessionId.empty()) {
    /*
     * Get session from request.
     */
    if (conf_.sessionTracking() == Configuration::CookiesURL
	&& !conf_.reloadIsNewSession())
      sessionId = sessionFromCookie(request->headerValue("Cookie"),
				    request->scriptName(),
				    conf_.sessionIdLength());

    if (sessionId.empty())
      if (wtdEntry)
	sessionId = wtdEntry->value();
  }

#ifdef THREADED
  /*
   * -- Begin critical section to handle the session.
   */
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // THREADED

  SessionMap::iterator i = sessions_.find(sessionId);
  WebSession *session = 0;

  if (i == sessions_.end() || i->second->done()) {
    try {
      if (singleSessionId_.empty())
	sessionId = conf_.generateSessionId();

      if (!ep)
	ep = getEntryPoint(request);

      session = new WebSession(sessionId,
			       conf_.runDirectory() + "/" + sessionId,
			       ep->type(), *request);

      sessions_[sessionId] = session;
    } catch (std::exception& e) {
      conf().log("error") << "Could not create new session: " << e.what();
      request->flush();
      return;
    }
  } else {
    session = i->second;
  }

  {

  WebSession::Handler handler(*session, request);

#ifdef THREADED
  /*
   * -- End critical section to handle the session.
   */
  sessionsLock.unlock();

  /*
   * -- Start critical section exclusive access to session
   */
  boost::mutex::scoped_lock sessionLock(session->mutex);
  handler.setLock(&sessionLock);
#endif // THREADED

  WebRenderer::ResponseType type = WebRenderer::FullResponse;

  CgiEntry *signalE = getSignal(cgi, "e0");
  CgiEntry *resourceE = cgi.getEntry("resource");

  /*
   * Under what circumstances do we allow a request which does not have
   * a the session ID (except for through a cookie?)
   *
   *  - when a new session is created
   *  - when reloading the page (we document in the API that you should not
   *    run business logic when doing that)
   *
   * in other cases: silenty discard the request
   */
  if ((!wtdEntry || (wtdEntry->value() != session->sessionId()))
      && session->state() != WebSession::JustCreated
      && (signalE || resourceE)) {
    request->setContentType("text/html");
    request->out() << "<html><head></head><body>CSRF prevention</body></html>";
  } else try {
    CgiEntry *requestE = cgi.getEntry("request");

    switch (session->state()) {
    case WebSession::JustCreated: {
      if (!ep)
	ep = getEntryPoint(request);

      if (conf_.sessionTracking() == Configuration::CookiesURL) {
	handler.request()
	  ->addHeader("Set-Cookie",
		      appSessionCookie(handler.request()->scriptName())
		      + "=" + sessionId + "; Version=1;");
      }

      switch (session->type()) {
      case WebSession::Application:
	session->init(cgi, *handler.request()); // env, url/internalpath

	// Handle requests from dead sessions:
	// 
	// We need to send JS to reload the page when we get:
	// - 'signal' and no 'request' arg: is an AJAX call.
	//   (with a 'request' arg it is from a non-AJAX POST)
	// - 'request' == "script": asks for initial page script.
	//
	// We simply ignore:
	// - 'resource' is defined: is a resource request
	//
	// In those cases, we send JS that reloads their entire page.

	if ((signalE && !requestE)
	    || (requestE && requestE->value() == "script")) {
	  session->log("notice") << "Signal from dead session, sending reload.";

	  CgiEntry *historyE = cgi.getEntry("historyKey");
          if (historyE)
	    session->env().setInternalPath('/' + historyE->value());

	  session->renderer().letReloadJS(*handler.request(), true);
	  handler.killSession();
	} else if (cgi.getEntry("resource")) {
	  session->log("notice") << "Not serving bootstrap for resource.";
	  handler.killSession();
	  request->setContentType("text/html");
	  request->out() << "<html><head></head><body></body></html>";
	} else {
	  if (session->env().agentIsSpiderBot()) {
	    session->env().doesJavaScript_ = false;
	    session->env().doesAjax_ = false;
	    session->env().doesCookies_ = false;

	    if (!session->start(ep->appCallback()))
	      throw WtException("Could not start application.");

	    if (session->env().internalPath() != "/") {
	      session->app()->setInternalPath("/");
	      session->app()->notify(WEvent(handler, cgi, WEvent::HashChange,
					    session->env().internalPath()));
	    }

	    session->app()->notify(WEvent(handler, cgi, WEvent::Render,
					  WebRenderer::FullResponse));

	    handler.killSession();
	  } else {
	    session->setState(WebSession::Bootstrap, 10);
	    session->renderer().serveBootstrap(*handler.request());
	  }
	}
	break;
      case WebSession::WidgetSet:
	session->init(cgi, *handler.request()); // env, url/internalpath
	session->env().doesJavaScript_ = true;
	session->env().doesAjax_ = true;

	if (!session->start(ep->appCallback()))
	  throw WtException("Could not start application.");

	session->app()->notify(WEvent(handler, cgi, WEvent::Render,
				      WebRenderer::FullResponse));
      }
      break;
    }
    case WebSession::Bootstrap: {
      if (!ep)
	ep = getEntryPoint(request);

      CgiEntry *jsE = cgi.getEntry("js");

      // How could this happen?
      if (!jsE) {
	session->setState(WebSession::Bootstrap, 10);
	session->renderer().serveBootstrap(*handler.request());
	break;
      }

      /*
       * The bootstrap page is the entire page in AJAX. When reloading the
       * full page, we therefore end up here again. But we should not restart
       * the application ofcourse...
       */
      if (!session->app()) {
	CgiEntry *ajaxE = cgi.getEntry("ajax");
	CgiEntry *hashE = cgi.getEntry("_");
	CgiEntry *scaleE = cgi.getEntry("scale");

	session->env().doesJavaScript_= jsE->value() == "yes";
	session->env().doesAjax_ = session->env().doesJavaScript_
	  && ajaxE && ajaxE->value() == "yes";
	session->env().doesCookies_
	  = !handler.request()->headerValue("Cookie").empty();

	if (session->env().doesAjax_ && !request->pathInfo().empty()) {
	  std::string url = session->baseUrl() + session->applicationName();
	  url += '#' + session->env().internalPath();

	  session->redirect(url);
	  session->renderer().serveMainWidget(*handler.request(),
					      WebRenderer::FullResponse);

	  session->log("notice") << "Redirecting to canonical URL: " << url;
	  handler.killSession();
	  break;
	}

	try {
	  session->env().dpiScale_ =
	    scaleE ? boost::lexical_cast<double>(scaleE->value()) : 1;
	} catch (boost::bad_lexical_cast &) {
	  session->env().dpiScale_ = 1;
	}

	// the internal path, when present as an anchor (#), is only
	// conveyed in the second request
	if (hashE)
	  session->env().setInternalPath(hashE->value());

	if (!session->start(ep->appCallback()))
	  throw WtException("Could not start application.");

	if (session->env().internalPath() != "/") {
	  session->app()->setInternalPath("/");
	  session->app()->notify(WEvent(handler, cgi, WEvent::HashChange,
					session->env().internalPath()));
	}
      } else if ((jsE->value() == "no") && session->env().doesAjax_) {
	// reload but disabled AJAX support: give user a new session
	// FIX this: redirect using Redirect result.

	request->setRedirect(session->baseUrl() + session->applicationName()
			     + '#' + session->env().internalPath());
	handler.killSession();
	break;
      } else
	if (!handler.request()->pathInfo().empty())
	  session->app()->notify(WEvent(handler, cgi, WEvent::HashChange,
					handler.request()->pathInfo()));

      session->app()->notify(WEvent(handler, cgi, WEvent::Render,
				    WebRenderer::FullResponse));
      break;
    }
    case WebSession::Loaded: {
      type = (signalE && !requestE && session->env().doesAjax_)
	? WebRenderer::UpdateResponse
	: WebRenderer::FullResponse;

      if (requestE && requestE->value() == "script") {
	if (session->env().agentGecko()) {
	  // an unexpected second request for the "script": do not send
	  // anything for firefox since that appears to be a bug triggered
	  // by loading ext...
	  handler.request()->setContentType("text/plain");
	} else {
	  // IE for example does this when returning to the page.
	  // we reserve the script.
	  session->renderer().serveMainWidget(*handler.request(),
					      WebRenderer::FullResponse);
	}
	break;
      }

      if (!resourceE && !signalE) {
	session->log("notice") << "Refreshing session";
	session->app()->notify(WEvent(handler, cgi, WEvent::Refresh));
	if (session->env().doesAjax_) {
	  session->setState(WebSession::Bootstrap, 10);
	  session->renderer().serveBootstrap(*handler.request());
	  break;
	}
      }

      CgiEntry *updateIdE = cgi.getEntry("updateId");
      try {
	if (updateIdE)
	  request->setId(boost::lexical_cast<int>(updateIdE->value()));
      } catch (boost::bad_lexical_cast) {
	session->log("error") << "Could not parse updateId: "
			      << updateIdE->value();
      }

      session->env().urlScheme_ = request->urlScheme();

      if (resourceE && resourceE->value()=="blank") {
	handler.request()->setContentType("text/html");
	handler.request()->out()
	  << "<html><head><title>"
	  << session->app()->title()
	  << "</title></head><body>&#160;</body></html>";
      } else if (resourceE) {
	WResource *resource = session->decodeResource(resourceE->value());

	if (resource) {
	  CgiEntry *dataE = cgi.getEntry("data");

	  if (dataE) {
	    resource->setFormData(dataE);
	  }
#ifdef THREADED
	  if (resource->reentrant()) {
	    sessionLock.unlock();
	  }
#endif // THREADED

	  try {
	    WResource::ArgumentMap arguments;
	    for (CgiParser::EntryMap::const_iterator i = cgi.entries().begin();
		 i != cgi.entries().end(); ++i) {
	      WResource::ArgumentValues v;

	      CgiEntry *e = i->second;

	      while (e) {
		v.push_back(e->value());
		e = e->next();
	      }

	      arguments[i->first] = v;
	    }

	    resource->setRequest(handler.request());
	    resource->setArguments(arguments);
	    handler.request()->setContentType(resource->resourceMimeType());
	    bool done = resource->streamResourceData(handler.request()->out(),
						     arguments);
	    handler.request()->setKeepConnectionOpen(!done);

	    if (done)
	      resource->setRequest(0);

	  } catch (std::exception& e) {
	    throw WtException("Exception while streaming resource", e);
	  } catch (...) {
	    throw WtException("Exception while streaming resource");
	  }

	  if (dataE) {
	    try {
	      if (cgi.postDataExceeded()) {
		session->log("error") << "post data exceeded";
		session->app()->requestTooLarge(cgi.postDataExceeded());
		resource->requestTooLarge(cgi.postDataExceeded());
	      } else
		resource->formDataSet();
	    } catch (std::exception& e) {
	      throw WtException("Exception while setting resource data", e);
	    } catch (...) {
	      throw WtException("Exception while setting resource data");
	    }
	  }

	} else {
	  handler.request()->setContentType("text/html");
	  handler.request()->out()
	    << "<html><body><h1>Session timeout.</h1></body></html>";
	}
      } else {
	if (type == WebRenderer::FullResponse
	    && !handler.request()->pathInfo().empty())
	  session->app()->notify(WEvent(handler, cgi, WEvent::HashChange,
					handler.request()->pathInfo()));

	CgiEntry *hashE = cgi.getEntry("_");

	if (signalE) {
	  if (signalE->value() != "res") {
	    //std::cerr << "signal: " << signalE->value() << std::endl;
	    /*
	     * Special signal values:
	     * 'none' : no event, but perhaps a synchronization
	     * 'load' : load invisible content
	     * 'res'  : after a resource received data
	     */

	    // First propagate form values -- they could be corrupted by
	    // the hash change
	    session->app()->notify
	      (WEvent(handler, cgi, WEvent::PropagateFormValues));

	    // Propagate change in hash. Do it after a phony reload
	    // event, since the reload does not carry actual hash
	    // information
	    if (hashE)
	      session->app()->notify
		(WEvent(handler, cgi, WEvent::HashChange, hashE->value()));

	    try {
	      session->app()->notify(WEvent(handler, cgi, WEvent::EmitSignal));
	    } catch (std::exception& e) {
	      throw WtException("Error during event handling", e);
	    } catch (...) {
	      throw WtException("Error during event handling");	    
	    }
	  }
	} else {
	  if (hashE)
	    session->app()->notify
	      (WEvent(handler, cgi, WEvent::HashChange, hashE->value()));

	  /*
	   * Is a reload
	   */
	  try {
	    session->app()->notify(WEvent(handler, cgi, WEvent::Refresh));
	  } catch (std::exception& e) {
	    throw WtException("Exception while refreshing session", e);
	  } catch (...) {
	    throw WtException("Exception while refreshing session");
	  }
	}

	if (handler.request())
	  session->app()->notify(WEvent(handler, cgi, WEvent::Render, type));
      }
    }
      break;
    case WebSession::Dead:
      throw WtException("Internal error: WebSession is dead?");
    }
  } catch (WtException& e) {
    session->log("fatal") << e.what();

    handler.killSession();
    session->renderer().serveError(*handler.request(), e, type);

  } catch (std::exception& e) {
    session->log("fatal") << e.what();

    handler.killSession();
    session->renderer().serveError(*handler.request(), e, type);
  } catch (...) {
    session->log("fatal") << "Unknown exception.";

    handler.killSession();
    session->renderer().serveError(*handler.request(),
				   std::string("Unknown exception"), type);
  }

  if (handler.sessionDead()) {
    removeSession(session);
  }

  if (handler.request())
    handler.request()->flush();
  }

  for (unsigned i = 0; i < sessionsToKill.size(); ++i) {
    WebSession::Handler handler(*sessionsToKill[i]);
    handler.killSession();
  }


#ifdef THREADED
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
#endif // THREADED
}

CgiEntry *WebController::getSignal(const CgiParser& cgi, const std::string& se)
{
  CgiEntry *signalE = cgi.getEntry(se + "signal");

  if (!signalE) {
    const CgiParser::EntryMap& entries = cgi.entries();

    for (CgiParser::EntryMap::const_iterator i = entries.begin();
	 i != entries.end(); ++i) {
      if (i->first.find(se + "signal=") == 0) {
	signalE = i->second;

	std::string v = i->first.substr(7 + se.length());
	if (v.length() >= 2) {
	  std::string e = v.substr(v.length() - 2);
	  if (e == ".x" || e == ".y")
	    v = v.substr(0, v.length() - 2);
	}

	signalE->setValue(v);
	break;
      }
    }
  }

  return signalE;
}

const EntryPoint *WebController::getEntryPoint(WebRequest *request)
{
  // Only one default entry point.
  if (conf_.entryPoints().size() == 1
      && conf_.entryPoints()[0].path().empty())
    return &conf_.entryPoints()[0];

  // Multiple entry points.
  std::string sName = request->scriptName();

  for (unsigned i = 0; i < conf_.entryPoints().size(); ++i) {
    const Wt::EntryPoint& ep = conf_.entryPoints()[i];

    if (boost::ends_with(sName, ep.path()))
      return &ep;
  }

  conf_.log("error") << "No entry point configured for: '" << sName
		     << "', using first entry point ('"
		     << conf_.entryPoints()[0].path() << "'):";

  return &conf_.entryPoints()[0];
}

void WebController::notify(const WEvent& e)
{
  switch (e.type) {
  case WEvent::EmitSignal:
    notifySignal(e);
    break;
  case WEvent::Refresh:
    e.handler.session()->app()->refresh();
    break;
  case WEvent::Render:
    render(e.handler, &e.cgi, e.responseType);
    break;
  case WEvent::HashChange:
    e.handler.session()->app()->changeInternalPath(e.hash);
    break;
  case WEvent::PropagateFormValues:
    propagateFormValues(e);
    break;
  }
}

void WebController::render(WebSession::Handler& handler, CgiParser *cgi,
			   WebRenderer::ResponseType type)
{
  WebSession *session = handler.session();

  try {
    if (!session->env().doesJavaScript_ && (type == WebRenderer::FullResponse))
      session->checkTimers();
  } catch (std::exception& e) {
    throw WtException("Exception while triggering timers", e);
  } catch (...) {
    throw WtException("Exception while triggering timers");
  }

  try {
    if (cgi && cgi->postDataExceeded())
      session->app()->requestTooLarge(cgi->postDataExceeded());
  } catch (std::exception& e) {
    throw WtException("Exception in WApplication::requestTooLarge", e);
  } catch (...) {
    throw WtException("Exception in WApplication::requestTooLarge");
  }

  if (session->app()->isQuited())
    handler.killSession();

  session->renderer().serveMainWidget(*handler.request(), type);
  session->setState(WebSession::Loaded, conf_.sessionTimeout());
}

void WebController::propagateFormValues(const WEvent& e)
{
  WebSession::Handler& handler = e.handler;
  CgiParser&           cgi = e.cgi;

  WebSession *session = handler.session();

  std::vector<WObject *> formObjects = session->renderer().formObjects();

  // not for signal = 'res' (see WWidget)
  for (unsigned i = 0; i < formObjects.size(); ++i) {
    WObject *obj = formObjects[i];
    std::string objname = obj->formName();

    CgiEntry *entry = cgi.getEntry(objname);
    if (entry) {
      if (!cgi.postDataExceeded()) {
	obj->setFormData(entry);
	obj->formDataSet();
      } else
	obj->requestTooLarge(cgi.postDataExceeded());
    } else {
      if (!cgi.postDataExceeded())
	obj->setNoFormData();
    }
  }
}

void WebController::notifySignal(const WEvent& e)
{
  WebSession::Handler& handler = e.handler;
  CgiParser&           cgi = e.cgi;

  WebSession *session = handler.session();

  for (unsigned i = 0;; ++i) {
    std::string se = 'e' + boost::lexical_cast<std::string>(i);
    CgiEntry *signalE = getSignal(cgi, se);

    if (!signalE)
      return;

    if (signalE->value() == "hash") {
      CgiEntry *hashE = cgi.getEntry("_");
      if (hashE)
	session->app()->changeInternalPath(hashE->value());
    } else if (signalE->value() == "none") {
      // not idle timeout timer
      if (session->app()->updatesEnabled())
	session->app()->triggerUpdate();
    } else {
      if (signalE->value() != "load" && signalE->value() != "res") {
	handler.setEventLoop(true);

	// Save pending changes (e.g. from resource completion)
	session->renderer().saveChanges();

	for (unsigned k = 0; k < 3; ++k) {
	  SignalKind kind = static_cast<SignalKind>(k);

	  if (kind == AutoLearnStateless && cgi.postDataExceeded())
	    break;

	  if (signalE->value() == "user") {
	    CgiEntry *idE = cgi.getEntry(se + "id");
	    CgiEntry *nameE = cgi.getEntry(se + "name");

	    if (!idE || !nameE)
	      break;

	    processSignal(session->decodeSignal(idE->value(), nameE->value()),
			  se, cgi, session, kind);
	  } else
	    processSignal(session->decodeSignal(signalE->value()),
			  se, cgi, session, kind);

	  if (kind == LearnedStateless)
	    session->renderer().discardChanges();
	}
      }
    }
  }
}

void WebController::processSignal(EventSignalBase *s, const std::string& se,
				  CgiParser& cgi, WebSession *session,
				  SignalKind kind)
{
  if (!s)
    return;

  switch (kind) {
  case LearnedStateless:
    s->processLearnedStateless();    
    break;
  case AutoLearnStateless:
    s->processAutoLearnStateless(&session->renderer());
    break;
  case Dynamic:
    JavaScriptEvent jsEvent;
    jsEvent.get(cgi, se);
    s->processDynamic(jsEvent);

    // ! handler.request() may be 0 now, if there was a
    // ! recursive call.
    // ! what with other slots triggered after the one that
    // ! did the recursive call ? That's very bad ??
  }
}

void WebController::addSocketNotifier(WSocketNotifier *notifier)
{
#ifdef THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // THREADED

  socketNotifiers_[notifier->socket()] = notifier;

  stream_.addSocketNotifier(notifier);
}

void WebController::removeSocketNotifier(WSocketNotifier *notifier)
{
#ifdef THREADED
  boost::recursive_mutex::scoped_lock sessionsLock(mutex_);
#endif // THREADED

  socketNotifiers_.erase(socketNotifiers_.find(notifier->socket()));

  stream_.removeSocketNotifier(notifier);
}
 
}
