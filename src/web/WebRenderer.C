/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <stdexcept>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WWebWidget"
#include "Wt/WStringUtil"

#include "DomElement.h"
#include "WebController.h"
#include "Configuration.h"
#include "WebRenderer.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "FileServe.h"
#include "Utils.h"
#include "WtRandom.h"
#include "EscapeOStream.h"
#ifdef WIN32
#include <process.h> // for getpid()
#ifdef min
#undef min
#endif
#endif

namespace skeletons {
  extern const char *Boot_html;
  extern const char *Plain_html;
  extern const char *JsNoAjax_html;
  extern const char *Wt_js;
  extern const char *CommAjax_js;
  extern const char *CommScript_js;
}

namespace Wt {

const int MESSAGE_COUNTER_SIZE = 5;

WebRenderer::WebRenderer(WebSession& session)
  : session_(session),
    visibleOnly_(true),
    twoPhaseThreshold_(5000),
    learning_(false)
{ }

void WebRenderer::setTwoPhaseThreshold(int bytes)
{
  twoPhaseThreshold_ = bytes;
}

//#define DEBUG_RENDER

void WebRenderer::needUpdate(WWidget *w)
{
  if (session_.env().ajax()) {
#ifdef DEBUG_RENDER
    std::cerr << "needUpdate: " << w->formName() << " (" << typeid(*w).name()
		  << ")" << std::endl;
#endif //DEBUG_RENDER
    updateMap_.insert(w);
  }
}

void WebRenderer::doneUpdate(WWidget *w)
{
#ifdef DEBUG_RENDER
    std::cerr << "doneUpdate: " << w->formName() << " (" << typeid(*w).name()
		  << ")" << std::endl;
#endif //DEBUG_RENDER
  updateMap_.erase(w);
}

const std::vector<WObject *>& WebRenderer::formObjects() const
{
  return currentFormObjects_;
}

void WebRenderer::saveChanges()
{
  collectJS(&collectedChanges_);
}

void WebRenderer::discardChanges()
{
  collectJS(0);
}

void WebRenderer::letReloadJS(WebResponse& response, bool newSession,
			      bool embedded)
{
  if (!embedded)
    setHeaders(response, "text/plain; charset=UTF-8");

  response.out() << "window.location.reload(true);";
}

void WebRenderer::letReloadHTML(WebResponse& response, bool newSession)
{
  setHeaders(response, "text/html; charset=UTF-8");
  response.out() << "<html><script type=\"text/javascript\">";
  letReloadJS(response, newSession, true);
  response.out() << "</script><body></body></html>";
}

void WebRenderer::streamRedirectJS(std::ostream& out,
				   const std::string& redirect)
{
  if (session_.app())
    out << "if (window." << session_.app()->javaScriptClass() << ") "
	<< session_.app()->javaScriptClass()
	<< "._p_.setHash('" << session_.app()->internalPath() << ".');";
  out <<
    "if (window.location.replace)"
    " window.location.replace('" << redirect << "');"
    "else"
    " window.location.href='" << redirect << "';"; 
}

void WebRenderer::serveMainWidget(WebResponse& response,
				  ResponseType responseType)
{
  switch (responseType) {
  case UpdateResponse:
    serveJavaScriptUpdate(response);
    break;
  case FullResponse:
    switch (session_.type()) {
    case WebSession::Application:
      if (session_.env().ajax())
	serveMainscript(response);
      else
	serveMainpage(response);
      break;
    case WebSession::WidgetSet:
      serveWidgetSet(response);
    }
  }
}

void WebRenderer::serveBootstrap(WebResponse& response)
{
  bool xhtml = session_.env().contentType() == WEnvironment::XHTML1;
  Configuration& conf = session_.controller()->configuration();

  FileServe boot(skeletons::Boot_html);
  boot.setVar("XMLPREAMBLE", "");
  boot.setVar("DOCTYPE", session_.docType());

  std::stringstream noJsRedirectUrl;
  DomElement::htmlAttributeValue
    (noJsRedirectUrl,
     session_.bootstrapUrl(response, WebSession::KeepInternalPath) + "&js=no");

  if (xhtml) {
    boot.setVar("HTMLATTRIBUTES",
		"xmlns=\"http://www.w3.org/1999/xhtml\""
		/*" xmlns:svg=\"http://www.w3.org/2000/svg\""*/);
    boot.setVar("METACLOSE", "/>");
    boot.setVar("AUTO_REDIRECT", "");
    boot.setVar("NOSCRIPT_TEXT", conf.redirectMessage());
  } else {
    if (session_.env().agentIE())
      boot.setVar("HTMLATTRIBUTES",
		  "xmlns:v=\"urn:schemas-microsoft-com:vml\""
		  " lang=\"en\" dir=\"ltr\"");
    else
      boot.setVar("HTMLATTRIBUTES", "lang=\"en\" dir=\"ltr\"");
    boot.setVar("METACLOSE", ">");
    boot.setVar("AUTO_REDIRECT",
		"<noscript><meta http-equiv=\"refresh\" content=\"0;url="
		+ noJsRedirectUrl.str() + "\"></noscript>");
    boot.setVar("NOSCRIPT_TEXT", conf.redirectMessage());
  }
  boot.setVar("BLANK_HTML",
	      session_.bootstrapUrl(response, WebSession::ClearInternalPath)
	      + "&amp;resource=blank");
  boot.setVar("SELF_URL",
	      session_.bootstrapUrl(response, WebSession::KeepInternalPath));
  boot.setVar("REDIRECT_URL", noJsRedirectUrl.str());
  boot.setVar("SESSION_ID", session_.sessionId());
  boot.setVar("RANDOMSEED",
	      boost::lexical_cast<std::string>(WtRandom::getUnsigned()
					       + getpid()));
  boot.setVar("RELOAD_IS_NEWSESSION", conf.reloadIsNewSession());
  boot.setVar("USE_COOKIES",
	      conf.sessionTracking() == Configuration::CookiesURL);

  response.addHeader("Cache-Control", "no-cache, no-store");
  response.addHeader("Expires", "-1");

  std::string contentType = xhtml ? "application/xhtml+xml" : "text/html";
  contentType += "; charset=UTF-8";
  response.setContentType(contentType);

  boot.stream(response.out());
}

void WebRenderer::serveError(WebResponse& response, const std::exception& e,
			     ResponseType responseType)
{
  serveError(response, std::string(e.what()), responseType);
}

void WebRenderer::serveError(WebResponse& response, const std::string& message,
			     ResponseType responseType)
{
  if (responseType == FullResponse
      && session_.type() == WebSession::Application) {
    response.setContentType("text/html");
    response.out()
      << "<title>Error occurred.</title>"
      << "<h2>Error occurred.</h2>"
      << WWebWidget::escapeText(WString(message), true).toUTF8()
      << std::endl;    
  } else {
    collectedChanges_ << "alert(";
    DomElement::jsStringLiteral(collectedChanges_,
				"Error occurred:\n" + message, '\'');
    collectedChanges_ << ");";
  }
}

void WebRenderer::setCookie(const std::string name, const std::string value,
			    int maxAge, const std::string domain,
			    const std::string path)
{
  cookiesToSet_.push_back(Cookie(name, value, path, domain, maxAge));
}

void WebRenderer::setHeaders(WebResponse& response, const std::string mimeType)
{
  std::string cookies;

  for (unsigned i = 0; i < cookiesToSet_.size(); ++i) {
    std::string value = cookiesToSet_[i].value;

    cookies += cookiesToSet_[i].name + "=" + value + "; Version=1;";
    if (cookiesToSet_[i].maxAge != -1)
      cookies += " Max-Age="
	+ boost::lexical_cast<std::string>(cookiesToSet_[i].maxAge) + ";";
    if (!cookiesToSet_[i].domain.empty())
      cookies += " Domain=" + cookiesToSet_[i].domain + ";";
    if (!cookiesToSet_[i].path.empty())
      cookies += " Path=" + cookiesToSet_[i].path + ";";
  }
  cookiesToSet_.clear();

  if (!cookies.empty())
    response.addHeader("Set-Cookie", cookies);

  response.setContentType(mimeType);
}

void WebRenderer::serveJavaScriptUpdate(WebResponse& response)
{
  setHeaders(response, "text/plain; charset=UTF-8");
  streamJavaScriptUpdate(response.out(), response.id(), true);
}

void WebRenderer::streamJavaScriptUpdate(std::ostream& out, int id,
					 bool doTwoPhaze)
{
  WApplication *app = session_.app();

  std::string redirect = session_.getRedirect();

  if (!redirect.empty()) {
    streamRedirectJS(out, redirect);
    return;
  }

  out << collectedChanges_.str();
  collectedChanges_.str("");

  app->styleSheet().javaScriptUpdate(app, out, false);
  loadStyleSheets(out, app);
  loadScriptLibraries(out, app, true);

  out << app->newBeforeLoadJavaScript();

  if (app->domRoot2_)
    app->domRoot2_->rootAsJavaScript(app, out, false);

  collectJavaScriptUpdate(out);

  if (visibleOnly_) {
    bool needFetchInvisible = false;

    if (!updateMap_.empty()) {
      needFetchInvisible = true;

      if (twoPhaseThreshold_ > 0) {
	/*
	 * See how large the invisible changes are, perhaps we can
	 * send them along
	 */
	visibleOnly_ = false;

	collectJavaScriptUpdate(collectedChanges_);

#ifndef WT_TARGET_JAVA
	int collectChangesLength = collectedChanges_.rdbuf()->in_avail();
#else // WT_TARGET_JAVA
	int collectChangesLength = collectedChanges_.length();
#endif // WT_TARGET_JAVA
	if (collectChangesLength < (int)twoPhaseThreshold_) {
	  out << collectedChanges_.str();
	  collectedChanges_.str("");
	  needFetchInvisible = false;
	}

	visibleOnly_ = true;
      }
    }

    if (needFetchInvisible)
      out << app->javaScriptClass()
	  << "._p_.update(null, 'none', null, false);";
  }

  if (app->autoJavaScriptChanged_) {
    out << app->javaScriptClass() << "._p_.autoJavaScript=function(){"
	<< app->autoJavaScript_ << "};";
    app->autoJavaScriptChanged_ = false;
  }

  visibleOnly_ = true;

  app->domRoot_->doneRerender();
  if (app->domRoot2_)
    app->domRoot2_->doneRerender();

  if (app->ajaxMethod() == WApplication::DynamicScriptTag)
    out << app->javaScriptClass() << "._p_.updateDone(" << id << ");";

  loadScriptLibraries(out, app, false);
}

void WebRenderer::streamCommJs(WApplication *app, std::ostream& out)
{
  Configuration& conf = session_.controller()->configuration();

  FileServe js(app->ajaxMethod() == WApplication::XMLHttpRequest
	       ? skeletons::CommAjax_js
	       : skeletons::CommScript_js);

  js.setVar("APP_CLASS", app->javaScriptClass());
  js.setVar("CLOSE_CONNECTION",
	    conf.serverType() == Configuration::WtHttpdServer);

  js.stream(out);
}

void WebRenderer::serveMainscript(WebResponse& response)
{ 
  Configuration& conf = session_.controller()->configuration();

  setHeaders(response, "text/javascript; charset=UTF-8");

  std::string redirect = session_.getRedirect();

  if (!redirect.empty()) {
    streamRedirectJS(response.out(), redirect);
    return;
  }

  WApplication *app = session_.app();
  const bool xhtml = app->environment().contentType() == WEnvironment::XHTML1;

  currentFormObjectsList_ = createFormObjectsList(app);

  FileServe script(skeletons::Wt_js);
  script.setVar("WT_CLASS", WT_CLASS);
  script.setVar("APP_CLASS", app->javaScriptClass());
  script.setVar("AUTO_JAVASCRIPT", app->autoJavaScript_);
  script.setVar("STRICTLY_SERIALIZED_EVENTS", conf.serializedEvents());

  /*
   * In fact only Opera (and Safari) cannot use innerHTML in XHTML
   * documents.  We could check for opera only since the workaround
   * innerHTML is substantially slower...
   */
  bool innerHtml = !xhtml || app->environment().agentGecko();

  script.setVar("INNER_HTML", innerHtml);
  script.setVar("FORM_OBJECTS", '[' + currentFormObjectsList_ + ']');
  script.setVar("RELATIVE_URL", '"'
		+ session_.bootstrapUrl(response, WebSession::ClearInternalPath)
		+ '"');
  script.setVar("KEEP_ALIVE",
		boost::lexical_cast<std::string>
		(conf.sessionTimeout() / 2));
  script.setVar("INITIAL_HASH", app->internalPath());
  script.setVar("INDICATOR_TIMEOUT", "500");
  script.setVar("ONLOAD", "loadWidgetTree();");
  script.setVar("WT_HISTORY_PREFIX", "Wt-history");
  script.stream(response.out());

  app->autoJavaScriptChanged_ = false;

  streamCommJs(app, response.out());

  app->styleSheet().javaScriptUpdate(app, response.out(), true);
  app->styleSheetsAdded_ = app->styleSheets_.size();
  loadStyleSheets(response.out(), app);

  app->scriptLibrariesAdded_ = app->scriptLibraries_.size();
  loadScriptLibraries(response.out(), app, true);
 
  response.out() << std::endl << app->beforeLoadJavaScript();

  WWebWidget *mainWebWidget = app->domRoot_;

  visibleOnly_ = true;

  /*
   * The element to render. This automatically creates loading stubs for
   * invisible widgets, which is excellent for both JavaScript and
   * non-JavaScript versions.
   */
  DomElement *mainElement = mainWebWidget->createSDomElement(app);

  collectedChanges_.str("");
  preLearnStateless(app);

  response.out() << "window.loadWidgetTree = function(){";

  std::string cvar;  
  {
    EscapeOStream sout(response.out()); 
    cvar = mainElement->asJavaScript(sout, DomElement::Create);
  }
  response.out() << "document.body.appendChild(" << cvar << ");";
  {
    EscapeOStream sout(response.out()); 
    mainElement->asJavaScript(sout, DomElement::Update);
  }

  delete mainElement;

  updateLoadIndicator(response.out(), app, true);

  response.out() << collectedChanges_.str() << app->afterLoadJavaScript()
		 << "};";
  collectedChanges_.str("");

  response.out() << "scriptLoaded = true; if (isLoaded) onLoad();";

  loadScriptLibraries(response.out(), app, false);
}

void WebRenderer::updateLoadIndicator(std::ostream& out, WApplication *app,
				      bool all)
{
  if (app->showLoadingIndicator_->needUpdate() || all) {
    out << "showLoadingIndicator = function() {"
	<< app->showLoadingIndicator_->javaScript() << "};";
    app->showLoadingIndicator_->updateOk();
  }

  if (app->hideLoadingIndicator_->needUpdate() || all) {
    out << "hideLoadingIndicator = function() {"
	<< app->hideLoadingIndicator_->javaScript() << "};";
    app->hideLoadingIndicator_->updateOk();
  }
}

void WebRenderer::serveMainpage(WebResponse& response)
{
  Configuration& conf = session_.controller()->configuration();

  WApplication *app = session_.app();

  std::string redirect = session_.getRedirect();

  if (!redirect.empty()) {
    response.setRedirect(redirect);
    return;
  }

  WWebWidget *mainWebWidget = app->domRoot_;

  visibleOnly_ = true;

  /*
   * The element to render. This automatically creates loading stubs for
   * invisible widgets, which is excellent for both JavaScript and
   * non-JavaScript versions.
   */
  DomElement *mainElement = mainWebWidget->createSDomElement(app);

  collectedChanges_.str("");

  const bool xhtml = app->environment().contentType() == WEnvironment::XHTML1;

  std::string styleSheets;
  for (unsigned i = 0; i < app->styleSheets_.size(); ++i) {
    styleSheets += "<link href='"
      + app->fixRelativeUrl(app->styleSheets_[i]) 
      + "' rel='stylesheet' type='text/css'" + (xhtml ? "/>" : ">");
  }
  app->styleSheetsAdded_ = 0;
  app->scriptLibrariesAdded_ = 0;

  FileServe page(skeletons::Plain_html);

  page.setVar("XMLPREAMBLE", "");
  page.setVar("DOCTYPE", session_.docType());
  page.setVar("SESSIONID", session_.sessionId());

  if (app->environment().agentIsSpiderBot())
    page.setVar("WTD_SESSIONID_FIELD", "");
  else
    page.setVar("WTD_SESSIONID_FIELD",
		"<input name='wtd' id='wtd' type='hidden' value='"
		+ session_.sessionId() + "' />");

  if (xhtml) {
    page.setVar("HTMLATTRIBUTES",
		"xmlns=\"http://www.w3.org/1999/xhtml\""
		/*" xmlns:svg=\"http://www.w3.org/2000/svg\""*/);
    page.setVar("METACLOSE", "/>");
  } else {
    if (session_.env().agentIE())
      page.setVar("HTMLATTRIBUTES",
		  "xmlns:v=\"urn:schemas-microsoft-com:vml\""
		  " lang=\"en\" dir=\"ltr\"");
    else
      page.setVar("HTMLATTRIBUTES", "lang=\"en\" dir=\"ltr\"");
    page.setVar("METACLOSE", ">");
  }

  std::string url
    = (conf.sessionTracking() == Configuration::CookiesURL
       && session_.env().supportsCookies())
    ? session_.bookmarkUrl(app->newInternalPath_)
    : session_.mostRelativeUrl(app->newInternalPath_);

  url = app->fixRelativeUrl(url);

  url = Wt::Utils::replace(url, '&', "&amp;");
  page.setVar("RELATIVE_URL", url);
  page.setVar("STYLESHEET", app->styleSheet().cssText(true));
  page.setVar("STYLESHEETS", styleSheets);

  page.setVar("TITLE", WWebWidget::escapeText(app->title()).toUTF8());
  app->titleChanged_ = false;

  std::string contentType = xhtml ? "application/xhtml+xml" : "text/html";

  contentType += "; charset=UTF-8";

  setHeaders(response, contentType);

  // Form objects, need in either case (Ajax or not)
  currentFormObjectsList_ = createFormObjectsList(app);

  page.streamUntil(response.out(), "HTML");

  DomElement::TimeoutList timeouts;
  {
    EscapeOStream out(response.out());
    mainElement->asHTML(out, timeouts);
    delete mainElement;
  }

  std::stringstream onload;
  DomElement::createTimeoutJs(onload, timeouts, app);

  int refresh = conf.sessionTimeout() / 3;
  for (unsigned i = 0; i < timeouts.size(); ++i)
    refresh = std::min(refresh, 1 + timeouts[i].msec/1000);
  if (app->isQuited())
    refresh = 100000; // ridiculously large
  page.setVar("REFRESH", boost::lexical_cast<std::string>(refresh));

  page.stream(response.out());

  app->internalPathIsChanged_ = false;
}

void WebRenderer::serveWidgetSet(WebResponse& response)
{ 
  Configuration& conf = session_.controller()->configuration();

  WApplication *app = session_.app();

  setHeaders(response, "text/javascript; charset=UTF-8");

  const bool xhtml = app->environment().contentType() == WEnvironment::XHTML1;
  const bool innerHtml = !xhtml || app->environment().agentGecko();

  currentFormObjectsList_ = createFormObjectsList(app);

  FileServe script(skeletons::Wt_js);
  script.setVar("WT_CLASS", WT_CLASS);
  script.setVar("APP_CLASS", app->javaScriptClass());
  script.setVar("AUTO_JAVASCRIPT", app->autoJavaScript_);
  script.setVar("STRICTLY_SERIALIZED_EVENTS", conf.serializedEvents());
  script.setVar("INNER_HTML", innerHtml);
  script.setVar("FORM_OBJECTS", '[' + currentFormObjectsList_ + ']');
  script.setVar("RELATIVE_URL", '"'
		+ session_.bootstrapUrl(response, WebSession::KeepInternalPath)
		+ '"');
  script.setVar("KEEP_ALIVE",
		boost::lexical_cast<std::string>(conf.sessionTimeout() / 2));
  script.setVar("INITIAL_HASH", app->internalPath());
  script.setVar("INDICATOR_TIMEOUT", "500");
  script.setVar("ONLOAD", "");
  script.stream(response.out());

  app->autoJavaScriptChanged_ = false;

  streamCommJs(app, response.out());

  app->styleSheet().javaScriptUpdate(app, response.out(), true);
  app->styleSheetsAdded_ = app->styleSheets_.size();
  loadStyleSheets(response.out(), app);

  app->scriptLibrariesAdded_ = app->scriptLibraries_.size();
  loadScriptLibraries(response.out(), app, true);
 
  response.out() << std::endl << app->beforeLoadJavaScript();

  WWebWidget *mainWebWidget = app->domRoot_;

  visibleOnly_ = true;

  /*
   * Render Root widgets (domRoot_ and children of domRoot2_) as
   * JavaScript
   */
  DomElement *mainElement = mainWebWidget->createSDomElement(app);
  std::string cvar;
  {
    EscapeOStream sout(response.out()); 
    mainElement->asJavaScript(sout, DomElement::Create);
    cvar = mainElement->asJavaScript(sout, DomElement::Update);
  }
  delete mainElement;

  response.out() << "document.body.insertBefore("
		<< cvar << ",document.body.firstChild);" << std::endl;

  app->domRoot2_->rootAsJavaScript(app, response.out(), true);

  collectedChanges_.str("");
  preLearnStateless(app);

  updateLoadIndicator(response.out(), app, true);

  response.out() << collectedChanges_.str();
  collectedChanges_.str("");

  const std::string *historyE = app->environment().getParameter("Wt-history");
  if (historyE) {
    response.out() << WT_CLASS << ".history.initialize('"
		   << (*historyE)[0] << "-field', '"
		   << (*historyE)[0] << "-iframe');";
  }

  response.out() << app->afterLoadJavaScript()
		<< app->javaScriptClass() << "._p_.load();";

  if (!app->title().empty()) {
    response.out() << app->javaScriptClass()
		  << "._p_.setTitle(";
    DomElement::jsStringLiteral(response.out(), app->title().toUTF8(), '\'');
    response.out() << ");";
  }
  app->titleChanged_ = false;

  loadScriptLibraries(response.out(), app, false);
}

void WebRenderer::loadScriptLibraries(std::ostream& out,
				      WApplication *app, bool start)
{
  int first = app->scriptLibraries_.size() - app->scriptLibrariesAdded_;

  if (start) {
    for (unsigned i = first; i < app->scriptLibraries_.size(); ++i) {
      std::string uri = app->fixRelativeUrl(app->scriptLibraries_[i].uri);

      out << app->scriptLibraries_[i].beforeLoadJS
	  << app->javaScriptClass() << "._p_.loadScript('" << uri << "',";
      DomElement::jsStringLiteral(out, app->scriptLibraries_[i].symbol, '\'');
      out << ");"
	  << app->javaScriptClass() << "._p_.onJsLoad(\""
	  << uri << "\",function() {";
    }
  } else {
    for (unsigned i = first; i < app->scriptLibraries_.size(); ++i) {
      out << "});";
    }
    app->scriptLibrariesAdded_ = 0;
  }
}

void WebRenderer::loadStyleSheets(std::ostream& out, WApplication *app)
{
  int first = app->styleSheets_.size() - app->styleSheetsAdded_;

  for (unsigned i = first; i < app->styleSheets_.size(); ++i) {
    out << WT_CLASS << ".addStyleSheet('"
	<< app->fixRelativeUrl(app->styleSheets_[i]) << "');";
  }

  app->styleSheetsAdded_ = 0;
}

void WebRenderer::collectChanges(std::vector<DomElement *>& changes)
{
  WApplication *app = session_.app();

  std::multimap<int, WWidget *> depthOrder;

  for (UpdateMap::const_iterator i = updateMap_.begin();
       i != updateMap_.end(); ++i) {
    int depth = 1;

    WWidget *ww = *i;
    WWidget *w = ww;
    for (; w->parent(); ++depth) 
      w = w->parent();

    if (w != app->domRoot_ && w != app->domRoot2_) {
#ifdef DEBUG_RENDER
      std::cerr << "ignoring: " << (*i)->formName()
                << " (" << typeid(**i).name()
                << ") " << w->formName()
                << " (" << typeid(*w).name()
                << ")" << std::endl;
#endif // DEBUG_RENDER
      // not in displayed widgets: will be removed from the update list
      depth = 0;
    }

#ifndef WT_TARGET_JAVA
    depthOrder.insert(std::make_pair(depth, ww));
#else
    depthOrder.insert(depth, ww);
#endif // WT_TARGET_JAVA
  }

  for (std::multimap<int, WWidget *>::const_iterator i = depthOrder.begin();
       i != depthOrder.end(); ++i) {
    UpdateMap::iterator j = updateMap_.find(i->second);
    if (j != updateMap_.end()) {
      WWidget *w = i->second;

      // depth == 0: remove it from the update list
      if (i->first == 0) {
	w->webWidget()->propagateRenderOk();
	continue;
      }

      //std::cerr << learning_ << " " << loading_ 
      //          << " updating: " << w->formName() << std::endl;

#ifdef DEBUG_RENDER
      std::cerr << "updating: " << w->formName()
		<< " (" << typeid(*w).name() << ")" << std::endl;
#endif

      if (!learning_ && visibleOnly_) {
	if (!w->isStubbed()) {
	  w->getSDomChanges(changes, app);

	  /* if (!w->isVisible()) {
	    // We should postpone rendering the changes -- but
	    // at the same time need to propageRenderOk() now for stateless
	    // slot learning to work properly.
	    w->getSDomChanges(changes, app);
	  } else
	  w->getSDomChanges(changes, app); */
	} else
#ifdef DEBUG_RENDER
	  std::cerr << "Ignoring: " << w->formName() << std::endl;
#else
	  ;
#endif // DEBUG_RENDER
      } else {
	w->getSDomChanges(changes, app);
      }
    }
  }
}

void WebRenderer::collectJavaScriptUpdate(std::ostream& out)
{
  WApplication *app = session_.app();

  if (&out != &collectedChanges_) {
    out << collectedChanges_.str();
    collectedChanges_.str("");
  }

  out << '{';

  collectJS(&out);

  /*
   * Now, as we have cleared and recorded all JavaScript changes that were
   * caused by the actual code, we can learn stateless code and collect
   * changes that result.
   */

  preLearnStateless(app);

  if (&out != &collectedChanges_) {
    out << collectedChanges_.str();
    collectedChanges_.str("");
  }

  collectJS(&out);

  if (formObjectsChanged_) {
    std::string formObjectsList = createFormObjectsList(app);
    if (formObjectsList != currentFormObjectsList_) {
      currentFormObjectsList_ = formObjectsList;
      out << app->javaScriptClass()
	  << "._p_.setFormObjects([" << currentFormObjectsList_ << "]);";
    }
  }

  out << app->afterLoadJavaScript();

  if (app->isQuited()) {
    out << app->javaScriptClass() << "._p_.quit();";

    WContainerWidget *timers = app->timerRoot();
    DomElement *d = DomElement::getForUpdate(timers, DomElement_DIV);
    d->setProperty(PropertyInnerHTML, "");
    EscapeOStream sout(out);
    d->asJavaScript(sout, DomElement::Update);

    delete d;
  }

  updateLoadIndicator(out, app, false);

  out << '}';
}

void WebRenderer::updateFormObjects(WWebWidget *source, bool checkDescendants)
{
  formObjectsChanged_ = true;
}

std::string WebRenderer::createFormObjectsList(WApplication *app)
{
  currentFormObjects_.clear();

  app->domRoot_->getFormObjects(currentFormObjects_);
  if (app->domRoot2_)
    app->domRoot2_->getFormObjects(currentFormObjects_);

  std::string result;
  for (unsigned i = 0; i < currentFormObjects_.size(); ++i) {
    if (i != 0)
      result += ',';
    result += "'" + currentFormObjects_[i]->formName() + "'";
  }

  formObjectsChanged_ = false;

  return result;
}

void WebRenderer::collectJS(std::ostream* js)
{
  std::vector<DomElement *> changes;

  collectChanges(changes);

  if (js) {
    EscapeOStream sout(*js);

    for (unsigned i = 0; i < changes.size(); ++i)
      changes[i]->asJavaScript(sout, DomElement::Delete);

    for (unsigned i = 0; i < changes.size(); ++i) {
      changes[i]->asJavaScript(sout, DomElement::Update);
      delete changes[i];
    }
  } else {
    for (unsigned i = 0; i < changes.size(); ++i)
      delete changes[i];
  }

  if (session_.type() == WebSession::WidgetSet) {
    return;
  }

  WApplication *app = session_.app();

  if (js) { 
    if (app->titleChanged_) {
      *js << app->javaScriptClass()
	  << "._p_.setTitle(";
      DomElement::jsStringLiteral(*js, app->title().toUTF8(), '\'');
      *js << ");";
    }

    if (app->internalPathIsChanged_)
      *js << app->javaScriptClass()
	  << "._p_.setHash('" << app->newInternalPath_ << "');";

    *js << app->afterLoadJavaScript();
  } else
    app->afterLoadJavaScript();

  app->internalPathIsChanged_ = false;
  app->titleChanged_ = false;
}

void WebRenderer::preLearnStateless(WApplication *app)
{
  bool isIEMobile = app->environment().agentIEMobile();

  if (isIEMobile || !session_.env().ajax())
    return;

  const WApplication::SignalMap& ss = session_.app()->exposedSignals();

  for (WApplication::SignalMap::const_iterator i = ss.begin();
       i != ss.end(); ++i) {
    if (i->second->sender() == app)
      i->second->processPreLearnStateless(this);

    WWidget *ww = dynamic_cast<WWidget *>(i->second->sender());

    if (ww && !ww->isStubbed()) {
      WWidget *a = ww->adam();
      if (a == app->domRoot_ || a == app->domRoot2_)
	i->second->processPreLearnStateless(this);
    }
  }
}

std::string WebRenderer::learn(WStatelessSlot* slot)
{
  collectJS(&collectedChanges_);

  if (slot->type() == WStatelessSlot::PreLearnStateless)
    learning_ = true;

  slot->trigger();

  std::stringstream js;
  collectJS(&js);

  std::string result = js.str();

  if (slot->type() == WStatelessSlot::PreLearnStateless) {
    slot->undoTrigger();
    collectJS(0);

    learning_ = false;
  } else { // AutoLearnStateless
    collectedChanges_ << result;
  }

  slot->setJavaScript(result);

  return result;
}

}
