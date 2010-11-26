/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WCombinedLocalizedStrings"
#include "Wt/WContainerWidget"
#include "Wt/WFormWidget"
#include "Wt/WResource"
#include "Wt/WTimerWidget"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "CgiParser.h"
#include "Configuration.h"
#include "DomElement.h"
#include "Utils.h"
#include "WebController.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WebSocketMessage.h"
#include "WtException.h"

#include <boost/algorithm/string.hpp>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef WT_TARGET_JAVA
#define RETHROW(e) throw e
#else
#define RETHROW(e) throw
#endif

namespace {
  #ifdef WT_TARGET_JAVA
  static Wt::Http::UploadedFile* uf;
  #endif
}

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
    asyncResponse_(0),
    updatesPending_(false),
    canWriteAsyncResponse_(false),
    embeddedEnv_(this),
    app_(0),
    debug_(controller_->configuration().debug()),
    recursiveEventLoop_(0)
#if defined(WT_TARGET_JAVA)
  ,
    recursiveEvent_(mutex_.newCondition())
#endif
{
#ifdef WT_THREADED
  syncLocks_.lastId_ = syncLocks_.lockedId_ = 0;
#endif // WT_THREADED

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
  /*
   * From here on, we cannot create a shared_ptr to this session. Therefore,
   * app_ uses a weak_ptr to this session for which lock() returns an empty
   * shared pointer.
   */
#ifndef WT_TARGET_JAVA
  Handler handler(this);

  if (app_)
    app_->notify(WEvent());

  delete app_;
#endif // WT_TARGET_JAVA

  if (asyncResponse_) {
    asyncResponse_->flush();
    asyncResponse_ = 0;
  }

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
  
  if (WebSession::Handler::instance()->response())
    return WebSession::Handler::instance()->response()->encodeURL(result);
  
return url;
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

  return app_;
}

std::string WebSession::getCgiValue(const std::string& varName) const
{
  WebRequest *request = WebSession::Handler::instance()->request();
  if (request)
    return request->envValue(varName);
  else
    return std::string();
}

std::string WebSession::getCgiHeader(const std::string& headerName) const
{
  WebRequest *request = WebSession::Handler::instance()->request();
  if (request)
    return request->headerValue(headerName);
  else
    return std::string();
}

void WebSession::kill()
{
  state_ = Dead;

  /*
   * Unlock the recursive eventloop that may be pending.
   */
  unlockRecursiveEventLoop();
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

WebSession::Handler::Handler()
  : 
    prevHandler_(0),
    session_(0),
    request_(0),
    response_(0)
{
#ifdef WT_TARGET_JAVA
  locked_ = true;
#endif // WT_TARGET_JAVA

  init();
}

WebSession::Handler::Handler(boost::shared_ptr<WebSession> session,
			     bool takeLock)
  : 
#ifdef WT_THREADED
    lock_(session->mutex_, boost::defer_lock),
#endif // WT_THREADED
    prevHandler_(0),
    session_(session.get()),
#ifndef WT_TARGET_JAVA
    sessionPtr_(session),
#endif // WT_TARGET_JAVA
    request_(0),
    response_(0)
{
#ifdef WT_THREADED
  if (takeLock)
    lock_.lock();
#endif
#ifdef WT_TARGET_JAVA
  if (takeLock) {
    session->mutex().lock();
    locked_ = true;
  }
#endif // WT_TARGET_JAVA

  init();
}

WebSession::Handler::Handler(WebSession *session)
  : prevHandler_(0),
    session_(session),
    request_(0),
    response_(0)
{
  init();
}

WebSession::Handler::Handler(boost::shared_ptr<WebSession> session,
			     WebRequest& request, WebResponse& response)
  :
#ifdef WT_THREADED
    lock_(session->mutex_),
#endif // WT_THREADED
    prevHandler_(0),
    session_(session.get()),
#ifndef WT_TARGET_JAVA
    sessionPtr_(session),
#endif // WT_TARGET_JAVA
    request_(&request),
    response_(&response)
{
#ifdef WT_TARGET_JAVA
  session->mutex().lock();
  locked_ = true;
#endif

  init();
}

WebSession::Handler *WebSession::Handler::instance()
{
#if defined(WT_TARGET_JAVA) || defined(WT_THREADED)
  return threadHandler_.get();
#else
  return threadHandler_;
#endif // WT_TARGET_JAVA || WT_THREADED
}

bool WebSession::Handler::haveLock() const
{
#ifdef WT_THREADED
  return lock_.owns_lock();
#else
#ifdef WT_TARGET_JAVA
  return locked_;
#else
  return false;
#endif
#endif // WT_THREADED
}

void WebSession::Handler::init()
{
  prevHandler_ = attachThreadToHandler(this);

#ifndef WT_TARGET_JAVA
  if (request_)
    session_->handlers_.push_back(this);
#endif
}

WebSession::Handler *
WebSession::Handler::attachThreadToHandler(Handler *handler)
{
  WebSession::Handler *result;

#if defined(WT_TARGET_JAVA) || defined(WT_THREADED)
  result = threadHandler_.release();
  threadHandler_.reset(handler);
#else
  result = threadHandler_;
  threadHandler_ = handler;
#endif

  return result;
}

void WebSession
::Handler::attachThreadToSession(boost::shared_ptr<WebSession> session)
{
  attachThreadToHandler(0);

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
  if (!session.get())
    return;

  /*
   * It may be that we still need to attach to a session while it is being
   * destroyed ?
   */
  if (session->state_ == Dead) {
    session->log("warn") << "Attaching to dead session?";
    attachThreadToHandler(new Handler(session, false));
    return;
  }

  /*
   * We assume that another handler has already locked this session for us.
   * We just need to find it.
   */
  for (unsigned i = 0; i < session->handlers_.size(); ++i)
    if (session->handlers_[i]->haveLock()) {
      attachThreadToHandler(session->handlers_[i]);
      return;
    }

  session->log("error") << "WApplication::attachThread(): "
			<< "no thread is holding this application's lock ?";
#else
  session->log("error") << "WApplication::attachThread(): "
			<< "needs Wt built with threading enabled";
#endif // WT_THREADED || WT_TARGET_JAVA
}

#ifdef WT_TARGET_JAVA
void WebSession::Handler::release()
{
  if (locked_)
    session_->mutex().unlock();

  attachThreadToHandler(prevHandler_);
}
#endif

WebSession::Handler::~Handler()
{
#ifndef WT_TARGET_JAVA
  Utils::erase(session_->handlers_, this);

  if (session_->handlers_.empty())
    session_->hibernate();

  attachThreadToHandler(prevHandler_);
#endif // WT_TARGET_JAVA
}

void WebSession::Handler::setRequest(WebRequest *request,
				     WebResponse *response)
{
  request_ = request;
  response_ = response;
}

void WebSession::hibernate()
{
  if (app_ && app_->localizedStrings_)
    app_->localizedStrings_->hibernate();
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
#if !defined(WT_THREADED) && !defined(WT_TARGET_JAVA)
  log("error") << "Cannot do recursive event loop without threads";
#else // WT_THREADED

#ifdef WT_TARGET_JAVA
  if (!WebController::isAsyncSupported())
    throw WtException("Recursive event loop requires a Servlet 3.0 API.");
#endif
  
  /*
   * Finish the request that is being handled
   */
  Handler *handler = WebSession::Handler::instance();

  /*
   * It could be that handler does not have a request/respone:
   *  if it is actually a long polling server push request.
   *  if we are somehow recursing recursive event loops (can anyone explain
   *  that ?)
   *
   * In that case, we do not need to finish it.
   */
  if (handler->request())
    handler->session()->notifySignal(WEvent(*handler));

  if (handler->response())
    handler->session()->render(*handler);

  /*
   * Register that we are doing a recursive event loop, this is used in
   * handleRequest() to let the recursive event loop do the actual
   * notification.
   */
  recursiveEventLoop_ = handler;
  newRecursiveEvent_ = false;

  /*
   * Release session mutex lock, wait for recursive event, and retake
   * the session mutex lock.
   */
#ifndef WT_TARGET_JAVA
  if (asyncResponse_ && asyncResponse_->isWebSocketRequest())
    asyncResponse_->readWebSocketMessage
     (boost::bind(&WebSession::handleWebSocketMessage, shared_from_this()));

  while (!newRecursiveEvent_)
    recursiveEvent_.wait(handler->lock());
#else
  while (!newRecursiveEvent_)
    recursiveEvent_.wait();
#endif

  if (state_ == Dead) {
    recursiveEventLoop_ = 0;
    throw WtException("doRecursiveEventLoop(): session was killed");
  }

  /*
   * We use recursiveEventLoop_ != null to postpone rendering: we only want
   * the event handling part.
   */
  app_->notify(WEvent(*handler));

  recursiveEventLoop_ = 0;
#endif // !WT_THREADED && !WT_TARGET_JAVA
}

void WebSession::expire()
{
  kill();
}

bool WebSession::unlockRecursiveEventLoop()
{
  if (!recursiveEventLoop_)
    return false;

  /*
   * Locate both the current and previous event loop handler.
   */
  Handler *handler = WebSession::Handler::instance();

  recursiveEventLoop_->setRequest(handler->request(), handler->response());
  // handler->response()->startAsync();
  handler->setRequest(0, 0);

  newRecursiveEvent_ = true;

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
  recursiveEvent_.notify_one();
#endif // WT_THREADED

  return true;
}

void WebSession::handleRequest(Handler& handler)
{
  WebRequest& request = *handler.request();

  if (request.isWebSocketRequest()) {
    handleWebSocketRequest(handler);
    return;
  }

  Configuration& conf = controller_->configuration();

  const std::string *wtdE = request.getParameter("wtd");
  const std::string *requestE = request.getParameter("request");

  handler.response()->setResponseType(WebResponse::Page);

  /*
   * Only handle GET and POST requests, unless a resource is
   * listening.
   */
  if (!((requestE && *requestE == "resource")
	|| request.requestMethod() == "POST"
	|| request.requestMethod() == "GET"))
    handler.response()->setStatus(400); // Bad Request

  /*
   * Under what circumstances do we allow a request which does not have
   * a the session ID (i.e. who as it only through a cookie?)
   *
   *  - when a new session is created
   *  - when reloading the page (we document in the API that you should not
   *    run business logic when doing that)
   *
   * in other cases: silenty discard the request
   */
  else if ((!wtdE || (*wtdE != sessionId_))
	   && state_ != JustCreated
	   && (requestE && (*requestE == "jsupdate" ||
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
	      handler.response()->setResponseType(WebResponse::Update);
	      log("notice") << "Signal from dead session, sending reload.";
	      renderer_.letReloadJS(*handler.response(), true);

	      kill();
	      break;
	    } else if (*requestE == "resource") {
	      log("notice") << "Not serving bootstrap for resource.";
	      handler.response()->setContentType("text/html");
	      handler.response()->out()
		<< "<html><head></head><body></body></html>";

	      kill();
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

	    app_->notify(WEvent(handler));
	    setLoaded();

	    if (env_->agentIsSpiderBot())
	      kill();
	  } else {
	    /*
	     * Delay application start
	     */
	    serveResponse(handler);
	    setState(Loaded, 10);
	  }
	  break; }
	case WidgetSet:
	  handler.response()->setResponseType(WebResponse::Script);

	  init(request); // env, url/internalpath
	  env_->doesAjax_ = true;

	  if (!start())
	    throw WtException("Could not start application.");

	  app_->notify(WEvent(handler));

	  setLoaded();

	  break;
	default:
	  assert(false); // StaticResource
	}

	break;
      }
      case Loaded: {
	if (requestE) {
	  if (*requestE == "jsupdate")
	    handler.response()->setResponseType(WebResponse::Update);
	  else if (*requestE == "script")
	    handler.response()->setResponseType(WebResponse::Script);
	}

	if (!app_) {
	  const std::string *resourceE = request.getParameter("resource");

	  if (handler.response()->responseType() == WebResponse::Script) {
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

	  } else if (requestE && *requestE == "resource"
		     && resourceE && *resourceE == "blank") {
	    handler.response()->setContentType("text/html");
	    handler.response()->out() <<
	      "<html><head><title>bhm</title></head>"
	      "<body>&#160;</body></html>";

	    break;
	  } else {
	    const std::string *jsE = request.getParameter("js");

	    if (jsE && *jsE == "no") {
	      if (!start())
		throw WtException("Could not start application.");
	    } else {
	      // This could be because the session Id was not as
	      // expected. At least, it should be correct now.
	      if (!conf.reloadIsNewSession() && wtdE && *wtdE == sessionId_) {
		serveResponse(handler);
		setState(Loaded, 10);
	      } else {
		handler.response()->setContentType("text/html");
		handler.response()->out() <<
		  "<html><body><h1>Refusing to respond.</h1></body></html>";
	      }

	      break;
	    }
	  }
	}

	bool requestForResource = requestE && *requestE == "resource";

	if (requestForResource || !unlockRecursiveEventLoop())
	  {
	    app_->notify(WEvent(handler));
	    if (handler.response() && !requestForResource)
	      /*
	       * This may be when an error was thrown during event
	       * propagation: then we want to render the error message.
	       */
	      app_->notify(WEvent(handler, true));
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

      kill();

      if (handler.response())
	serveError(handler, e.what());

    } catch (std::exception& e) {
      log("fatal") << e.what();

#ifdef WT_TARGET_JAVA
      e.printStackTrace();
#endif // WT_TARGET_JAVA

      kill();

      if (handler.response())
	serveError(handler, e.what());
    } catch (...) {
      log("fatal") << "Unknown exception.";

      kill();

      if (handler.response())
	serveError(handler, "Unknown exception");
    }

  if (handler.response())
    handler.response()->flush();
}

void WebSession::handleWebSocketRequest(Handler& handler)
{
#ifndef WT_TARGET_JAVA
  if (state_ != Loaded) {
    handler.response()->flush(WebRequest::ResponseDone);
    return;
  }

  if (asyncResponse_) {
    asyncResponse_->flush();
    asyncResponse_ = 0;
  }

  asyncResponse_ = handler.response();

  char buf[16];
  handler.request()->in().read(buf, 16);
  handler.response()->out().write(buf, 16);

  asyncResponse_->flush
    (WebRequest::ResponseFlush,
     boost::bind(&WebSession::webSocketReady,				    
		 boost::weak_ptr<WebSession>(shared_from_this())));
  canWriteAsyncResponse_ = false;

  asyncResponse_->readWebSocketMessage
    (boost::bind(&WebSession::handleWebSocketMessage,
		 boost::weak_ptr<WebSession>(shared_from_this())));


  handler.setRequest(0, 0);
#endif // WT_TARGET_JAVA
}

void WebSession::handleWebSocketMessage(boost::weak_ptr<WebSession> session)
{
#ifndef WT_TARGET_JAVA
  boost::shared_ptr<WebSession> lock = session.lock();
  if (lock) {
    WebSocketMessage *message = new WebSocketMessage(lock.get());
    
    if (message->contentLength() == 0)
      return;

    CgiParser cgi(lock->controller_->configuration().maxRequestSize());
    try {
      cgi.parse(*message);
    } catch (std::exception& e) {
      std::cerr << "Could not parse request: " << e.what();

      delete message;
      lock->asyncResponse_->flush();
      lock->asyncResponse_ = 0;
      return;
    }

    {
      Handler handler(lock, *message, (WebResponse &)(*message));

      lock->handleRequest(handler);
    }

    lock->asyncResponse_->readWebSocketMessage
      (boost::bind(&WebSession::handleWebSocketMessage, session));
  }
#endif // WT_TARGET_JAVA
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

  if (canWriteAsyncResponse_) {
    if (asyncResponse_->isWebSocketRequest()
	&& asyncResponse_->webSocketMessagePending())
      return;

    asyncResponse_->setResponseType(WebResponse::Update);
    renderer_.serveResponse(*asyncResponse_);
    updatesPending_ = false;

    if (!asyncResponse_->isWebSocketRequest()) {
      asyncResponse_->flush();
      asyncResponse_ = 0;
      canWriteAsyncResponse_ = false;
    } else {
#ifndef WT_TARGET_JAVA
      canWriteAsyncResponse_ = false;
      asyncResponse_->flush
	(WebRequest::ResponseFlush,
	 boost::bind(&WebSession::webSocketReady,
		     boost::weak_ptr<WebSession>(shared_from_this())));
#endif // WT_TARGET_JAVA
    }
  }
}

void WebSession::webSocketReady(boost::weak_ptr<WebSession> session)
{
#ifndef WT_TARGET_JAVA
  boost::shared_ptr<WebSession> lock = session.lock();
  if (lock) {
    Handler handler(lock, true);

    lock->canWriteAsyncResponse_ = true;

    if (lock->updatesPending_)
      lock->pushUpdates();
  }
#endif // WT_TARGET_JAVA
}

const std::string *WebSession::getSignal(const WebRequest& request,
					 const std::string& se)
{
  const std::string *signalE = request.getParameter(se + "signal");

  if (!signalE) {
    const int signalLength = 7 + se.length();

    const Http::ParameterMap& entries = request.getParameterMap();

    for (Http::ParameterMap::const_iterator i = entries.begin();
	 i != entries.end(); ++i) {
      if (i->first.length() > static_cast<unsigned>(signalLength)
	  && i->first.substr(0, signalLength) == se + "signal=") {
	signalE = &i->second[0];

	std::string v = i->first.substr(signalLength);
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
#ifndef WT_TARGET_JAVA
  if (event.handler == 0) {
    app_->finalize();
    return;
  }
#endif // WT_TARGET_JAVA

  Handler& handler = *event.handler;
  WebRequest& request = *handler.request();
  WebResponse& response = *handler.response();

  if (WebSession::Handler::instance() != &handler)
    // We will want to set these right before doing anything !
    WebSession::Handler::instance()->setRequest(&request, &response);

  if (event.renderOnly) {
    render(handler);
    return;
  }

  const std::string *requestE = request.getParameter("request");

  if (!app_->initialized_) {
    app_->initialize();
    app_->initialized_ = true;
  }

  switch (state_) {
  case WebSession::JustCreated:

#ifdef WT_WITH_OLD_INTERNALPATH_API
    if (app_->oldInternalPathAPI() && env_->internalPath() != "/") {
      app_->setInternalPath("/");
      app_->changeInternalPath(env_->internalPath());
    }
#endif // WT_WITH_OLD_INTERNALPATH_API

    render(handler);

    break;
  case WebSession::Loaded:
    if (handler.response()->responseType() == WebResponse::Script) {
      if (!env_->doesAjax_) {
	// upgrade to AJAX -> this becomes a first update we may need
	// to replay this, so we cannot commit these changes until
	// we have received an ack for this.

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

      render(handler);
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

      WResource *resource = 0;
      if (!requestE && !request.pathInfo().empty())
	resource = app_->decodeExposedResource("/path/" + request.pathInfo());

      const std::string *resourceE = request.getParameter("resource");
      const std::string *signalE = getSignal(request, "");

      if (signalE)
	progressiveBoot_ = false; 

      if (resource || (requestE && *requestE == "resource" && resourceE)) {
	if (resourceE && *resourceE == "blank") {
	  handler.response()->setContentType("text/html");
	  handler.response()->out() <<
	    "<html><head><title>bhm</title></head>"
	    "<body>&#160;</body></html>";
	  handler.response()->flush();
	  handler.setRequest(0, 0);
	} else {
	  if (!resource)
	    resource = app_->decodeExposedResource(*resourceE);

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
	    log("error") << "decodeResource(): resource '"
			 << *resourceE << "' not exposed";
	    handler.response()->setContentType("text/html");
	    handler.response()->out() <<
	      "<html><body><h1>Nothing to say about that.</h1></body></html>";
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

	  /*
	   * In case we are not using websocket but long polling, the client
	   * aborts the previous poll request to indicate a client-side event.
	   *
	   * So we also discard the previous asyncResponse_ server-side.
	   */
	  if (asyncResponse_ && !asyncResponse_->isWebSocketRequest()) {

	    /* Not sure of this is still relevant? */
	    if (*signalE == "poll")
	      renderer_.letReloadJS(*asyncResponse_, true);

#ifndef WT_TARGET_JAVA
	    asyncResponse_->flush();
#endif
	    asyncResponse_ = 0;
	    canWriteAsyncResponse_ = false;
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
	      handler.nextSignal = -1;
	      notifySignal(event);
	    } catch (std::exception& e) {
	      log("error") << "Error during event handling: " << e.what();
	      RETHROW(e);
	    } catch (...) {
	      log("error") << "Error during event handling: ";
	      throw;
	    }
	  } else if (*signalE == "poll" && !updatesPending_) {
	    if (!asyncResponse_) {
	      asyncResponse_ = handler.response();
	      canWriteAsyncResponse_ = true;
	      handler.setRequest(0, 0);
            } else {
	      std::cerr << "Poll after web socket connect. Ignoring" << std::endl;
            }
	  }
	} else {
	  log("notice") << "Refreshing session";

	  if (handler.response()->responseType() == WebResponse::Page) {
	    const std::string *hashE = request.getParameter("_");
	    if (hashE)
	      app_->changeInternalPath(*hashE);
	    else if (!request.pathInfo().empty())
	      app_->changeInternalPath(request.pathInfo());
	    else
	      app_->changeInternalPath("");
	  }

#ifndef WT_TARGET_JAVA
	  // if we are persisting sessions, then we should make sure we
	  // are listening to only one browser at a time: do this by
	  // generating a new session id when a new browser connects
	  if (controller_->configuration().persistentSessions()) {
	    log("info") << "Refresh for persistent session";
	    WEnvironment oldEnv = *env_;
	    env_->init(request);
	    env_->parameters_ = handler.request()->getParameterMap();

	    try {
	      app_->refresh();

	      app_->connected_ = true;
	    } catch (std::exception& e) {
	      *env_ = oldEnv;

	      log("info") << "Bad refresh attempt: " << e.what();
	      handler.response()->setContentType("text/html");
	      handler.response()->out() <<
		"<html><body><h1>Are you trying some shenanigans?"
		"</h1></body></html>";
	      handler.response()->flush();
	      handler.setRequest(0, 0);    
	    }
	  } else {
#endif // WT_TARGET_JAVA
	    env_->parameters_ = handler.request()->getParameterMap();
	    app_->refresh();
#ifndef WT_TARGET_JAVA
	  }
#endif // WT_TARGET_JAVA
	}

	if (handler.response() && !recursiveEventLoop_)
	  render(handler);
      }
    }
  case Dead:
    break;
  }
}

void WebSession::render(Handler& handler)
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
      kill();

    serveResponse(handler);
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

void WebSession::serveError(Handler& handler, const std::string& e)
{
  renderer_.serveError(*handler.response(), e);
  handler.response()->flush();
  handler.setRequest(0, 0);
}

void WebSession::serveResponse(Handler& handler)
{
  /*
   * If the request is a web socket message, then we should not actually
   * render -- there may be more messages following.
   */
  if (!handler.request()->isWebSocketMessage())
    renderer_.serveResponse(*handler.response());

  handler.response()->flush();
  handler.setRequest(0, 0);
}

void WebSession::propagateFormValues(const WEvent& e, const std::string& se)
{
  const WebRequest& request = *e.handler->request();

  renderer_.updateFormObjectsList(app_);
  WebRenderer::FormObjectsMap formObjects = renderer_.formObjects();

  const std::string *focus = request.getParameter(se + "focus");
  if (focus) {
    int selectionStart = -1, selectionEnd = -1;
    try {
      const std::string *selStart = request.getParameter(se + "selstart");
      if (selStart)
	selectionStart = boost::lexical_cast<int>(*selStart);

      const std::string *selEnd = request.getParameter(se + "selend");
      if (selEnd)
	selectionEnd = boost::lexical_cast<int>(*selEnd);
    } catch (boost::bad_lexical_cast& ee) {
      log("error") << "Could not lexical cast selection range";
    }

    app_->setFocus(*focus, selectionStart, selectionEnd);
  } else
    app_->setFocus(std::string(), -1, -1);

  for (WebRenderer::FormObjectsMap::const_iterator i = formObjects.begin();
       i != formObjects.end(); ++i) {
    std::string formName = i->first;
    WObject *obj = i->second;

    if (!request.postDataExceeded())
      obj->setFormData(getFormData(request, se + formName));
    else
      obj->setRequestTooLarge(request.postDataExceeded());
  }
}

WObject::FormData WebSession::getFormData(const WebRequest& request,
					  const std::string& name)
{
  std::vector<Http::UploadedFile> files;
  Utils::find(request.uploadedFiles(), name, files);

  return WObject::FormData(request.getParameterValues(name), files);
}

std::vector<unsigned int> WebSession::getSignalProcessingOrder(const WEvent& e)
{
  // Rush 'onChange' events. Reason: if a user edits a text area and
  // a subsequent click on another element deletes the text area, we
  // have seen situations (at least on firefox) where the clicked event
  // is processed before the changed event, causing the changed event
  // to fail because the event target was deleted.
  WebSession::Handler& handler = *e.handler;

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
  WebSession::Handler& handler = *e.handler;

  // Reorder signals, as browsers sometimes generate them in a strange order
  if (handler.nextSignal == -1) {
    handler.signalOrder = getSignalProcessingOrder(e);
    handler.nextSignal = 0;
  }

  for (unsigned i = handler.nextSignal; i < handler.signalOrder.size(); ++i) {
    if (!handler.request())
      return;

    const WebRequest& request = *handler.request();

    int signalI = handler.signalOrder[i];
    std::string se = signalI > 0
      ? 'e' + boost::lexical_cast<std::string>(signalI) : std::string();
    const std::string *signalE = getSignal(request, se);

    if (!signalE)
      return;

    // Save pending changes (e.g. from resource completion)
    if (i == 0)
      renderer_.saveChanges();

    propagateFormValues(e, se);

    handler.nextSignal = i + 1;

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
  sessionId_ = controller_->generateNewSessionId(shared_from_this());
  log("notice") << "New session id for " << oldId;

  if (controller_->configuration().sessionTracking()
      == Configuration::CookiesURL) {
    std::string cookieName = env_->deploymentPath();
    renderer().setCookie(cookieName, sessionId_, -1, "", "");
  }
}
#endif // WT_TARGET_JAVA

}
