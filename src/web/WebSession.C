/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WTimerWidget"

#include "CgiParser.h"
#include "WebSession.h"
#include "WebRequest.h"
#include "WebController.h"
#include "Configuration.h"
#include "WtException.h"
#include "DomElement.h"
#include "Utils.h"

namespace Wt {

/*
 * About the mutexes.
 *
 * A living session is identified and a handler is created for it while
 * holding the global sessions lock.
 *
 * Sessions are expired when the global sessions lock is in possession, and
 * these two conditions make that a session dies atomically.
 *
 */

#ifdef THREADED
boost::thread_specific_ptr<WebSession *> WebSession::threadSession_;
#else
WebSession *WebSession::threadSession_;
#endif // THREADED

WebSession::WebSession(const std::string& sessionId,
		       const std::string& sessionPath, Type type,
		       const WebRequest& request)
  : type_(type),
    state_(JustCreated),
    sessionId_(sessionId),
    sessionPath_(sessionPath),
    renderer_(*this),
    env_(this),
    app_(0),
    debug_(false)
{
  /*
   * Obtain the applicationName_ as soon as possible for log().
   */
  applicationUrl_ = request.scriptName();

  applicationName_ = applicationUrl_;
  baseUrl_ = applicationUrl_;

  std::string::size_type slashpos = applicationName_.find_last_of('/');
  if (slashpos != std::string::npos) {
    applicationName_.erase(0, slashpos+1);
    baseUrl_.erase(slashpos+1);
  }

  log("notice") << "Session created (#sessions = "
		<< (WebController::instance()->sessionCount() + 1)
		<< ")";
  expire_ = Time() + 60*1000;
}

WLogEntry WebSession::log(const std::string& type)
{
  WLogEntry e = WebController::conf().logger().entry();

  e << WLogger::timestamp << WLogger::sep
    << WebController::conf().pid() << WLogger::sep
    << '[' << baseUrl_ << applicationName_ << ' ' << sessionId()
    << ']' << WLogger::sep
    << '[' << type << ']' << WLogger::sep;

  return e;
}

WebSession::~WebSession()
{
  if (app_)
    app_->finalize();
  delete app_;

  unlink(sessionPath_.c_str());

  log("notice") << "Session destroyed (#sessions = "
		<< WebController::instance()->sessionCount()
		<< ")";
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
#ifdef THREADED
  boost::mutex::scoped_lock stateLock(stateMutex_);
#endif // THREADED

  if (state_ != Dead) {
    state_ = state;
    expire_ = Time() + timeout*1000;
  }
}

void WebSession::init(const CgiParser& cgi, const WebRequest& request)
{
  env_.init(cgi, request);

  CgiEntry *hashE = cgi.getEntry("_");

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
    path = hashE->value();
  env_.setInternalPath(path);
}

std::string WebSession::bootstrapUrl(const WebRequest& request,
				     BootstrapOption option) const
{
  if (request.pathInfo().empty())
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
}

std::string WebSession::mostRelativeUrl(const std::string& internalPath) const
{
  return appendSessionQuery(bookmarkUrl(internalPath));
}

std::string WebSession::appendSessionQuery(const std::string& url) const
{
  if (env_.agentIsSpiderBot())
    return url;

  std::size_t questionPos = url.find('?');

  if (questionPos == std::string::npos)
    return url + sessionQuery_;
  else if (questionPos == url.length() - 1)
    return url + sessionQuery_.substr(1);
  else
    return url + '&' + sessionQuery_.substr(1);
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

bool WebSession::start(WApplication::ApplicationCreator createApplication)
{
  try {
    app_ = createApplication(env_);
  } catch (...) {
    app_ = 0;

    kill();
    throw;
  }

  if (app_)
    app_->initialize();

  return app_;
}

bool WebSession::doingFullPageRender() const
{
  return !env_.ajax();
}

void WebSession::kill()
{
#ifdef THREADED
  boost::mutex::scoped_lock stateLock(stateMutex_);
#endif // THREADED

  state_ = Dead;

  /*
   * Unlock the recursive eventloop that may be pending.
   */
#ifdef THREADED
  recursiveEventDone_.notify_all();
#endif // THREADED

  if (handlers_.empty()) {
#ifdef THREADED
    // we may unlock because the session has already been removed
    // from the sessions list, and thus, once the list is empty it is
    // guaranteed to stay empty.
    stateLock.unlock();
#endif // THREADED

    delete this;
  }
}

void WebSession::setDebug(bool debug)
{
  debug_ = debug;
}

void WebSession::refresh()
{
  app_->refresh();
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
    expired[i]->clicked.emit(dummy);
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

void WebSession::setEnvRequest(WebRequest *request)
{
  env_.setRequest(request);
}

WebSession::Handler::Handler(WebSession& session, WebRequest *request)
  : session_(session),
    request_(request),
    eventLoop_(false),
    killed_(false)
{
  session_.setEnvRequest(request);

#ifdef THREADED
  lock_ = 0;
  boost::mutex::scoped_lock stateLock(session_.stateMutex_);
#endif // THREADED
  session_.handlers_.push_back(this);
  //std::cerr << "handlers: " << session_.handlers_.size() << std::endl;

#ifdef THREADED
  if (threadSession_.get())
    prevSessionPtr_ = threadSession_.release();
  else
    prevSessionPtr_ = 0;

  sessionPtr_ = &session_;
  threadSession_.reset(&sessionPtr_);
#else
  threadSession_ = &session;
#endif // THREADED
}

void WebSession::Handler::attachThreadToSession(WebSession& session)
{
#ifdef THREADED
  threadSession_.reset(new (WebSession *)(&session));
#else
  session.log("error") <<
    "attachThreadToSession() requires that Wt is built with threading enabled";
#endif // THREADED
}

WebSession::Handler::~Handler()
{
  session_.setEnvRequest(0);

  {
#ifdef THREADED
    boost::mutex::scoped_lock stateLock(session_.stateMutex_);
#endif // THREADED

    Utils::erase(session_.handlers_, this);
  }

  if (session_.handlers_.size() == 0)
    session_.hibernate();

  if (killed_)
    session_.kill();

#ifdef THREADED
  threadSession_.release();

  if (prevSessionPtr_)
    threadSession_.reset(prevSessionPtr_);
#endif // THREADED
}

void WebSession::Handler::killSession()
{
  killed_ = true;
  session_.state_ = Dead;
}

#ifdef THREADED
void WebSession::Handler::setLock(boost::mutex::scoped_lock *lock)
{
  lock_ = lock;
}
#endif // THREADED

void WebSession::Handler::setRequest(WebRequest *request)
{
  request_ = request;
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
  if (app_)
    app_->messageResourceBundle().hibernate();
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
#ifdef THREADED
  return threadSession_.get() ? *threadSession_ : 0;
#else
  return threadSession_;
#endif // THREADED
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
#ifndef THREADED
  log("error") << "Cannot do recursive event loop without threads";
#else // THREADED
  /*
   * Locate the handler, and steal its pending request.
   */
  Handler *handler = findEventloopHandler(0);

  if (handler == 0)
    throw WtException("doRecursiveEventLoop(): inconsistent state");

  if (handler->request()) {
    /*
     * Finish the request.
     */
    WebController::instance()->render(*handler, 0,
				      app_->environment().ajax() 
				      ? WebRenderer::UpdateResponse
				      : WebRenderer::FullResponse);
    handler->request()->out() << javascript;
    handler->request()->flush();

    /*
     * Remove the request from the handler.
     */
    handler->setRequest(0);
  }

  /*
   * Release session mutex lock, wait for recursive event response,
   * and retake the session mutex lock.
   */
  recursiveEventDone_.wait(*handler->lock());

  if (state_ == Dead)
    throw WtException("doRecursiveEventLoop(): session was killed");
  
  env_.setRequest(handler->request());

#endif // THREADED
}

void WebSession::unlockRecursiveEventLoop()
{
  /*
   * Locate both the current and previous event loop handler.
   */
  Handler *handler = findEventloopHandler(0);
  Handler *handlerPrevious = findEventloopHandler(1);

  handlerPrevious->setRequest(handler->request());
  handler->setRequest(0);

#ifdef THREADED
  recursiveEventDone_.notify_one();
#endif // THREADED
}

WebSession::Handler *WebSession::findEventloopHandler(int index)
{
#ifdef THREADED
  boost::mutex::scoped_lock stateLock(stateMutex_);
#endif // THREADED

  for (int i = handlers_.size() - 1; i >= 0; --i) {
    if (handlers_[i]->eventLoop())
      if (index == 0)
	return handlers_[i];
      else
	--index;
  }

  return 0;
}

}
