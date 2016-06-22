/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/Utils"
#include "Wt/WApplication"
#include "Wt/WCombinedLocalizedStrings"
#include "Wt/WContainerWidget"
#include "Wt/WException"
#include "Wt/WFormWidget"
#ifndef WT_TARGET_JAVA
#include "Wt/WIOService"
#endif
#include "Wt/WResource"
#include "Wt/WServer"
#include "Wt/WTimerWidget"
#include "Wt/Http/Request"

#include "CgiParser.h"
#include "Configuration.h"
#include "DomElement.h"
#include "WebController.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WebSocketMessage.h"
#include "WebUtils.h"

#include <boost/algorithm/string.hpp>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef WT_WIN32
#include <process.h>
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

  bool isAbsoluteUrl(const std::string& url) {
    return url.find(":") != std::string::npos;
  }

  std::string host(const std::string& url) {
    std::size_t pos = 0;
    for (unsigned i = 0; i < 3; ++i) { 
      pos = url.find('/', pos);
      if (pos == std::string::npos)
	return url;
      else
	++pos;
    }
    return url.substr(0, pos - 1);
  }

  inline std::string str(const char *v) {
    return v ? std::string(v) : std::string();
  }

  inline bool isEqual(const char *s1, const char *s2) {
#ifdef WT_TARGET_JAVA
    if (s1 == 0) {
      return s2 == 0;
    } else {
      return std::string(s1) == s2;
    }
#else
    return strcmp(s1, s2) == 0;
#endif
  }
}

namespace Wt {

LOGGER("Wt");

#ifdef WT_BOOST_THREADS
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
    sessionIdChanged_(false),
    sessionIdCookieChanged_(false),
    sessionIdInUrl_(false),
    controller_(controller),
    renderer_(*this),
    asyncResponse_(0),
    webSocket_(0),
    bootStyleResponse_(0),
    canWriteWebSocket_(false),
    pollRequestsIgnored_(0),
    progressiveBoot_(false),
    deferredRequest_(0),
    deferredResponse_(0),
    deferCount_(0),
#ifdef WT_TARGET_JAVA
    recursiveEvent_(mutex_.newCondition()),
    recursiveEventDone_(mutex_.newCondition()),
    newRecursiveEvent_(0),
    updatesPendingEvent_(mutex_.newCondition()),
#else
    newRecursiveEvent_(0),
#endif
    updatesPending_(false),
    triggerUpdate_(false),
    embeddedEnv_(this),
    app_(0),
    debug_(controller_->configuration().debug()),
    recursiveEventHandler_(0)
{
  env_ = env ? env : &embeddedEnv_;

  /*
   * Obtain the applicationName_ as soon as possible for log().
   */
  if (request)
    applicationUrl_ = request->scriptName();
  else
    applicationUrl_ = "/";

  deploymentPath_ = applicationUrl_;

  std::string::size_type slashpos = deploymentPath_.rfind('/');
  if (slashpos != std::string::npos) {
    basePath_ = deploymentPath_.substr(0, slashpos + 1);
    applicationName_ = deploymentPath_.substr(slashpos + 1);
  } else { // ?
    basePath_ = "";
    applicationName_ = applicationUrl_; 
  }

#ifndef WT_TARGET_JAVA
  LOG_INFO("session created (#sessions = " <<
	   (controller_->sessionCount() + 1) << ")");

  expire_ = Time() + 60*1000;
#endif // WT_TARGET_JAVA

  if (controller_->configuration().sessionIdCookie()) {
    sessionIdCookie_ = WRandom::generateId();
    sessionIdCookieChanged_ = true;
    renderer().setCookie("Wt" + sessionIdCookie_, "1", Wt::WDateTime(), "", "",
			 env_->urlScheme() == "https");
  }
}

void WebSession::setApplication(WApplication *app)
{
  app_ = app;
}

void WebSession::deferRendering()
{
  if (!deferredRequest_) {
    Handler *handler = WebSession::Handler::instance();
    deferredRequest_ = handler->request();
    deferredResponse_ = handler->response();
    handler->setRequest(0, 0);
  }

  ++deferCount_;
}

void WebSession::resumeRendering()
{
  if (--deferCount_ == 0) {
    Handler *handler = WebSession::Handler::instance();
    handler->setRequest(deferredRequest_, deferredResponse_);
    deferredRequest_ = 0;
    deferredResponse_ = 0;
  }
}

void WebSession::setTriggerUpdate(bool update)
{
  triggerUpdate_ = update;
}

#ifndef WT_TARGET_JAVA
WLogger& WebSession::logInstance() const
{
    return controller_->server()->logger();
}

WLogEntry WebSession::log(const std::string& type) const
{
  WLogEntry e = controller_->server()->logger().entry(type);

#ifndef WT_TARGET_JAVA
  e << WLogger::timestamp << WLogger::sep << getpid() << WLogger::sep
    << '[' << deploymentPath_ << ' ' << sessionId()
    << ']' << WLogger::sep << '[' << type << ']' << WLogger::sep;
#endif // WT_TARGET_JAVA

  return e;
}
#endif // WT_TARGET_JAVA

WebSession::~WebSession()
{
  /*
   * From here on, we cannot create a shared_ptr to this session. Therefore,
   * app_ uses a weak_ptr to this session for which lock() returns an empty
   * shared pointer.
   */
  state_ = Dead;

#ifndef WT_TARGET_JAVA
  Handler handler(this);

  if (app_)
    app_->notify
      (WEvent(WEvent::Impl(&handler,
			   boost::bind(&WApplication::finalize, app_))));

  delete app_;
  app_ = 0;
#endif // WT_TARGET_JAVA

  if (asyncResponse_) {
    asyncResponse_->flush();
    asyncResponse_ = 0;
  }

  if (webSocket_) {
    webSocket_->flush();
    webSocket_ = 0;
  }

  if (deferredResponse_) {
    deferredResponse_->flush();
    deferredResponse_ = 0;
  }    

#ifdef WT_BOOST_THREADS
  updatesPendingEvent_.notify_one();
#endif // WT_BOOST_THREADS

  flushBootStyleResponse();

  controller_->configuration().registerSessionId(sessionId_, std::string());
  controller_->sessionDeleted();

#ifndef WT_TARGET_JAVA
  LOG_INFO("session destroyed (#sessions = " << controller_->sessionCount()
	   << ")");
#endif // WT_TARGET_JAVA

}

#ifdef WT_TARGET_JAVA
void WebSession::destruct()
{
  if (asyncResponse_) {
    asyncResponse_->flush();
    asyncResponse_ = 0;
  }

  if (deferredResponse_) {
    deferredResponse_->flush();
    deferredResponse_ = 0;
  }    

  mutex_.lock();
  updatesPendingEvent_.notify_one();
  mutex_.unlock();

  flushBootStyleResponse();
}
#endif // WT_TARGET_JAVA

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
    return
#ifdef HTML4_DOCTYPE
      "<!DOCTYPE html PUBLIC "
      "\"-//W3C//DTD HTML 4.01 Transitional//EN\" "
      "\"http://www.w3.org/TR/html4/loose.dtd\">";
#else
      "<!DOCTYPE html>"; // HTML5 hoeray
#endif
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

void WebSession::setExpectLoad()
{
  if (controller_->configuration().ajaxPuzzle())
    setState(ExpectLoad, controller_->configuration().bootstrapTimeout());
  else
    setLoaded();
}

void WebSession::setState(State state, int timeout)
{
#ifdef WT_THREADED
  // this assertion is not true for when we are working from an attached
  // thread: that thread does not have an associated handler, but its contract
  // dictates that it should work on behalf of a thread that has the lock.
  //assert(WebSession::Handler::instance()->haveLock());
#endif // WT_THREADED

  if (state_ != Dead) {
    state_ = state;

    LOG_DEBUG("Setting to expire in " << timeout << "s");

#ifndef WT_TARGET_JAVA
    if (controller_->configuration().sessionTimeout() != -1)
      expire_ = Time() + timeout*1000;
#endif // WT_TARGET_JAVA
  }
}

std::string WebSession::sessionQuery() const
{
  std::string result ="?wtd=" + DomElement::urlEncodeS(sessionId_);
  if (type() == WidgetSet)
    result += "&wtt=widgetset";
  return result;
}

void WebSession::init(const WebRequest& request)
{
  env_->init(request);

  const std::string *hashE = request.getParameter("_");

  absoluteBaseUrl_ = env_->urlScheme() + "://" + env_->hostName() + basePath_;

  bool useAbsoluteUrls;
#ifndef WT_TARGET_JAVA
  useAbsoluteUrls 
    = app_->readConfigurationProperty("baseURL", absoluteBaseUrl_);
#else
  std::string* absoluteBaseUrl 
    = app_->readConfigurationProperty("baseURL", absoluteBaseUrl_);
  if (absoluteBaseUrl != &absoluteBaseUrl_) {
    absoluteBaseUrl_ = *absoluteBaseUrl;
    useAbsoluteUrls = true;
  } else {
    useAbsoluteUrls = false;
  }
#endif

  if (useAbsoluteUrls) {
    std::string::size_type slashpos = absoluteBaseUrl_.rfind('/');
    if (slashpos != std::string::npos
	&& slashpos != absoluteBaseUrl_.length() - 1)
      absoluteBaseUrl_ = absoluteBaseUrl_.substr(0, slashpos + 1);

    slashpos = absoluteBaseUrl_.find("://");

    if (slashpos != std::string::npos) {
      slashpos = absoluteBaseUrl_.find("/", slashpos + 3);
      if (slashpos != std::string::npos) {
	deploymentPath_ = absoluteBaseUrl_.substr(slashpos) + applicationName_;
      }
    }
  }

  bookmarkUrl_ = applicationName_;

  if (type() == WidgetSet || useAbsoluteUrls) {
    applicationUrl_ = absoluteBaseUrl_ + applicationName_;
    bookmarkUrl_ = applicationUrl_;
  }

  std::string path = request.pathInfo();
  if (path.empty() && hashE)
    path = *hashE;

  env_->setInternalPath(path);
  pagePathInfo_ = request.pathInfo();

  // Cache document root
  docRoot_ = getCgiValue("DOCUMENT_ROOT");
}

bool WebSession::useUglyInternalPaths() const
{
#ifndef WT_TARGET_JAVA
  /*
   * We need ugly ?_= internal paths if the server does not route
   * /app/foo to an application deployed as /app/
   */
  if (applicationName_.empty() && controller_->server()) {
    Configuration& conf = controller_->configuration();
    return conf.useSlashExceptionForInternalPaths();
  } else
    return false;
#else
  return false;
#endif
}

std::string WebSession::bootstrapUrl(const WebResponse& response,
				     BootstrapOption option) const
{
  switch (option) {
  case KeepInternalPath: {
    std::string url;

    std::string internalPath
      = app_ ? app_->internalPath() : env_->internalPath();

    if (useUglyInternalPaths()) {
      if (internalPath.length() > 1)
	url = "?_=" + DomElement::urlEncodeS(internalPath, "#/");

      if (isAbsoluteUrl(applicationUrl_))
	url = applicationUrl_ + url;
    } else {
      if (!isAbsoluteUrl(applicationUrl_)) {
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

	  url = ""; /* lastPart; */
	} else
	  url = applicationName_;
      } else {
	if (applicationName_.empty() && internalPath.length() > 1)
	  internalPath = internalPath.substr(1);

	url = applicationUrl_ + internalPath;
      }
    }

    return appendSessionQuery(url);
  }
  case ClearInternalPath: {
    std::string url;
    if (applicationName_.empty()) {
      url = fixRelativeUrl(".");
      url = url.substr(0, url.length() - 1);
    } else
      url = fixRelativeUrl(applicationName_);

    return appendSessionQuery(url);
  }
  default:
    assert(false);
  }

  return std::string();
}

std::string WebSession::fixRelativeUrl(const std::string& url) const
{
  if (isAbsoluteUrl(url))
    return url;

  if (url.length() > 0 && url[0] == '#') {
    if (!isAbsoluteUrl(applicationUrl_))
      return url;
    else
      // we have <href base=...> which requires us to put the application
      // name before a named anchor
      return applicationName_ + url;
  }

  if (!isAbsoluteUrl(applicationUrl_)) {
    if (url.empty() || url[0] == '/')
      return url;
    else if (!env_->publicDeploymentPath_.empty()) {
      std::string dp = env_->publicDeploymentPath_;
      if (url[0] != '?') {
	std::size_t s = dp.rfind('/');
	dp = dp.substr(0, s + 1);
      }
      return dp + url;
    } else {
      /*
       * The public deployment path may lack if:
       *  - we are a widget set script, but then we should have absolute
       *    applicationUrl and internal paths are not really going to work
       *  - we are a plain HTML session. but then we are not hashing internal
       *    paths, so first condition should never be met
       */
      if (env_->internalPathUsingFragments())
	return url;
      else {
	std::string rel = "";
	std::string pi = pagePathInfo_;

	for (unsigned i = 0; i < pi.length(); ++i) {
	  if (pi[i] == '/')
	    rel += "../";
	}

	return rel + url;
      }
    }
  } else
    return makeAbsoluteUrl(url);
}

std::string WebSession::makeAbsoluteUrl(const std::string& url) const
{
  if (isAbsoluteUrl(url))
    return url;
  else {
    if (url.empty() || url[0] != '/')
      return absoluteBaseUrl_ + url;
    else
      return host(absoluteBaseUrl_) + url;
  }
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
  else {
    /*
     * This may happen if we are inside a WServer::posted() function.
     * Unfortunately, then we cannot use Servlet API to URL encode.
     */
    questionPos = result.find('?');
    return result.substr(0, questionPos) + ";jsessionid=" + sessionId()
      + result.substr(questionPos);
  }
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

  return appendInternalPath(result, internalPath);
}

std::string WebSession::appendInternalPath(const std::string& baseUrl,
					   const std::string& internalPath)
  const
{
  if (internalPath.empty() || internalPath == "/")
    if (baseUrl.empty())
      return ".";
    else
      return baseUrl;
  else {
    if (useUglyInternalPaths())
      return baseUrl + "?_=" + DomElement::urlEncodeS(internalPath, "#/");
    else {
      if (applicationName_.empty())
	return baseUrl + DomElement::urlEncodeS(internalPath.substr(1), "#/");
      else
	return baseUrl + DomElement::urlEncodeS(internalPath, "#/");
    }
  }
}

bool WebSession::start(WebResponse *response)
{
  try {
    app_ = controller_->doCreateApplication(this);
    if (!app_->internalPathValid_)
      if (response->responseType() == WebResponse::Page)
	response->setStatus(404);
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
    return str(request->envValue(varName.c_str()));
  else if(varName == "DOCUMENT_ROOT")
	return docRoot_;
  else 
    return std::string();
}

std::string WebSession::getCgiHeader(const std::string& headerName) const
{
  WebRequest *request = WebSession::Handler::instance()->request();
  if (request)
    return str(request->headerValue(headerName.c_str()));
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
  : nextSignal(-1),
    prevHandler_(0),
    session_(0),
    request_(0),
    response_(0),
    killed_(false)
{
  init();
}

WebSession::Handler::Handler(boost::shared_ptr<WebSession> session,
			     LockOption lockOption)
  : nextSignal(-1),
#ifndef WT_TARGET_JAVA
    sessionPtr_(session),
#endif // WT_TARGET_JAVA
#ifdef WT_THREADED
    lock_(session->mutex_, boost::defer_lock),
#endif // WT_THREADED
    prevHandler_(0),
    session_(session.get()),
    request_(0),
    response_(0),
    killed_(false)
{
  switch (lockOption) {
  case NoLock:
    break;
  case TakeLock:
#ifdef WT_THREADED
    lockOwner_ = boost::this_thread::get_id();
    lock_.lock();
#endif
#ifdef WT_TARGET_JAVA
    session->mutex().lock();
#endif
    break;
  case TryLock:
#ifdef WT_THREADED
    if (lock_.try_lock())
      lockOwner_ = boost::this_thread::get_id();
#endif
#ifdef WT_TARGET_JAVA
    session->mutex().try_lock();
#endif
    break;
  }

  init();
}

WebSession::Handler::Handler(WebSession *session)
  : nextSignal(-1),
#ifdef WT_THREADED
    lock_(session->mutex_),
#endif // WT_THREADED
    prevHandler_(0),
    session_(session),
    request_(0),
    response_(0),
    killed_(false)
{
#ifdef WT_THREADED
  lockOwner_ = boost::this_thread::get_id();
#endif
#ifdef WT_TARGET_JAVA
  session->mutex().lock();
#endif // WT_TARGET_JAVA

  init();
}

WebSession::Handler::Handler(boost::shared_ptr<WebSession> session,
			     WebRequest& request, WebResponse& response)
  : nextSignal(-1),  
#ifndef WT_TARGET_JAVA
    sessionPtr_(session),
#endif // WT_TARGET_JAVA
#ifdef WT_THREADED
    lock_(session->mutex_),
#endif // WT_THREADED
    prevHandler_(0),
    session_(session.get()),
    request_(&request),
    response_(&response),
    killed_(false)
{
#ifdef WT_THREADED
  lockOwner_ = boost::this_thread::get_id();
#endif
#ifdef WT_TARGET_JAVA
  session->mutex().lock();
#endif

  init();
}

WebSession::Handler *WebSession::Handler::instance()
{
#ifdef WT_BOOST_THREADS
  return threadHandler_.get();
#else
  return threadHandler_;
#endif
}

bool WebSession::Handler::haveLock() const
{
#ifdef WT_THREADED
  return lock_.owns_lock();
#else
#ifdef WT_TARGET_JAVA
  return session_->mutex().owns_lock();
#else
  return true;
#endif
#endif
}

void WebSession::Handler::unlock()
{
#ifndef WT_TARGET_JAVA
  if (haveLock()) {
    Utils::erase(session_->handlers_, this);
#ifdef WT_THREADED
    lock_.unlock();
#endif // WT_THREADED
  }
#endif // WT_TARGET_JAVA
}

void WebSession::Handler::init()
{
  prevHandler_ = attachThreadToHandler(this);

#ifndef WT_TARGET_JAVA
  if (haveLock())
    session_->handlers_.push_back(this);
#endif
}

WebSession::Handler *
WebSession::Handler::attachThreadToHandler(Handler *handler)
{
  WebSession::Handler *result;

#ifdef WT_BOOST_THREADS
  result = threadHandler_.release();
  threadHandler_.reset(handler);
#else
  result = threadHandler_;
  threadHandler_ = handler;
#endif

  return result;
}

bool WebSession::attachThreadToLockedHandler()
{
#if !defined(WT_TARGET_JAVA)
  /*
   * We assume that another handler has already locked this session for us.
   * We just need to find it.
   */
  for (unsigned i = 0; i < handlers_.size(); ++i)
    if (handlers_[i]->haveLock()) {
      WebSession::Handler::attachThreadToHandler(handlers_[i]);
      return true;
    }

  return false;
#else
  Handler::attachThreadToHandler(new Handler(this, Handler::NoLock));
  return true;
#endif
}

void WebSession
::Handler::attachThreadToSession(boost::shared_ptr<WebSession> session)
{
  attachThreadToHandler(0);

#ifdef WT_BOOST_THREADS
  if (!session.get())
    return;

  /*
   * It may be that we still need to attach to a session while it is being
   * destroyed ? I'm not sure why this is useful, but cannot see anything
   * wrong about it either ?
   */
  if (session->state_ == Dead)
    LOG_WARN_S(session, "attaching to dead session?");

  if (!session.get()->attachThreadToLockedHandler()) {
    /*
     * We actually have two scenarios:
     * - attachThread() once to have WApplication::instance() work. This will
     *   give the warning, and will not work reliably !
     * - attachThread() in the wtwithqt case should execute what we have above
     */
    LOG_WARN_S(session,
	       "attachThread(): no thread is holding this application's "
	       "lock ?");
    WebSession::Handler::attachThreadToHandler
      (new Handler(session, Handler::NoLock));
  }
#else
  LOG_ERROR_S(session, "attachThread(): needs Wt built with threading enabled");
#endif
}

ApplicationEvent WebSession::popQueuedEvent()
{
#ifdef WT_BOOST_THREADS
#ifndef WT_TARGET_JAVA
  boost::mutex::scoped_lock lock(eventQueueMutex_);
#else
  eventQueueMutex_.lock();
#endif // WT_TARGET_JAVA
#endif // WT_BOOST_THREADS

  ApplicationEvent result;

  LOG_DEBUG("popQueuedEvent(): " << eventQueue_.size());

  if (!eventQueue_.empty()) {
    result = eventQueue_.front();
    eventQueue_.pop_front();
  }

#ifdef WT_TARGET_JAVA
  eventQueueMutex_.unlock();
#endif // WT_TARGET_JAVA

  return result;
}

void WebSession::queueEvent(const ApplicationEvent& event)
{
#ifdef WT_BOOST_THREADS
#ifndef WT_TARGET_JAVA
  boost::mutex::scoped_lock lock(eventQueueMutex_);
#else
  eventQueueMutex_.lock();
#endif // WT_TARGET_JAVA
#endif // WT_BOOST_THREADS

  eventQueue_.push_back(event);

  LOG_DEBUG("queueEvent(): " << eventQueue_.size());

#ifdef WT_TARGET_JAVA
  eventQueueMutex_.unlock();
#endif // WT_TARGET_JAVA
}

void WebSession::processQueuedEvents(WebSession::Handler& handler)
{
  for (;;) {
    ApplicationEvent event = popQueuedEvent();

    if (!event.empty()) {
      if (!dead()) {
	externalNotify(WEvent::Impl(&handler, event.function));

	if (app() && app()->isQuited())
	  kill();

	if (dead())
	  controller()->removeSession(event.sessionId);
      } else {
	if (event.fallbackFunction)
	  WT_CALL_FUNCTION(event.fallbackFunction);
      }
    } else
      break;
  }
}

#ifdef WT_TARGET_JAVA
void WebSession::Handler::release()
{
  if (haveLock()) {
    session_->processQueuedEvents(*this);
    if (session_->triggerUpdate_)
      session_->pushUpdates();
    session_->mutex().unlock();
  }

  attachThreadToHandler(prevHandler_);
}
#endif

WebSession::Handler::~Handler()
{
#ifndef WT_TARGET_JAVA
  if (haveLock()) {
    /* We should check that the session state is not dead ? */
    session_->processQueuedEvents(*this);
    if (session_->triggerUpdate_)
      session_->pushUpdates();
    else if (response_ && session_->state_ != Dead)
      session()->render(*this);

    Utils::erase(session_->handlers_, this);
  }

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

void WebSession::Handler::flushResponse()
{
  if (response_) {
    response_->flush();
    setRequest(0, 0);
  }
}

void WebSession::hibernate()
{
  if (app_ && app_->localizedStrings_)
    app_->localizedStrings_->hibernate();
}

EventSignalBase *WebSession::decodeSignal(const std::string& signalId,
					  bool checkExposed) const
{
  EventSignalBase *result = app_->decodeExposedSignal(signalId);

  if (result && checkExposed) {
    WWidget *w = dynamic_cast<WWidget *>(result->sender());
    if (w && !app_->isExposed(w))
      result = 0;
  }

  if (!result && checkExposed) {
    if (app_->justRemovedSignals().find(signalId) ==
	app_->justRemovedSignals().end())
      LOG_ERROR("decodeSignal(): signal '" << signalId << "' not exposed");
  }

  return result;
}

EventSignalBase *WebSession::decodeSignal(const std::string& objectId,
					  const std::string& name,
					  bool checkExposed) const
{
  std::string signalId = app_->encodeSignal(objectId, name);

  return decodeSignal(signalId, checkExposed && name != "resized");
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
  Handler *handler = WebSession::Handler::instance();

#ifndef WT_BOOST_THREADS
  LOG_ERROR("cannot do recursive event loop without threads");
#else

#ifdef WT_TARGET_JAVA
  if (handler->request() && !WebController::isAsyncSupported())
    throw WException("Recursive eventloop requires a Servlet 3.0 "
		     "enabled servlet container and an application "
		     "with async-supported enabled.");
#endif

  /*
   * Finish the request that is being handled
   *
   * It could be that handler does not have a request/response:
   *  if it is actually a long polling server push request.
   *  if we are somehow recursing recursive event loops: e.g. 
   *    processEvents() during exec()
   *
   * In that case, we do not need to finish it.
   */
  if (handler->request())
    handler->session()->notifySignal(WEvent(WEvent::Impl(handler)));
  else
    if (app_->updatesEnabled())
      app_->triggerUpdate();

  if (handler->response())
    handler->session()->render(*handler);

  if (state_ == Dead) {
    recursiveEventHandler_ = 0;
    throw WException("doRecursiveEventLoop(): session was killed");
  }

  /*
   * Register that we are doing a recursive event loop, this is used in
   * handleRequest() to let the recursive event loop do the actual
   * notification.
   */
  Handler *prevRecursiveEventHandler = recursiveEventHandler_;
  recursiveEventHandler_ = handler;
  newRecursiveEvent_ = 0;

  /*
   * Release session mutex lock, wait for recursive event, and retake
   * the session mutex lock.
   */
#ifndef WT_TARGET_JAVA
  if (webSocket_)
    webSocket_->readWebSocketMessage
      (boost::bind(&WebSession::handleWebSocketMessage, shared_from_this(),
		   _1));

  if (controller_->server()->ioService().requestBlockedThread()) {
    while (!newRecursiveEvent_)
      try {
	recursiveEvent_.wait(handler->lock());
    } catch (...) {
      controller_->server()->ioService().releaseBlockedThread();
      throw;
    }
    controller_->server()->ioService().releaseBlockedThread();
  } else {
    // Allow at least one thread to serve requests in order to avoid a
    // locked-up Wt. Even worse, Wt deadlocks if all threads are
    // occupied in internal event loops and all those browser windows
    // are closed (session time out does not work anymore)
    throw WException("doRecursiveEventLoop(): all threads are busy. "
		     "Avoid using recursive event loops.");
  }
#else
  while (!newRecursiveEvent_)
    recursiveEvent_.wait();
#endif

  if (state_ == Dead) {
    recursiveEventHandler_ = 0;
    delete newRecursiveEvent_;
    newRecursiveEvent_ = 0;
    throw WException("doRecursiveEventLoop(): session was killed");
  }

  setLoaded();

  /*
   * We use recursiveEventHandler_ != 0 to postpone rendering: we only want
   * the event handling part.
   */
  app_->notify(WEvent(*newRecursiveEvent_));
  delete newRecursiveEvent_;
  newRecursiveEvent_ = 0;
  recursiveEventDone_.notify_one();

  recursiveEventHandler_ = prevRecursiveEventHandler;
#endif // WT_BOOST_THREADS
}

void WebSession::expire()
{
  kill();
}

bool WebSession::unlockRecursiveEventLoop()
{
  if (!recursiveEventHandler_)
    return false;

  /*
   * Pass on the handler to the recursive event loop.
   */
  Handler *handler = WebSession::Handler::instance();

  recursiveEventHandler_->setRequest(handler->request(), handler->response());
  handler->setRequest(0, 0);

  newRecursiveEvent_ = new WEvent::Impl(recursiveEventHandler_);

#ifdef WT_BOOST_THREADS
  recursiveEvent_.notify_one();
#endif

  return true;
}

void WebSession::handleRequest(Handler& handler)
{
  WebRequest& request = *handler.request();

  const std::string *wtdE = request.getParameter("wtd");

  /*
   * Cross-Origin Resource Sharing
   */
  const char *origin = request.headerValue("Origin");
  if (origin) {
    /*
     * Do we allow this XMLHttpRequest or WebSocketRequest?
     *
     * Only if it's proven itself by a correct (existing) wtd, and thus
     * not for a new session.
     */
    if ((wtdE && *wtdE == sessionId_) || state_ == JustCreated) {
      if (isEqual(origin, "null"))
	origin = "*";
      handler.response()->addHeader("Access-Control-Allow-Origin", origin);
      handler.response()->addHeader("Access-Control-Allow-Credentials", "true");

      if (isEqual(request.requestMethod(), "OPTIONS")) {
	WebResponse *response = handler.response();

	response->setStatus(200);
	response->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
	response->addHeader("Access-Control-Max-Age", "1728000");
	handler.flushResponse();

	return;
      }
    } else
      if (request.isWebSocketRequest()) {
	/*
	 * FIXME: not so for new WebSocket protocol versions
	 *
	 * We are already passed the websocket hand-shake so it is too late
	 * to indicate it by omitting the Access-Control-Allow-Origin header.
	 *
	 * But we close the socket nevertheless.
	 */
	handler.flushResponse();
	return;
      }
  }

  const std::string *requestE = request.getParameter("request");

  if (requestE && *requestE == "ws" && !request.isWebSocketRequest()) {
    LOG_ERROR("invalid WebSocket request, ignoring");

    LOG_INFO("Connection: " << str(request.headerValue("Connection")));
    LOG_INFO("Upgrade: " << str(request.headerValue("Upgrade")));
    LOG_INFO("Sec-WebSocket-Version: "
	     << str(request.headerValue("Sec-WebSocket-Version")));

    handler.flushResponse();
    return;
  }

  if (request.isWebSocketRequest()) {
    if (state_ != JustCreated) {
      handleWebSocketRequest(handler);
      return;
    } else {
      handler.flushResponse();
      kill();
      return;
    }
  }

  Configuration& conf = controller_->configuration();

  handler.response()->setResponseType(WebResponse::Page);

  /*
   * Only handle GET, POST and OPTIONS requests, unless a resource is
   * listening.
   */
  if (!((requestE && *requestE == "resource")
	|| isEqual(request.requestMethod(), "POST")
	|| isEqual(request.requestMethod(), "GET"))) {
    handler.response()->setStatus(400); // Bad Request
    handler.flushResponse();
    return;
  }

  /*
   * Under what circumstances do we allow a request which does not have
   * a session ID (i.e. who as it only through a cookie?)
   *  - when a new session is created
   *  - when reloading the page
   *
   * in other cases: discard the request
   */
  if ((!wtdE || (*wtdE != sessionId_))
	   && state_ != JustCreated
	   && (requestE && (*requestE == "jsupdate" ||
			    *requestE == "jserror" ||
			    *requestE == "resource"))) {
    LOG_DEBUG("CSRF: " << (wtdE ? *wtdE : "no wtd") << " != " << sessionId_ <<
	      ", requestE: " << (requestE ? *requestE : "none"));
    LOG_SECURE("CSRF prevention kicked in.");
    serveError(403, handler, "Forbidden");
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
	    if (*requestE == "jsupdate" || 
		*requestE == "jserror" || 
		*requestE == "script") {
	      handler.response()->setResponseType(WebResponse::Update);
	      LOG_INFO("signal from dead session, sending reload.");
	      renderer_.letReloadJS(*handler.response(), true);

	      kill();
	      break;
	    } else if (*requestE != "page") {
	      LOG_INFO("not serving this.");
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
	  try {
	    std::string internalPath = env_->getCookie("WtInternalPath");
	    env_->setInternalPath(internalPath);
	  } catch (std::exception& e) {
	  }

	  bool forcePlain
	    = env_->agentIsSpiderBot() || !env_->agentSupportsAjax();

	  progressiveBoot_ =
	    !forcePlain && conf.progressiveBoot(env_->internalPath());

	  if (forcePlain || progressiveBoot_) {
	    /*
	     * First start the application
	     */
	    if (!start(handler.response()))
	      throw WException("Could not start application.");

	    app_->notify(WEvent(WEvent::Impl(&handler)));

	    if (env_->agentIsSpiderBot())
	      kill();
	    else if (controller_->limitPlainHtmlSessions()) {
	      LOG_SECURE("DoS: plain HTML sessions being limited");

	      if (forcePlain)
		kill();
	      else // progressiveBoot_
		setState(Loaded, conf.bootstrapTimeout());
	    } else
	      setLoaded();
	  } else {
	    /*
	     * Delay application start
	     */
	    serveResponse(handler);
	    setState(Loaded, conf.bootstrapTimeout());
	  }
	  break; }
	case WidgetSet:
	  if (requestE && *requestE == "resource") {
	    const std::string *resourceE = request.getParameter("resource");
	    if (resourceE && *resourceE == "blank") {
	      handler.response()->setContentType("text/html");
	      handler.response()->out() <<
		"<html><head><title>bhm</title></head>"
		"<body> </body></html>";
	    } else {
	      LOG_INFO("not starting session for resource.");
	      handler.response()->setContentType("text/html");
	      handler.response()->out()
		<< "<html><head></head><body></body></html>";
	    }

	    kill();
	  } else {
	    handler.response()->setResponseType(WebResponse::Script);

	    init(request); // env, url/internalpath, initial query parameters
	    env_->enableAjax(request);

	    if (!start(handler.response()))
	      throw WException("Could not start application.");

	    app_->notify(WEvent(WEvent::Impl(&handler)));

	    setExpectLoad();
	  }

	  break;
	default:
	  assert(false); // StaticResource
	}

	break;
      }
      case ExpectLoad:
      case Loaded: {
	if (requestE) {
	  if (*requestE == "jsupdate" ||
	      *requestE == "jserror")
	    handler.response()->setResponseType(WebResponse::Update);
	  else if (*requestE == "script") {
	    handler.response()->setResponseType(WebResponse::Script);
	    if (state_ == Loaded)
	      setExpectLoad();
	  } else if (*requestE == "style") {
	    flushBootStyleResponse();

	    const std::string *page = request.getParameter("page");

	    // See:
	    // http://www.blaze.io/mobile/ios5-top10-performance-changes/
	    // Mozilla/5.0 (iPad; CPU OS 5_1_1 like Mac OS X) 
	    //  AppleWebKit/534.46 (KHTML, like Gecko)
	    //  Version/5.1 Mobile/9B206 Safari/7534.48.3
	    bool ios5 = env_->agentIsMobileWebKit()
	      && (env_->userAgent().find("OS 5_") != std::string::npos
		  || env_->userAgent().find("OS 6_") != std::string::npos
		  || env_->userAgent().find("OS 7_") != std::string::npos
		  || env_->userAgent().find("OS 8_") != std::string::npos);

	    // check js parameter
	    const std::string *jsE = request.getParameter("js");
	    bool nojs = jsE && *jsE == "no";

	    bool bootStyle = 
	      (app_ || (!ios5 && !nojs)) &&
	      page && 
	      *page == boost::lexical_cast<std::string>(renderer_.pageId());

	    if (!bootStyle) {
	      handler.response()->setContentType("text/css");
	      handler.flushResponse();
	    } else {
#ifndef WT_TARGET_JAVA
	      if (!app_) {
		bootStyleResponse_ = handler.response();
		handler.setRequest(0, 0);

		controller_->server()
		  ->schedule(2000, sessionId_,
			     boost::bind(&WebSession::flushBootStyleResponse,
					 this));
	      } else {
		renderer_.serveLinkedCss(*handler.response());
		handler.flushResponse();
	      }
#else
	      /*
	       * In Servlet2, we canont defer responding the request. So we
	       * do a little spin lock here.
	       *
	       * There is no reason why the second request (script) does not
	       * arrive within seconds.
	       */
	      unsigned i = 0;
	      const unsigned MAX_TRIES = 1000;

	      while (!app_ && i < MAX_TRIES) {
		mutex_.unlock();
		boost::this_thread::sleep(boost::posix_time::milliseconds(5));
		mutex_.lock();

		++i;
	      }

	      if (i < MAX_TRIES) {
		renderer_.serveLinkedCss(*handler.response());
	      }

	      handler.flushResponse();
#endif // WT_TARGET_JAVA
	    }

	    break;
	  }
	}

	bool requestForResource = requestE && *requestE == "resource";

	if (!app_) {
	  const std::string *resourceE = request.getParameter("resource");

	  if (handler.response()->responseType() == WebResponse::Script) {
	    if (!request.getParameter("skeleton")) {
	      env_->enableAjax(request);

	      if (!start(handler.response()))
		throw WException("Could not start application.");
	    } else {
	      serveResponse(handler);
	      return;
	    }
	  } else if (requestForResource && resourceE && *resourceE == "blank") {
	    handler.response()->setContentType("text/html");
	    handler.response()->out() <<
	      "<html><head><title>bhm</title></head>"
	      "<body> </body></html>";

	    break;
	  } else {
	    const std::string *jsE = request.getParameter("js");

	    if (jsE && *jsE == "no") {
	      if (!start(handler.response()))
		throw WException("Could not start application.");

	      if (controller_->limitPlainHtmlSessions()) {
		LOG_SECURE("DoS: plain HTML sessions being limited");
		kill();
	      }
	    } else {
	      // This could be because the session Id was not as
	      // expected. At least, it should be correct now.
	      if (!conf.reloadIsNewSession() && wtdE && *wtdE == sessionId_) {
		serveResponse(handler);
		setState(Loaded, conf.bootstrapTimeout());
	      } else {
		handler.response()->setContentType("text/html");
		handler.response()->out() <<
		  "<html><body><h1>Refusing to respond.</h1></body></html>";
	      }

	      break;
	    }
	  }
	}

        bool doNotify = false;

	if (handler.request()) {
	  const std::string *signalE
	    = handler.request()->getParameter("signal");
	  bool isPoll = signalE && *signalE == "poll";

	  if (requestForResource || isPoll || !unlockRecursiveEventLoop()) {
            doNotify = true;

	    if (env_->ajax()) {
	      if (state_ != ExpectLoad &&
		  handler.response()->responseType() == WebResponse::Update)
	        setLoaded();
	    } else if (state_ != ExpectLoad &&
		       !controller_->limitPlainHtmlSessions())
	      setLoaded();	    
          }
        } else {
#ifndef WT_TARGET_JAVA
          doNotify = !app_->initialized_;
#else
	  doNotify = false;
#endif
	}

        if (doNotify) {
	  app_->notify(WEvent(WEvent::Impl(&handler)));
	  if (handler.response() && !requestForResource) {
	    /*
	     * This may be when an error was thrown during event
	     * propagation: then we want to render the error message.
	     */
	    app_->notify(WEvent(WEvent::Impl(&handler, true)));
	  }
	}

	break;
      }
      case Dead:
	LOG_INFO("request to dead session, ignoring");
	break;
      }
    } catch (WException& e) {
      LOG_ERROR("fatal error: " << e.what());

#ifdef WT_TARGET_JAVA
      e.printStackTrace();
#endif // WT_TARGET_JAVA

      kill();

      if (handler.response())
	serveError(500, handler, e.what());

    } catch (std::exception& e) {
      LOG_ERROR("fatal error: " << e.what());

#ifdef WT_TARGET_JAVA
      e.printStackTrace();
#endif // WT_TARGET_JAVA

      kill();

      if (handler.response())
	serveError(500, handler, e.what());
    } catch (...) {
      LOG_ERROR("fatal error: caught unknown exception.");

      kill();

      if (handler.response())
	serveError(500, handler, "Unknown exception");
    }

  if (handler.response())
    handler.flushResponse();
}

void WebSession::flushBootStyleResponse()
{
  if (bootStyleResponse_) {
    bootStyleResponse_->flush();
    bootStyleResponse_ = 0;
  }
}

void WebSession::handleWebSocketRequest(Handler& handler)
{
#ifndef WT_TARGET_JAVA
  if (state_ != Loaded && state_ != ExpectLoad) {
    handler.flushResponse();
    return;
  }

  /*
   * This triggers an orderly switch from Ajax to WebSocetk:
   *
   *  First we ask for a 'connect', and in the mean time we do not yet
   *  use the socket. On the JS side, the connect disables any pending
   *  ajax long poll, and waits for the current pending response, if any.
   *  only then, we confirm the connect, transferring the last ackId.
   */
  if (webSocket_) {
    webSocket_->flush();
    webSocket_ = 0;
  }

  webSocket_ = handler.response();
  canWriteWebSocket_ = false;
  webSocketConnected_ = false;

  webSocket_->flush
    (WebRequest::ResponseFlush,
     boost::bind(&WebSession::webSocketConnect,				    
		 boost::weak_ptr<WebSession>(shared_from_this()), _1));

  handler.setRequest(0, 0);
#endif // WT_TARGET_JAVA
}

void WebSession::webSocketConnect(boost::weak_ptr<WebSession> session,
				  WebWriteEvent event)
{
  LOG_DEBUG("webSocketConnect()");

#ifndef WT_TARGET_JAVA
  boost::shared_ptr<WebSession> lock = session.lock();
  if (lock) {
    Handler handler(lock, Handler::TakeLock);

    if (!lock->webSocket_)
      return;

    switch (event) {
    case WriteCompleted:
      lock->webSocket_->out() << "connect";

      lock->webSocket_->flush
	(WebRequest::ResponseFlush,
	 boost::bind(&WebSession::webSocketReady,
		     boost::weak_ptr<WebSession>(lock), _1));

      lock->webSocket_->readWebSocketMessage
	(boost::bind(&WebSession::handleWebSocketMessage,
		     boost::weak_ptr<WebSession>(lock), _1));

      break;
    case WriteError:
      lock->webSocket_->flush();
      lock->webSocket_ = 0;

      break;
    }
  }
#endif // WT_TARGET_JAVA
  
}

void WebSession::handleWebSocketMessage(boost::weak_ptr<WebSession> session,
					WebReadEvent event)
{

#ifndef WT_TARGET_JAVA
  LOG_DEBUG("handleWebSocketMessage: " << (int)event);
  boost::shared_ptr<WebSession> lock = session.lock();
  if (lock) {
    Handler handler(lock, Handler::TakeLock);

    if (!lock->webSocket_)
      return;

    switch (event) {
    case ReadError:
      {
	if (lock->canWriteWebSocket_) {
	  lock->webSocket_->flush();
	  lock->webSocket_ = 0;
	}

	return;
      }

    case ReadPing:
      {
	WebSocketMessage *message = new WebSocketMessage(lock.get());

	if (lock->canWriteWebSocket_) {
	  lock->canWriteWebSocket_ = false;
	  lock->webSocket_->out() << "{}";
	  lock->webSocket_->flush
	    (WebRequest::ResponseFlush,
	     boost::bind(&WebSession::webSocketReady, session, _1));
	}

	delete message;

	lock->webSocket_->readWebSocketMessage
	  (boost::bind(&WebSession::handleWebSocketMessage, session, _1));

	break;
      }

    case ReadMessage:
      {
	WebSocketMessage *message = new WebSocketMessage(lock.get());

	bool closing = message->contentLength() == 0;

	if (!closing) {
	  CgiParser cgi(lock->controller_->configuration().maxRequestSize());
	  try {
	    cgi.parse(*message, CgiParser::ReadDefault);
	  } catch (std::exception& e) {
	    LOG_ERROR("could not parse ws message: " << e.what());
	    delete message;
	    closing = true;
	  }

	  const std::string *connectedE = message->getParameter("connected");
	  if (connectedE) {
	    if (lock->asyncResponse_) {
	      lock->asyncResponse_->flush();
	      lock->asyncResponse_ = 0;
	    }

	    lock->renderer_.ackUpdate(boost::lexical_cast<int>(*connectedE));
	    lock->webSocketConnected_ = true;
	  }

	  const std::string *signalE = message->getParameter("signal");

	  if (signalE && *signalE == "ping") {
	    LOG_DEBUG("ws: handle ping");
	    if (lock->canWriteWebSocket_) {
	      lock->canWriteWebSocket_ = false;
	      lock->webSocket_->out() << "{}";
	      lock->webSocket_->flush
		(WebRequest::ResponseFlush,
		 boost::bind(&WebSession::webSocketReady, session, _1));
	    }

	    lock->webSocket_->readWebSocketMessage
	      (boost::bind(&WebSession::handleWebSocketMessage, session, _1));
	    
	    delete message;

	    return;
	  }

	  const std::string *pageIdE = message->getParameter("pageId");
	  if (pageIdE && *pageIdE
	      != boost::lexical_cast<std::string>(lock->renderer_.pageId()))
	    closing = true;
	}

	if (!closing) {
	  handler.setRequest(message, (WebResponse *)(message));
	  lock->handleRequest(handler);
	} else
	  delete message;

	if (lock->dead()) {
	  closing = true;
	  lock->controller_->removeSession(lock->sessionId());
	}

	if (closing) {
	  if (lock->webSocket_ && lock->canWriteWebSocket_) {
	    lock->webSocket_->flush();
	    lock->webSocket_ = 0;
	  }
	} else
	  if (lock->webSocket_)
	    lock->webSocket_->readWebSocketMessage
	      (boost::bind(&WebSession::handleWebSocketMessage, session, _1));
      }
    }
  }
#endif // WT_TARGET_JAVA
}

std::string WebSession::ajaxCanonicalUrl(const WebResponse& request) const
{
  const std::string *hashE = 0;
  if (applicationName_.empty())
    hashE = request.getParameter("_");

  if (!pagePathInfo_.empty() || (hashE && hashE->length() > 1)) {
    std::string url;
    if (applicationName_.empty()) {
      url = fixRelativeUrl("?");
      url = url.substr(0, url.length() - 1);
    } else
      url = fixRelativeUrl(applicationName_);

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
  LOG_DEBUG("pushUpdates()");

  triggerUpdate_ = false;

  if (!app_ || !renderer_.isDirty()) {
    LOG_DEBUG("pushUpdates(): nothing to do");
    return;
  }

  updatesPending_ = true;

  if (asyncResponse_) {
    asyncResponse_->setResponseType(WebResponse::Update);
    app_->notify(WEvent(WEvent::Impl(asyncResponse_)));
    updatesPending_ = false;
    asyncResponse_->flush();
    asyncResponse_ = 0;
  } else if (webSocket_ && webSocketConnected_) {
    if (webSocket_->webSocketMessagePending()) {
      LOG_DEBUG("pushUpdates(): web socket message pending");
      return;
    }

    if (canWriteWebSocket_) {
#ifndef WT_TARGET_JAVA
      {
        WebSocketMessage m(this);
        m.setResponseType(WebResponse::Update);
        app_->notify(WEvent(WEvent::Impl((WebResponse *)&m)));
      }

      updatesPending_ = false;
      canWriteWebSocket_ = false;
      webSocket_->flush
	(WebRequest::ResponseFlush,
	 boost::bind(&WebSession::webSocketReady,
		     boost::weak_ptr<WebSession>(shared_from_this()),
		     _1));
#endif
    }
  }

  if (updatesPending_) {
    LOG_DEBUG("pushUpdates(): cannot write now");
#ifdef WT_BOOST_THREADS
    updatesPendingEvent_.notify_one();
#endif
  }
}

void WebSession::webSocketReady(boost::weak_ptr<WebSession> session,
				WebWriteEvent event)
{
  LOG_DEBUG("webSocketReady()");

#ifndef WT_TARGET_JAVA
  boost::shared_ptr<WebSession> lock = session.lock();
  if (lock) {
    Handler handler(lock, Handler::TakeLock);

    LOG_DEBUG("webSocketReady: webSocket_ = " << lock->webSocket_
	      << " updatesPending = " << lock->updatesPending_
	      << " event = " << (int)event);

    switch (event) {
    case WriteCompleted:
      if (lock->webSocket_) {
	lock->canWriteWebSocket_ = true;

	if (lock->updatesPending_)
	  lock->pushUpdates();
      }

      break;
    case WriteError:
      if (lock->webSocket_) {
	lock->webSocket_->flush();
	lock->webSocket_ = 0;
	lock->canWriteWebSocket_ = false;
      }

      break;
    }
  }
#endif // WT_TARGET_JAVA
}

const std::string *WebSession::getSignal(const WebRequest& request,
					 const std::string& se) const
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

void WebSession::externalNotify(const WEvent::Impl& event)
{
  if (recursiveEventHandler_) {
#ifdef WT_BOOST_THREADS
    newRecursiveEvent_ = new WEvent::Impl(event);
    recursiveEvent_.notify_one();
    while (newRecursiveEvent_) {
#ifdef WT_TARGET_JAVA
      recursiveEventDone_.wait();
#else
      recursiveEventDone_.wait(event.handler->lock());
#endif
    }
#endif
  } else {
    if (app_)
      app_->notify(WEvent(event));
    else
      notify(WEvent(event));
  }
}

void WebSession::notify(const WEvent& event)
{
  if (event.impl_.response) {
    try {
      renderer_.serveResponse(*event.impl_.response);
    } catch (std::exception& e) {
      LOG_ERROR("Exception in WApplication::notify()" << e.what());
    } catch (...) {
    }
    return;
  }

  if (event.impl_.function) {
    WT_CALL_FUNCTION(event.impl_.function);

    if (event.impl_.handler->request())
      render(*event.impl_.handler);

    return;
  }

  Handler& handler = *event.impl_.handler;

#ifndef WT_TARGET_JAVA
  if (!app_->initialized_) {
    app_->initialized_ = true;
    app_->initialize();
  }
#endif

  if (!handler.response())
    return;

  WebRequest& request = *handler.request();
  WebResponse& response = *handler.response();

  if (WebSession::Handler::instance() != &handler)
    // We will want to set these right before doing anything !
    WebSession::Handler::instance()->setRequest(&request, &response);

  if (event.impl_.renderOnly) {
    render(handler);
    return;
  }

  const std::string *requestE = request.getParameter("request");

  /*
   * Capture JavaScript error server-side.
   */
  if (requestE && *requestE == "jserror") {
    app_->handleJavaScriptError(*request.getParameter("err"));
    renderer_.setJSSynced(false);
    render(handler);
    return;
  }

  const std::string *pageIdE = request.getParameter("pageId");
  if (pageIdE &&
      *pageIdE != boost::lexical_cast<std::string>(renderer_.pageId())) {
    handler.response()->setContentType("text/javascript; charset=UTF-8");
    handler.response()->out() << "{}";
    handler.flushResponse();
    return;
  }

  switch (state_) {
  case WebSession::JustCreated:
    render(handler);

    break;
  case WebSession::ExpectLoad:
  case WebSession::Loaded:
    /*
     * Excluding resources here ?
     */
    if ((!requestE || (*requestE != "resource"))
	&& handler.response()->responseType() == WebResponse::Page) {
      /*
       * Prevent a session fixation attack and a session stealing attack:
       * - user agent has changed: close the session
       * - remote IP address changes: only allowed if the session cookie
       *   is not empty and matches.
       * - prevent attack on ajax sessions:
       *   - use random initial ackUpdateId to prevent attacks on ajax sessions
       *     (in the case somehow the session Id got stolen): this ackUpdateId
       *     is not exposed in a referer
       *   - tie script with unique id to page to prevent attack on a ajax
       *     session: this scriptId is not exposed in a referer
       *
       * Note: this may interfere with the use-case for the undocumented
       *       persistent session configuration option
       */
      if (!env_->agentIsIE())
	if (str(handler.request()->headerValue("User-Agent")) 
	    != env_->userAgent()) {
	  LOG_SECURE("change of user-agent not allowed.");
	  LOG_INFO("old user agent: " << env_->userAgent());
	  LOG_INFO("new user agent: "
		   << str(handler.request()->headerValue("User-Agent")));
	  serveError(403, handler, "Forbidden");
	  return;
	}

      std::string ca = WEnvironment::getClientAddress
	(*handler.request(), controller_->configuration());

      if (ca != env_->clientAddress()) {
	bool isInvalid = sessionIdCookie_.empty();

	if (!isInvalid) {
	  std::string cookie = str(request.headerValue("Cookie"));
	  if (cookie.find("Wt" + sessionIdCookie_) == std::string::npos)
	    isInvalid = true;
	}

	if (isInvalid) {
	  LOG_SECURE("change of IP address (" << env_->clientAddress()
		     << " -> " << ca << ") not allowed.");
	  serveError(403, handler, "Forbidden");
	  return;
	}
      }
    }

    if (sessionIdCookieChanged_) {
      std::string cookie = str(request.headerValue("Cookie"));
      if (cookie.find("Wt" + sessionIdCookie_) == std::string::npos) {
	sessionIdCookie_.clear();
	LOG_INFO("session id cookie not working");
      }

      sessionIdCookieChanged_ = false;
    }

    if (handler.response()->responseType() == WebResponse::Script) {
      const std::string *sidE = request.getParameter("sid");
      if (!sidE
	  || *sidE != boost::lexical_cast<std::string>(renderer_.scriptId())) {
	throw WException("Script id mismatch");
      }

      if (!request.getParameter("skeleton")) {
	if (!env_->ajax()) {
	  env_->enableAjax(request);
	  app_->enableAjax();
	  if (env_->internalPath().length() > 1)
	    changeInternalPath(env_->internalPath(), handler.response());
	} else {
	  const std::string *hashE = request.getParameter("_");
	  if (hashE)
	    changeInternalPath(*hashE, handler.response());
	}
      }

      render(handler);
    } else {
      // a normal request to a loaded application
      try {
	if (request.postDataExceeded())
	  app_->requestTooLarge().emit(request.postDataExceeded());
      } catch (std::exception& e) {
	LOG_ERROR("Exception in WApplication::requestTooLarge" << e.what());
	RETHROW(e);
      } catch (...) {
	LOG_ERROR("Exception in WApplication::requestTooLarge");
	throw;
      }

      const std::string *hashE = request.getParameter("_");

      WResource *resource = 0;
      if (!requestE) {
	if (!request.pathInfo().empty())
	  resource = app_->decodeExposedResource
	    ("/path/" + Utils::prepend(request.pathInfo(), '/'));

	if (!resource && hashE)
	  resource = app_->decodeExposedResource
	    ("/path/" + *hashE);
      } 

      const std::string *resourceE = request.getParameter("resource");
      const std::string *signalE = getSignal(request, "");

      if (signalE)
	progressiveBoot_ = false; 

      if (resource || (requestE && *requestE == "resource" && resourceE)) {
	if (resourceE && *resourceE == "blank") {
	  handler.response()->setContentType("text/html");
	  handler.response()->out() <<
	    "<html><head><title>bhm</title></head>"
	    "<body> </body></html>";
	  handler.flushResponse();
	} else {
	  if (!resource)
	    resource = app_->decodeExposedResource(*resourceE);

	  if (resource) {
	    try {
	      resource->handle(&request, &response);
	      handler.setRequest(0, 0);
	    } catch (std::exception& e) {
	      LOG_ERROR("Exception while streaming resource" << e.what());
	      RETHROW(e);
	    } catch (...) {
	      LOG_ERROR("Exception while streaming resource");
	      throw;
	    }
	  } else {
	    LOG_ERROR("decodeResource(): resource '" << *resourceE
		      << "' not exposed");
	    handler.response()->setContentType("text/html");
	    handler.response()->out() <<
	      "<html><body><h1>Nothing to say about that.</h1></body></html>";
	    handler.flushResponse();
	  }
	}
      } else {
		env_->updateUrlScheme(request);

	if (signalE) {
	  /*
	   * Check the ackIdE. This is required for a request carrying a signal.
	   */
	  const std::string *ackIdE = request.getParameter("ackId");

	  bool invalidAckId = env_->ajax() 
	    && !request.isWebSocketMessage();

	  if (invalidAckId && ackIdE) {
	    try {
	      if (renderer_.ackUpdate(boost::lexical_cast<int>(*ackIdE)))
		invalidAckId = false;
	    } catch (const boost::bad_lexical_cast& e) {
	    }
	  }

 	  if (invalidAckId) {
	    if (!ackIdE)
	      LOG_SECURE("missing ackId");
	    else
	      LOG_SECURE("invalid ackId");
	    serveError(403, handler, "Forbidden");
	    return;
	  }

	  /*
	   * In case we are not using websocket but long polling, the client
	   * aborts the previous poll request to indicate a client-side event.
	   *
	   * So we also discard the previous asyncResponse_ server-side.
	   * We don't do this if we have a websocket request -- it might be
	   * a race between the websocket being established and a poll
	   * request.
	   */
	  if (asyncResponse_) {
	    asyncResponse_->flush();
	    asyncResponse_ = 0;
	  }

	  if (*signalE == "poll") {
#ifdef WT_BOOST_THREADS
	    /*
	     * If we cannot do async I/O, we cannot suspend the current
	     * request and return. Thus we need to block the thread, waiting
	     * for a push update. We wait at most twice as long as the client
	     * will renew this poll connection.
	     */
	    if (!WebController::isAsyncSupported()) {
	      updatesPendingEvent_.notify_one();
	      if (!updatesPending_) {
#ifndef WT_TARGET_JAVA
		updatesPendingEvent_.wait(handler.lock());
#else
		try {
		  updatesPendingEvent_.timed_wait
		    (controller_->configuration().serverPushTimeout() * 2);
		} catch (InterruptedException& e) { }
#endif // WT_TARGET_JAVA
	      }
	      if (!updatesPending_) {
		handler.flushResponse();
		return;
	      }
	    }
#endif // WT_BOOST_THREADS

	    // LOG_DEBUG("poll: " << updatesPending_ << ", " << (asyncResponse_ ? "async" : "no async"));
	    if (!updatesPending_) {
	      /*
	       * If we are ignoring many poll requests (because we are
	       * assuming to have a websocket), we will need to assume
	       * the web socket isn't working properly.
	       */
	      if (!webSocket_ || (pollRequestsIgnored_ == 2)) {
		if (webSocket_) {
		  LOG_INFO("discarding broken websocket");
		  webSocket_->flush();
		  webSocket_ = 0;
		}

		pollRequestsIgnored_ = 0;
		asyncResponse_ = handler.response();
		handler.setRequest(0, 0);
	      } else {
		++pollRequestsIgnored_;
		LOG_DEBUG("ignored poll request (#" << pollRequestsIgnored_
			  << ")");
	      }
	    } else
	      pollRequestsIgnored_ = 0;
	  } else {
#ifdef WT_BOOST_THREADS
	    if (!WebController::isAsyncSupported()) {
	      updatesPending_ = false;
	      updatesPendingEvent_.notify_one();
	    }
#endif
	  }

	  if (handler.request()) {
	    LOG_DEBUG("signal: " << *signalE);

	    /*
	     * Special signal values:
	     * 'poll' : long poll
	     * 'none' : no event, but perhaps a synchronization
	     * 'load' : load invisible content
	     */

	    try {
	      handler.nextSignal = -1;
	      notifySignal(event);
	    } catch (std::exception& e) {
	      LOG_ERROR("error during event handling: " << e.what());
	      RETHROW(e);
	    } catch (...) {
	      LOG_ERROR("error during event handling");
	      throw;
	    }
	  }
	}

	if (handler.response()
	    && handler.response()->responseType() == WebResponse::Page
	    && (!env_->ajax() ||
		!controller_->configuration().reloadIsNewSession())) {
	  app_->domRoot()->setRendered(false);

	  env_->parameters_ = handler.request()->getParameterMap();

	  if (hashE)
	    changeInternalPath(*hashE, handler.response());
	  else if (!handler.request()->pathInfo().empty()) {
	    changeInternalPath(handler.request()->pathInfo(),
			       handler.response());
	  } else
	    changeInternalPath("", handler.response());
	}

	if (!signalE) {
	  if (type() == WidgetSet) {
	    LOG_ERROR("bogus request: missing signal, discarding");
	    handler.flushResponse();
	    return;
	  }

	  LOG_INFO("refreshing session");

	  flushBootStyleResponse();

#ifndef WT_TARGET_JAVA
	  // if we are persisting sessions, then we should make sure we
	  // are listening to only one browser at a time: do this by
	  // generating a new session id when a new browser connects
	  if (controller_->configuration().persistentSessions()) {
	    LOG_INFO("refresh for persistent session");
	    WEnvironment oldEnv = *env_;
	    env_->init(*handler.request());
	    env_->parameters_ = handler.request()->getParameterMap();

	    try {
	      app_->refresh();

	      app_->connected_ = true;
	    } catch (std::exception& e) {
	      *env_ = oldEnv;

	      LOG_INFO("bad refresh attempt: " << e.what());
	      handler.response()->setContentType("text/html");
	      handler.response()->out() <<
		"<html><body><h1>Are you trying some shenanigans?"
		"</h1></body></html>";
	      handler.flushResponse();
	    }
	  } else {
#endif // WT_TARGET_JAVA
	    if (handler.request()) {
	      env_->parameters_ = handler.request()->getParameterMap();
		  env_->updateHostName(*handler.request());
		}
	    app_->refresh();
#ifndef WT_TARGET_JAVA
	  }
#endif // WT_TARGET_JAVA
	}

	if (handler.response() && !recursiveEventHandler_)
	  render(handler);
      }
    }
  case Dead:
    break;
  }
}

void WebSession::changeInternalPath(const std::string& path,
				    WebResponse *response)
{
  if (!app_->internalPathIsChanged_)
    if (!app_->changedInternalPath(path))
      if (response->responseType() == WebResponse::Page)
	response->setStatus(404);
}

EventType WebSession::getEventType(const WEvent& event) const
{  
  if (event.impl_.handler == 0) 
    return OtherEvent;
    
  Handler& handler = *event.impl_.handler;

#ifndef WT_TARGET_JAVA
  if (event.impl_.function)
    return OtherEvent;
#endif // WT_TARGET_JAVA

  WebRequest& request = *handler.request();

  if (event.impl_.renderOnly || !handler.request())
    return OtherEvent;

  const std::string *requestE = request.getParameter("request");

  const std::string *pageIdE = handler.request()->getParameter("pageId");
  if (pageIdE && 
      *pageIdE != boost::lexical_cast<std::string>(renderer_.pageId()))
    return OtherEvent;

  switch (state_) {
  case WebSession::ExpectLoad:
  case WebSession::Loaded:
    if (handler.response()->responseType() == WebResponse::Script)
      return OtherEvent;
    else {
      WResource *resource = 0;
      if (!requestE && !request.pathInfo().empty())
	resource = app_->decodeExposedResource("/path/" + request.pathInfo());

      const std::string *resourceE = request.getParameter("resource");
      const std::string *signalE = getSignal(request, "");

      if (resource || (requestE && *requestE == "resource" && resourceE))
	return ResourceEvent;
      else if (signalE) {
	if (*signalE == "none" || *signalE == "load" || 
	    *signalE == "hash" || *signalE == "poll")
	  return OtherEvent;
	else {
	  std::vector<unsigned int> signalOrder
	    = getSignalProcessingOrder(event);

	  unsigned timerSignals = 0;

	  for (unsigned i = 0; i < signalOrder.size(); ++i) {
	    int signalI = signalOrder[i];
	    std::string se = signalI > 0
	      ? 'e' + boost::lexical_cast<std::string>(signalI) : std::string();
	    const std::string *s = getSignal(request, se);
	  
	    if (!s)
	      break;
	    else if (*signalE == "user")
	      return UserEvent;
	    else {
	      EventSignalBase* esb = decodeSignal(*s, false);

	      if (!esb)
		continue;

	      WTimerWidget* t = dynamic_cast<WTimerWidget*>(esb->sender());
	      if (t)
		++timerSignals;
	      else
		return UserEvent;
	    }
	  }

	  if (timerSignals)
	    return TimerEvent;
	}
      } else
	return OtherEvent;
    }
  default:
    return OtherEvent;
  }
}

void WebSession::render(Handler& handler)
{
  LOG_DEBUG("render()");
  /*
   * In any case, render() will flush the response, even if an error
   * occurred. Since we are already rendering the response, we can no longer
   * show a nice error message.
   */

  try {
    if (!env_->ajax())
      try {
	checkTimers();
      } catch (std::exception& e) {
	LOG_ERROR("Exception while triggering timers" << e.what());
	RETHROW(e);
      } catch (...) {
	LOG_ERROR("Exception while triggering timers");
	throw;
      }

    if (app_ && app_->isQuited())
      kill();

    if (handler.response()) { // a recursive eventloop may remove it in kill()
      updatesPending_ = false;
      serveResponse(handler);
    }

  } catch (std::exception& e) {
    handler.flushResponse();

    RETHROW(e);
  } catch (...) {
    handler.flushResponse();

    throw;
  }
}

void WebSession::serveError(int status, Handler& handler, const std::string& e)
{
  renderer_.serveError(status, *handler.response(), e);
  handler.flushResponse();
}

void WebSession::serveResponse(Handler& handler)
{
  if (handler.response()->responseType() == WebResponse::Page) {
    pagePathInfo_ = handler.request()->pathInfo();
    const std::string *wtdE = handler.request()->getParameter("wtd");
    if (wtdE && *wtdE == sessionId_)
      sessionIdInUrl_ = true;
    else
      sessionIdInUrl_ = false;
  }

  /*
   * If the request is a web socket message, then we should not actually
   * render -- there may be more messages following.
   */
  if (!handler.request()->isWebSocketMessage()) {
    /*
     * In any case, flush the style request when we are serving a new
     * page (without Ajax) or the main script (with Ajax).
     */
    if (handler.response()->responseType() == WebResponse::Script
	&& !handler.request()->getParameter("skeleton")) {
#ifndef WT_TARGET_JAVA
      if (bootStyleResponse_) {
	renderer_.serveLinkedCss(*bootStyleResponse_);
	flushBootStyleResponse();
      }
#else
      /*
       * Preempt the thread waiting for the bootstyle to be ready.
       * Otherwise there is a reflow as the page already renders
       * without having the CSS (Duh?)
       */
      mutex_.unlock();
      try {
	boost::this_thread::sleep(boost::posix_time::milliseconds(1));
      } catch (InterruptedException& e) { }
      mutex_.lock();
#endif
    }

    renderer_.serveResponse(*handler.response());
  }

  handler.flushResponse();
}

void WebSession::propagateFormValues(const WEvent& e, const std::string& se)
{
  const WebRequest& request = *e.impl_.handler->request();

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
      LOG_ERROR("Could not lexical cast selection range");
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

std::vector<unsigned int> 
WebSession::getSignalProcessingOrder(const WEvent& e) const
{
  // Rush 'onChange' events. Reason: if a user edits a text area and
  // a subsequent click on another element deletes the text area, we
  // have seen situations (at least on firefox) where the clicked event
  // is processed before the changed event, causing the changed event
  // to fail because the event target was deleted.
  WebSession::Handler& handler = *e.impl_.handler;

  std::vector<unsigned int> highPriority;
  std::vector<unsigned int> normalPriority;

  for (unsigned i = 0;; ++i) {
    const WebRequest& request = *handler.request();

    std::string se = i > 0
      ? 'e' + boost::lexical_cast<std::string>(i) : std::string();
    const std::string *signalE = getSignal(request, se);
    if (!signalE)
      break;
    if (*signalE != "user" &&
        *signalE != "hash" &&
        *signalE != "none" &&
        *signalE != "poll" &&
        *signalE != "load") {
      EventSignalBase *signal = decodeSignal(*signalE, true);
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
  WebSession::Handler& handler = *e.impl_.handler;

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

    LOG_DEBUG("signal: " << *signalE);

    if (type() != WidgetSet ||
	(*signalE != "none" && *signalE != "load"))
      renderer_.setRendered(true);

    if (*signalE == "none" || *signalE == "load") {
      if (*signalE == "load") {
	if (!renderer_.checkResponsePuzzle(request))
	  app_->quit();
	else
	  setLoaded();
      }

      // We will want invisible changes now too.
      renderer_.setVisibleOnly(false);
    } else if (*signalE != "poll") {
      propagateFormValues(e, se);

      // Save pending changes (e.g. from resource completion)
      // This is needed because we will discard changes from learned
      // signals (see below) and thus need to start with a clean slate
      bool discardStateless = !request.isWebSocketMessage() && i == 0;
      if (discardStateless)
	renderer_.saveChanges();

      handler.nextSignal = i + 1;

      if (*signalE == "hash") {
	const std::string *hashE = request.getParameter(se + "_");
	if (hashE) {
	  changeInternalPath(*hashE, handler.response());
	  app_->doJavaScript(WT_CLASS ".scrollIntoView("
			     + WWebWidget::jsStringLiteral(*hashE) + ");");
	} else
	  changeInternalPath("", handler.response());
      } else {
	for (unsigned k = 0; k < 3; ++k) {
	  SignalKind kind = (SignalKind)k;

	  if (kind == AutoLearnStateless && request.postDataExceeded())
	    break;

	  EventSignalBase *s;
	  if (*signalE == "user") {
	    const std::string *idE = request.getParameter(se + "id");
	    const std::string *nameE = request.getParameter(se + "name");

	    if (!idE || !nameE)
	      break;

	    s = decodeSignal(*idE, *nameE, k == 0);
	  } else
	    s = decodeSignal(*signalE, k == 0);

	  processSignal(s, se, request, kind);

	  if (kind == LearnedStateless && discardStateless)
	    renderer_.discardChanges();
	}
      }
    }
  }

  app_->justRemovedSignals().clear();
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

void WebSession::setPagePathInfo(const std::string& path)
{
  if (!useUglyInternalPaths())
    pagePathInfo_ = path;
}

bool WebSession::useUrlRewriting()
{
  Configuration& conf = controller_->configuration();
  return !(conf.sessionTracking() == Configuration::CookiesURL &&
	   env_->supportsCookies());
}

#ifndef WT_TARGET_JAVA
void WebSession::generateNewSessionId()
{
  if (!renderer_.isRendered())
    return;

  std::string oldId = sessionId_;
  sessionId_ = controller_->generateNewSessionId(shared_from_this());
  sessionIdChanged_ = true;

  LOG_INFO("new session id for " << oldId);

  if (!useUrlRewriting()) {
    std::string cookieName = env_->deploymentPath();
    renderer().setCookie(cookieName, sessionId_, WDateTime(), "", "", env_->urlScheme() == "https");
  }

  if (controller_->configuration().sessionIdCookie()) {
    sessionIdCookie_ = WRandom::generateId();
    sessionIdCookieChanged_ = true;
    renderer().setCookie("Wt" + sessionIdCookie_, "1", WDateTime(), "", "",
			 env_->urlScheme() == "https");
  }
}
#endif // WT_TARGET_JAVA

}
