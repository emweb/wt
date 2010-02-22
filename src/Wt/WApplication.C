/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WDefaultLoadingIndicator"
#include "Wt/WMemoryResource"
#include "Wt/WServer"
#include "Wt/WText"

#include "WtException.h"
#include "WebSession.h"
#include "DomElement.h"
#include "Configuration.h"
#include "SoundManager.h"
#include "WebController.h"
#include "WebRequest.h"
#include "Utils.h"

#include <boost/pool/pool.hpp>

#ifdef min
#undef min
#endif

//#define WTDEBUG

namespace Wt {

const char *WApplication::RESOURCES_URL = "resourcesURL";

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

WApplication::WApplication(const WEnvironment& env)
  : session_(env.session_),
    titleChanged_(false),
    internalPathChanged_(this),
#ifndef WT_TARGET_JAVA
    serverPush_(0),
#ifndef WT_CNOR
    eventSignalPool_(new boost::pool<>(sizeof(EventSignal<>))),
#endif
    shouldTriggerUpdate_(false),
#endif // WT_TARGET_JAVA
    javaScriptClass_("Wt"),
    dialogCover_(0),
    quited_(false),
    rshLoaded_(false),
    exposedOnly_(0),
    loadingIndicator_(0),
    connected_(true),
    bodyHtmlClassChanged_(false),
    enableAjax_(false),
    scriptLibrariesAdded_(0),
    theme_("default"),
    styleSheetsAdded_(0),
    exposeSignals_(true),
    autoJavaScriptChanged_(false),
    soundManager_(0)
{
  session_->setApplication(this);
  locale_ = environment().locale();

  newInternalPath_ = environment().internalPath();
  internalPathIsChanged_ = false;

#ifndef WT_TARGET_JAVA
  localizedStrings_ = new WMessageResourceBundle();
#else
  localizedStrings_ = 0;
#endif // !WT_TARGET_JAVA

  domRoot_ = new WContainerWidget();
  WT_DEBUG(domRoot_->setObjectName("wt-dom-root"));
  domRoot_->load();

  if (session_->type() == Application)
    domRoot_->resize(WLength::Auto, WLength(100, WLength::Percentage));

  timerRoot_ = new WContainerWidget(domRoot_);
  WT_DEBUG(timerRoot_->setObjectName("wt-timer-root"));
  timerRoot_->resize(WLength::Auto, 0);
  timerRoot_->setPositionScheme(Absolute);

  if (session_->type() == Application) {
    ajaxMethod_ = XMLHttpRequest;

    domRoot2_ = 0;
    widgetRoot_ = new WContainerWidget(domRoot_);
    WT_DEBUG(widgetRoot_->setObjectName("wt-app-root"));
    widgetRoot_->resize(WLength(100, WLength::Percentage),
			WLength(100, WLength::Percentage));
  } else {
    ajaxMethod_ = DynamicScriptTag;

    domRoot2_ = new WContainerWidget();
    domRoot2_->load();
    widgetRoot_ = 0;
  }

  /*
   * Subset of typical CSS "reset" styles
   */
  styleSheet_.addRule("table", "border-collapse: collapse; border: 0px");
  styleSheet_.addRule("div, td, img",
		      "margin: 0px; padding: 0px; border: 0px");
  styleSheet_.addRule("td", "vertical-align: top; text-align: left;");
  styleSheet_.addRule("button", "white-space: nowrap");

  if (environment().contentType() == WEnvironment::XHTML1) {
    //styleSheet_.addRule("img", "margin: -5px 0px;");
    styleSheet_.addRule("button", "display: inline");
  }

  if (environment().agentIsGecko())
    styleSheet_.addRule("html", "overflow: auto;");

  /*
   * Standard Wt CSS styles: resources, button wrap and form validation
   */
  styleSheet_.addRule("iframe.Wt-resource",
		      "width: 0px; height: 0px; border: 0px;");
  if (environment().agentIsIE())
    styleSheet_.addRule("iframe.Wt-shim",
			"position: absolute; top: -1px; left: -1px; "
			"z-index: -1;"
			"opacity: 0; filter: alpha(opacity=0);"
			"border: none; margin: 0; padding: 0;");
  styleSheet_.addRule("button.Wt-wrap",
		      "border: 0px !important;"
		      "text-align: left;"
		      "margin: 0px !important;"
		      "padding: 0px !important;"
		      "font-size: inherit; "
		      "pointer: hand; cursor: pointer; cursor: hand;"
		      "background-color: transparent;"
		      "color: inherit;");
  styleSheet_.addRule("a.Wt-wrap", "text-decoration: none;");
  styleSheet_.addRule(".Wt-invalid", "background-color: #f79a9a;");
  styleSheet_.addRule("span.Wt-disabled", "color: gray;");
  styleSheet_.addRule("fieldset.Wt-disabled legend", "color: gray;");
  styleSheet_.addRule(".unselectable",
		      "-moz-user-select:-moz-none;"
		      "-khtml-user-select: none;"
		      "user-select: none;");
  styleSheet_.addRule(".selectable",
		      "-moz-user-select: text;"
		      "-khtml-user-select: normal;"
		      "user-select: text;");
  styleSheet_.addRule(".Wt-sbspacer", "float: right; width: 16px; height: 1px;"
		      "border: 0px; display: none;");
  styleSheet_.addRule("body.Wt-layout", std::string() +
		      "height: 100%; width: 100%;"
		      "margin: 0px; padding: 0px; border: none;"
		      + (environment().javaScript() ? "overflow:hidden" : ""));
  styleSheet_.addRule("html.Wt-layout", std::string() +
		      "height: 100%; width: 100%;"
		      "margin: 0px; padding: 0px; border: none;"
		      + (environment().javaScript() ? "overflow:hidden" : ""));

  if (environment().agentIsOpera())
    if (environment().userAgent().find("Mac OS X") != std::string::npos)
      styleSheet_.addRule("img.Wt-indeterminate", "margin: 4px 1px -3px 2px;");
    else
      styleSheet_.addRule("img.Wt-indeterminate", "margin: 4px 2px -3px 0px;");
  else
    if (environment().userAgent().find("Mac OS X") != std::string::npos)
      styleSheet_.addRule("img.Wt-indeterminate", "margin: 4px 3px 0px 4px;");
    else
      styleSheet_.addRule("img.Wt-indeterminate", "margin: 3px 3px 0px 4px;");

  showLoadingIndicator_ = new EventSignal<>("showload", this);
  hideLoadingIndicator_ = new EventSignal<>("hideload", this);

  setLoadingIndicator(new WDefaultLoadingIndicator());
}

void WApplication::setLoadingIndicator(WLoadingIndicator *indicator)
{
  delete loadingIndicator_;
  loadingIndicator_ = indicator;

  if (loadingIndicator_) {
    loadingIndicatorWidget_ = indicator->widget();
    domRoot_->addWidget(loadingIndicatorWidget_);

#ifndef WT_TARGET_JAVA
    showLoadingIndicator_->connect
      (SLOT(loadingIndicatorWidget_, WWidget::show));
    hideLoadingIndicator_->connect
      (SLOT(loadingIndicatorWidget_, WWidget::hide));
#else
    // stateless learning does not yet work
    JSlot *showLoadJS = new JSlot();
    showLoadJS->setJavaScript
      ("function(o,e) {"
       "" WT_CLASS ".inline('" + loadingIndicatorWidget_->id() + "');"
       "}");
    showLoadingIndicator_->connect(*showLoadJS);

    JSlot *hideLoadJS = new JSlot();
    hideLoadJS->setJavaScript
      ("function(o,e) {"
       "" WT_CLASS ".hide('" + loadingIndicatorWidget_->id() + "');"
       "}");
    hideLoadingIndicator_->connect(*hideLoadJS);
#endif

    loadingIndicatorWidget_->hide();
  }
}

void WApplication::initialize()
{ }

#ifndef WT_TARGET_JAVA
void WApplication::finalize()
{ }
#endif // !WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
WMessageResourceBundle& WApplication::messageResourceBundle() const
{
  return *(dynamic_cast<WMessageResourceBundle *>(localizedStrings_));
}
#endif // !WT_TARGET_JAVA

std::string WApplication::onePixelGifUrl()
{
  if (onePixelGifUrl_.empty()) {
    WMemoryResource *w = new WMemoryResource("image/gif", this);

    static const unsigned char gifData[]
      = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00,
	  0x80, 0x00, 0x00, 0xdb, 0xdf, 0xef, 0x00, 0x00, 0x00, 0x21,
	  0xf9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
	  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44,
	  0x01, 0x00, 0x3b };

    w->setData(gifData, 43);
    onePixelGifUrl_ = w->url();
  }

  return onePixelGifUrl_;
}

WApplication::~WApplication()
{
  delete showLoadingIndicator_;
  delete hideLoadingIndicator_;

  dialogCover_ = 0;

  WContainerWidget *tmp = domRoot_;
  domRoot_ = 0;
  delete tmp;

  tmp = domRoot2_;
  domRoot2_ = 0;
  delete tmp;

  delete localizedStrings_;

  styleSheet_.clear();

  session_->setApplication(0);

#ifndef WT_TARGET_JAVA
  delete eventSignalPool_;
#endif
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
#ifndef WT_TARGET_JAVA
  std::string result = "resources/";
  readConfigurationProperty(WApplication::RESOURCES_URL, result);

  if (!result.empty() && result[result.length()-1] != '/')
    result += '/';

  return WApplication::instance()->fixRelativeUrl(result);
#else
  const std::string* path = WApplication::instance()->session_->controller()
    ->configuration().property(WApplication::RESOURCES_URL);
  /*
   * Arghll... we should in fact know when we need the absolute URL: only
   * when we are having a request.pathInfo().
   */
  if (path == "/wt-resources/") {
    std::string result = 
      WApplication::instance()->environment().deploymentPath();
    if (!result.empty() && result[result.length() - 1] == '/')
      return result + path->substr(1);
    else
      return result + *path;
  } else 
    return *path;
#endif // WT_TARGET_JAVA
}

void WApplication::bindWidget(WWidget *widget, const std::string& domId)
{
  if (session_->type() != WidgetSet)
    throw WtException("WApplication::bind() can be used only "
		      "in WidgetSet mode.");

  widget->setId(domId);
  domRoot2_->addWidget(widget);
}

WContainerWidget *WApplication::dialogCover(bool create)
{
  if (dialogCover_ == 0 && create) {
    dialogCover_ = new WContainerWidget(domRoot_);
    dialogCover_->setStyleClass("Wt-dialogcover");
    dialogCover_->hide();
  }

  return dialogCover_;
}

void WApplication::constrainExposed(WWidget *w)
{
  exposedOnly_ = w;
}

bool WApplication::isExposed(WWidget *w) const
{
  if (w != domRoot_ && exposedOnly_) {
    for (WWidget *p = w; p; p = p->parent())
      if (p == exposedOnly_ || p == timerRoot_)
	return true;
    return false;
  } else {
    WWidget *p = w->adam();
    return (p == domRoot_ || p == domRoot2_);
  }
}

std::string WApplication::sessionId() const
{
  return session_->sessionId();
}

void WApplication::setCssTheme(const std::string& theme)
{
  // TODO: allow modifying the theme
  theme_ = theme;
}

void WApplication::useStyleSheet(const std::string& uri)
{
  styleSheets_.push_back(uri);
  ++styleSheetsAdded_;
}

void WApplication::useStyleSheet(const std::string& uri,
				 const std::string& condition)
{
  if (environment().agentIsIE()) {
    bool display = false;

    int thisVersion = 4;

    switch (environment().agent()) {
    case WEnvironment::IEMobile:
      thisVersion = 5; break;
    case WEnvironment::IE6:
      thisVersion = 6; break;
    default:
      thisVersion = 7;
    }

    enum { lte, lt, eq, gt, gte } cond = eq;

    bool invert = false;
    std::string r = condition;

    while (!r.empty()) {
      if (r.length() >= 3 && r.substr(0, 3) == "IE ") {
	r = r.substr(3);
      } else if (r[0] == '!') {
	r = r.substr(1);
	invert = !invert;
      } else if (r.length() >= 4 && r.substr(0, 4) == "lte ") {
	r = r.substr(4);
	cond = lte;
      } else if (r.length() >= 3 && r.substr(0, 3) == "lt ") {
	r = r.substr(3);
	cond = lt;
      } else if (r.length() >= 3 && r.substr(0, 3) == "gt ") {
	r = r.substr(3);
	cond = gt;
      } else if (r.length() >= 4 && r.substr(0, 4) == "gte ") {
	r = r.substr(4);
	cond = gte;
      } else {
	try {
	  int version = boost::lexical_cast<int>(r);
	  switch (cond) {
	  case eq:  display = thisVersion == version; break;
	  case lte: display = thisVersion <= version; break;
	  case lt:  display = thisVersion <  version; break;
	  case gte: display = thisVersion >= version; break;
	  case gt:  display = thisVersion >  version; break;
	  }
	  if (invert)
	    display = !display;
	} catch (std::exception& e) {
	  log("error") << "Could not parse condition: '" << condition << "'";
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

std::string WApplication::fixRelativeUrl(const std::string& url) const
{
  if (url.find("://") != std::string::npos)
    return url;

  if (url.length() > 0 && url[0] == '#')
    return url;

  if (ajaxMethod_ == XMLHttpRequest) {
    if (!environment().javaScript()
	&& !WebSession::Handler::instance()->request()->pathInfo().empty())
      // This will break reverse proxies:
      // We could do a '../path/' trick? we could do this to correct
      // for the current internal path: as many '../' as there are
      // internal path folders. but why bother ? we need to fix URLs
      // in external resources anyway for reverse proxies
      if (!url.empty() && url[0] == '/')
	return /*session_->baseUrl() + url.substr(1) */ url;
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
    if (!w || isExposed(w) || boost::ends_with(signalName, ".resized"))
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
    = (objectId == "app" ? id() : objectId) + '.' + name;

  return decodeExposedSignal(signalName);
}

const WApplication::SignalMap& WApplication::exposedSignals() const
{
  return exposedSignals_;
}

std::string WApplication::addExposedResource(WResource *resource)
{
  exposedResources_[resource->id()] = resource;

  std::string fn = resource->suggestedFileName();
  if (!fn.empty() && fn[0] != '/')
    fn = '/' + fn;

  return session_->mostRelativeUrl(fn)
    + "&request=resource&resource=" + Utils::urlEncode(resource->id())
    + "&rand=" + boost::lexical_cast<std::string>(WtRandom::getUnsigned());
}

void WApplication::removeExposedResource(WResource *resource)
{
  exposedResources_.erase(resource->id());
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

void WApplication::setLocale(const WT_LOCALE& locale)
{
  locale_ = locale;
  refresh();
}

void WApplication::setBodyClass(const std::string& styleClass)
{
  bodyClass_ = styleClass;
  bodyHtmlClassChanged_ = true;
}

void WApplication::setHtmlClass(const std::string& styleClass)
{
  htmlClass_ = styleClass;
  bodyHtmlClassChanged_ = true;
}

EventSignal<WKeyEvent>& WApplication::globalKeyWentDown()
{
  return domRoot_->keyWentDown();
}

EventSignal<WKeyEvent>& WApplication::globalKeyWentUp()
{
  return domRoot_->keyWentUp();
}

EventSignal<WKeyEvent>& WApplication::globalKeyPressed()
{
  return domRoot_->keyPressed();
}

EventSignal<>& WApplication::globalEnterPressed()
{
  return domRoot_->enterPressed();
}

EventSignal<>& WApplication::globalEscapePressed()
{
  return domRoot_->escapePressed();
}

void WApplication::setLocalizedStrings(WLocalizedStrings *translator)
{
  delete localizedStrings_;
  localizedStrings_ = translator;
}

void WApplication::refresh()
{
  if (localizedStrings_)
    localizedStrings_->refresh();

  if (domRoot2_) {
    domRoot2_->refresh();
  } else {
    widgetRoot_->refresh();
  }

  if (title_.refresh())
    titleChanged_ = true;
}

void WApplication::enableAjax()
{
  enableAjax_ = true;

  afterLoadJavaScript_ = newBeforeLoadJavaScript_ + afterLoadJavaScript_;
  newBeforeLoadJavaScript_.clear();

  domRoot_->enableAjax();

  if (domRoot2_)
    domRoot2_->enableAjax();
}

void WApplication::redirect(const std::string& url)
{
  session_->redirect(url);
}

void WApplication::redirectToSession(const std::string& newSessionId)
{
  const Configuration& conf = session_->controller()->configuration();

  std::string redirectUrl = bookmarkUrl();
  if (conf.sessionTracking() == Configuration::CookiesURL
      && environment().supportsCookies()) {
    std::string cookieName = environment().deploymentPath();
    setCookie(cookieName, newSessionId, -1);
  } else
    redirectUrl += "?wtd=" + newSessionId;

  redirect(redirectUrl);
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
  return session_->controller()->configuration().maxRequestSize() * 1024;
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
  if (query.length() <= path.length()
      && path.substr(0, query.length()) == query)
    return true;
  else
    return false;
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
  return newInternalPath_;
}

void WApplication::setInternalPath(const std::string& path, bool emitChange)
{
  loadRsh();

  if (!internalPathIsChanged_)
    oldInternalPath_ = newInternalPath_;

  if (!session_->renderer().preLearning() && emitChange)
    changeInternalPath(path);
  else
    newInternalPath_ = path;

  internalPathIsChanged_ = true;
}

#ifdef WT_WITH_OLD_INTERNALPATH_API
bool WApplication::oldInternalPathAPI() const
{
  std::string v;
  return readConfigurationProperty("oldInternalPathAPI", v) && v == "true";
}
#endif // WT_WITH_OLD_INTERNALPATH_API

void WApplication::changeInternalPath(const std::string& aPath)
{
  std::string path = aPath;

  // internal paths start with a '/'; other anchor changes are not reacted on
  if (path != newInternalPath_ && (path.empty() || path[0] == '/')) {
    std::string v;

#ifdef WT_WITH_OLD_INTERNALPATH_API
    if (oldInternalPathAPI()) {
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
	internalPathChanged().emit(common);

	if (next.empty()) {
	  newInternalPath_ = path;
	  break;
	}

	common = newInternalPath_;
      }

      return;
    }
#endif // WT_WITH_OLD_INTERNALPATH_API

    newInternalPath_ = path;
    internalPathChanged().emit(newInternalPath_);
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

WLogEntry WApplication::log(const std::string& type) const
{
  return session_->log(type);
}

#ifndef WT_TARGET_JAVA
void WApplication::enableUpdates(bool enabled)
{
  if (enabled)
    ++serverPush_;
  else
    --serverPush_;

  if ((enabled && serverPush_ == 1) || (!enabled && serverPush_ == 0))
    doJavaScript(javaScriptClass_ + "._p_.setServerPush("
		 + (enabled ? "true" : "false") + ");");
}

void WApplication::triggerUpdate()
{
  if (!shouldTriggerUpdate_)
    return;

  if (serverPush_ > 0)
    session_->pushUpdates();
  else
    throw WtException("WApplication::triggerUpdate() called but "
		      "server-triggered updates not enabled using "
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
    : handler_(*(app.session()), true)
  { 
    app.shouldTriggerUpdate_ = true;
#ifndef WT_THREADED
    throw WtException("UpdateLock needs Wt with thread support");
#endif // WT_THREADED
  }

  ~UpdateLockImpl()
  {
    wApp->shouldTriggerUpdate_ = false;
  }

private:
  WebSession::Handler handler_;
};

WApplication::UpdateLock::UpdateLock(WApplication& app)
  : impl_(0)
{
  /*
   * If we are already handling this application, then we already have
   * exclusive access.
   *
   * However, this is not the way to detect because a user may also just
   * have done attachThread(): we need to check if we already have
   * a handler
   */
  WebSession::Handler *handler = WebSession::Handler::instance();

  if (!handler || !handler->haveLock() || handler->session() != app.session_)
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

#endif // WT_TARGET_JAVA

void WApplication::doJavaScript(const std::string& javascript,
				bool afterLoaded)
{
  if (afterLoaded) {
    afterLoadJavaScript_ += javascript;
    afterLoadJavaScript_ += '\n';
  } else {
    beforeLoadJavaScript_ += javascript;
    beforeLoadJavaScript_ += '\n';
    newBeforeLoadJavaScript_ += javascript;
    newBeforeLoadJavaScript_ += '\n';
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
  session_->notify(e);
}

void WApplication::processEvents()
{
  /* set timeout to allow other events to be interleaved */
  doJavaScript("setTimeout(\"" + javaScriptClass_
	       + "._p_.update(null,'none',null,false);\",0);");

  session_->doRecursiveEventLoop();
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

#ifndef WT_TARGET_JAVA
bool WApplication::readConfigurationProperty(const std::string& name,
					     std::string& value)
{
  const std::string* property
    = WApplication::instance()->session_->controller()
    ->configuration().property(name);

  if (property) {
    value = *property;
    return true;
  } else
    return false;
}
#else
std::string *WApplication::readConfigurationProperty(const std::string& name,
						     const std::string& value)
{
  std::string* property
    = WApplication::instance()->session_->controller()
    ->configuration().property(name);

  if (property)
    return property;
  else
    return &value;
}
#endif // WT_TARGET_JAVA

bool WApplication::debug() const
{
  return session_->debug();
}

SoundManager *WApplication::getSoundManager()
{
  if (!soundManager_)
    soundManager_ = new SoundManager(this);

  return soundManager_;
}

#ifndef WT_TARGET_JAVA
WServer::Exception::Exception(const std::string what)
  : what_(what)
{ }

WServer::Exception::~Exception() throw()
{ }
#endif // WT_TARGET_JAVA

}
