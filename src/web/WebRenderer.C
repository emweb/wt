/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <map>
#include <stdexcept>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WLoadingIndicator"
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

//#define DEBUG_JS
//#define DEBUG_RENDER

namespace skeletons {
  extern const char *Boot_html;
  extern const char *Plain_html;
  extern const char *Hybrid_html;
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
    expectedAckId_(0),
    learning_(false)
{ }

void WebRenderer::setTwoPhaseThreshold(int bytes)
{
  twoPhaseThreshold_ = bytes;
}

void WebRenderer::needUpdate(WWidget *w, bool laterOnly)
{
#ifdef DEBUG_RENDER
  std::cerr << "needUpdate: " << w->id() << " (" << typeid(*w).name()
	    << ")" << std::endl;
#endif //DEBUG_RENDER
  updateMap_.insert(w);

  if (!laterOnly)
    moreUpdates_ = true;
}

void WebRenderer::doneUpdate(WWidget *w)
{
#ifdef DEBUG_RENDER
  std::cerr << "doneUpdate: " << w->id() << " (" << typeid(*w).name()
	    << ")" << std::endl;
#endif //DEBUG_RENDER
  updateMap_.erase(w);
}

bool WebRenderer::isDirty() const
{
  return !updateMap_.empty()
    || Utils::length(collectedJS1_) > 0
    || Utils::length(collectedJS2_) > 0
    || Utils::length(invisibleJS_) > 0;
}

const WebRenderer::FormObjectsMap& WebRenderer::formObjects() const
{
  return currentFormObjects_;
}

void WebRenderer::saveChanges()
{
  collectJS(&collectedJS1_);
}

void WebRenderer::discardChanges()
{
  collectJS(0);
}

void WebRenderer::ackUpdate(int updateId)
{
  if (updateId == expectedAckId_)
    setJSSynced(false);

  ++expectedAckId_;
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
	<< "._p_.setHash('" << session_.app()->internalPath() << ".');\n";
  out <<
    "if (window.location.replace)"
    " window.location.replace('" << redirect << "');"
    "else"
    " window.location.href='" << redirect << "';\n";
}

void WebRenderer::serveResponse(WebResponse& response,
				ResponseType responseType)
{
  switch (responseType) {
  case Update:
    serveJavaScriptUpdate(response);
    break;
  case Page:
    if (session_.app())
      serveMainpage(response);
    else
      serveBootstrap(response);
    break;
  case Script:
    serveMainscript(response);
    break;
  }
}

void WebRenderer::setPageVars(FileServe& page)
{
  bool xhtml = session_.env().contentType() == WEnvironment::XHTML1;
  WApplication *app = session_.app();

  page.setVar("DOCTYPE", session_.docType());

  std::string htmlAttr;
  if (app && !app->htmlClass_.empty())
    htmlAttr = " class=\"" + app->htmlClass_ + "\"";

  if (xhtml) {
    page.setVar("HTMLATTRIBUTES",
		"xmlns=\"http://www.w3.org/1999/xhtml\"" + htmlAttr);
    page.setVar("METACLOSE", "/>");
  } else {
    if (session_.env().agentIsIE())
      page.setVar("HTMLATTRIBUTES",
		  "xmlns:v=\"urn:schemas-microsoft-com:vml\""
		  " lang=\"en\" dir=\"ltr\"" + htmlAttr);
    else
      page.setVar("HTMLATTRIBUTES", "lang=\"en\" dir=\"ltr\"" + htmlAttr);
    page.setVar("METACLOSE", ">");
  }

  page.setVar("BODYATTRIBUTES", (!app || app->bodyClass_.empty())
	      ? "" : " class=\"" + app->bodyClass_ + "\"");

  page.setVar("HEADDECLARATIONS", headDeclarations());

  page.setCondition("FORM", !session_.env().agentIsSpiderBot()
		    && !session_.env().ajax());
}

void WebRenderer::setBootVars(WebResponse& response,
			      FileServe& boot)
{
  Configuration& conf = session_.controller()->configuration();

  boot.setVar("BLANK_HTML",
	      session_.bootstrapUrl(response, WebSession::ClearInternalPath)
	      + "&amp;request=resource&amp;resource=blank");
  boot.setVar("SELF_URL",
	      session_.bootstrapUrl(response, WebSession::KeepInternalPath));
  boot.setVar("SESSION_ID", session_.sessionId());
  boot.setVar("RANDOMSEED",
	      boost::lexical_cast<std::string>(WtRandom::getUnsigned()
					       + getpid()));
  boot.setVar("RELOAD_IS_NEWSESSION", conf.reloadIsNewSession());
  boot.setVar("USE_COOKIES",
	      conf.sessionTracking() == Configuration::CookiesURL);

  boot.setVar("AJAX_CANONICAL_URL", session_.ajaxCanonicalUrl(response));
}

void WebRenderer::serveBootstrap(WebResponse& response)
{
  bool xhtml = session_.env().contentType() == WEnvironment::XHTML1;
  Configuration& conf = session_.controller()->configuration();

  FileServe boot(skeletons::Boot_html);
  setPageVars(boot);
  setBootVars(response, boot);

  std::stringstream noJsRedirectUrl;
  DomElement::htmlAttributeValue
    (noJsRedirectUrl,
     session_.bootstrapUrl(response, WebSession::KeepInternalPath) + "&js=no");

  if (xhtml) {
    boot.setVar("AUTO_REDIRECT", "");
    boot.setVar("NOSCRIPT_TEXT", conf.redirectMessage());
  } else {
    boot.setVar("AUTO_REDIRECT",
		"<noscript><meta http-equiv=\"refresh\" content=\"0;url="
		+ noJsRedirectUrl.str() + "\"></noscript>");
    boot.setVar("NOSCRIPT_TEXT", conf.redirectMessage());
  }
  boot.setVar("REDIRECT_URL", noJsRedirectUrl.str());

  response.addHeader("Cache-Control", "no-cache, no-store");
  response.addHeader("Expires", "-1");

  std::string contentType = xhtml ? "application/xhtml+xml" : "text/html";
  contentType += "; charset=UTF-8";

  setHeaders(response, contentType);

  boot.stream(response.out());
}

void WebRenderer::serveError(WebResponse& response, const std::string& message,
			     ResponseType responseType)
{
  bool js = responseType == Update || responseType == Script;

  if (!js) {
    response.setContentType("text/html");
    response.out()
      << "<title>Error occurred.</title>"
      << "<h2>Error occurred.</h2>"
      << WWebWidget::escapeText(WString(message), true).toUTF8()
      << std::endl;    
  } else {
    collectedJS1_ <<
      "document.title = 'Error occurred.';"
      "document.body.innerHtml='<h2>Error occurred.</h2>' +";
    DomElement::jsStringLiteral(collectedJS1_, message, '\'');
    collectedJS1_ << ";";
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

  for (unsigned i = 0; i < cookiesToSet_.size(); ++i) {
    std::string cookies;
    std::string value = cookiesToSet_[i].value;

    cookies += Utils::urlEncode(cookiesToSet_[i].name)
      + "=" + Utils::urlEncode(value) + "; Version=1;";
    if (cookiesToSet_[i].maxAge != -1)
      cookies += " Max-Age="
	+ boost::lexical_cast<std::string>(cookiesToSet_[i].maxAge) + ";";
    if (!cookiesToSet_[i].domain.empty())
      cookies += " Domain=" + cookiesToSet_[i].domain + ";";
    if (!cookiesToSet_[i].path.empty())
      cookies += " Path=" + cookiesToSet_[i].path + ";";

    if (!cookies.empty())
      response.addHeader("Set-Cookie", cookies);
  }
  cookiesToSet_.clear();

  response.setContentType(mimeType);
}

void WebRenderer::serveJavaScriptUpdate(WebResponse& response)
{
  setHeaders(response, "text/plain; charset=UTF-8");

  collectJavaScript();

#ifdef DEBUG_JS
  std::cerr << collectedJS1_.str() << collectedJS2_.str() << std::endl;
#endif // DEBUG_JS

  response.out()
    << collectedJS1_.str()
    << session_.app()->javaScriptClass() << "._p_.response(" << expectedAckId_ << ");"
    << collectedJS2_.str();
}

void WebRenderer::collectJavaScript()
{
  WApplication *app = session_.app();
  Configuration& conf = session_.controller()->configuration();

  /*
   * Pending invisible changes are also collected into JS1.
   * This is also done in ackUpdate(), but just in case an update was not
   * acknowledged:
   */
  collectedJS1_ << invisibleJS_.str();
  invisibleJS_.str("");

  if (conf.inlineCss())
    app->styleSheet().javaScriptUpdate(app, collectedJS1_, false);

  loadStyleSheets(collectedJS1_, app);

  if (app->bodyHtmlClassChanged_) {
    collectedJS1_ << "document.body.parentNode.className='"
		  << app->htmlClass_ << "';"
		  << "document.body.className='" << app->bodyClass_ << "';";
    app->bodyHtmlClassChanged_ = false;
  }

  /*
   * This opens scopes, waiting for new libraries to be loaded.
   */
  loadScriptLibraries(collectedJS1_, app, true);

  /*
   * This closes the same scopes.
   */
  loadScriptLibraries(collectedJS2_, app, false);

  /*
   * Everything else happens inside JS1: after libraries have been loaded.
   */
  collectedJS1_ << app->newBeforeLoadJavaScript();

  if (app->domRoot2_)
    app->domRoot2_->rootAsJavaScript(app, collectedJS1_, false);

  collectJavaScriptUpdate(collectedJS1_);

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

	collectJavaScriptUpdate(invisibleJS_);

	if (Utils::length(invisibleJS_) < (int)twoPhaseThreshold_) {
	  collectedJS1_ << invisibleJS_.str();
	  invisibleJS_.str("");
	  needFetchInvisible = false;
	}

	visibleOnly_ = true;
      }
    }

    if (needFetchInvisible)
      collectedJS1_ << app->javaScriptClass()
		    << "._p_.update(null, 'none', null, false);";
  }

  if (app->autoJavaScriptChanged_) {
    collectedJS1_ << app->javaScriptClass()
		  << "._p_.autoJavaScript=function(){"
		  << app->autoJavaScript_ << "};";
    app->autoJavaScriptChanged_ = false;
  }

  visibleOnly_ = true;

  app->domRoot_->doneRerender();
  if (app->domRoot2_)
    app->domRoot2_->doneRerender();

  std::string redirect = session_.getRedirect();
  if (!redirect.empty())
    streamRedirectJS(collectedJS1_, redirect);
}

void WebRenderer::streamCommJs(WApplication *app, std::ostream& out)
{
  Configuration& conf = session_.controller()->configuration();

  FileServe js(app->ajaxMethod() == WApplication::XMLHttpRequest
	       ? skeletons::CommAjax_js
	       : skeletons::CommScript_js);

  js.setVar("APP_CLASS", app->javaScriptClass());

  /*
   * Mozilla Bugzilla #246651
   */
  js.setVar("CLOSE_CONNECTION",
	    (conf.serverType() == Configuration::WtHttpdServer)
	    && session_.env().agentIsGecko()
	    && session_.env().agent() < WEnvironment::Firefox3);
  
  js.stream(out);
}

void WebRenderer::serveMainscript(WebResponse& response)
{
  Configuration& conf = session_.controller()->configuration();
  bool widgetset = session_.type() == WidgetSet;

  setHeaders(response, "text/javascript; charset=UTF-8");

  if (!widgetset) {
    std::string redirect = session_.getRedirect();

    if (!redirect.empty()) {
      streamRedirectJS(response.out(), redirect);
      return;
    }
  }

  WApplication *app = session_.app();

  /*
   * Opera and Safari cannot use innerHTML in XHTML documents.
   */
  const bool xhtml = app->environment().contentType() == WEnvironment::XHTML1;
  const bool innerHtml = !xhtml || app->environment().agentIsGecko();

  formObjectsChanged_ = true;
  currentFormObjectsList_ = createFormObjectsList(app);

  FileServe script(skeletons::Wt_js);
  script.setCondition("DEBUG", conf.debug());
  script.setVar("WT_CLASS", WT_CLASS);
  script.setVar("APP_CLASS", app->javaScriptClass());
  script.setVar("AUTO_JAVASCRIPT", app->autoJavaScript_);

  script.setCondition("STRICTLY_SERIALIZED_EVENTS", conf.serializedEvents());
  script.setVar("INNER_HTML", innerHtml);
  script.setVar("FORM_OBJECTS", '[' + currentFormObjectsList_ + ']');

  script.setVar("RELATIVE_URL", '"'
		+ session_.bootstrapUrl(response, WebSession::ClearInternalPath)
		+ '"');
  script.setVar("KEEP_ALIVE",
		boost::lexical_cast<std::string>(conf.sessionTimeout() / 2));
  script.setVar("INITIAL_HASH", app->internalPath());
  script.setVar("INDICATOR_TIMEOUT", "500");
  script.setVar("SERVER_PUSH_TIMEOUT", conf.serverPushTimeout() * 1000);
  script.setVar("ONLOAD", widgetset ? "" : "window.loadWidgetTree();");
  script.stream(response.out());

  app->autoJavaScriptChanged_ = false;

  streamCommJs(app, response.out());

  if (session_.state() == WebSession::JustCreated)
    serveMainAjax(response);
  else {
    response.out() << "window.loadWidgetTree = function(){\n";

    if (app->enableAjax_) {
      response.out()
	<< beforeLoadJS_.str()
	<< "var domRoot = " << app->domRoot_->jsRef() << ";"
	"var form = " WT_CLASS ".getElement('Wt-form');"
	"domRoot.style.display = form.style.display;"
	"document.body.replaceChild(domRoot, form);"
	<< app->afterLoadJavaScript();

      beforeLoadJS_.str("");
    }

    visibleOnly_ = false;

    collectJavaScript();

#ifdef DEBUG_JS
    std::cerr << collectedJS1_.str() << collectedJS2_.str() << std::endl;
#endif // DEBUG_JS

    response.out()
      << collectedJS1_.str()
      << app->javaScriptClass()
      << "._p_.response(" << expectedAckId_ << ");";

    updateLoadIndicator(response.out(), app, true);

    if (app->enableAjax_)
      response.out()
	<< "domRoot.style.display = 'block';"
	<< app->javaScriptClass() << "._p_.autoJavaScript();";

    response.out()
	<< app->javaScriptClass()
	<< "._p_.update(null, 'load', null, false);"
	<< collectedJS2_.str() <<
	"};"
	"window.WtScriptLoaded = true;"
	"if (window.isLoaded) onLoad();\n";

    app->enableAjax_ = false;
  }
}

void WebRenderer::serveMainAjax(WebResponse& response)
{
  Configuration& conf = session_.controller()->configuration();
  bool widgetset = session_.type() == WidgetSet;
  WApplication *app = session_.app();

  WWebWidget *mainWebWidget = app->domRoot_;

  visibleOnly_ = true;

  /*
   * Render root widgets (domRoot_, and for widget set, also children of
   * domRoot2_). This automatically creates loading stubs for
   * invisible widgets.
   */
  app->loadingIndicatorWidget_->show();
  DomElement *mainElement = mainWebWidget->createSDomElement(app);
  app->loadingIndicatorWidget_->hide();

  /*
   * Need to do this after createSDomElement, since additional CSS/JS
   * may be made during rendering, e.g. from WViewWidget::render()
   */
  if (conf.inlineCss())
    app->styleSheet().javaScriptUpdate(app, response.out(), true);

  if (!app->cssTheme().empty()) {
    response.out() << WT_CLASS << ".addStyleSheet('"
		   << WApplication::resourcesUrl() << "/themes/"
		   << app->cssTheme() << "/wt.css');";
    if (app->environment().agentIsIE())
      response.out() << WT_CLASS << ".addStyleSheet('"
		     << WApplication::resourcesUrl() << "/themes/"
		     << app->cssTheme() << "/wt_ie.css');";
  }

  app->styleSheetsAdded_ = app->styleSheets_.size();
  loadStyleSheets(response.out(), app);

  app->scriptLibrariesAdded_ = app->scriptLibraries_.size();
  loadScriptLibraries(response.out(), app, true);

  response.out() << std::endl << app->beforeLoadJavaScript();

  if (!widgetset)
    response.out() << "window.loadWidgetTree = function(){\n";

  if (app->bodyHtmlClassChanged_) {
    response.out() << "document.body.parentNode.className='"
		   << app->htmlClass_ << "';"
		   << "document.body.className='" << app->bodyClass_ << "';";
    app->bodyHtmlClassChanged_ = false;
  }

#ifdef DEBUG_JS
  std::stringstream s;
#else
  std::ostream& s = response.out();
#endif // DEBUG_JS

  mainElement->addToParent(s, "document.body", widgetset ? 0 : -1, app);
  delete mainElement;

  if (widgetset)
    app->domRoot2_->rootAsJavaScript(app, s, true);

#ifdef DEBUG_JS
  std::cerr << s.str();
  response.out() << s.str();
#endif // DEBUG_JS

  setJSSynced(true);

  preLearnStateless(app, collectedJS1_);

#ifdef DEBUG_JS
  std::cerr << collectedJS1_.str();
#endif // DEBUG_JS

  response.out() << collectedJS1_.str();
  collectedJS1_.str("");  

  updateLoadIndicator(response.out(), app, true);

  if (widgetset) {
    const std::string *historyE = app->environment().getParameter("Wt-history");
    if (historyE) {
      response.out() << WT_CLASS << ".history.initialize('"
		     << (*historyE)[0] << "-field', '"
		     << (*historyE)[0] << "-iframe');\n";
    }
  }

  response.out() << app->afterLoadJavaScript()
		 << "{var o=null,e=null;"
		 << app->hideLoadingIndicator_->javaScript()
		 << "}";

  if (widgetset)
    response.out() << app->javaScriptClass() << "._p_.load();\n";

  response.out() << session_.app()->javaScriptClass()
		 << "._p_.update(null, 'load', null, false);\n";

  if (!widgetset) {
    response.out() << "};\n";

#ifndef WT_TARGET_JAVA
    response.out() << app->javaScriptClass_
		   << "._p_.setServerPush("
		   << (app->serverPush_ ? "true" : "false") << ");";
#endif // WT_TARGET_JAVA

    response.out() << "window.WtScriptLoaded = true;"
                      "if (window.isLoaded) onLoad();\n";
  }

  loadScriptLibraries(response.out(), app, false);
}

void WebRenderer::setJSSynced(bool invisibleToo)
{
  collectedJS1_.str("");
  collectedJS2_.str("");

  if (!invisibleToo)
    collectedJS1_ << invisibleJS_.str();

  invisibleJS_.str("");
}

void WebRenderer::updateLoadIndicator(std::ostream& out, WApplication *app,
				      bool all)
{
  if (app->showLoadingIndicator_->needUpdate() || all) {
    out << "showLoadingIndicator = function() {var o=null,e=null;\n"
	<< app->showLoadingIndicator_->javaScript() << "};\n";
    app->showLoadingIndicator_->updateOk();
  }

  if (app->hideLoadingIndicator_->needUpdate() || all) {
    out << "hideLoadingIndicator = function() {var o=null,e=null;\n"
	<< app->hideLoadingIndicator_->javaScript() << "};\n";
    app->hideLoadingIndicator_->updateOk();
  }
}

/*
 * Serves the Plain or Hybrid HTML page.
 *
 * Requires that the application has been started.
 * The Hybrid page is served when a progressive bootstrap is indicated
 * or when we are in an ajax session.
 */
void WebRenderer::serveMainpage(WebResponse& response)
{
  Configuration& conf = session_.controller()->configuration();

  WApplication *app = session_.app();

  /*
   * This implements the redirect for Post-Redirect-Get, or when the
   * internal path changed.
   *
   * Post-Redirect-Get does not work properly though: refresh() may misbehave
   * and have unintended side effects ?
   */
  if (!app->environment().ajax()
      && (/*response.requestMethod() == "POST"
	  || */(app->internalPathIsChanged_
	      && app->oldInternalPath_ != app->newInternalPath_)))
    session_.redirect(app->bookmarkUrl(app->newInternalPath_));

  std::string redirect = session_.getRedirect();

  if (!redirect.empty()) {
    std::cerr << "Redirect: " << redirect << std::endl;
    response.setRedirect(redirect);
    return;
  }

  WWebWidget *mainWebWidget = app->domRoot_;

  visibleOnly_ = true;

  /*
   * The element to render. This automatically creates loading stubs
   * for invisible widgets, which is also what we want for
   * non-JavaScript versions.
   */
  DomElement *mainElement = mainWebWidget->createSDomElement(app);

  setJSSynced(true);

  const bool xhtml = app->environment().contentType() == WEnvironment::XHTML1;

  std::string styleSheets;

  if (!app->cssTheme().empty()) {
    styleSheets += "<link href=\""
      + WApplication::resourcesUrl() + "/themes/" + app->cssTheme()
      + "/wt.css\" rel=\"stylesheet\" type=\"text/css\""
      + (xhtml ? "/>" : ">");

    if (app->environment().agentIsIE())
      styleSheets += "<link href=\""
	+ WApplication::resourcesUrl() + "/themes/" + app->cssTheme()
	+ "/wt_ie.css\" rel=\"stylesheet\" type=\"text/css\""
	+ (xhtml ? "/>" : ">");
  }

  for (unsigned i = 0; i < app->styleSheets_.size(); ++i) {
    styleSheets += "<link href=\""
      + app->fixRelativeUrl(app->styleSheets_[i]) 
      + "\" rel=\"stylesheet\" type=\"text/css\"" + (xhtml ? "/>" : ">")
      + '\n';
  }
  app->styleSheetsAdded_ = 0;

  beforeLoadJS_.str("");
  for (unsigned i = 0; i < app->scriptLibraries_.size(); ++i) {
    styleSheets += "<script src='"
      + app->fixRelativeUrl(app->scriptLibraries_[i].uri) 
      + "'></script>\n";

    beforeLoadJS_ << app->scriptLibraries_[i].beforeLoadJS;
  }
  app->scriptLibrariesAdded_ = 0;

  app->newBeforeLoadJavaScript_ = app->beforeLoadJavaScript_;

  bool hybridPage = session_.progressiveBoot() || session_.env().ajax();
  FileServe page(hybridPage ? skeletons::Hybrid_html : skeletons::Plain_html);

  setPageVars(page);
  page.setVar("SESSION_ID", session_.sessionId());

  if (hybridPage) {
    setBootVars(response, page);
    page.setVar("INTERNAL_PATH", app->internalPath());
  }

  std::string url
    = (app->environment().agentIsSpiderBot()
       || (conf.sessionTracking() == Configuration::CookiesURL
	   && session_.env().supportsCookies()))
    ? session_.bookmarkUrl(app->newInternalPath_)
    : session_.mostRelativeUrl(app->newInternalPath_);

  url = app->fixRelativeUrl(url);

  url = Wt::Utils::replace(url, '&', "&amp;");
  page.setVar("RELATIVE_URL", url);

  if (conf.inlineCss())
    page.setVar("STYLESHEET", app->styleSheet().cssText(true));
  else
    page.setVar("STYLESHEET", "");

  page.setVar("STYLESHEETS", styleSheets);

  page.setVar("TITLE", WWebWidget::escapeText(app->title()).toUTF8());

  app->titleChanged_ = false;

  if (hybridPage) {
    response.addHeader("Cache-Control", "no-cache, no-store");
    response.addHeader("Expires", "-1");
  }

  std::string contentType = xhtml ? "application/xhtml+xml" : "text/html";

  contentType += "; charset=UTF-8";

  setHeaders(response, contentType);

  formObjectsChanged_ = true;
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
      out << ");\n";
      out << app->javaScriptClass() << "._p_.onJsLoad(\""
	  << uri << "\",function() {\n";
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
	<< app->fixRelativeUrl(app->styleSheets_[i]) << "');\n";
  }

  app->styleSheetsAdded_ = 0;
}

void WebRenderer::collectChanges(std::vector<DomElement *>& changes)
{
  WApplication *app = session_.app();

  do {
    moreUpdates_ = false;

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
	std::cerr << "ignoring: " << (*i)->id()
		  << " (" << typeid(**i).name()
		  << ") " << w->id()
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
	//          << " updating: " << w->id() << std::endl;

#ifdef DEBUG_RENDER
	std::cerr << "updating: " << w->id()
		  << " (" << typeid(*w).name() << ")" << std::endl;
#endif

	if (!learning_ && visibleOnly_) {
	  if (w->isRendered()) {
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
	    std::cerr << "Ignoring: " << w->id() << std::endl;
#else
	  ;
#endif // DEBUG_RENDER
	} else {
	  w->getSDomChanges(changes, app);
	}
      }
    }
  } while (!learning_ && moreUpdates_);
}

void WebRenderer::collectJavaScriptUpdate(std::ostream& out)
{
  WApplication *app = session_.app();

  out << '{';

  collectJS(&out);

  /*
   * Now, as we have cleared and recorded all JavaScript changes that were
   * caused by the actual code, we can learn stateless code and collect
   * changes that result.
   */

  preLearnStateless(app, out);

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

void WebRenderer::updateFormObjectsList(WApplication *app)
{
  if (formObjectsChanged_) {
    currentFormObjects_.clear();

    app->domRoot_->getFormObjects(currentFormObjects_);
    if (app->domRoot2_)
      app->domRoot2_->getFormObjects(currentFormObjects_);
  }
}

std::string WebRenderer::createFormObjectsList(WApplication *app)
{
  updateFormObjectsList(app);

  std::string result;

  for (FormObjectsMap::const_iterator i = currentFormObjects_.begin();
       i != currentFormObjects_.end(); ++i) {
    if (!result.empty())
      result += ',';

    result += "'" + i->first + "'";
  }

  formObjectsChanged_ = false;

  return result;
}

void WebRenderer::collectJS(std::ostream* js)
{
  std::vector<DomElement *> changes;

  collectChanges(changes);

  WApplication *app = session_.app();

  if (js) {
    *js << app->newBeforeLoadJavaScript();

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

  if (js) { 
    if (app->titleChanged_) {
      *js << app->javaScriptClass()
	  << "._p_.setTitle(";
      DomElement::jsStringLiteral(*js, app->title().toUTF8(), '\'');
      *js << ");\n";
    }
  }

  app->titleChanged_ = false;

  if (js) {
    if (app->internalPathIsChanged_)
      *js << app->javaScriptClass()
	  << "._p_.setHash('" << app->newInternalPath_ << "');\n";

    *js << app->afterLoadJavaScript();
  } else
    app->afterLoadJavaScript();

  app->internalPathIsChanged_ = false;
}

void WebRenderer::preLearnStateless(WApplication *app, std::ostream& out)
{
  bool isIEMobile = app->environment().agentIsIEMobile();

  if (isIEMobile || !session_.env().ajax())
    return;

  const WApplication::SignalMap& ss = session_.app()->exposedSignals();

  for (WApplication::SignalMap::const_iterator i = ss.begin();
       i != ss.end(); ++i) {

    if (i->second->sender() == app)
      i->second->processPreLearnStateless(this);

    WWidget *ww = dynamic_cast<WWidget *>(i->second->sender());

    if (ww && ww->isRendered())
      i->second->processPreLearnStateless(this);
  }

  out << statelessJS_.str();
  statelessJS_.str("");
}

std::string WebRenderer::learn(WStatelessSlot* slot)
{
  collectJS(&statelessJS_);

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
    statelessJS_ << result;
  }

  slot->setJavaScript(result);

  return result;
}

std::string WebRenderer::headDeclarations() const
{
  if (!session_.favicon().empty()) {
    const bool xhtml = session_.env().contentType() == WEnvironment::XHTML1;

    return "<link rel=\"icon\" type=\"image/vnd.microsoft.icon\" href=\""
      + session_.favicon() + (xhtml ? "\"/>" : "\">");
  } else
    return std::string();
}

}
