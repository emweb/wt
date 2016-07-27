/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <fstream>
#include <boost/lexical_cast.hpp>

#include "Wt/Utils"
#include "Wt/WApplication"
#include "Wt/WCombinedLocalizedStrings"
#include "Wt/WContainerWidget"
#include "Wt/WCssTheme"
#include "Wt/WDate"
#include "Wt/WDefaultLoadingIndicator"
#include "Wt/WException"
#include "Wt/WMemoryResource"
#include "Wt/WServer"

#include "WebSession.h"
#include "DomElement.h"
#include "Configuration.h"
#include "SoundManager.h"
#include "WebController.h"
#include "WebUtils.h"

#include <boost/pool/pool.hpp>

#ifdef min
#undef min
#endif

namespace skeletons {
  extern const char * Wt_xml1;
}

namespace Wt {

LOGGER("WApplication");

#if !(defined(DOXYGEN_ONLY) || defined(WT_TARGET_JAVA))
const WtLibVersion WT_INCLUDED_VERSION = WtLibVersion();
#endif

const char *WApplication::RESOURCES_URL = "resourcesURL";

MetaHeader::MetaHeader(MetaHeaderType aType,
		       const std::string& aName,
		       const WString& aContent,
		       const std::string& aLang,
		       const std::string& aUserAgent)
  : type(aType), name(aName), lang(aLang), userAgent(aUserAgent),
    content(aContent)
{ }

WApplication::ScriptLibrary::ScriptLibrary(const std::string& anUri,
					   const std::string& aSymbol)
  : uri(anUri), symbol(aSymbol)
{ }

WApplication::MetaLink::MetaLink(const std::string &aHref,
				 const std::string &aRel,
				 const std::string &aMedia,
				 const std::string &aHreflang,
				 const std::string &aType,
				 const std::string &aSizes,
				 bool aDisabled)
  : href(aHref), rel(aRel), media(aMedia), hreflang(aHreflang), type(aType), 
    sizes(aSizes), disabled(aDisabled)
{ }

bool WApplication::ScriptLibrary::operator< (const ScriptLibrary& other) const
{
  return uri < other.uri;
}

bool WApplication::ScriptLibrary::operator== (const ScriptLibrary& other) const
{
  return uri == other.uri;
}

WApplication::WApplication(const WEnvironment& env
#if !(defined(DOXYGEN_ONLY) || defined(WT_TARGET_JAVA))
			   , WtLibVersion
#endif
)
  : session_(env.session_),
#ifndef WT_CNOR
    weakSession_(session_->shared_from_this()),
#endif // WT_CNOR
    titleChanged_(false),
    closeMessageChanged_(false),
    localeChanged_(false),
    localizedStrings_(0),
    internalPathChanged_(this),
    serverPush_(0),
    serverPushChanged_(true),
#ifndef WT_CNOR
    eventSignalPool_(new boost::pool<>(sizeof(EventSignal<>))),
#endif // WT_CNOR
    javaScriptClass_("Wt"),
    quitted_(false),
    onePixelGifR_(0),
    internalPathsEnabled_(false),
    exposedOnly_(0),
    loadingIndicator_(0),
    connected_(true),
    bodyHtmlClassChanged_(true),
    enableAjax_(false),
#ifndef WT_TARGET_JAVA
    initialized_(false),
#endif // WT_TARGET_JAVA
    selectionStart_(-1),
    selectionEnd_(-1),
    layoutDirection_(LeftToRight),
    scriptLibrariesAdded_(0),
    theme_(0),
    styleSheetsAdded_(0),
    exposeSignals_(true),
    newBeforeLoadJavaScript_(0),
    autoJavaScriptChanged_(false),
#ifndef WT_DEBUG_JS
    newJavaScriptPreamble_(0),
#endif // WT_DEBUG_JS
    customJQuery_(false),
    showLoadingIndicator_("showload", this),
    hideLoadingIndicator_("hideload", this),
    unloaded_(this, "Wt-unload"),
    soundManager_(0)
{
  session_->setApplication(this);
  locale_ = environment().locale();

  renderedInternalPath_ = newInternalPath_ = environment().internalPath();
  internalPathIsChanged_ = false;
  internalPathDefaultValid_ = true;
  internalPathValid_ = true;

  theme_ = new WCssTheme("default", this);

#ifndef WT_TARGET_JAVA
  setLocalizedStrings(new WMessageResourceBundle());
#else
  setLocalizedStrings(0);
#endif // !WT_TARGET_JAVA

  if (!environment().javaScript() && environment().agentIsIE()) {
    /*
     * WARNING: Similar code in WebRenderer.C must be kept in sync for 
     *          plain boot.
     */
    if (environment().agent() < WEnvironment::IE9) {
      const Configuration& conf = environment().server()->configuration(); 
      bool selectIE7 = conf.uaCompatible().find("IE8=IE7")
	!= std::string::npos;

      if (selectIE7)
	addMetaHeader(MetaHttpHeader, "X-UA-Compatible", "IE=7");
    } else if (environment().agent() == WEnvironment::IE9) {
	addMetaHeader(MetaHttpHeader, "X-UA-Compatible", "IE=9");
    } else if (environment().agent() == WEnvironment::IE10) {
	addMetaHeader(MetaHttpHeader, "X-UA-Compatible", "IE=10");
    } else {
	addMetaHeader(MetaHttpHeader, "X-UA-Compatible", "IE=11");
    }
  }

  domRoot_ = new WContainerWidget();
  domRoot_->setGlobalUnfocused(true);
  domRoot_->setStyleClass("Wt-domRoot");

  if (session_->type() == Application)
    domRoot_->resize(WLength::Auto, WLength(100, WLength::Percentage));

  timerRoot_ = new WContainerWidget(domRoot_);
  timerRoot_->setId("Wt-timers");
  timerRoot_->resize(WLength::Auto, 0);
  timerRoot_->setPositionScheme(Absolute);

  if (session_->type() == Application) {
    ajaxMethod_ = XMLHttpRequest;

    domRoot2_ = 0;
    widgetRoot_ = new WContainerWidget(domRoot_);
    widgetRoot_->resize(WLength::Auto,
			WLength(100, WLength::Percentage));
  } else {
    ajaxMethod_ = DynamicScriptTag;

    domRoot2_ = new WContainerWidget();
    widgetRoot_ = 0;
  }

  // a define so that it shouts at us !
  #define RTL ".Wt-rtl "

  /*
   * Subset of typical CSS "reset" styles, only those that are needed
   * for Wt's built-in widgets and are relatively harmless.
   */
  styleSheet_.addRule("table", "border-collapse: collapse; border: 0px;"
		      "border-spacing: 0px");
  styleSheet_.addRule("div, td, img",
		      "margin: 0px; padding: 0px; border: 0px");
  styleSheet_.addRule("td", "vertical-align: top;");
  styleSheet_.addRule("td", "text-align: left;");
  styleSheet_.addRule(RTL "td", "text-align: right;");
  styleSheet_.addRule("button", "white-space: nowrap;");
  styleSheet_.addRule("video", "display: block");

  if (environment().agentIsGecko())
    styleSheet_.addRule("html", "overflow: auto;");

  /*
   * Standard Wt CSS styles: resources, button wrap and form validation
   */
  styleSheet_.addRule("iframe.Wt-resource",
		      "width: 0px; height: 0px; border: 0px;");
  if (environment().agentIsIElt(9))
    styleSheet_.addRule("iframe.Wt-shim",
			"position: absolute; top: -1px; left: -1px; "
			"z-index: -1;"
			"opacity: 0; filter: alpha(opacity=0);"
			"border: none; margin: 0; padding: 0;");
  styleSheet_.addRule(".Wt-wrap",
		      "border: 0px;"
		      "margin: 0px;"
		      "padding: 0px;"
		      "font: inherit; "
		      "cursor: pointer; cursor: hand;"
		      "background: transparent;"
		      "text-decoration: none;"
		      "color: inherit;");

  styleSheet_.addRule(".Wt-wrap", "text-align: left;");
  styleSheet_.addRule(RTL ".Wt-wrap", "text-align: right;");
  styleSheet_.addRule("div.Wt-chwrap", "width: 100%; height: 100%");

  if (environment().agentIsIE())
    styleSheet_.addRule(".Wt-wrap",
			"margin: -1px 0px -3px;");
  //styleSheet_.addRule("a.Wt-wrap", "text-decoration: none;");
  styleSheet_.addRule(".unselectable",
		      "-moz-user-select:-moz-none;"
		      "-khtml-user-select: none;"
		      "-webkit-user-select: none;"
		      "user-select: none;");
  styleSheet_.addRule(".selectable",
		      "-moz-user-select: text;"
		      "-khtml-user-select: normal;"
		      "-webkit-user-select: text;"
		      "user-select: text;");
  styleSheet_.addRule(".Wt-domRoot", "position: relative;");
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

  if (environment().supportsCss3Animations()) {
    std::string prefix = "";
    if (environment().agentIsWebKit())
      prefix = "webkit-";
    else if (environment().agentIsGecko())
      prefix = "moz-";

    useStyleSheet(WApplication::relativeResourcesUrl()
		  + prefix + "transitions.css");
  }

  setLoadingIndicator(new WDefaultLoadingIndicator());

  unloaded_.connect(this, &WApplication::doUnload);
}

void WApplication::setJavaScriptClass(const std::string& javaScriptClass)
{
  if (session_->type() != Application)
    javaScriptClass_ = javaScriptClass;
}

void WApplication::setLoadingIndicator(WLoadingIndicator *indicator)
{
#ifdef WT_TARGET_JAVA
  if (!loadingIndicator_) {
    showLoadingIndicator_.connect(showLoadJS);
    hideLoadingIndicator_.connect(hideLoadJS);
  }
#endif

  delete loadingIndicator_;
  loadingIndicator_ = indicator;

  if (loadingIndicator_) {
    loadingIndicatorWidget_ = indicator->widget();
    domRoot_->addWidget(loadingIndicatorWidget_);

#ifndef WT_TARGET_JAVA
    showLoadingIndicator_.connect(loadingIndicatorWidget_, &WWidget::show);
    hideLoadingIndicator_.connect(loadingIndicatorWidget_, &WWidget::hide);
#else
    // stateless learning does not yet work
    showLoadJS.setJavaScript
      ("function(o,e) {"
       "" WT_CLASS ".inline('" + loadingIndicatorWidget_->id() + "');"
       "}");

    hideLoadJS.setJavaScript
      ("function(o,e) {"
       "" WT_CLASS ".hide('" + loadingIndicatorWidget_->id() + "');"
       "}");
#endif

    loadingIndicatorWidget_->hide();
  }
}

#ifndef WT_TARGET_JAVA

void WApplication::initialize()
{ }

void WApplication::finalize()
{ }

#else

void WApplication::destroy()
{ }

#endif // !WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
WMessageResourceBundle& WApplication::messageResourceBundle()
{
  return *(dynamic_cast<WMessageResourceBundle *>(localizedStrings()));
}
#endif // !WT_TARGET_JAVA

std::string WApplication::onePixelGifUrl()
{
  if (environment().agentIsIElt(7)) {
    if (!onePixelGifR_) {
      WMemoryResource *w = new WMemoryResource("image/gif", this);
  
      static const unsigned char gifData[]
	= { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00,
            0x80, 0x00, 0x00, 0xdb, 0xdf, 0xef, 0x00, 0x00, 0x00, 0x21,
            0xf9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
            0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44,
            0x01, 0x00, 0x3b };

      w->setData(gifData, 43);
      onePixelGifR_ = w;
    }

    return onePixelGifR_->url();
  } else
    return "data:image/gif;base64,"
      "R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7";
}

WApplication::~WApplication()
{
  timerRoot_ = 0; // marker for being deleted

  WContainerWidget *r = domRoot_;
  domRoot_ = 0;
  delete r;

  delete domRoot2_;
  domRoot2_ = 0;

  delete localizedStrings_;

  styleSheet_.clear();

  session_->setApplication(0);

#ifndef WT_TARGET_JAVA
  delete eventSignalPool_;
#endif
}

void WApplication::attachThread(bool attach)
{
#ifndef WT_CNOR
  if (attach) {
    boost::shared_ptr<WebSession> session = weakSession_.lock();
    if (session)
      WebSession::Handler::attachThreadToSession(session);
    else
      session_->attachThreadToLockedHandler();
  } else
    WebSession::Handler::attachThreadToSession(boost::shared_ptr<WebSession>());
#else
  if (attach)
    WebSession::Handler::attachThreadToSession(session_);
  else
    WebSession::Handler::attachThreadToSession(boost::shared_ptr<WebSession>());
#endif
}

void WApplication::setAjaxMethod(AjaxMethod method)
{
  ajaxMethod_ = method;
}

std::string WApplication::relativeResourcesUrl()
{
#ifndef WT_TARGET_JAVA
  std::string result = "resources/";
  readConfigurationProperty(WApplication::RESOURCES_URL, result);

  if (!result.empty() && result[result.length()-1] != '/')
    result += '/';

  return result;
#else
  WApplication *app = WApplication::instance(); 
  const Configuration& conf = app->environment().server()->configuration(); 
  const std::string* path = conf.property(WApplication::RESOURCES_URL);
  /*
   * Arghll... we should in fact know when we need the absolute URL: only
   * when we are having a request.pathInfo().
   */
  if (path == "/wt-resources/") {
    std::string result = app->environment().deploymentPath();
    if (!result.empty() && result[result.length() - 1] == '/')
      return result + path->substr(1);
    else
      return result + *path;
  } else 
    return *path;
#endif // WT_TARGET_JAVA
}

std::string WApplication::resourcesUrl()
{
  return WApplication::instance()->resolveRelativeUrl
    (WApplication::relativeResourcesUrl());
}

#ifndef WT_TARGET_JAVA
std::string WApplication::appRoot()
{
  return WServer::instance()->appRoot();
}

std::string WApplication::docRoot() const
{
  return environment().getCgiValue("DOCUMENT_ROOT");
}

void WApplication::setConnectionMonitor(const std::string& jsFunction) {
  doJavaScript(javaScriptClass_
	       + "._p_.setConnectionMonitor("+ jsFunction + ")");

}

#endif // WT_TARGET_JAVA

void WApplication::bindWidget(WWidget *widget, const std::string& domId)
{
  if (session_->type() != WidgetSet)
    throw WException("WApplication::bindWidget() can be used only "
		     "in WidgetSet mode.");

  widget->setId(domId);
  domRoot2_->addWidget(widget);
}

void WApplication::pushExposedConstraint(WWidget *w)
{
  exposedOnly_ = w;
}

void WApplication::popExposedConstraint(WWidget *w)
{
  assert (exposedOnly_ == w);
  exposedOnly_ = 0;
}

void WApplication::addGlobalWidget(WWidget *w)
{
  domRoot_->addWidget(w);
}

void WApplication::removeGlobalWidget(WWidget *)
{ }

bool WApplication::isExposed(WWidget *w) const
{
  /*
   * This not right: for example a file upload is usually hidden while
   * uploading, but then could not receive the upload event

  if (!w->isVisible())
    return false;
  */

  if (w == domRoot_)
    return true;

  if (w->parent() == timerRoot_)
    return true;

  if (exposedOnly_)
    return exposedOnly_->isExposed(w);
  else {
    WWidget *p = w->adam();
    return (p == domRoot_ || p == domRoot2_);
  }
}

std::string WApplication::sessionId() const
{
  return session_->sessionId();
}

#ifndef WT_TARGET_JAVA
void WApplication::changeSessionId()
{
  session_->generateNewSessionId();
}
#endif // WT_TARGET_JAVA

void WApplication::setCssTheme(const std::string& theme)
{
  setTheme(new WCssTheme(theme, this));
}

void WApplication::setTheme(const WTheme *theme)
{
  theme_ = theme;
}

void WApplication::useStyleSheet(const WLink& link, const std::string& media)
{
  useStyleSheet(WCssStyleSheet(link, media));
}

void WApplication::useStyleSheet(const WLink& link,
				 const std::string& condition,
				 const std::string& media)
{
  useStyleSheet(WCssStyleSheet(link, media), condition);
}

void WApplication::useStyleSheet(const WCssStyleSheet& styleSheet,
				 const std::string& condition)
{

  if (styleSheet.link().isNull())
    throw WException(
        "WApplication::useStyleSheet stylesheet must have valid link!");

  bool display = true;

  if (!condition.empty()) {
    display = false;
    if (environment().agentIsIE()) {
      int thisVersion = 4;

      switch (environment().agent()) {
      case WEnvironment::IEMobile:
	thisVersion = 5; break;
      case WEnvironment::IE6:
	thisVersion = 6; break;
      case WEnvironment::IE7:
	thisVersion = 7; break;
      case WEnvironment::IE8:
	thisVersion = 8; break;
      default:
	thisVersion = 9; break;	
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
	    LOG_ERROR("Could not parse condition: '" << condition << "'");
	  }
	  r.clear();
	}
      }
    } 
  }

  if (display) {
    for (unsigned i = 0; i < styleSheets_.size(); ++i) {
      if (styleSheets_[i].link() == styleSheet.link()
	  && styleSheets_[i].media() == styleSheet.media()) {
	return;
      }
    }

    styleSheets_.push_back(styleSheet);
    ++styleSheetsAdded_;
  }
}

void WApplication::removeStyleSheet(const WLink& link)
{
  for (int i = (int)styleSheets_.size() - 1; i > -1; --i) {
    if (styleSheets_[i].link() == link) {
      WCssStyleSheet &sheet = styleSheets_[i];
      styleSheetsToRemove_.push_back(sheet);
      if (i > (int)styleSheets_.size() + styleSheetsAdded_ - 1)
        styleSheetsAdded_--;
      styleSheets_.erase(styleSheets_.begin() + i);
      break;
    }
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
  if (session_->renderer().preLearning() || title_ != title) {
    title_ = title;
    titleChanged_ = true;
  }
}

void WApplication::setConfirmCloseMessage(const WString& message)
{
  if (message != closeMessage_) {
    closeMessage_ = message;
    closeMessageChanged_ = true;
  }
}

std::string WApplication::url(const std::string& internalPath) const
{
  return resolveRelativeUrl(session_->mostRelativeUrl(internalPath));
}

std::string WApplication::makeAbsoluteUrl(const std::string& url) const
{
  return session_->makeAbsoluteUrl(url);
}

std::string WApplication::resolveRelativeUrl(const std::string& url) const
{
  return session_->fixRelativeUrl(url);
}

void WApplication::quit()
{
  quit(WString::tr("Wt.QuittedMessage"));
}

void WApplication::quit(const WString& restartMessage)
{
  quitted_ = true;
  quittedMessage_ = restartMessage;
}

WWidget *WApplication::findWidget(const std::string& name)
{
  WWidget *result = domRoot_->find(name);
  if (!result && domRoot2_)
    result = domRoot2_->find(name);

  return result;
}

void WApplication::doUnload()
{
  const Configuration& conf = environment().server()->configuration();

  if (conf.reloadIsNewSession())
    unload();
  else
    session_->setState(WebSession::Loaded, 5);
}

void WApplication::unload()
{
#ifndef WT_TARGET_JAVA
  if (session_->shouldDisconnect()) {
    if (connected_) {
      connected_ = false;
      LOG_INFO("Session disconnected on unload()");
    }

    return;
  }
#endif // WT_TARGET_JAVA

  quit();
}

void WApplication::handleJavaScriptError(const std::string& errorText)
{
  LOG_ERROR("JavaScript error: " << errorText);
  quit();
}

void WApplication::addExposedSignal(Wt::EventSignalBase *signal)
{
  std::string s = signal->encodeCmd();
  Utils::insert(exposedSignals_, s, signal);

  LOG_DEBUG("addExposedSignal: " << s);
}

void WApplication::removeExposedSignal(Wt::EventSignalBase *signal)
{
  std::string s = signal->encodeCmd();

  if (exposedSignals_.erase(s)) {
    justRemovedSignals_.insert(s);
    LOG_DEBUG("removeExposedSignal: " << s);
  } else {
    LOG_DEBUG("removeExposedSignal of non-exposed " << s << "??");
  }
}

EventSignalBase *
WApplication::decodeExposedSignal(const std::string& signalName) const
{
  SignalMap::const_iterator i = exposedSignals_.find(signalName);

  if (i != exposedSignals_.end()) {
    return i->second;
  } else
    return 0;
}

std::string WApplication::encodeSignal(const std::string& objectId,
				       const std::string& name) const
{
  return (objectId == "app" ? id() : objectId) + '.' + name;
}

std::string WApplication::resourceMapKey(WResource *resource)
{
  return resource->internalPath().empty()
    ? resource->id() : "/path/" + resource->internalPath();
}

std::string WApplication::addExposedResource(WResource *resource)
{
  exposedResources_[resourceMapKey(resource)] = resource;

  std::string fn = resource->suggestedFileName().toUTF8();
  if (!fn.empty() && fn[0] != '/')
    fn = '/' + fn;

  static unsigned long seq = 0;

  if (resource->internalPath().empty())
    return session_->mostRelativeUrl(fn)
      + "&request=resource&resource=" + Utils::urlEncode(resource->id())
      + "&rand=" + boost::lexical_cast<std::string>(seq++);
  else {
    fn = resource->internalPath() + fn;
    if (!session_->applicationName().empty() && fn[0] != '/')
      fn = '/' + fn;
    return session_->mostRelativeUrl(fn);
  }
}

bool WApplication::removeExposedResource(WResource *resource)
{
  std::string key = resourceMapKey(resource);
  ResourceMap::iterator i = exposedResources_.find(key);

  if (i != exposedResources_.end() && i->second == resource) {
#ifndef WT_TARGET_JAVA
    exposedResources_.erase(i);
#else
    exposedResources_.erase(key);
#endif
    return true;
  } else
    return false;
}

WResource *WApplication::decodeExposedResource(const std::string& resourceKey) 
  const
{
  ResourceMap::const_iterator i = exposedResources_.find(resourceKey);
  
  if (i != exposedResources_.end())
    return i->second;
  else {
    std::size_t j = resourceKey.rfind('/');
    if (j != std::string::npos && j > 1)
      return decodeExposedResource(resourceKey.substr(0, j));
    else
      return 0;
  }
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

void WApplication::setLocale(const WLocale& locale)
{
  locale_ = locale;
  localeChanged_ = true;
  refresh();
}

void WApplication::setBodyClass(const std::string& styleClass)
{
  bodyClass_ = styleClass;
  bodyHtmlClassChanged_ = true;
}

void WApplication::setLayoutDirection(LayoutDirection direction)
{
  if (direction != layoutDirection_) {
    layoutDirection_ = direction;
    bodyHtmlClassChanged_ = true;
  }
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

WLocalizedStrings *WApplication::localizedStrings()
{
  if (localizedStrings_->items().size() > 1)
    return localizedStrings_->items()[0];
  else
    return 0;
}

WMessageResourceBundle& WApplication::builtinLocalizedStrings()
{
  return *(dynamic_cast<WMessageResourceBundle *>
	   (localizedStrings_->items().back()));
}

void WApplication::setLocalizedStrings(WLocalizedStrings *translator)
{
  if (!localizedStrings_) {
    localizedStrings_ = new WCombinedLocalizedStrings();

    WMessageResourceBundle *defaultMessages = new WMessageResourceBundle();
    defaultMessages->useBuiltin(skeletons::Wt_xml1);
    localizedStrings_->add(defaultMessages);
  }

  if (localizedStrings_->items().size() > 1) {
    WLocalizedStrings *previous = localizedStrings_->items()[0];
    localizedStrings_->remove(previous);
    delete previous;
  }

  if (translator)
    localizedStrings_->insert(0, translator);
}

void WApplication::refresh()
{
  if (localizedStrings_)
    localizedStrings_->refresh();

  if (domRoot2_) {
    domRoot2_->refresh();
  } else {
    domRoot_->refresh();
  }

  if (title_.refresh())
    titleChanged_ = true;

  if (closeMessage_.refresh())
    closeMessageChanged_ = true;
}

void WApplication::enableAjax()
{
  enableAjax_ = true;

  streamBeforeLoadJavaScript(session_->renderer().beforeLoadJS_, false);
  streamAfterLoadJavaScript(session_->renderer().beforeLoadJS_);

  domRoot_->enableAjax();

  if (domRoot2_)
    domRoot2_->enableAjax();

  doJavaScript
    (WT_CLASS ".ajaxInternalPaths(" +
     WWebWidget::jsStringLiteral(resolveRelativeUrl(bookmarkUrl("/"))) + ");");
}

void WApplication::redirect(const std::string& url)
{
  session_->redirect(url);
}

void WApplication::redirectToSession(const std::string& newSessionId)
{
  std::string redirectUrl = bookmarkUrl();
  if (!session_->useUrlRewriting()) {
    std::string cookieName = environment().deploymentPath();
    setCookie(cookieName, newSessionId, -1, "", "", environment().urlScheme() == "https");
  } else
    redirectUrl += "?wtd=" + DomElement::urlEncodeS(newSessionId);

  redirect(redirectUrl);
}

std::string WApplication::encodeUntrustedUrl(const std::string& url) const
{
  /*
   * If url is an absolute URL, then we jump through a redirect
   * page, to strip the session ID from the referer URL, in case the
   * current page has the session ID in the URL.
   */

  bool needRedirect = (url.find("://") != std::string::npos
		       || boost::starts_with(url, "//"))
    && session_->hasSessionIdInUrl();

  if (needRedirect) {
    WebController *c = session_->controller();
    return "?request=redirect&url=" + Utils::urlEncode(url)
      + "&hash="
      + Utils::urlEncode(c->computeRedirectHash(url));
  } else
    return url;
}

void WApplication::setTwoPhaseRenderingThreshold(int bytes)
{
  session_->renderer().setTwoPhaseThreshold(bytes);
}

void WApplication::setCookie(const std::string& name,
			     const std::string& value, int maxAge,
			     const std::string& domain,
			     const std::string& path,
			     bool secure)
{
  WDateTime expires = WDateTime::currentDateTime();
  expires = expires.addSecs(maxAge);
  session_->renderer().setCookie(name, value, expires, domain, path, secure);
}

#ifndef WT_TARGET_JAVA
void WApplication::setCookie(const std::string& name,
			     const std::string& value,
			     const WDateTime& expires,
			     const std::string& domain,
			     const std::string& path,
			     bool secure)
{
  session_->renderer().setCookie(name, value, expires, domain, path, secure);
}
#endif // WT_TARGET_JAVA

void WApplication::removeCookie(const std::string& name,
				const std::string& domain,
				const std::string& path)
{
  session_->renderer().setCookie(name, std::string(),
				 WDateTime(WDate(1970,1,1)),
				 domain, path, false);
}

void WApplication::addMetaLink(const std::string &href,
			       const std::string &rel,
			       const std::string &media,
			       const std::string &hreflang,
			       const std::string &type,
			       const std::string &sizes,
			       bool disabled)
{
  if (environment().javaScript())
    LOG_WARN("WApplication::addMetaLink() with no effect");

  if (href.empty()) 
    throw WException("WApplication::addMetaLink() href cannot be empty!");
  if (rel.empty()) 
    throw WException("WApplication::addMetaLink() rel cannot be empty!");

  for (unsigned i = 0; i < metaLinks_.size(); ++i) {
    MetaLink& ml = metaLinks_[i];
    if (ml.href == href) {
      ml.rel = rel;
      ml.media = media;
      ml.hreflang = hreflang;
      ml.type = type;
      ml.sizes = sizes;
      ml.disabled = disabled;
      return;
    }
  }

  MetaLink ml(href, rel, media, hreflang, type, sizes, disabled);
  metaLinks_.push_back(ml);
}

void WApplication::removeMetaLink(const std::string &href)
{
  for (unsigned i = 0; i < metaLinks_.size(); ++i) {
    MetaLink& ml = metaLinks_[i];
    if (ml.href == href) {
      metaLinks_.erase(metaLinks_.begin() + i);
      return;
    }
  }
}

void WApplication::addMetaHeader(const std::string& name,
				 const WString& content,
				 const std::string& lang)
{
  addMetaHeader(MetaName, name, content, lang);
}

WString WApplication::metaHeader(MetaHeaderType type, const std::string& name) const
{
  for (unsigned i = 0; i < metaHeaders_.size(); ++i) {
    const MetaHeader& m = metaHeaders_[i];

    if (m.type == type && m.name == name)
      return m.content;
  }

  return WString::Empty;
}

void WApplication::addMetaHeader(MetaHeaderType type,
				 const std::string& name,
				 const WString& content,
				 const std::string& lang)
{
  if (environment().javaScript())
    LOG_WARN("WApplication::addMetaHeader() with no effect");

  /*
   * Replace or remove existing value
   */
  for (unsigned i = 0; i < metaHeaders_.size(); ++i) {
    MetaHeader& m = metaHeaders_[i];

    if (m.type == type && m.name == name) {
      if (content.empty())
	metaHeaders_.erase(metaHeaders_.begin() + i);
      else
	m.content = content;
      return;
    }
  }

  if (!content.empty())
    metaHeaders_.push_back(MetaHeader(type, name, content, lang,
				      std::string()));
}

void WApplication::removeMetaHeader(MetaHeaderType type,
				    const std::string& name)
{
  if (environment().javaScript())
    LOG_WARN("removeMetaHeader() with no effect");

  for (unsigned i = 0; i < metaHeaders_.size(); ++i) {
    MetaHeader& m = metaHeaders_[i];

    if (m.type == type && (name.empty() || m.name == name)) {
      metaHeaders_.erase(metaHeaders_.begin() + i);

      if (name.empty())
	--i;
      else
	break;
    }
  }
}

WApplication *WApplication::instance()
{
  WebSession *session = WebSession::instance();

  return session ? session->app() : 0;
}

::int64_t WApplication::maximumRequestSize() const
{
  return environment().server()->configuration().maxRequestSize();
}

std::string WApplication::docType() const
{
  return session_->docType();
}

void WApplication::enableInternalPaths()
{
  if (!internalPathsEnabled_) {
    internalPathsEnabled_ = true;

    doJavaScript
      (javaScriptClass() + "._p_.enableInternalPaths("
       + WWebWidget::jsStringLiteral(renderedInternalPath_)
       + ");");

    if (session_->useUglyInternalPaths())
      LOG_WARN("Deploy-path ends with '/', using /?_= for internal paths");
  }
}

Signal<std::string>& WApplication::internalPathChanged()
{
  enableInternalPaths();

  return internalPathChanged_;
}

bool WApplication::internalPathMatches(const std::string& path) const
{
  if (session_->renderer().preLearning())
    return false;
  else
    return pathMatches(Utils::append(newInternalPath_, '/'), path);
}

bool WApplication::pathMatches(const std::string& path,
			       const std::string& query)
{
  /* Returns whether the current path start with the query */
  if (query == path
      || (path.length() > query.length()
	  && path.substr(0, query.length()) == query
	  && (query[query.length() - 1] == '/' || path[query.length()] == '/')))
    return true;
  else
    return false;
}

std::string WApplication::internalPathNextPart(const std::string& path) const
{
  std::string subPath = internalSubPath(path);

  std::size_t t = subPath.find('/');

  if (t == std::string::npos)
    return subPath;
  else
    return subPath.substr(0, t);
}

std::string WApplication::internalSubPath(const std::string& path) const
{
  std::string current = Utils::append(newInternalPath_, '/');

  if (!pathMatches(current, path)) {
    LOG_WARN("internalPath(): path '"
	     << path << "' not within current path '" << internalPath()
	     << "'");
    return std::string();
  }

  return current.substr(path.length());
}

std::string WApplication::internalPath() const
{
  return Utils::prepend(newInternalPath_, '/');
}

void WApplication::setInternalPath(const std::string& path, bool emitChange)
{
  enableInternalPaths();

  if (!session_->renderer().preLearning() && emitChange)
    changeInternalPath(path);
  else
    newInternalPath_ = path;

  internalPathValid_ = true;
  internalPathIsChanged_ = true;
}

void WApplication::setInternalPathValid(bool valid)
{
  internalPathValid_ = valid;
}

void WApplication::setInternalPathDefaultValid(bool valid)
{
  internalPathDefaultValid_ = valid;
}

bool WApplication::changeInternalPath(const std::string& aPath)
{
  std::string path = Utils::prepend(aPath, '/');

  if (path != internalPath()) {
    renderedInternalPath_ = newInternalPath_ = path;
    internalPathValid_ = internalPathDefaultValid_;
    internalPathChanged_.emit(newInternalPath_);

    if (!internalPathValid_)
      internalPathInvalid_.emit(newInternalPath_);
  }

  return internalPathValid_;
}

bool WApplication::changedInternalPath(const std::string& path)
{
  if (!environment().internalPathUsingFragments())
    session_->setPagePathInfo(path);

  return changeInternalPath(path);
}

std::string WApplication::bookmarkUrl() const
{
  return bookmarkUrl(newInternalPath_);
}

std::string WApplication::bookmarkUrl(const std::string& internalPath) const
{
  // ? return session_->bookmarkUrl("") + '#' + internalPath;
  // to avoid an extra round trip
  return session_->bookmarkUrl(internalPath);
}

#ifndef WT_TARGET_JAVA
WLogEntry WApplication::log(const std::string& type) const
{
  return session_->log(type);
}
#endif // WT_TARGET_JAVA

void WApplication::enableUpdates(bool enabled)
{
  if (enabled) {
    if (serverPush_ == 0 && !WebSession::Handler::instance()->request())
      LOG_WARN("WApplication::enableUpdates(true): "
	       "should be called from within event loop");
    ++serverPush_;
  } else
    --serverPush_;

  if ((enabled && serverPush_ == 1) || (!enabled && serverPush_ == 0))
    serverPushChanged_ = true;
}

void WApplication::triggerUpdate()
{
  if (WebSession::Handler::instance()->request())
    return;

  if (!serverPush_)
    LOG_WARN("WApplication::triggerUpdate(): updates not enabled?");

  session_->setTriggerUpdate(true);
}

WApplication::UpdateLock WApplication::getUpdateLock()
{
  return UpdateLock(this);
}

#ifndef WT_TARGET_JAVA

class UpdateLockImpl
{
public:
  UpdateLockImpl(WApplication *app)
    : handler_(0)
  {
#ifdef WT_THREADED
    handler_ = new WebSession::Handler(app->weakSession_.lock(),
				       WebSession::Handler::TakeLock);
#endif // WT_THREADED
  }

#ifdef WT_THREADED
  ~UpdateLockImpl() {
    delete handler_;
  }
#endif // WT_THREADED

private:
  // Handler which we created for actual lock
  WebSession::Handler *handler_;
};

WApplication::UpdateLock::UpdateLock(WApplication *app)
  : impl_(0),
    ok_(true)
{
#ifndef WT_THREADED
  return;
#else
  /*
   * If we are already handling this application, then we already have
   * exclusive access, unless we are not having the lock (e.g. from a
   * WResource::handleRequest()).
   */
  WebSession::Handler *handler = WebSession::Handler::instance();

  boost::shared_ptr<WebSession> appSession = app->weakSession_.lock();
  if (handler && handler->haveLock() && handler->session() == appSession.get())
    return;

  if (appSession.get() && !appSession->dead())
    impl_ = new UpdateLockImpl(app);
  else
    ok_ = false;
#endif // WT_THREADED
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

#else

WApplication::UpdateLock::UpdateLock(WApplication *app)
{
  WebSession::Handler *handler = WebSession::Handler::instance();

  createdHandler_ = false;
  if (handler && handler->haveLock() && handler->session() == app->session_)
    return;

  new WebSession::Handler(app->session_, WebSession::Handler::TakeLock);

  createdHandler_ = true;
}

void WApplication::UpdateLock::release()
{
  if (createdHandler_) {
    WebSession::Handler::instance()->release();
  }
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
    newBeforeLoadJavaScript_ += javascript.length() + 1;
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

void WApplication::streamAfterLoadJavaScript(WStringStream& out)
{
  out << afterLoadJavaScript_;
  afterLoadJavaScript_.clear();
}

void WApplication::streamBeforeLoadJavaScript(WStringStream& out, bool all)
{
  streamJavaScriptPreamble(out, all);

  if (!all) {
    if (newBeforeLoadJavaScript_)
      out << beforeLoadJavaScript_.substr(beforeLoadJavaScript_.length()
					  - newBeforeLoadJavaScript_);
  } else {
    out << beforeLoadJavaScript_;
  }
  newBeforeLoadJavaScript_ = 0;
}

void WApplication::notify(const WEvent& e)
{
  session_->notify(e);
}

void WApplication::processEvents()
{
  /* set timeout to allow other events to be interleaved */
  doJavaScript("setTimeout(\"" + javaScriptClass_
	       + "._p_.update(null,'none',null,true);\",0);");

  waitForEvent();
}

void WApplication::waitForEvent()
{
  if (!environment().isTest())
    session_->doRecursiveEventLoop();
}

bool WApplication::require(const std::string& uri, const std::string& symbol)
{
  ScriptLibrary sl(uri, symbol);

  if (Utils::indexOf(scriptLibraries_, sl) == -1) {
    WStringStream ss;
    streamBeforeLoadJavaScript(ss, false);
    sl.beforeLoadJS = ss.str();

    scriptLibraries_.push_back(sl);
    ++scriptLibrariesAdded_;

    return true;
  } else
    return false;
}

bool WApplication::requireJQuery(const std::string& uri)
{
  customJQuery_ = true;
  return require(uri);
}

#ifndef WT_TARGET_JAVA
bool WApplication::readConfigurationProperty(const std::string& name,
					     std::string& value)
{
  WebSession *session = WebSession::instance();
  if (session)
    return session->env().server()->readConfigurationProperty(name, value);
  else
    return false;
}
#else
std::string *WApplication::readConfigurationProperty(const std::string& name,
						     const std::string& value)
{
  WebSession *session = WebSession::instance();
  if (session)
    return session->env().server()->readConfigurationProperty(name, value);
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
    soundManager_ = new SoundManager(domRoot());

  return soundManager_;
}

#ifdef WT_DEBUG_JS

void WApplication::loadJavaScript(const char *jsFile)
{
  if (!javaScriptLoaded(jsFile)) {
    javaScriptLoaded_.insert(jsFile);
    newJavaScriptToLoad_.push_back(jsFile);
  }
}

void WApplication::streamJavaScriptPreamble(WStringStream& out, bool all)
{
  if (all) {
    out << "window.currentApp = " + javaScriptClass_ + ";";
    for (std::set<const char *>::const_iterator i = javaScriptLoaded_.begin();
	 i != javaScriptLoaded_.end(); ++i)
      loadJavaScriptFile(out, *i);
  } else {
    if (!newJavaScriptToLoad_.empty()) {
      out << "window.currentApp = " + javaScriptClass_ + ";";
      for (unsigned i = 0; i < newJavaScriptToLoad_.size(); ++i)
	loadJavaScriptFile(out, newJavaScriptToLoad_[i]);
    }
  }

  newJavaScriptToLoad_.clear();
}

void WApplication::loadJavaScriptFile(WStringStream& out, const char *jsFile)
{
#define xstr(s) str(s)
#define str(s) #s
  std::string fname = std::string( xstr(WT_DEBUG_JS) "/") + jsFile;
  out << Utils::readFile(fname);
}

#else

void WApplication::loadJavaScript(const char *jsFile,
				  const WJavaScriptPreamble& preamble)
{
  if (!javaScriptLoaded(preamble.name)) {
    javaScriptLoaded_.insert(jsFile);
    javaScriptLoaded_.insert(preamble.name);

    javaScriptPreamble_.push_back(preamble);
    ++newJavaScriptPreamble_;
  }
}

void WApplication::streamJavaScriptPreamble(WStringStream& out, bool all)
{
  if (all)
    newJavaScriptPreamble_ = javaScriptPreamble_.size();

  for (unsigned i = javaScriptPreamble_.size() - newJavaScriptPreamble_;
       i < javaScriptPreamble_.size(); ++i) {
    const WJavaScriptPreamble& preamble = javaScriptPreamble_[i];
    std::string scope
      = preamble.scope == ApplicationScope ? javaScriptClass() : WT_CLASS;

    if (preamble.type == JavaScriptFunction) {
      out << scope << '.' << (char *)preamble.name
	  << " = function() { return ("
	  << (char *)preamble.src << ").apply(" << scope << ", arguments) };";
    } else {
      out << scope << '.' << (char *)preamble.name
	  << " = " << (char *)preamble.src << '\n';
    }
  }

  newJavaScriptPreamble_ = 0;
}

#endif

bool WApplication::javaScriptLoaded(const char *jsFile) const
{
  return javaScriptLoaded_.find(jsFile) != javaScriptLoaded_.end();
}

void WApplication::setFocus(const std::string& id,
			    int selectionStart, int selectionEnd)
{
  focusId_ = id;
  selectionStart_ = selectionStart;
  selectionEnd_ = selectionEnd;
}

#ifndef WT_TARGET_JAVA
void WApplication::deferRendering()
{
  session_->deferRendering();
}

void WApplication::resumeRendering()
{
  session_->resumeRendering();
}
#endif // WT_TARGET_JAVA

#ifndef WT_CNOR
void WtEmitBindSignal(const boost::shared_ptr< Wt::Signals::signal<void()> >& s)
{
  (*s)();
}
#endif // WTEMITBINDSIGNAL

}
