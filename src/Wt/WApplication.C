/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>
#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WDefaultLoadingIndicator"
#include "Wt/WMemoryResource"
#include "Wt/WServer"
#include "Wt/WText"

#include "WtException.h"
#include "WServerPushResource.h"
#include "WebSession.h"
#include "DomElement.h"
#include "Configuration.h"
#include "WebController.h"
#include "WebRequest.h"
#include "Utils.h"

#ifdef min
#undef min
#endif

namespace Wt {

//#define WTDEBUG
//#define WTDEBUG_STATE(x) x
#define WTDEBUG_STATE(x)

WApplication::ScriptLibrary::ScriptLibrary(const std::string& anUri,
					   const std::string& aSymbol)
  : uri(anUri), symbol(aSymbol)
{ }

bool WApplication::ScriptLibrary::operator< (const ScriptLibrary& other) const
{
  return uri < other.uri;
}

bool WApplication::ScriptLibrary::operator== (const ScriptLibrary& other) const
{
  return uri == other.uri;
}

WApplication::State::State(const std::string& aScope, const std::string& aValue)
  : scope(aScope), value(aValue)
{ }

WApplication::WApplication(const WEnvironment& env)
  : session_(env.session_),
    titleChanged_(false),
    serverPush_(0),
    shouldTriggerUpdate_(false),
    javaScriptClass_("Wt"),
    dialogCover_(0),
    dialogWindow_(0),
    quited_(false),
    rshLoaded_(false),
    exposedOnly_(0),
    loadingIndicator_(0),
    scriptLibrariesAdded_(0),
    styleSheetsAdded_(0),
    exposeSignals_(true),
    autoJavaScriptChanged_(false),
    javaScriptResponse_(this),
    showLoadingIndicator_(this),
    hideLoadingIndicator_(this)
{
  session_->app_ = this;

  locale_ = environment().locale();

  newInternalPath_ = environment().internalPath();
  useStates_ = internalPathChanged_ = false;

  localizedStrings_ = new WMessageResourceBundle();

  domRoot_ = new WContainerWidget();
  domRoot_->load();

  if (session_->type() == WebSession::Application)
    domRoot_->resize(WLength(), WLength(100, WLength::Percentage));

  timerRoot_ = new WContainerWidget(domRoot_);
  timerRoot_->resize(WLength(), WLength(0));
  timerRoot_->setPositionScheme(Absolute);

  if (session_->type() == WebSession::Application) {
    ajaxMethod_ = XMLHttpRequest;

    domRoot2_ = 0;
    widgetRoot_ = new WContainerWidget(domRoot_);
    widgetRoot_->resize(WLength(100, WLength::Percentage),
			WLength(100, WLength::Percentage));
  } else {
    ajaxMethod_ = DynamicScriptTag;

    domRoot2_ = new WContainerWidget();
    domRoot2_->load();
    widgetRoot_ = 0;
  }

  std::string resourcesURL = resourcesUrl();

  /*
   * Subset of typical CSS "reset" styles
   */
  styleSheet_.addRule("table", "border-collapse: collapse; border: 0px");
  styleSheet_.addRule("div, td, img",
		      "margin: 0px; padding: 0px; border: 0px");
  styleSheet_.addRule("td", "vertical-align: top; text-align: left;");
  styleSheet_.addRule("button", "white-space: nowrap");

  if (environment().contentType() == WEnvironment::XHTML1) {
    styleSheet_.addRule("img", "margin-top: -1px");
    styleSheet_.addRule("button", "display: inline");
  }

  if (environment().agentIE())
    styleSheet_.addRule("html, body", "overflow: auto;");
  else if (environment().agentGecko())
    styleSheet_.addRule("html", "overflow: auto;");

  /*
   * Standard Wt CSS styles: resources, button wrap and form validation
   */
  styleSheet_.addRule("iframe.Wt-resource",
		      "width: 0px; height: 0px; border: 0px;");
  styleSheet_.addRule("button.Wt-wrap",
		      "border: 0px; text-align: left;"
		      "background-color: transparent; "
		      "margin: 0px; padding: 0px; font-size: inherit; "
		      "pointer: hand; cursor: pointer; cursor: hand; "
		      "color: inherit;");
  styleSheet_.addRule("a.Wt-wrap", "text-decoration: none;");
  styleSheet_.addRule(".Wt-invalid", "background-color: #f79a9a;");
  styleSheet_.addRule(".unselectable",
		      "-moz-user-select: none;"
		      "-khtml-user-select: none;"
		      "user-select: none;");
  styleSheet_.addRule(".Wt-sbspacer", "float: right; width: 16px; height: 1px;"
		      "border: 0px; display: none;");
  javaScriptResponse_.connect(SLOT(this,
				   WApplication::handleJavaScriptResponse));

  setLoadingIndicator(new WDefaultLoadingIndicator());
}

void WApplication::setLoadingIndicator(WLoadingIndicator *indicator)
{
  delete loadingIndicator_;
  loadingIndicator_ = indicator;

  if (loadingIndicator_) {
    WWidget *w = indicator->widget();
    domRoot_->addWidget(w);

    showLoadingIndicator_.connect(SLOT(w, WWidget::show));
    hideLoadingIndicator_.connect(SLOT(w, WWidget::hide));

    w->hide();
  }
}

void WApplication::initialize()
{ }

void WApplication::finalize()
{ }

WMessageResourceBundle& WApplication::messageResourceBundle() const
{
  return *(dynamic_cast<WMessageResourceBundle *>(localizedStrings_));
}

std::string WApplication::onePixelGifUrl()
{
  if (onePixelGifUrl_.empty()) {
    WMemoryResource *w = new WMemoryResource("image/gif", this);

    static const char gifData[]
      = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00,
	  0x80, 0x00, 0x00, 0xdb, 0xdf, 0xef, 0x00, 0x00, 0x00, 0x21,
	  0xf9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
	  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44,
	  0x01, 0x00, 0x3b };

    w->setData(gifData, 43);
    onePixelGifUrl_ = w->generateUrl();
  }

  return onePixelGifUrl_;
}

WApplication::~WApplication()
{
  dialogCover_ = 0;
  dialogWindow_ = 0;

  WContainerWidget *tmp = domRoot_;
  domRoot_ = 0;
  delete tmp;

  tmp = domRoot2_;
  domRoot2_ = 0;
  delete tmp;

  delete serverPush_;
  delete localizedStrings_;

  styleSheet_.clear();

  session_->app_ = 0;
}

void WApplication::attachThread()
{
  WebSession::Handler::attachThreadToSession(*session_);
}

void WApplication::setAjaxMethod(AjaxMethod method)
{
  ajaxMethod_ = method;
}

std::string WApplication::resourcesUrl()
{
  std::string result = "resources/";
  readConfigurationProperty("resourcesURL", result);

  if (!result.empty() && result[result.length()-1] != '/')
    result += '/';

  return instance()->fixRelativeUrl(result);
}

void WApplication::bindWidget(WWidget *widget, const std::string& divId)
{
  if (session_->type() != WebSession::WidgetSet)
    throw WtException("WApplication::bind() can be used only "
		      "in WidgetSet mode.");

  widget->setId(divId);
  domRoot2_->addWidget(widget);
}

WContainerWidget *WApplication::dialogCover(bool create)
{
  if (dialogCover_ == 0 && create) {
    dialogCover_ = new WContainerWidget(domRoot_);
    dialogCover_->setStyleClass("Wt-dialogcover");
  }

  return dialogCover_;
}

void WApplication::exposeOnly(WWidget *w)
{
  exposedOnly_ = w;
}

bool WApplication::isExposed(WWidget *w) const
{
  if (exposedOnly_) {
    for (WWidget *p = w; p; p = p->parent())
      if (p == exposedOnly_ || p == timerRoot_)
	return true;
    return false;
  } else
    return true;
}

WContainerWidget *WApplication::dialogWindow(bool create)
{
  if (dialogWindow_ == 0 && create) {
    dialogWindow_ = new WContainerWidget(domRoot_);
    dialogWindow_->setStyleClass("Wt-dialogwindow");

    addAutoJavaScript
      ("{"
       "" "var w=" + dialogWindow_->jsRef() + ";"
       "" "if (w && w.style.display != 'none') {"
       ""   "var ml='-' + Math.round(w.clientWidth/2) + 'px';"
       ""   "if (w.style.marginLeft != ml)"
       ""     "w.style.marginLeft=ml;"
       ""   "var mt='-' + Math.round(w.clientHeight/2) + 'px';"
       ""   "if (w.style.marginTop != mt)"
       ""     "w.style.marginTop=mt;"
       "" "}"
       "}");
  }

  return dialogWindow_;
}

std::string WApplication::sessionId() const
{
  return session_->sessionId();
}

void WApplication::useStyleSheet(const std::string& uri)
{
  styleSheets_.push_back(uri);
  ++styleSheetsAdded_;
}

void WApplication::useStyleSheet(const std::string& uri,
				 const std::string& cond)
{
  if (environment().agentIE()) {
    bool display = false;

    int thisVersion = environment().agentIEMobile() ? 5 :
      environment().agentIE6() ? 6 : 7;
    enum { lte, lt, eq, gt, gte } condition = eq;

    bool invert = false;
    std::string r = cond;

    while (!r.empty()) {
      if (r.substr(0, 3) == "IE ") {
	r = r.substr(3);
      } else if (r[0] == '!') {
	r = r.substr(1);
	invert = !invert;
      } else if (r.substr(0, 4) == "lte ") {
	r = r.substr(4);
	condition = lte;
      } else if (r.substr(0, 3) == "lt ") {
	r = r.substr(3);
	condition = lt;
      } else if (r.substr(0, 3) == "gt ") {
	r = r.substr(3);
	condition = gt;
      } else if (r.substr(0, 3) == "gte ") {
	r = r.substr(4);
	condition = gte;
      } else {
	try {
	  int version = boost::lexical_cast<int>(r);
	  switch (condition) {
	  case eq:  display = thisVersion == version; break;
	  case lte: display = thisVersion <= version; break;
	  case lt:  display = thisVersion <  version; break;
	  case gte: display = thisVersion >= version; break;
	  case gt:  display = thisVersion >  version; break;
	  }
	  if (invert)
	    display = !display;
	} catch (std::exception& e) {
	  log("error") << "Could not parse condition: '" << cond << "'";
	}
	r.clear();
      }
    }

    if (display)
      useStyleSheet(uri);
  }
}

const WEnvironment& WApplication::environment() const
{
  return session_->env();
}

WEnvironment& WApplication::env()
{
  return session_->env();
}

void WApplication::setTitle(const WString& title)
{
  if (session()->renderer().preLearning() || title_ != title) {
    title_ = title;
    titleChanged_ = true;
  }
}

std::string WApplication::url() const
{
  return fixRelativeUrl(session_->applicationUrl());
}

std::string WApplication::applicationName() const
{
  /*
   * The old behaviour included the internal path
   */
  if (!environment().internalPath().empty()) {
    std::string result = environment().internalPath();
    std::string::size_type slashpos = result.find_last_of('/');
    if (slashpos != std::string::npos)
      result.erase(0, slashpos+1);

    return result;
  } else
    return session_->applicationName();
}

std::string WApplication::fixRelativeUrl(const std::string& url) const
{
  if (url.find("://") != std::string::npos)
    return url;

  if (url.length() > 0 && url[0] == '#')
    return url;

  if (ajaxMethod_ == XMLHttpRequest) {
    if (!environment().javaScript()
	&& !environment().request_->pathInfo().empty())
      // This will break reverse proxies:
      // We could do a '../path/' trick? we could do this to correct
      // for the current internal path: as many '../' as there are
      // internal path folders. but why bother ? we need to fix URLs
      // in external resources anyway for reverse proxies
      if (!url.empty() && url[0] == '/')
	return session_->baseUrl() + url.substr(1);
      else
	return session_->baseUrl() + url;
    else
      return url;
  } else {
    if (!url.empty()) {
      if (url[0] != '/')
	return session_->absoluteBaseUrl() + url;
      else
	return environment().urlScheme() + "://"
	  + environment().hostName() + url;
    } else
      return url;
  }
}

void WApplication::quit()
{
  quited_ = true;
}

void WApplication::addExposedSignal(Wt::EventSignalBase *signal)
{
  std::string s = signal->encodeCmd();
  exposedSignals_[s] = signal;

#ifdef WTDEBUG
  std::cerr << "WApplication::addExposedSignal: " << s << std::endl;
#endif
}

void WApplication::removeExposedSignal(Wt::EventSignalBase *signal)
{
  std::string s = signal->encodeCmd();

  if (exposedSignals_.erase(s)) {
#ifdef WTDEBUG
    std::cerr << " WApplication::removeExposedSignal: " << s << std::endl;    
#endif
  } else {
    std::cerr << " WApplication::removeExposedSignal of non-exposed "
	      << s << "??" << std::endl;    
  }
}

EventSignalBase *
WApplication::decodeExposedSignal(const std::string& signalName) const
{
  SignalMap::const_iterator i = exposedSignals_.find(signalName);

  if (i != exposedSignals_.end()) {
    WWidget *w = dynamic_cast<WWidget *>(i->second->sender());
    if (!w || isExposed(w))
      return i->second;
    else
      return 0;
  } else
    return 0;
}

EventSignalBase *
WApplication::decodeExposedSignal(const std::string& objectId,
				  const std::string& name)
{
  std::string signalName
    = (objectId == "app" ? formName() : objectId) + '.' + name;

  return decodeExposedSignal(signalName);
}

const WApplication::SignalMap& WApplication::exposedSignals() const
{
  return exposedSignals_;
}

std::string WApplication::addExposedResource(WResource *resource)
{
  exposedResources_[resource->formName()] = resource;

  std::string fn = resource->suggestedFileName();
  if (!fn.empty() && fn[0] != '/')
    fn = '/' + fn;

  return session_->mostRelativeUrl(fn)
    + "&resource=" + DomElement::urlEncode(resource->formName())
    + "&rand=" + boost::lexical_cast<std::string>(WtRandom::getUnsigned());
}

void WApplication::removeExposedResource(WResource *resource)
{
  exposedResources_.erase(resource->formName());
}

WResource *WApplication::decodeExposedResource(const std::string& resourceName) 
  const
{
  ResourceMap::const_iterator i = exposedResources_.find(resourceName);
  
  if (i != exposedResources_.end())
    return i->second;
  else
    return 0;
}

std::string WApplication::encodeObject(WObject *object)
{
  std::string result = "w" + object->uniqueId();

  encodedObjects_[result] = object;

  return result;
}

WObject *WApplication::decodeObject(const std::string& objectId) const
{
  ObjectMap::const_iterator i = encodedObjects_.find(objectId);

  if (i != encodedObjects_.end()) {
    return i->second;
  } else
    return 0;
}

void WApplication::setLocale(const std::string& locale)
{
  locale_ = locale;
  refresh();
}

void WApplication::setLocalizedStrings(WLocalizedStrings *translator)
{
  delete localizedStrings_;
  localizedStrings_ = translator;
}

void WApplication::refresh()
{
  localizedStrings_->refresh();
  widgetRoot_->refresh();

  if (title_.refresh())
    titleChanged_ = true;
}

void WApplication::redirect(const std::string& url)
{
  session_->redirect(url);
}

void WApplication::setTwoPhaseRenderingThreshold(int bytes)
{
  session_->renderer().setTwoPhaseThreshold(bytes);
}

void WApplication::setCookie(const std::string& name, const std::string& value,
			     int maxAge, const std::string& domain,
			     const std::string& path)
{
  session_->renderer().setCookie(name, value, maxAge, domain, path);
}

WApplication *WApplication::instance()
{
  WebSession *session = WebSession::instance();

  return session ? session->app() : 0;
}

int WApplication::maximumRequestSize() const
{
  return WebController::conf().maxRequestSize() * 1024;
}

std::string WApplication::docType() const
{
  return session_->docType();
}

bool WApplication::loadRsh()
{
  if (!rshLoaded_) {
    rshLoaded_ = true;

    if (session_->applicationName().empty())
      log("warn") << "Deploy-path ends with '/', using /?_= for "
	"internal paths";

    return true;
  } else
    return false;
}

bool WApplication::internalPathMatches(const std::string& path) const
{
  if (session_->renderer().preLearning())
    return false;
  else
    return pathMatches(Utils::terminate(newInternalPath_, '/'), path);
}

bool WApplication::pathMatches(const std::string& path,
			       const std::string& query)
{
  if (!query.empty() &&
      (query.length() > path.length()
       || path.substr(0, query.length()) != query)) {
    return false;
  } else
    return true;
}

std::string WApplication::internalPathNextPart(const std::string& path) const
{
  std::string current = Utils::terminate(newInternalPath_, '/');

  if (!pathMatches(current, path)) {
    log("warn") << "WApplication::internalPath(): path '"
		<< path << "' not within current path '" << newInternalPath_
		<< "'";
    return std::string();
  }

  int startPos = path.length();
  std::size_t t = current.find('/', startPos);

  std::string result;
  if (t == std::string::npos)
    result = current.substr(startPos);
  else
    result = current.substr(startPos, t - startPos);

  return result;
}

std::string WApplication::internalPath() const
{
  //  if (session_->renderer().preLearning())
  //  return std::string();
  //else
    return newInternalPath_;
}

void WApplication::setInternalPath(const std::string& path, bool emitChange)
{
  loadRsh();

  if (!session_->renderer().preLearning() && emitChange)
    changeInternalPath(path);
  else
    newInternalPath_ = path;

  internalPathChanged_ = true;
}

void WApplication::setState(const std::string& scope, const std::string& value)
{
  useStates_ = true;

  if (!session()->renderer().preLearning() && state(scope) == value)
    return;

  for (unsigned i = 0; i < states_.size(); ++i)
    if (states_[i].scope == scope) {
      states_[i].value = value;

      if (i + 1 < states_.size())
	states_.erase(states_.begin() + i + 1, states_.end());

      setInternalPathFromStates();
      return;
    }

  states_.push_back(State(scope, value));
  setInternalPathFromStates();
}

std::string WApplication::state(const std::string& scope) const
{
  for (unsigned i = 0; i < states_.size(); ++i)
    if (states_[i].scope == scope)
      return states_[i].value;

  return std::string();
}

void WApplication::setInternalPathFromStates()
{
  std::string p("/");

  for (unsigned i = 0; i < states_.size(); ++i) {
    p += states_[i].scope + '/' + states_[i].value + '/';
  }

  setInternalPath(p);
}

void WApplication::setStatesFromInternalPath()
{
  std::string scope;
  std::string value;

  enum { FindScope, FindValue } state = FindScope;

  unsigned j = 0;

  std::size_t i = newInternalPath_.empty()
    ? 0
    : newInternalPath_[0] == '/' ? 1 : 0;

  for (;;) {
    if (i >= newInternalPath_.length())
      break;

    std::size_t nextSlash = newInternalPath_.find('/', i);

    std::string s;
    s = newInternalPath_.substr(i, nextSlash - i);

    if (state == FindScope) {
      scope = s;
      state = FindValue;
    } else {
      value = s;

      if (j < states_.size()) {
	if (scope != states_[j].scope) 
	  log("error") << "Inconsistent state change?";
	if (states_[j].value != value) {
	  states_[j].value = value;
	  if (j + 1 < states_.size())
	    states_.erase(states_.begin() + j + 1, states_.end());
	  stateChanged.emit(scope, value);
	}
      } else {
	states_.push_back(State(scope, value));
	stateChanged.emit(scope, value);
      }

      state = FindScope;
      ++j;
    }

    i = nextSlash + 1;
  }

  if (j < states_.size()) {
    // we lost some states at the end -- propagate "" to the first one
    scope = states_[j].scope;
    states_.erase(states_.begin() + j, states_.end());
    stateChanged.emit(scope, std::string());
  }
}

void WApplication::changeInternalPath(const std::string& aPath)
{
  std::string path = aPath;

  if (path != newInternalPath_) {
    std::size_t fileStart = 0;
    std::size_t i = 0;
    std::size_t length = std::min(path.length(), newInternalPath_.length());
    for (; i < length; ++i) {
      if (path[i] == newInternalPath_[i]) {
	if (path[i] == '/')
	  fileStart = i+1;
      } else {
	i = fileStart;
	break;
      }
    }

    std::string common = path.substr(0, fileStart);

    for (;;) {
      common = Utils::terminate(common, '/');
      newInternalPath_ = path;
      std::string next = internalPathNextPart(common);

      if (!next.empty())
        newInternalPath_ = common + next;
      internalPathChanged.emit(common);

      if (next.empty()) {
	newInternalPath_ = path;
	break;
      }

      common = newInternalPath_;
    }

    if (useStates_)
      setStatesFromInternalPath();
  }
}

std::string WApplication::bookmarkUrl() const
{
  return bookmarkUrl(newInternalPath_);
}

std::string WApplication::bookmarkUrl(const std::string& internalPath) const
{
  if (!environment().javaScript())
    if (environment().agentIsSpiderBot())
      return session_->bookmarkUrl(internalPath);
    else
      return session_->mostRelativeUrl(internalPath);
  else
    // return session_->bookmarkUrl("") + '#' + internalPath;
    // to avoid an extra round trip
    return session_->bookmarkUrl(internalPath);
}

std::string WApplication::bookmarkUrl(const std::string& scope,
				      const std::string& newState)
{
  std::string currentPath = newInternalPath_;
  StateList currentStates = states_;

  setState(scope, newState);
  std::string result = bookmarkUrl();

  states_ = currentStates;
  newInternalPath_ = currentPath;

  return result;
}

WLogEntry WApplication::log(const std::string& type) const
{
  return session_->log(type);
}

void WApplication::enableUpdates()
{
  if (!serverPush_) {
    serverPush_ = new WServerPushResource(this);

    require(resourcesUrl() + "orbited.js");
    doJavaScript("setTimeout(\"Orbited.connect(eval,'none','"
		 + serverPush_->generateUrl() + "','0');\",1000);");
  }
}

bool WApplication::updatesEnabled() const
{
  return serverPush_ != 0;
}

void WApplication::triggerUpdate()
{
  if (!shouldTriggerUpdate_)
    return;

  if (serverPush_)
    serverPush_->triggerUpdate();
  else
    throw WtException("WApplication::update() called but server-triggered "
		      "updates not enabled using "
		      "WApplication::enableUpdates()"); 
}

WApplication::UpdateLock WApplication::getUpdateLock()
{
  return UpdateLock(*this);
}

class UpdateLockImpl
{
public:
  UpdateLockImpl(WApplication& app)
    : handler_(*(app.session()))
#ifdef THREADED
    ,sessionLock_(app.session()->mutex)
#endif // THREADED
  { 
    app.shouldTriggerUpdate_ = true;
#ifndef THREADED
    throw WtException("UpdateLock needs Wt with thread support");
#endif // THREADED
  }

  ~UpdateLockImpl()
  {
    wApp->shouldTriggerUpdate_ = false;
  }

private:
  WebSession::Handler handler_;

#ifdef THREADED
  boost::mutex::scoped_lock sessionLock_;
#endif // THREADED
};

WApplication::UpdateLock::UpdateLock(WApplication& app)
  : impl_(0)
{
  /*
   * If we are already handling this application, then we already have
   * exclusive access.
   */
  if (WApplication::instance() != &app)
    impl_ = new UpdateLockImpl(app);
}

WApplication::UpdateLock::UpdateLock(const UpdateLock& other)
{
  impl_ = other.impl_;
  other.impl_ = 0;
}

WApplication::UpdateLock::~UpdateLock()
{
  delete impl_;
}

void WApplication::doJavaScript(const std::string& javascript,
				bool afterLoaded)
{
  if (afterLoaded) {
    afterLoadJavaScript_ += javascript;
  } else {
    beforeLoadJavaScript_ += javascript;
    newBeforeLoadJavaScript_ += javascript;
  }
}

void WApplication::addAutoJavaScript(const std::string& javascript)
{
  autoJavaScript_ += javascript;
  autoJavaScriptChanged_ = true;
}

void WApplication::declareJavaScriptFunction(const std::string& name,
					     const std::string& function)
{
  doJavaScript(javaScriptClass_ + '.' + name + '=' + function + ';', false);
}

std::string WApplication::afterLoadJavaScript()
{
  std::string result = afterLoadJavaScript_;
  afterLoadJavaScript_.clear();
  return result;
}

std::string WApplication::newBeforeLoadJavaScript()
{
  std::string result = newBeforeLoadJavaScript_;
  newBeforeLoadJavaScript_.clear();
  return result;
}

std::string WApplication::beforeLoadJavaScript()
{
  newBeforeLoadJavaScript_.clear();
  return beforeLoadJavaScript_;
}

void WApplication::notify(const WEvent& e)
{
  WebController::instance()->notify(e);
}

void WApplication::processEvents()
{
  /* set timeout to allow other events to be interleaved */
  session_->doRecursiveEventLoop
    ("setTimeout(\"" + javaScriptClass_
     + "._p_.update(null,'" + javaScriptResponse_.encodeCmd()
     + "',null,false);\",0);");
}

void WApplication::handleJavaScriptResponse(WResponseEvent event)
{
  session_->unlockRecursiveEventLoop();
}

bool WApplication::require(const std::string& uri, const std::string& symbol)
{
  ScriptLibrary sl(uri, symbol);

  if (Utils::indexOf(scriptLibraries_, sl) == -1) {
    sl.beforeLoadJS = newBeforeLoadJavaScript_;
    newBeforeLoadJavaScript_.clear();

    scriptLibraries_.push_back(sl);
    ++scriptLibrariesAdded_;

    return true;
  } else
    return false;
}

bool WApplication::readConfigurationProperty(const std::string& name,
					     std::string& value)
{
  const Configuration::PropertyMap& properties
    = WebController::conf().properties();

  Configuration::PropertyMap::const_iterator i = properties.find(name);

  if (i != properties.end()) {
    value = i->second;
    return true;
  } else
    return false;
}

#ifndef JAVA
WServer::Exception::Exception(const std::string what)
  : what_(what)
{ }

WServer::Exception::~Exception() throw()
{ }
#endif // JAVA

}
