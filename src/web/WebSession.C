/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WFormWidget"
#include "Wt/WResource"
#include "Wt/WTimerWidget"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "WebSession.h"
#include "WebRequest.h"
#include "WebController.h"
#include "Configuration.h"
#include "WtException.h"
#include "DomElement.h"
#include "Utils.h"

#include <boost/algorithm/string.hpp>

#ifdef WT_TARGET_JAVA
#define RETHROW(e) throw e
#else
#define RETHROW(e) throw
#endif

namespace Wt {

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
boost::thread_specific_ptr<WebSession::Handler> WebSession::threadHandler_;
#else
WebSession::Handler * WebSession::threadHandler_;
#endif

WebSession::WebSession(WebController *controller,
		       const std::string& sessionId,
		       EntryPointType type,
		       const std::string& favicon,
                       const WebRequest *request,
		       WEnvironment *env)
  : type_(type),
    favicon_(favicon),
    state_(JustCreated),
    sessionId_(sessionId),
    controller_(controller),
    renderer_(*this),
    pollResponse_(0),
    updatesPending_(false),
    embeddedEnv_(this),
    app_(0),
    debug_(controller_->configuration().debug()),
    recursiveEventLoop_(0)
{
  env_ = env ? env : &embeddedEnv_;

  /*
   * Obtain the applicationName_ as soon as possible for log().
   */
  if (request)
    deploymentPath_ = request->scriptName();
  else
    deploymentPath_ = "/";

  applicationUrl_ = deploymentPath_;

  applicationName_ = applicationUrl_;
  baseUrl_ = applicationUrl_;

  std::string::size_type slashpos = applicationName_.rfind('/');
  if (slashpos != std::string::npos) {
    applicationName_ = applicationName_.substr(slashpos + 1);
    baseUrl_ = baseUrl_.substr(0, slashpos+1);
  }

#ifndef WT_TARGET_JAVA
  log("notice") << "Session created (#sessions = "
		<< (controller_->sessionCount() + 1)
		<< ")";

  expire_ = Time() + 60*1000;
#else  // WT_TARGET_JAVA
  log("notice") << "Session created";
#endif // WT_TARGET_JAVA
}

void WebSession::setApplication(WApplication *app)
{
  app_ = app;
}

WLogEntry WebSession::log(const std::string& type)
{
  Configuration& conf = controller_->configuration();
  WLogEntry e = conf.logger().entry();

#ifndef WT_TARGET_JAVA
  e << WLogger::timestamp << WLogger::sep
    << conf.pid() << WLogger::sep
    << '[' << baseUrl_ << applicationName_ << ' ' << sessionId()
    << ']' << WLogger::sep
    << '[' << type << ']' << WLogger::sep;
#endif // WT_TARGET_JAVA

  return e;
}

WebSession::~WebSession()
{
#ifndef WT_TARGET_JAVA
  if (app_)
    app_->finalize();
  delete app_;
#endif // WT_TARGET_JAVA

  if (pollResponse_)
    pollResponse_->flush();

#ifndef WT_TARGET_JAVA
  unlink(controller_->configuration().sessionSocketPath(sessionId_).c_str());

  log("notice") << "Session destroyed (#sessions = "
		<< controller_->sessionCount() << ")";
#else // WT_TARGET_JAVA
  log("notice") << "Session destroyed";
#endif // WT_TARGET_JAVA

}

std::string WebSession::docType() const
{
  const bool xhtml = env_->contentType() == WEnvironment::XHTML1;

  if (xhtml)
    /*
     * This would be what we want, but it is too strict (does not
     * validate iframe's and target attribute for links):
     
     "\"-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN\" "
     "\"http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd\">"
     * so instead we use transitional xhtml -- it will fail to
     * validate properly when we have svg !
     */ 
    return "<!DOCTYPE html PUBLIC "
      "\"-//W3C//DTD XHTML 1.0 Transitional//EN\" "
      "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">";
  else
    return "<!DOCTYPE html PUBLIC "
      "\"-//W3C//DTD HTML 4.01 Transitional//EN\" "
      "\"http://www.w3.org/TR/html4/loose.dtd\">";
}

#ifndef WT_TARGET_JAVA
bool WebSession::shouldDisconnect() const
{
  return state_ == Loaded && controller_->configuration().persistentSessions();
}
#endif // WT_TARGET_JAVA

void WebSession::setLoaded()
{
  setState(Loaded, controller_->configuration().sessionTimeout());
}

void WebSession::setState(State state, int timeout)
{
#ifdef WT_THREADED
  // this assertion is not true for when we are working from an attached
  // thread: that thread does not have an associated handler, but its contract
  // dictates that it should work on behalf of a thread that has the lock.
  //assert(WebSession::Handler::instance()->lock().owns_lock());
#endif // WT_THREADED

  if (state_ != Dead) {
    state_ = state;

#ifndef WT_TARGET_JAVA
    expire_ = Time() + timeout*1000;
#endif // WT_TARGET_JAVA
  }
}

std::string WebSession::sessionQuery() const
{
  // the session query is used for all requests (except for reload if the
  // session is not in the main url), to prevent CSRF.

  return "?wtd=" + sessionId_;
}

void WebSession::init(const WebRequest& request)
{
  env_->init(request);

  const std::string *hashE = request.getParameter("_");

  absoluteBaseUrl_ = env_->urlScheme() + "://" + env_->hostName() + baseUrl_;

  bookmarkUrl_ = applicationName_;

  if (applicationName_.empty())
    bookmarkUrl_ = applicationUrl_;

  if (type() == WidgetSet) {
    /*
     * We are embedded in another website: we only use absolute URLs.
     */
    applicationUrl_ = env_->urlScheme() + "://" + env_->hostName()
      + applicationUrl_;

    bookmarkUrl_ = applicationUrl_;
  }

  std::string path = request.pathInfo();
  if (path.empty() && hashE)
    path = *hashE;
  env_->setInternalPath(path);
}

std::string WebSession::bootstrapUrl(const WebResponse& response,
				     BootstrapOption option) const
{
  switch (option) {
  case KeepInternalPath: {
    std::string url, internalPath;

    if (applicationName_.empty()) {
      internalPath = app_ ? app_->internalPath() : env_->internalPath();

      if (internalPath.length() > 1)
	url = "?_=" + Utils::urlEncode(internalPath);

      if (type() == WidgetSet)
	url = applicationUrl_ + url;
    } else {
      internalPath = WebSession::Handler::instance()->request()->pathInfo();

      if (type() != WidgetSet) {
	/*
	 * Java application servers use ";jsessionid=..." which generates
	 * URLs relative to the current directory, not current filename
	 * (unlike '?=...')
	 *
	 * Therefore we start with the current 'filename', this does no harm
	 * for C++ well behaving servers either.
	 */
	if (internalPath.length() > 1) {
	  std::string lastPart
	    = internalPath.substr(internalPath.rfind('/') + 1);

	  url = lastPart;
	} else
	  url = applicationName_;
      } else
	url = applicationUrl_ + internalPath;
    }

    return appendSessionQuery(url);
  }
  case ClearInternalPath: {
    if (type() != WidgetSet) {
      if (WebSession::Handler::instance()->request()->pathInfo().length() > 1)
	return appendSessionQuery(baseUrl_ + applicationName_);
      else
	return appendSessionQuery(applicationName_);
    } else {
      return appendSessionQuery(applicationUrl_);
    }
  }
  default:
    assert(false);
  }

  return std::string();
}

std::string WebSession::mostRelativeUrl(const std::string& internalPath) const
{
  return appendSessionQuery(bookmarkUrl(internalPath));
}

std::string WebSession::appendSessionQuery(const std::string& url) const
{
  std::string result = url;
  
  if (env_->agentIsSpiderBot())
    return result;

  std::size_t questionPos = result.find('?');

  if (questionPos == std::string::npos)
    result += sessionQuery();
  else if (questionPos == result.length() - 1)
    result += sessionQuery().substr(1);
  else
    result += '&' + sessionQuery().substr(1);

#ifndef WT_TARGET_JAVA
  return result;
#else
  if (boost::starts_with(result, "?"))
    result = applicationUrl_ + result;

  return WebSession::Handler::instance()->response()->encodeURL(result);
#endif // WT_TARGET_JAVA
}

std::string WebSession::bookmarkUrl() const
{
  if (app_)
    return bookmarkUrl(app_->internalPath());
  else
    return bookmarkUrl(env_->internalPath());
}

std::string WebSession::bookmarkUrl(const std::string& internalPath) const
{
  std::string result = bookmarkUrl_;

  // Without Ajax, we either should use an absolute URL, or a relative
  // url that takes into account the internal path of the current request.
  //
  // For now, we make an absolute URL, and will fix this in Wt 3.0 since
  // there we always know the current request (?)
  if (!env_->ajax() && result.find("://") == std::string::npos
      && (env_->internalPath().length() > 1 || internalPath.length() > 1))
    result = baseUrl_ + applicationName_;

  return appendInternalPath(result, internalPath);
}

std::string WebSession::appendInternalPath(const std::string& baseUrl,
					   const std::string& internalPath)
  const
{
  if (internalPath.empty() || internalPath == "/")
    if (baseUrl.empty())
      return "?";
    else
      return baseUrl;
  else {
    if (applicationName_.empty())
      return baseUrl + "?_=" + Utils::urlEncode(internalPath);
    else
      return baseUrl + Utils::urlEncode(internalPath);
  }
}

bool WebSession::start()
{
  try {
    app_ = controller_->doCreateApplication(this);
  } catch (std::exception& e) {
    app_ = 0;

    kill();
    RETHROW(e);
  } catch (...) {
    app_ = 0;

    kill();
    throw;
  }

  if (app_)
    app_->initialize();

  return app_;
}

std::string WebSession::getCgiValue(const std::string& varName) const
{
  WebRequest *request = WebSession::Handler::instance()->request();
  if (request)
    return request->envValue(varName);
  else
    return "";
}

std::string WebSession::getCgiHeader(const std::string& headerName) const
{
  WebRequest *request = WebSession::Handler::instance()->request();
  if (request)
    return request->headerValue(headerName);
  else
    return "";
}

void WebSession::kill()
{
#ifdef WT_THREADED
  // see supra for why commented out
  //assert(WebSession::Handler::instance()->lock().owns_lock());
#endif // WT_THREADED

  state_ = Dead;

  /*
   * Unlock the recursive eventloop that may be pending.
   */
#ifndef WT_TARGET_JAVA
  unlockRecursiveEventLoop();
#endif

  if (handlers_.empty()) {
    // we may delete because the session has already been removed
    // from the sessions list, and thus, once the list is empty it is
    // guaranteed to stay empty.
#ifdef WT_THREADED
    WebSession::Handler::instance()->lock().unlock();
#endif // WT_THREADED

    delete this;
  }
}

void WebSession::checkTimers()
{
  WContainerWidget *timers = app_->timerRoot();

  const std::vector<WWidget *>& timerWidgets = timers->children();

  std::vector<WTimerWidget *> expired;

  for (unsigned i = 0; i < timerWidgets.size(); ++i) {
    WTimerWidget *wti = dynamic_cast<WTimerWidget *>(timerWidgets[i]);

    if (wti->timerExpired())
      expired.push_back(wti);
  }

  WMouseEvent dummy;

  for (unsigned i = 0; i < expired.size(); ++i)
    expired[i]->clicked().emit(dummy);
}

void WebSession::redirect(const std::string& url)
{
  redirect_ = url;
  if (redirect_.empty())
    redirect_ = "?";
}

std::string WebSession::getRedirect()
{
  std::string result = redirect_;
  redirect_.clear();
  return result;
}

WebSession::Handler::Handler(WebSession& session, bool takeLock)
  : 
#ifdef WT_THREADED
    lock_(session.mutex_, boost::defer_lock),
    prevHandler_(0),
#endif // WT_THREADED
    session_(session),
    request_(0),
    response_(0),
    killed_(false)
{
#ifdef WT_THREADED
  if (takeLock)
    lock_.lock();
#endif

  init();
}

WebSession::Handler::Handler(WebSession& session,
			     WebRequest& request, WebResponse& response)
  :
#ifdef WT_THREADED
    lock_(session.mutex_),
    prevHandler_(0),
#endif // WT_THREADED
    session_(session),
    request_(&request),
    response_(&response),
    killed_(false)
{
  init();
}

WebSession::Handler *WebSession::Handler::instance()
{
#ifdef WT_TARGET_JAVA
  return threadHandler_.get();
#else
#ifdef WT_THREADED
  return threadHandler_.get();
#else
  return threadHandler_;
#endif // WT_THREADED
#endif // WT_TARGET_JAVA
}

bool WebSession::Handler::haveLock() const
{
#ifdef WT_THREADED
  return lock_.owns_lock();
#else
  return false;
#endif // WT_THREADED
}

void WebSession::Handler::init()
{
#ifdef WT_TARGET_JAVA
  threadHandler_.reset(this);
#else
#ifdef WT_THREADED

  if (request_)
    session_.handlers_.push_back(this);

  prevHandler_ = threadHandler_.release();
  threadHandler_.reset(this);
#else
  threadHandler_ = this;
#endif // WT_THREADED
#endif // WT_TARGET_JAVA
}

void WebSession::Handler::attachThreadToSession(WebSession& session)
{
#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
  threadHandler_.reset(new Handler(session, false));
#else
  session.log("error") <<
    "attachThreadToSession() requires that Wt is built with threading enabled";
#endif // WT_THREADED
}

WebSession::Handler::~Handler()
{
#ifdef WT_THREADED
  // see supra for why commented out
  //assert(WebSession::Handler::instance()->lock().owns_lock());
#endif // WT_THREADED

#ifdef WT_TARGET_JAVA
  threadHandler_.reset(0);
#else
  Utils::erase(session_.handlers_, this);

  if (session_.handlers_.size() == 0)
    session_.hibernate();

  if (killed_)
    session_.kill();

#ifdef WT_THREADED
  threadHandler_.release();
  if (prevHandler_)
    threadHandler_.reset(prevHandler_);
#endif // WT_THREADED
#endif // WT_TARGET_JAVA
}

void WebSession::Handler::killSession()
{
#ifdef WT_THREADED
  // see supra for why commented out
  //assert(lock().owns_lock());
#endif // WT_THREADED

  killed_ = true;
  session_.state_ = Dead;
}

void WebSession::Handler::setRequest(WebRequest *request,
				      WebResponse *response)
{
  request_ = request;
  response_ = response;
}

bool WebSession::Handler::sessionDead()
{
  return (killed_ || session_.done());
}

void WebSession::hibernate()
{
  if (app_ && app_->localizedStrings())
    app_->localizedStrings()->hibernate();
}

WResource *WebSession::decodeResource(const std::string& resourceId)
{
  WResource *resource = app_->decodeExposedResource(resourceId);

  if (resource)
    return resource;
  else {
    log("error") << "decodeResource(): resource '"
		 << resourceId << "' not exposed";
    return 0;
  }
}

EventSignalBase *WebSession::decodeSignal(const std::string& signalId)
{
  EventSignalBase *result = app_->decodeExposedSignal(signalId);

  if (result)
    return result;
  else {
    log("error") << "decodeSignal(): signal '"
		 << signalId << "' not exposed";
    return 0;
  }
}

EventSignalBase *WebSession::decodeSignal(const std::string& objectId,
					  const std::string& name)
{
  EventSignalBase *result = app_->decodeExposedSignal(objectId, name);

  if (result)
    return result;
  else {
    log("error") << "decodeSignal(): signal '"
		 << objectId << '.' << name << "' not exposed";
    return 0;
  }
}

WebSession *WebSession::instance()
{
  Handler *handler = WebSession::Handler::instance();
  return handler ? handler->session() : 0;
}

void WebSession::pushEmitStack(WObject *o)
{
  emitStack_.push_back(o);
}

void WebSession::popEmitStack()
{
  emitStack_.pop_back();
}

WObject *WebSession::emitStackTop()
{
  if (!emitStack_.empty())
    return emitStack_.back();
  else
    return 0;
}

void WebSession::doRecursiveEventLoop()
{
#ifndef WT_THREADED
  log("error") << "Cannot do recursive event loop without threads";
#else // WT_THREADED
  
  /*
   * Finish the request that is being handled
   */
  Handler *handler = WebSession::Handler::instance();
  if (handler->response())
    handler->session()->render(*handler, app_->environment().ajax()
			       ? WebRenderer::Update : WebRenderer::Page);

  /*
   * Register that we are doing a recursive event loop, this is used in
   * handleRequest() to let the recursive event loop do the actual
   * notification.
   */
  recursiveEventLoop_ = handler;

  /*
   * Release session mutex lock, wait for recursive event, and retake
   * the session mutex lock.
   */
  recursiveEvent_.wait(handler->lock());

  if (state_ == Dead) {
    recursiveEventLoop_ = 0;
    throw WtException("doRecursiveEventLoop(): session was killed");
  }

  /*
   * We use recursiveEventLoop_ != null to postpone rendering: we only want
   * the event handling part.
   */
  app_->notify(WEvent(*handler, app_->environment().ajax() 
		      ? WebRenderer::Update : WebRenderer::Page));
  recursiveEventLoop_ = 0;
#endif // WT_THREADED
}

#ifndef WT_TARGET_JAVA
bool WebSession::unlockRecursiveEventLoop()
{
  if (!recursiveEventLoop_)
    return false;

  /*
   * Locate both the current and previous event loop handler.
   */
  Handler *handler = WebSession::Handler::instance();
 
  // handlerPrevious can be 0 if the event loop was already unlocked by
  // another event while doing WApplication::processEvents()
  recursiveEventLoop_->setRequest(handler->request(), handler->response());
  handler->setRequest(0, 0);

#ifdef WT_THREADED
  recursiveEvent_.notify_one();
#endif // WT_THREADED

  return true;
}
#endif // WT_TARGET_JAVA

bool WebSession::handleRequest(WebRequest& request, WebResponse& response)
{
  Configuration& conf = controller_->configuration();

  /*
   * -- Start critical section exclusive access to session
   */
#ifdef WT_TARGET_JAVA
  synchronized(mutex_)
#endif
  {
    Handler handler(*this, request, response);

    const std::string *wtdE = request.getParameter("wtd");
    const std::string *requestE = request.getParameter("request");

    WebRenderer::ResponseType responseType = WebRenderer::Page;

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
    if ((!wtdE || (*wtdE != sessionId_))
	&& state_ != JustCreated
	&& (requestE && (*requestE == "signal" ||
			 *requestE == "resource"))) {
      handler.response()->setContentType("text/html");
      handler.response()->out()
	<< "<html><head></head><body>CSRF prevention</body></html>";
    } else
      try {
	/*
	 * If we have just created a new session, we need to take care:
	 * - requests from a dead session -> reload
	 * - otherwise: serve Boot.html, Hybrid.html or Plain.html
	 *
	 * Otherwise, we are Loaded: we need to react to:
	 * - when missing a request: rerender (Plain or Hybrid)
	 * - if request for 'script':
	 *   (if appropriate, upgrade to Ajax)
	 *     serve script
	 * - if signal ...
	 * - if resource ...
	 */
	switch (state_) {
	case JustCreated: {
	  switch (type_) {
	  case Application: {
	    init(request); // env, url/internalpath

	    // Handle requests from dead sessions:
	    //
	    // We need to send JS to reload the page when we get
	    // 'request' == 'updatejs' or 'request' == "script"
	    // We ignore 'request' == 'resource'
	    //
	    // In other cases we can simply start

	    if (requestE) {
	      if (*requestE == "jsupdate" || *requestE == "script") {
		log("notice") << "Signal from dead session, sending reload.";
		renderer_.letReloadJS(*handler.response(), true);

		handler.killSession();
		break;
	      } else if (*requestE == "resource") {
		log("notice") << "Not serving bootstrap for resource.";
		handler.response()->setContentType("text/html");
		handler.response()->out()
		  << "<html><head></head><body></body></html>";

		handler.killSession();
		break;
	      }
	    }

	    /*
	     * We can simply bootstrap.
	     */
	    bool forcePlain
	      = env_->agentIsSpiderBot() || !env_->agentSupportsAjax();

	    progressiveBoot_ = !forcePlain && conf.progressiveBoot();

	    if (forcePlain || progressiveBoot_) {
	      /*
	       * First start the application
	       */
	      env_->doesAjax_ = false;
	      env_->doesCookies_ = false;

	      try {
		std::string internalPath = env_->getCookie("WtInternalPath");
		env_->setInternalPath(internalPath);
	      } catch (std::exception& e) {
	      }

	      if (!start())
		throw WtException("Could not start application.");

	      app_->notify(WEvent(handler, WebRenderer::Page));
	      setLoaded();

	      if (env_->agentIsSpiderBot())
		handler.killSession();
	    } else {
	      /*
	       * Delay application start
	       */
	      serveResponse(handler, WebRenderer::Page);
	      setState(Loaded, 10);
	    }
	    break; }
	  case WidgetSet:
	    init(request); // env, url/internalpath
	    env_->doesAjax_ = true;

	    if (!start())
	      throw WtException("Could not start application.");

	    app_->notify(WEvent(handler, WebRenderer::Script));

	    setLoaded();
	  }

	  break;
	}
	case Loaded: {
	  responseType = WebRenderer::Page;

	  if (requestE)
	    if (*requestE == "jsupdate")
	      responseType = WebRenderer::Update;
	    else if (*requestE == "script")
	      responseType = WebRenderer::Script;

	  if (!app_) {
	    if (responseType == WebRenderer::Script) {
	      const std::string *hashE = request.getParameter("_");
	      const std::string *scaleE = request.getParameter("scale");

	      env_->doesAjax_ = true;
	      env_->doesCookies_ = !request.headerValue("Cookie").empty();

	      try {
		env_->dpiScale_
		  = scaleE ? boost::lexical_cast<double>(*scaleE) : 1;
	      } catch (boost::bad_lexical_cast &e) {
		env_->dpiScale_ = 1;
	      }

	      // the internal path, when present as an anchor (#), is only
	      // conveyed in the second request
	      if (hashE)
		env_->setInternalPath(*hashE);

	      if (!start())
		throw WtException("Could not start application.");
	    } else {
	      const std::string *jsE = request.getParameter("js");

	      if (jsE && *jsE == "no") {
		if (!start())
		  throw WtException("Could not start application.");
	      } else {
		// This could be because the session Id was not as
		// expected. At least, it should be correct now.
		if (!conf.reloadIsNewSession() && wtdE && *wtdE == sessionId_) {
		  serveResponse(handler, WebRenderer::Page);
		  setState(Loaded, 10);
		} else {
		  handler.response()->setContentType("text/html");
		  handler.response()->out() <<
		    "<html><body><h1>Refusing to respond.</h1></body></html>";
		}

		break;
	      }
	    }

	    state_ = JustCreated;
	  }

	  bool requestForResource = requestE && *requestE == "resource";

#ifndef WT_TARGET_JAVA
	  if (requestForResource || !unlockRecursiveEventLoop())
#endif // WT_TARGET_JAVA
	    {
	      app_->notify(WEvent(handler, responseType));
	      if (handler.response() && !requestForResource)
		/*
		 * This may be when an error was thrown during event
		 * propagation: then we want to render the error message.
		 */
		app_->notify(WEvent(handler, responseType, true));
	    }

	  setLoaded();
	  break;
	}
	case Dead:
	  throw WtException("Internal error: WebSession is dead?");
	}

      } catch (WtException& e) {
	log("fatal") << e.what();

#ifdef WT_TARGET_JAVA
	e.printStackTrace();
#endif // WT_TARGET_JAVA

	handler.killSession();

	if (handler.response())
	  serveError(handler, e.what(), responseType);

      } catch (std::exception& e) {
	log("fatal") << e.what();

#ifdef WT_TARGET_JAVA
	e.printStackTrace();
#endif // WT_TARGET_JAVA

	handler.killSession();

	if (handler.response())
	  serveError(handler, e.what(), responseType);
      } catch (...) {
	log("fatal") << "Unknown exception.";

	handler.killSession();

	if (handler.response())
	  serveError(handler, "Unknown exception", responseType);
      }

    if (handler.response())
      handler.response()->flush();

    if (handler.sessionDead())
      controller_->removeSession(sessionId_);

#ifdef WT_TARGET_JAVA
    handler.~Handler();
#endif // WT_TARGET_JAVA

    return !handler.sessionDead();
  }
}

std::string WebSession::ajaxCanonicalUrl(const WebResponse& request) const
{
  const std::string *hashE = 0;
  if (applicationName_.empty())
    hashE = request.getParameter("_");

  if (!request.pathInfo().empty() || (hashE && hashE->length() > 1)) {
    std::string url;
    if (!request.pathInfo().empty()) {
      std::string pi = request.pathInfo();
      for (std::size_t t = pi.find('/'); t != std::string::npos;
	   t = pi.find('/', t+1)) {
	url += "../";
      }
      url += applicationName();
    } else
      url = baseUrl() + applicationName();

    bool firstParameter = true;
    for (Http::ParameterMap::const_iterator i
	   = request.getParameterMap().begin();
	 i != request.getParameterMap().end(); ++i) {
      if (i->first != "_") {
	url += (firstParameter ? '?' : '&')
	  + Utils::urlEncode(i->first) + '='
	  + Utils::urlEncode(i->second[0]);
	firstParameter = false;
      }
    }

    url += '#' + (app_ ? app_->internalPath() : env_->internalPath());

    return url;
  } else
    return std::string();
}

void WebSession::pushUpdates()
{
  if (!renderer_.isDirty())
    return;

  updatesPending_ = true;

  if (pollResponse_) {
    renderer_.serveResponse(*pollResponse_, WebRenderer::Update);
    pollResponse_->flush();
    pollResponse_ = 0;
  }
}

const std::string *WebSession::getSignal(const WebRequest& request,
					 const std::string& se)
{
  const std::string *signalE = request.getParameter(se + "signal");

  if (!signalE) {
    const Http::ParameterMap& entries = request.getParameterMap();

    for (Http::ParameterMap::const_iterator i = entries.begin();
	 i != entries.end(); ++i) {
      if (i->first.find(se + "signal=") == 0) {
	signalE = &i->second[0];

	std::string v = i->first.substr(7 + se.length());
	if (v.length() >= 2) {
	  std::string e = v.substr(v.length() - 2);
	  if (e == ".x" || e == ".y")
	    v = v.substr(0, v.length() - 2);
	}

	*(const_cast<std::string *>(signalE)) = v;
	break;
      }
    }
  }

  return signalE;
}

void WebSession::notify(const WEvent& event)
{
  Handler& handler = event.handler;
  WebRequest& request = *handler.request();
  WebResponse& response = *handler.response();

  if (WebSession::Handler::instance() != &handler)
    // We will want to set these right before doing anything !
    WebSession::Handler::instance()->setRequest(&request, &response);

  if (event.renderOnly) {
    render(handler, event.responseType);
    return;
  }

  const std::string *requestE = request.getParameter("request");

  switch (state_) {
  case WebSession::JustCreated:

#ifdef WT_WITH_OLD_INTERNALPATH_API
    if (app_->oldInternalPathAPI() && env_->internalPath() != "/") {
      app_->setInternalPath("/");
      app_->changeInternalPath(env_->internalPath());
    }
#endif // WT_WITH_OLD_INTERNALPATH_API

    render(handler, event.responseType);

    break;
  case WebSession::Loaded:
    if (event.responseType == WebRenderer::Script) {
      if (!env_->doesAjax_) {
	// upgrade to AJAX
	const std::string *hashE = request.getParameter("_");
	const std::string *scaleE = request.getParameter("scale");

	env_->doesAjax_ = true;
	env_->doesCookies_ = !request.headerValue("Cookie").empty();

	try {
	  env_->dpiScale_ = scaleE ? boost::lexical_cast<double>(*scaleE)
	    : 1;
	} catch (boost::bad_lexical_cast &e) {
	  env_->dpiScale_ = 1;
	}

	if (hashE)
	  env_->setInternalPath(*hashE);

	app_->enableAjax();
	if (env_->internalPath().length() > 1)
	  app_->changeInternalPath(env_->internalPath());
      }

      render(handler, event.responseType);
    } else {
      // a normal request to a loaded application
      try {
	if (request.postDataExceeded())
	  app_->requestTooLarge().emit(request.postDataExceeded());
      } catch (std::exception& e) {
	log("error") << "Exception in WApplication::requestTooLarge"
		     << e.what();
	RETHROW(e);
      } catch (...) {
	log("error") << "Exception in WApplication::requestTooLarge";
	throw;
      }

      const std::string *resourceE = request.getParameter("resource");
      const std::string *signalE = getSignal(request, "");

      if (signalE)
	progressiveBoot_ = false; 

      if (requestE && *requestE == "resource" && resourceE) {
	if (*resourceE == "blank") {
	  handler.response()->setContentType("text/html");
	  handler.response()->out() <<
	    "<html><head><title>bhm</title></head>"
	    "<body>&#160;</body></html>";
	  handler.response()->flush();
	  handler.setRequest(0, 0);
	} else {
	  WResource *resource = decodeResource(*resourceE);

	  if (resource) {
	    try {
	      resource->handle(&request, &response);
	      handler.setRequest(0, 0);
#ifdef WT_THREADED
	      if (!handler.lock().owns_lock())
		handler.lock().lock();
#endif // WT_THREADED
	    } catch (std::exception& e) {
	      log("error") << "Exception while streaming resource" << e.what();
	      RETHROW(e);
	    } catch (...) {
	      log("error") << "Exception while streaming resource";
	      throw;
	    }
	  } else {
	    // ???
	    handler.response()->setContentType("text/html");
	    handler.response()->out() <<
	      "<html><body><h1>Refusing to respond.</h1></body></html>";
	    handler.response()->flush();
	    handler.setRequest(0, 0);
	  }
	}
      } else {
	env_->urlScheme_ = request.urlScheme();

	if (signalE) {
	  const std::string *ackIdE = request.getParameter("ackId");
	  try {
	    if (ackIdE)
	      renderer_.ackUpdate(boost::lexical_cast<int>(*ackIdE));
	  } catch (const boost::bad_lexical_cast& e) {
	    log("error") << "Could not parse ackId: " << *ackIdE;
	  }

	  if (pollResponse_) {
	    if (*signalE == "poll")
	      renderer_.letReloadJS(*pollResponse_, true);

	    pollResponse_->flush();
	    pollResponse_ = 0;
	  }

	  if (*signalE != "res" && *signalE != "poll") {
	    //std::cerr << "signal: " << *signalE << std::endl;

	    /*
	     * Special signal values:
	     * 'poll' : long poll
	     * 'none' : no event, but perhaps a synchronization
	     * 'load' : load invisible content
	     * 'res'  : after a resource received data
	     */

	    try {
	      notifySignal(event);
	    } catch (std::exception& e) {
	      log("error") << "Error during event handling: " << e.what();
	      RETHROW(e);
	    } catch (...) {
	      log("error") << "Error during event handling: ";
	      throw;
	    }
	  } else if (*signalE == "poll" && !updatesPending_) {
	    pollResponse_ = handler.response();
	    //pollResponse_->setContentType("text/plain; charset=UTF-8");
	    //pollResponse_->flush(WebResponse::ResponseWaitMore);
	    handler.setRequest(0, 0);
	  }
	} else {
	  log("notice") << "Refreshing session";

	  if (event.responseType == WebRenderer::Page) {
	    if (!request.pathInfo().empty())
	      app_->changeInternalPath(request.pathInfo());
	    else {
	      const std::string *hashE = request.getParameter("_");
	      if (hashE)
		app_->changeInternalPath(*hashE);
	      else
		app_->changeInternalPath("");
	    }
	  }

#ifndef WT_TARGET_JAVA
	  // if we are persisting sessions, then we should make sure we
	  // are listening to only one browser at a time: do this by
	  // generating a new session id when a new browser connects
	  if (controller_->configuration().persistentSessions()) {
	    app_->connected_ = true;
	    generateNewSessionId();
	    env_->init(request);
	  }
#endif // WT_TARGET_JAVA

	  env_->parameters_ = handler.request()->getParameterMap();
	  app_->refresh();
	}

	if (handler.response() && !recursiveEventLoop_)
	  render(handler, event.responseType);
      }
    }
  case Dead:
    break;
  }
}

void WebSession::render(Handler& handler,
			WebRenderer::ResponseType responseType)
{
  /*
   * In any case, render() will flush the response, even if an error
   * occurred. Since we are already rendering the response, we can no longer
   * show a nice error message.
   */
  try {
    if (!env_->doesAjax_)
      try {
	checkTimers();
      } catch (std::exception& e) {
	log("error") << "Exception while triggering timers" << e.what();
	RETHROW(e);
      } catch (...) {
	log("error") << "Exception while triggering timers";
	throw;
      }

    if (app_->isQuited())
      handler.killSession();

    serveResponse(handler, responseType);
  } catch (std::exception& e) {
    handler.response()->flush();
    handler.setRequest(0, 0);

    RETHROW(e);
  } catch (...) {
    handler.response()->flush();
    handler.setRequest(0, 0);

    throw;
  }

  updatesPending_ = false;
}

void WebSession::serveError(Handler& handler, const std::string& e,
			    WebRenderer::ResponseType responseType)
{
  renderer_.serveError(*handler.response(), e, responseType);
  handler.response()->flush();
  handler.setRequest(0, 0);
}

void WebSession::serveResponse(Handler& handler,
			       WebRenderer::ResponseType responseType)
{
  renderer_.serveResponse(*handler.response(), responseType);
  handler.response()->flush();
  handler.setRequest(0, 0);
}

void WebSession::propagateFormValues(const WEvent& e, const std::string& se)
{
  const WebRequest& request = *e.handler.request();

  renderer_.updateFormObjectsList(app_);
  WebRenderer::FormObjectsMap formObjects = renderer_.formObjects();

  for (WebRenderer::FormObjectsMap::const_iterator i = formObjects.begin();
       i != formObjects.end(); ++i) {
    std::string formName = i->first;
    WObject *obj = i->second;

    if (!request.postDataExceeded())
      obj->setFormData(getFormData(request, se + formName));
    else
      obj->requestTooLarge(request.postDataExceeded());
  }
}

WObject::FormData WebSession::getFormData(const WebRequest& request,
					  const std::string& name)
{
  Http::UploadedFileMap::const_iterator file
    = request.uploadedFiles().find(name);

  return WObject::FormData(request.getParameterValues(name),
			   file != request.uploadedFiles().end()
			   ? &file->second : 0);
}

std::vector<unsigned int> WebSession::getSignalProcessingOrder(const WEvent& e)
{
  // Rush 'onChange' events. Reason: if a user edits a text area and
  // a subsequent click on another element deletes the text area, we
  // have seen situations (at least on firefox) where the clicked event
  // is processed before the changed event, causing the changed event
  // to fail because the event target was deleted.
  WebSession::Handler& handler = e.handler;

  std::vector<unsigned int> highPriority;
  std::vector<unsigned int> normalPriority;

  for (unsigned i = 0;; ++i) {
    const WebRequest& request = *handler.request();

    std::string se = i > 0
      ? 'e' + boost::lexical_cast<std::string>(i) : std::string();
    const std::string *signalE = getSignal(request, se);
    if (!signalE) {
      break;
    }
    if (*signalE != "user" &&
        *signalE != "hash" &&
        *signalE != "res" &&
        *signalE != "none" &&
        *signalE != "load") {
      EventSignalBase *signal = decodeSignal(*signalE);
      if (!signal) {
        // Signal was not exposed, do nothing
      } else if (signal->name() == WFormWidget::CHANGE_SIGNAL) {
        // compare by pointer in the condition above is ok
        highPriority.push_back(i);
      } else {
        normalPriority.push_back(i);
      }
    } else {
      normalPriority.push_back(i);
    }
  }

  Utils::insert(highPriority, normalPriority);

  return highPriority;
}

void WebSession::notifySignal(const WEvent& e)
{
  WebSession::Handler& handler = e.handler;

  // Save pending changes (e.g. from resource completion)
  renderer_.saveChanges();

  // Reorder signals, as browsers sometimes generate them in a strange order
  std::vector<unsigned int> order = getSignalProcessingOrder(e);
  for (unsigned i = 0; i < order.size(); ++i) {
    if (!handler.request())
      return;

    const WebRequest& request = *handler.request();

    std::string se = i > 0
      ? 'e' + boost::lexical_cast<std::string>(i) : std::string();
    const std::string *signalE = getSignal(request, se);

    if (!signalE)
      return;

    propagateFormValues(e, se);

    if (*signalE == "hash") {
      const std::string *hashE = request.getParameter(se + "_");
      if (hashE) {
	app_->changeInternalPath(*hashE);
	app_->doJavaScript(WT_CLASS ".scrollIntoView('" + *hashE + "');");
      }
    } else if (*signalE == "none" || *signalE == "load") {
      // We will want invisible changes now too.
      renderer_.setVisibleOnly(false);
    } else {
      if (*signalE != "res") {
	for (unsigned k = 0; k < 3; ++k) {
	  SignalKind kind = (SignalKind)k;

	  if (kind == AutoLearnStateless && request.postDataExceeded())
	    break;

	  if (*signalE == "user") {
	    const std::string *idE = request.getParameter(se + "id");
	    const std::string *nameE = request.getParameter(se + "name");

	    if (!idE || !nameE)
	      break;

	    processSignal(decodeSignal(*idE, *nameE), se, request, kind);
	  } else
	    processSignal(decodeSignal(*signalE), se, request, kind);

	  if (kind == LearnedStateless && i == 0)
	    renderer_.discardChanges();
	}
      }
    }
  }
}

void WebSession::processSignal(EventSignalBase *s, const std::string& se,
			       const WebRequest& request, SignalKind kind)
{
  if (!s)
    return;

  switch (kind) {
  case LearnedStateless:
    s->processLearnedStateless();
    break;
  case AutoLearnStateless:
    s->processAutoLearnStateless(&renderer_);
    break;
  case Dynamic:
    JavaScriptEvent jsEvent;
    jsEvent.get(request, se);
    s->processDynamic(jsEvent);

    // ! handler.request() may be 0 now, if there was a
    // ! recursive call.
    // ! what with other slots triggered after the one that
    // ! did the recursive call ? That's very bad ??
  }
}

#ifndef WT_TARGET_JAVA
void WebSession::generateNewSessionId()
{
  std::string oldId = sessionId_;
  sessionId_ = controller_->generateNewSessionId(this);
  log("notice") << "New session id for " << oldId;

  if (controller_->configuration().sessionTracking()
      == Configuration::CookiesURL) {
    std::string cookieName = env_->deploymentPath();
    renderer().setCookie(cookieName, sessionId_, -1, "", "");
  }
}
#endif // WT_TARGET_JAVA

}
