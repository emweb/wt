/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
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

namespace Wt {

class WEvent {
public:
  enum EventType { EmitSignal, Refresh, Render, HashChange,
		   PropagateFormValues
  };

  WebSession::Handler& handler;
  EventType            type;

  WebSession *session() const { return handler.session(); }

  /* For Render type */
  WebRenderer::ResponseType responseType;

  /* For HashChange type */
  std::string hash;

  WEvent(WebSession::Handler& aHandler, EventType aType,
	 WebRenderer::ResponseType aResponseType = WebRenderer::FullResponse)
    : handler(aHandler),
      type(aType),
      responseType(aResponseType)
  { }

  WEvent(WebSession::Handler& aHandler, EventType aType,
	 const std::string& aHash)
    : handler(aHandler),
      type(aType),
      responseType(WebRenderer::FullResponse),
      hash(aHash)
  { }
};

#ifdef WT_THREADED
boost::thread_specific_ptr<WebSession::Handler *>
  WebSession::Handler::threadHandler_;
#else
#ifdef WT_TARGET_JAVA
boost::thread_specific_ptr<WebSession::Handler>
  WebSession::threadHandler_;
#else
WebSession::Handler * WebSession::Handler::threadHandler_;
#endif // WT_TARGET_JAVA
#endif // WT_THREADED

WebSession::WebSession(WebController *controller,
		       const std::string& sessionId,
		       const std::string& sessionPath, Type type,
		       const WebRequest& request)
  : type_(type),
    state_(JustCreated),
    sessionId_(sessionId),
    sessionPath_(sessionPath),
    controller_(controller),
    renderer_(*this),
    env_(this),
    app_(0),
    debug_(false)
{
  /*
   * Obtain the applicationName_ as soon as possible for log().
   */
  deploymentPath_ = request.scriptName();
  applicationUrl_ = deploymentPath_;

  applicationName_ = applicationUrl_;
  baseUrl_ = applicationUrl_;

  std::string::size_type slashpos = applicationName_.find_last_of('/');
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
  if (app_)
    app_->finalize();
  delete app_;

#ifndef WT_TARGET_JAVA
  unlink(sessionPath_.c_str());

  log("notice") << "Session destroyed (#sessions = "
		<< controller_->sessionCount() << ")";
#else // WT_TARGET_JAVA
  log("notice") << "Session destroyed";
#endif // WT_TARGET_JAVA
}

std::string WebSession::docType() const
{
  const bool xhtml = env_.contentType() == WEnvironment::XHTML1;

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

void WebSession::setState(State state, int timeout)
{
#ifdef WT_TARGET_JAVA
  synchronized (stateMutex_)
#endif
  {
#ifdef WT_THREADED
    boost::mutex::scoped_lock stateLock(stateMutex_);
#endif // WT_THREADED

    if (state_ != Dead) {
      state_ = state;

#ifndef WT_TARGET_JAVA
      expire_ = Time() + timeout*1000;
#endif // WT_TARGET_JAVA
    }
  }
}

void WebSession::init(const WebRequest& request)
{
  env_.init(request);

  const std::string *hashE = request.getParameter("_");

  // the session query is used for all requests (except for reload if the
  // session is not in the main url), to prevent CSRF.
  sessionQuery_ = "?wtd=" + sessionId_;

  applicationUrl_ += sessionQuery_;
  absoluteBaseUrl_ = env_.urlScheme() + "://" + env_.hostName() + baseUrl_;

  bookmarkUrl_ = applicationName_;

  if (applicationName_.empty())
    bookmarkUrl_ = baseUrl_ + applicationName_;

  if (type() == WidgetSet) {
    /*
     * We are embedded in another website: we only use absolute URLs.
     */
    applicationUrl_ = env_.urlScheme() + "://" + env_.hostName()
      + applicationUrl_;

    bookmarkUrl_ = absoluteBaseUrl_ + bookmarkUrl_;
  }

  std::string path = request.pathInfo();
  if (path.empty() && hashE)
    path = *hashE;
  env_.setInternalPath(path);
}

std::string WebSession::bootstrapUrl(const WebResponse& response,
				     BootstrapOption option) const
{
  if (response.pathInfo().empty())
    return mostRelativeUrl();
  else {
    switch (option) {
    case KeepInternalPath:
      if (applicationName_.empty()) {
	std::string internalPath
	  = app_ ? app_->internalPath() : env_.internalPath();

	if (internalPath.length() > 1)
	  return appendSessionQuery("?_="
				    + DomElement::urlEncode(internalPath));
      }

      return appendSessionQuery("");
    case ClearInternalPath:
      return appendSessionQuery(baseUrl_ + applicationName_);
    default:
      assert(false);
    }
  }

  return std::string();
}

std::string WebSession::mostRelativeUrl(const std::string& internalPath) const
{
  return appendSessionQuery(bookmarkUrl(internalPath));
}

std::string WebSession::appendSessionQuery(const std::string& url) const
{
#ifndef WT_TARGET_JAVA
  std::string result = url;
#else
  std::string result = Handler::instance()->response()->encodeURL(url);
#endif // WT_TARGET_JAVA

  if (env_.agentIsSpiderBot())
    return result;

  std::size_t questionPos = result.find('?');

  if (questionPos == std::string::npos)
    return result + sessionQuery_;
  else if (questionPos == result.length() - 1)
    return result + sessionQuery_.substr(1);
  else
    return result + '&' + sessionQuery_.substr(1);
}

std::string WebSession::bookmarkUrl() const
{
  if (app_)
    return bookmarkUrl(app_->internalPath());
  else
    return bookmarkUrl(env_.internalPath());
}

std::string WebSession::bookmarkUrl(const std::string& internalPath) const
{
  std::string result = bookmarkUrl_;

  // Without Ajax, we either should use an absolute URL, or a relative
  // url that takes into account the internal path of the current request.
  //
  // For now, we make an absolute URL, and will fix this in Wt 3.0 since
  // there we always know the current request
  if (!env_.ajax() && result.find("://") == std::string::npos
      && (env_.internalPath().length() > 1 || internalPath.length() > 1))
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
      return baseUrl + "?_=" + DomElement::urlEncode(internalPath);
    else
      return baseUrl + DomElement::urlEncode(internalPath);
  }
}

bool WebSession::start()
{
  try {
    app_ = controller_->doCreateApplication(this);
  } catch (std::exception& e) {
    app_ = 0;

    kill();
    throw e;
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
  WebRequest *request = Handler::instance()->request();
  if (request)
    return request->envValue(varName);
  else
    return "";
}

void WebSession::kill()
{
#ifdef WT_TARGET_JAVA
  synchronized(stateMutex_)
#endif
  {
#ifdef WT_THREADED
    boost::mutex::scoped_lock stateLock(stateMutex_);
#endif // WT_THREADED

    state_ = Dead;

    /*
     * Unlock the recursive eventloop that may be pending.
     */
#ifdef WT_THREADED
    recursiveEventDone_.notify_all();
#endif // WT_THREADED

    if (handlers_.empty()) {
#ifdef WT_THREADED
      // we may unlock because the session has already been removed
      // from the sessions list, and thus, once the list is empty it is
      // guaranteed to stay empty.
      stateLock.unlock();
#endif // WT_THREADED

      delete this;
    }
  }
}

void WebSession::setDebug(bool debug)
{
  debug_ = debug;
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

WebSession::Handler::Handler(WebSession& session)
  : session_(session),
    request_(0),
    response_(0),
    eventLoop_(false),
    killed_(false)
{
  init();
}

WebSession::Handler::Handler(WebSession& session,
			     WebRequest& request, WebResponse& response)
  : session_(session),
    request_(&request),
    response_(&response),
    eventLoop_(false),
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
  return threadHandler_.get() ? *threadHandler_ : 0;
#else
  return threadHandler_;
#endif // WT_THREADED
#endif // WT_TARGET_JAVA
}

void WebSession::Handler::init()
{
#ifdef WT_TARGET_JAVA
  threadHandler_.reset(this);
#else
#ifdef WT_THREADED
  lock_ = 0;
  boost::mutex::scoped_lock stateLock(session_.stateMutex_);
  session_.handlers_.push_back(this);

  if (threadHandler_.get())
    prevHandlerPtr_ = threadHandler_.release();
  else
    prevHandlerPtr_ = 0;

  handlerPtr_ = this;
  threadHandler_.reset(&handlerPtr_);
#else
  threadHandler_ = this;
#endif // WT_THREADED
#endif // WT_TARGET_JAVA
}

void WebSession::Handler::attachThreadToSession(WebSession& session)
{
#ifdef WT_THREADED
  Handler *handler = instance();
  threadHandler_.reset(&handler->handlerPtr_);
#else
  session.log("error") <<
    "attachThreadToSession() requires that Wt is built with threading enabled";
#endif // WT_THREADED
}

WebSession::Handler::~Handler()
{
#ifdef WT_TARGET_JAVA
  threadHandler_.reset(0);
#else
  {
#ifdef WT_THREADED
    boost::mutex::scoped_lock stateLock(session_.stateMutex_);
#endif // WT_THREADED
    Utils::erase(session_.handlers_, this);
  }

  if (session_.handlers_.size() == 0)
    session_.hibernate();

  if (killed_)
    session_.kill();

#ifdef WT_THREADED
  threadHandler_.release();

  if (prevHandlerPtr_)
    threadHandler_.reset(prevHandlerPtr_);
#endif // WT_THREADED
#endif // WT_TARGET_JAVA
}

void WebSession::Handler::killSession()
{
  killed_ = true;
  session_.state_ = Dead;
}

#ifdef WT_THREADED
void WebSession::Handler::setLock(boost::mutex::scoped_lock *lock)
{
  lock_ = lock;
}
#endif // WT_THREADED

void WebSession::Handler::swapRequest(WebRequest *request,
				      WebResponse *response)
{
  request_ = request;
  response_ = response;
}

void WebSession::Handler::setEventLoop(bool how)
{
  eventLoop_ = how;
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
  Handler *handler = Handler::instance();
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

void WebSession::doRecursiveEventLoop(const std::string& javascript)
{
#ifndef WT_THREADED
  log("error") << "Cannot do recursive event loop without threads";
#else // WT_THREADED
  /*
   * Locate the handler, and steal its pending response.
   */
  Handler *handler = findEventloopHandler(0);

  if (handler == 0)
    throw WtException("doRecursiveEventLoop(): inconsistent state");

  if (handler->response()) {
    /*
     * Finish the request.
     */
    handler->session()->render(*handler, app_->environment().ajax() 
			       ? WebRenderer::UpdateResponse
			       : WebRenderer::FullResponse);
    handler->response()->out() << javascript;
    handler->response()->flush(WebResponse::ResponseDone);

    /*
     * Remove the request from the handler.
     */
    handler->swapRequest(0, 0);
  }

  /*
   * Release session mutex lock, wait for recursive event response,
   * and retake the session mutex lock.
   */
  recursiveEventDone_.wait(*handler->lock());

  if (state_ == Dead)
    throw WtException("doRecursiveEventLoop(): session was killed");
  
#endif // WT_THREADED
}

void WebSession::unlockRecursiveEventLoop()
{
#ifndef WT_TARGET_JAVA
  /*
   * Locate both the current and previous event loop handler.
   */
  Handler *handler = findEventloopHandler(0);
  Handler *handlerPrevious = findEventloopHandler(1);

  handlerPrevious->swapRequest(handler->request(), handler->response());
  handler->swapRequest(0, 0);

#ifdef WT_THREADED
  recursiveEventDone_.notify_one();
#endif // WT_THREADED

#endif // WT_TARGET_JAVA
}

WebSession::Handler *WebSession::findEventloopHandler(int index)
{
#ifndef WT_TARGET_JAVA

#ifdef WT_THREADED
  boost::mutex::scoped_lock stateLock(stateMutex_);
#endif // WT_THREADED

  for (int i = handlers_.size() - 1; i >= 0; --i) {
    if (handlers_[i]->eventLoop())
      if (index == 0)
	return handlers_[i];
      else
	--index;
  }
#endif // WT_TARGET_JAVA

  return 0;
}

bool WebSession::handleRequest(WebRequest& request, WebResponse& response)
{
  Handler handler(*this, request, response);

  bool flushed = false;

  /*
   * -- Start critical section exclusive access to session
   */
#ifdef WT_TARGET_JAVA
  synchronized(mutex_)
#endif
  {
#ifdef WT_THREADED
    boost::mutex::scoped_lock sessionLock(mutex_);
    handler.setLock(&sessionLock);
#endif // WT_THREADED

  WebRenderer::ResponseType responseType = WebRenderer::FullResponse;

  const std::string *wtdE = request.getParameter("wtd");
  const std::string *signalE = getSignal(request, "e0");
  const std::string *resourceE = request.getParameter("resource");

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
      && state_ != JustCreated && (signalE || resourceE)) {
    handler.response()->setContentType("text/html");
    handler.response()->out()
      << "<html><head></head><body>CSRF prevention</body></html>";
  } else
    try {
      const std::string *requestE = request.getParameter("request");

      switch (state_) {
      case JustCreated: {
	switch (type_) {
	case Application:
	  init(request); // env, url/internalpath

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
	      || (requestE && *requestE == "script")) {
	    log("notice") << "Signal from dead session, sending reload.";
	    renderer_.letReloadJS(*handler.response(), true);
	    handler.killSession();
	  } else if (request.getParameter("resource")) {
	    log("notice") << "Not serving bootstrap for resource.";
	    handler.killSession();
	    handler.response()->setContentType("text/html");
	    handler.response()->out() << "<html><head></head><body></body></html>";
	  } else {
	    if (env_.agentIsSpiderBot()) {
	      env_.doesJavaScript_ = false;
	      env_.doesAjax_ = false;
	      env_.doesCookies_ = false;

	    if (!start())
	      throw WtException("Could not start application.");

	    if (env_.internalPath() != "/") {
	      app_->setInternalPath("/");
	      app_->notify(WEvent(handler, WEvent::HashChange,
				  env_.internalPath()));
	    }

	    app_->notify(WEvent(handler, WEvent::Render,
				WebRenderer::FullResponse));

	    handler.killSession();
	  } else {
	    setState(Bootstrap, 10);
	    renderer_.serveBootstrap(*handler.response());
	  }
	}
	break;
      case WidgetSet:
	init(request); // env, url/internalpath
	env_.doesJavaScript_ = true;
	env_.doesAjax_ = true;

	if (!start())
	  throw WtException("Could not start application.");

	app_->notify(WEvent(handler, WEvent::Render,
			    WebRenderer::FullResponse));
      }
      break;
    }
    case Bootstrap: {
      const std::string *jsE = request.getParameter("js");

      // How could this happen?
      if (!jsE) {
	setState(Bootstrap, 10);
	renderer_.serveBootstrap(*handler.response());
	break;
      }

      /*
       * The bootstrap page is the entire page in AJAX. When reloading the
       * full page, we therefore end up here again. But we should not restart
       * the application ofcourse...
       */
      if (!app_) {
	const std::string *ajaxE = request.getParameter("ajax");
	const std::string *hashE = request.getParameter("_");
	const std::string *scaleE = request.getParameter("scale");

	env_.doesJavaScript_= *jsE == "yes";
	env_.doesAjax_ = env_.doesJavaScript_ && ajaxE && *ajaxE == "yes";
	env_.doesCookies_ = request.headerValue("Cookie").empty();

	if (env_.doesAjax_ && !request.pathInfo().empty()) {
	  std::string url = baseUrl() + applicationName();
	  url += '#' + env_.internalPath();
	  redirect(url);
	  renderer_.serveMainWidget(*handler.response(),
				    WebRenderer::FullResponse);

	  log("notice") << "Redirecting to canonical URL: " << url;
	  handler.killSession();
	  break;
	}

	try {
	  env_.dpiScale_ = scaleE ? boost::lexical_cast<double>(*scaleE) : 1;
	} catch (boost::bad_lexical_cast &e) {
	  env_.dpiScale_ = 1;
	}

	// the internal path, when present as an anchor (#), is only
	// conveyed in the second request
	if (hashE)
	  env_.setInternalPath(*hashE);

	if (!start())
	  throw WtException("Could not start application.");

	if (env_.internalPath() != "/") {
	  app_->setInternalPath("/");
	  app_->notify(WEvent(handler, WEvent::HashChange,
			      env_.internalPath()));
	}
      } else if ((*jsE == "no") && env_.doesAjax_) {
	// reload but disabled AJAX support: give user a new session
	// FIX this: redirect using Redirect result.

	handler.response()->setRedirect(baseUrl() + applicationName()
				       + '#' + env_.internalPath());
	handler.killSession();
	break;
      } else
	if (!request.pathInfo().empty())
	  app_->notify(WEvent(handler, WEvent::HashChange, request.pathInfo()));

      app_->notify(WEvent(handler, WEvent::Render, WebRenderer::FullResponse));
      break;
    }
    case Loaded: {
      responseType = (signalE && !requestE && env_.doesAjax_)
	? WebRenderer::UpdateResponse
	: WebRenderer::FullResponse;

      try {
	if (request.postDataExceeded())
	  app_->requestTooLarge().emit(request.postDataExceeded());
      } catch (std::exception& e) {
	throw WtException("Exception in WApplication::requestTooLarge", e);
      } catch (...) {
	throw WtException("Exception in WApplication::requestTooLarge");
      }

      if (requestE && *requestE == "script") {
	// an unexpected second request for the "script": do not send
	// anything
	handler.response()->setContentType("text/plain");

	break;
      }

      if (!resourceE && !signalE) {
	log("notice") << "Refreshing session";
	app_->notify(WEvent(handler, WEvent::Refresh));
	if (env_.doesAjax_) {
	  setState(Bootstrap, 10);
	  renderer_.serveBootstrap(*handler.response());
	  break;
	}
      }

      const std::string *updateIdE = request.getParameter("updateId");
      try {
	if (updateIdE)
	  handler.response()->setId(boost::lexical_cast<int>(*updateIdE));
      } catch (const boost::bad_lexical_cast& e) {
	log("error") << "Could not parse updateId: " << *updateIdE;
      }

      env_.urlScheme_ = request.urlScheme();

      if (resourceE && *resourceE == "blank") {
	handler.response()->setContentType("text/html");
	handler.response()->out()
	  << "<html><head><title>bhm</title></head><body>&#160;</body></html>";
      } else if (resourceE) {
	WResource *resource = decodeResource(*resourceE);

	if (resource) {
	  try {
	    resource->handle(&request, &response);
	    flushed = true;
	  } catch (std::exception& e) {
	    throw WtException("Exception while streaming resource", e);
	  } catch (...) {
	    throw WtException("Exception while streaming resource");
	  }

	} else {
	  handler.response()->setContentType("text/html");
	  handler.response()->out()
	    << "<html><body><h1>Session timeout.</h1></body></html>";
	}
      } else {
	if (responseType == WebRenderer::FullResponse && !env_.doesAjax_)
	  app_->notify(WEvent(handler, WEvent::HashChange, request.pathInfo()));

	const std::string *hashE = request.getParameter("_");

	if (signalE) {
	  if (*signalE != "res") {
	    //std::cerr << "signal: " << *signalE << std::endl;
	    /*
	     * Special signal values:
	     * 'none' : no event, but perhaps a synchronization
	     * 'load' : load invisible content
	     * 'res'  : after a resource received data
	     */

	    // First propagate form values -- they could be corrupted by
	    // the hash change
	    app_->notify(WEvent(handler, WEvent::PropagateFormValues));

	    // Propagate change in hash. Do it after a phony reload
	    // event, since the reload does not carry actual hash
	    // information
	    if (hashE)
	      app_->notify(WEvent(handler, WEvent::HashChange, *hashE));

	    try {
	      app_->notify(WEvent(handler, WEvent::EmitSignal));
	    } catch (std::exception& e) {
	      throw WtException("Error during event handling", e);
	    } catch (...) {
	      throw WtException("Error during event handling");	    
	    }
	  }
	} else {
	  if (hashE)
	    app_->notify(WEvent(handler, WEvent::HashChange, *hashE));

	  /*
	   * Is a reload
	   */
	  try {
	    app_->notify(WEvent(handler, WEvent::Refresh));
	  } catch (std::exception& e) {
	    throw WtException("Exception while refreshing session", e);
	  } catch (...) {
	    throw WtException("Exception while refreshing session");
	  }
	}

	if (handler.response())
	  app_->notify(WEvent(handler, WEvent::Render, responseType));
      }
    }

      break;
    case Dead:
      throw WtException("Internal error: WebSession is dead?");
    }
  } catch (WtException& e) {
    log("fatal") << e.what();

#ifdef WT_TARGET_JAVA
    e.printStackTrace();
#endif // WT_TARGET_JAVA

    handler.killSession();
    renderer_.serveError(*handler.response(), e, responseType);

  } catch (std::exception& e) {
    log("fatal") << e.what();

#ifdef WT_TARGET_JAVA
    e.printStackTrace();
#endif // WT_TARGET_JAVA

    handler.killSession();
    renderer_.serveError(*handler.response(), e, responseType);
  } catch (...) {
    log("fatal") << "Unknown exception.";

    handler.killSession();
    renderer_.serveError(*handler.response(), std::string("Unknown exception"),
			 responseType);
  }

  if (handler.response() && !flushed)
    handler.response()->flush();

#ifdef WT_TARGET_JAVA
  handler.~Handler();
#endif // WT_TARGET_JAVA

  }

  return !handler.sessionDead();
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

void WebSession::notify(const WEvent& e)
{
  WebSession *session = e.session();

  switch (e.type) {
  case WEvent::EmitSignal:
    session->notifySignal(e);
    break;
  case WEvent::Refresh:
    session->app_->refresh();
    break;
  case WEvent::Render:
    session->render(e.handler, e.responseType);
    break;
  case WEvent::HashChange:
    session->app_->changeInternalPath(e.hash);
    break;
  case WEvent::PropagateFormValues:
    session->propagateFormValues(e);
    break;
  }
}

void WebSession::render(Handler& handler, WebRenderer::ResponseType type)
{
  try {
    if (!env_.doesJavaScript_ && (type == WebRenderer::FullResponse))
      checkTimers();
  } catch (std::exception& e) {
    throw WtException("Exception while triggering timers", e);
  } catch (...) {
    throw WtException("Exception while triggering timers");
  }

  if (app_->isQuited())
    handler.killSession();

  renderer_.serveMainWidget(*handler.response(), type);
  setState(WebSession::Loaded, controller_->configuration().sessionTimeout());
}

void WebSession::propagateFormValues(const WEvent& e)
{
  const WebRequest& request = *e.handler.request();
  std::vector<WObject *> formObjects = renderer_.formObjects();

  for (unsigned i = 0; i < formObjects.size(); ++i) {
    WObject *obj = formObjects[i];

    if (!request.postDataExceeded())
      obj->setFormData(getFormData(request, obj->formName()));
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

void WebSession::notifySignal(const WEvent& e)
{
  WebSession::Handler& handler = e.handler;

  // Save pending changes (e.g. from resource completion)
  renderer_.saveChanges();

  for (unsigned i = 0;; ++i) {
    if (!handler.request())
      return;

    const WebRequest& request = *handler.request();

    std::string se = 'e' + boost::lexical_cast<std::string>(i);
    const std::string *signalE = getSignal(request, se);

    if (!signalE)
      return;

    if (*signalE == "hash") {
      const std::string *hashE = request.getParameter("_");
      if (hashE)
	app_->changeInternalPath(*hashE);
    } else if (*signalE == "none") {
#ifndef WT_TARGET_JAVA
      // We will want invisible changes now too.
      renderer_.setVisibleOnly(false);
      // not idle timeout timer
      if (app_->updatesEnabled())
	app_->triggerUpdate();
#endif // WT_TARGET_JAVA
    } else {
      if (*signalE != "load" && *signalE != "res") {
	handler.setEventLoop(true);

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

}
