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
  extern const char *JQuery_js;
}

namespace Wt {

const int MESSAGE_COUNTER_SIZE = 5;

WebRenderer::WebRenderer(WebSession& session)
  : session_(session),
    visibleOnly_(true),
    rendered_(false),
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
    || !session_.app()->afterLoadJavaScript_.empty()
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
  /*
   * If we are using an unreliable transport, then we remember
   * JavaScript updates until they are ack'ed. This works because
   * requests are not pipelined.
   *
   * WebSocket requests are pipelined so this simple mechanism will
   * not work. When switching from web sockets to AJAX or vice-versa ?
   * 
   * If normal AJAX request -> web socket closes. We assume everything
   * got delivered and start doing ack updates again.
   *
   * If web socket request -> we assume last AJAX request got
   * delivered ?
   */
  if (updateId == expectedAckId_) {
    setJSSynced(false);
    ++expectedAckId_;
  }
}

void WebRenderer::letReloadJS(WebResponse& response, bool newSession,
			      bool embedded)
{
  if (!embedded)
    setHeaders(response, "text/javascript; charset=UTF-8");

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

void WebRenderer::serveResponse(WebResponse& response)
{
  switch (response.responseType()) {
  case WebResponse::Update:
    serveJavaScriptUpdate(response);
    break;
  case WebResponse::Page:
    if (session_.app())
      serveMainpage(response);
    else
      serveBootstrap(response);
    break;
  case WebResponse::Script:
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
	      safeJsStringLiteral
	      (session_.bootstrapUrl(response, WebSession::KeepInternalPath)));
  boot.setVar("SESSION_ID", session_.sessionId());
  boot.setVar("RANDOMSEED",
	      boost::lexical_cast<std::string>(WtRandom::getUnsigned()
					       + getpid()));
  boot.setVar("RELOAD_IS_NEWSESSION", conf.reloadIsNewSession());
  boot.setVar("USE_COOKIES",
	      conf.sessionTracking() == Configuration::CookiesURL);

  boot.setVar("AJAX_CANONICAL_URL",
	      safeJsStringLiteral(session_.ajaxCanonicalUrl(response)));
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

  rendered_ = false;
}

void WebRenderer::serveError(WebResponse& response, const std::string& message)
{
  bool js = response.responseType() != WebResponse::Page;

  WApplication *app = session_.app();
  if (!js || !app) {
    response.setContentType("text/html");
    response.out()
      << "<title>Error occurred.</title>"
      << "<h2>Error occurred.</h2>"
      << WWebWidget::escapeText(WString(message), true).toUTF8()
      << std::endl;    
  } else {
    response.out() <<
      app->javaScriptClass() << "._p_.quit();"
      "document.title = 'Error occurred.';"
      "document.body.innerHtml='<h2>Error occurred.</h2>' +";
    DomElement::jsStringLiteral(response.out(), message, '\'');
    response.out() << ";";
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
  rendered_ = true;

  setHeaders(response, "text/javascript; charset=UTF-8");

  collectJavaScript();

#ifdef DEBUG_JS
  std::cerr << collectedJS1_.str() << collectedJS2_.str() << std::endl;
#endif // DEBUG_JS

  /*
   * Passing the expectedAckId_ within the collectedJS1_ +
   * collectedJS2_ risks of inflating responses when a script loading
   * is blocked. The purpose of the ackId is to detect what has been
   * succesfully transmitted (mostly in the presence of server push
   * which can cancel ajax requests. Therefore we chose here to use
   * the ackIds_ only to signal proper ajax transfers, and thus at the
   * end of the request transfer.
   *
   * It does present us with another probem: what if e.g. an ExtJS
   * library is still loading and we already update one of its widgets
   * assuming it has been rendered ? This should be handled
   * client-side: only when libraries have been loaded, the application can
   * continue. TO BE DONE.
   */
  response.out()
    << collectedJS1_.str()
    << collectedJS2_.str()
    << session_.app()->javaScriptClass()
    << "._p_.response(" << expectedAckId_ << ");";

  if (response.isWebSocketRequest()
      || response.isWebSocketMessage())
    setJSSynced(false);
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
  js.setVar("WT_CLASS", WT_CLASS);

  /*
   * FIXME: is this still required?
   * Mozilla Bugzilla #246651
   */
  js.setVar("CLOSE_CONNECTION",
	    (conf.serverType() == Configuration::WtHttpdServer)
	    && session_.env().agentIsGecko()
	    && session_.env().agent() < WEnvironment::Firefox3_0);
  
  js.stream(out);
}

void WebRenderer::serveMainscript(WebResponse& response)
{
  /*
   * Serving a script is using a GET request, which can be replayed.
   * Therefore we need to either be able to reconstruct the response
   * (possible if !rendered_), or we need to save the response in
   * collectedJS variables.
   */

  Configuration& conf = session_.controller()->configuration();
  bool widgetset = session_.type() == WidgetSet;

  setHeaders(response, "text/javascript; charset=UTF-8");

  if (!widgetset) {
    // FIXME: this cannot be replayed
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

  FileServe jquery(skeletons::JQuery_js);
  jquery.stream(response.out());

  FileServe script(skeletons::Wt_js);
  script.setCondition("DEBUG", conf.debug());

#ifdef WT_DEBUG_JS
  script.setCondition("DYNAMIC_JS", true);
#else
  script.setCondition("DYNAMIC_JS", false);
#endif // WT_DEBUG_JS

  script.setVar("WT_CLASS", WT_CLASS);
  script.setVar("APP_CLASS", app->javaScriptClass());
  script.setVar("AUTO_JAVASCRIPT",
		"(function(){" + app->autoJavaScript_ + "})");
  script.setCondition("STRICTLY_SERIALIZED_EVENTS", conf.serializedEvents());
  script.setCondition("WEB_SOCKETS", conf.webSockets());
  script.setVar("INNER_HTML", innerHtml);
  script.setVar("FORM_OBJECTS", '[' + currentFormObjectsList_ + ']');

  script.setVar("RELATIVE_URL", WWebWidget::jsStringLiteral
		(session_.bootstrapUrl(response,
				       WebSession::ClearInternalPath)));
  script.setVar("KEEP_ALIVE",
		boost::lexical_cast<std::string>(conf.sessionTimeout() / 2));
  script.setVar("INITIAL_HASH",
		WWebWidget::jsStringLiteral(app->internalPath()));
  script.setVar("INDICATOR_TIMEOUT", conf.indicatorTimeout());
  script.setVar("SERVER_PUSH_TIMEOUT", conf.serverPushTimeout() * 1000);
  script.setVar("ONLOAD",
		std::string("(function() {")
		+ (widgetset ? "" : "window.loadWidgetTree();") + "})");
  script.stream(response.out());

  app->autoJavaScriptChanged_ = false;

  streamCommJs(app, response.out());

  if (!rendered_)
    serveMainAjax(response);
  else {
    if (app->enableAjax_) {
      // Before-load JavaScript of libraries that were loaded directly
      // in HTML
      collectedJS1_ << beforeLoadJS_.str();
      beforeLoadJS_.str("");

      collectedJS1_
	<< "var domRoot = " << app->domRoot_->jsRef() << ";"
	"var form = " WT_CLASS ".getElement('Wt-form');"
	"domRoot.style.display = form.style.display;"
	"document.body.replaceChild(domRoot, form);";

      // Load JavaScript libraries that were added during enableAjax()
      loadScriptLibraries(collectedJS1_, app, true); 

      collectedJS1_ << app->newBeforeLoadJavaScript();
	
      collectedJS2_
	<< "domRoot.style.visibility = 'visible';"
	<< app->javaScriptClass() << "._p_.autoJavaScript();";

      loadScriptLibraries(collectedJS2_, app, false);

      app->enableAjax_ = false;
    }

    response.out() << "window.loadWidgetTree = function(){\n";

    visibleOnly_ = false;

    collectJavaScript();
    updateLoadIndicator(collectedJS1_, app, true);

#ifdef DEBUG_JS
    std::cerr << collectedJS1_.str() << collectedJS2_.str() << std::endl;
#endif // DEBUG_JS

    response.out()
      << collectedJS1_.str()
      << app->javaScriptClass()
      << "._p_.response(" << expectedAckId_ << ");";

    response.out()
	<< app->javaScriptClass()
	<< "._p_.update(null, 'load', null, false);"
	<< collectedJS2_.str()
	<< "};" // loadWidgetTree = function() { ... }
	<< app->javaScriptClass()
	<< "._p_.setServerPush("
	<< (app->updatesEnabled() ? "true" : "false") << ");"
	<< "window.WtScriptLoaded = true;"
	<< "if (window.isLoaded) onLoad();\n";
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
		   << app->cssTheme() << "/wt.css', 'all');";
    if (app->environment().agentIsIE())
      response.out() << WT_CLASS << ".addStyleSheet('"
		     << WApplication::resourcesUrl() << "/themes/"
		     << app->cssTheme() << "/wt_ie.css', 'all');";
    if (app->environment().agent() == WEnvironment::IE6)
      response.out() << WT_CLASS << ".addStyleSheet('"
		     << WApplication::resourcesUrl() << "/themes/"
		     << app->cssTheme() << "/wt_ie6.css', 'all');";
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

  if (app->isQuited())
    s << app->javaScriptClass() << "._p_.quit();";

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
		 << app->hideLoadingIndicator_.javaScript()
		 << "}";

  if (widgetset)
    response.out() << app->javaScriptClass() << "._p_.load();\n";

  if (!app->isQuited())
    response.out() << session_.app()->javaScriptClass()
		   << "._p_.update(null, 'load', null, false);\n";

  if (!widgetset) {
    response.out() << "};\n";

    response.out() << app->javaScriptClass()
		   << "._p_.setServerPush("
		   << (app->updatesEnabled() ? "true" : "false") << ");";

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

std::string WebRenderer::safeJsStringLiteral(const std::string& value)
{
  std::string s = WWebWidget::jsStringLiteral(value);
  return Wt::Utils::replace(s, "<", "<'+'");
}

void WebRenderer::updateLoadIndicator(std::ostream& out, WApplication *app,
				      bool all)
{
  if (app->showLoadingIndicator_.needsUpdate(all)) {
    out << "showLoadingIndicator = function() {var o=null,e=null;\n"
	<< app->showLoadingIndicator_.javaScript() << "};\n";
    app->showLoadingIndicator_.updateOk();
  }

  if (app->hideLoadingIndicator_.needsUpdate(all)) {
    out << "hideLoadingIndicator = function() {var o=null,e=null;\n"
	<< app->hideLoadingIndicator_.javaScript() << "};\n";
    app->hideLoadingIndicator_.updateOk();
  }
}

/*
 * Serves the Plain or Hybrid HTML page.
 *
 * Requires that the application has been started.
 *
 * The Hybrid page is served when a progressive bootstrap is indicated
 * or when we are in an ajax session. We need to remember that in the next
 * serveMainscript() we only need to serve an update, not render the whole
 * interface.
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
		&& app->oldInternalPath_ != app->newInternalPath_))) {
    app->oldInternalPath_ = app->newInternalPath_;
    session_.redirect(app->bookmarkUrl(app->newInternalPath_));
  }

  std::string redirect = session_.getRedirect();

  if (!redirect.empty()) {
    response.setStatus(302); // Should be 303 in fact ?
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
  rendered_ = true;

  setJSSynced(true);

  const bool xhtml = app->environment().contentType() == WEnvironment::XHTML1;

  std::string styleSheets;

  if (!app->cssTheme().empty()) {
    styleSheets += "<link href=\""
      + WApplication::resourcesUrl() + "/themes/" + app->cssTheme()
      + "/wt.css\" rel=\"stylesheet\" type=\"text/css\""
      + (xhtml ? "/>" : ">") + "\n";

    if (app->environment().agentIsIE())
      styleSheets += "<link href=\""
	+ WApplication::resourcesUrl() + "/themes/" + app->cssTheme()
	+ "/wt_ie.css\" rel=\"stylesheet\" type=\"text/css\""
	+ (xhtml ? "/>" : ">") + "\n";

    if (app->environment().agent() == WEnvironment::IE6)
      styleSheets += "<link href=\""
	+ WApplication::resourcesUrl() + "/themes/" + app->cssTheme()
	+ "/wt_ie6.css\" rel=\"stylesheet\" type=\"text/css\""
	+ (xhtml ? "/>" : ">") + "\n";
  }

  for (unsigned i = 0; i < app->styleSheets_.size(); ++i) {
    std::string url = app->styleSheets_[i].uri;
    url = Wt::Utils::replace(url, '&', "&amp;");
    styleSheets += "<link href=\""
      + app->fixRelativeUrl(url) + "\" rel=\"stylesheet\" type=\"text/css\"";

    if (!app->styleSheets_[i].media.empty()
	&& app->styleSheets_[i].media != "all")
      styleSheets += " media=\"" + app->styleSheets_[i].media + '"';

    styleSheets += xhtml ? "/>" : ">";
    styleSheets += "\n";
  }
  app->styleSheetsAdded_ = 0;

  beforeLoadJS_.str("");
  for (unsigned i = 0; i < app->scriptLibraries_.size(); ++i) {
    std::string url = app->scriptLibraries_[i].uri;
    url = Wt::Utils::replace(url, '&', "&amp;");
    styleSheets += "<script src='" + app->fixRelativeUrl(url) + "'></script>\n";

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
    page.setVar("INTERNAL_PATH", safeJsStringLiteral(app->internalPath()));
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
    EscapeOStream js;
    EscapeOStream out(response.out());
    mainElement->asHTML(out, js, timeouts);
    app->afterLoadJavaScript_ = js.str() + app->afterLoadJavaScript_;
    delete mainElement;
  }

  int refresh;
  if (app->environment().ajax()) {
    std::stringstream str;
    DomElement::createTimeoutJs(str, timeouts, app);
    app->doJavaScript(str.str());

    refresh = 1000000;
  } else {
    if (app->isQuited())
      refresh = 1000000;
    else {
      refresh = conf.sessionTimeout() / 3;
      for (unsigned i = 0; i < timeouts.size(); ++i)
	refresh = std::min(refresh, 1 + timeouts[i].msec/1000);
    }
  }
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
    if (app->scriptLibraries_.size() - first > 0)
      out << app->javaScriptClass() << "._p_.autoJavaScript();";
    for (unsigned i = first; i < app->scriptLibraries_.size(); ++i)
      out << "});";
    app->scriptLibrariesAdded_ = 0;
  }
}

void WebRenderer::loadStyleSheets(std::ostream& out, WApplication *app)
{
  int first = app->styleSheets_.size() - app->styleSheetsAdded_;

  for (unsigned i = first; i < app->styleSheets_.size(); ++i) {
    out << WT_CLASS << ".addStyleSheet('"
	<< app->fixRelativeUrl(app->styleSheets_[i].uri) << "', '"
	<< app->styleSheets_[i].media << "');\n";
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

  if (app->isQuited())
    out << app->javaScriptClass() << "._p_.quit();";

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
	  << "._p_.setTitle(" << app->title().jsStringLiteral() << ");\n";
    }

    if (app->closeMessageChanged_) {
      *js << app->javaScriptClass()
	  << "._p_.setCloseMessage(" << app->closeMessage().jsStringLiteral()
	  << ");\n";
    }
  }

  app->titleChanged_ = false;
  app->closeMessageChanged_ = false;

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

  collectJS(&out);

  WApplication::SignalMap& ss = session_.app()->exposedSignals();

  for (WApplication::SignalMap::iterator i = ss.begin();
       i != ss.end(); ) {

#ifdef WT_TARGET_JAVA
    Wt::EventSignalBase *s = i->second.get();
    if (!s) {
      Utils::eraseAndNext(ss, i);
      continue;
    }
#else
    Wt::EventSignalBase* s = i->second;
#endif // WT_TARGET_JAVA

    if (s->sender() == app)
      s->processPreLearnStateless(this);

    WWidget *ww = dynamic_cast<WWidget *>(s->sender());

    if (ww && ww->isRendered())
      s->processPreLearnStateless(this);

    ++i;
  }

  out << statelessJS_.str();
  statelessJS_.str("");
}

std::string WebRenderer::learn(WStatelessSlot* slot)
{
  if (slot->type() == WStatelessSlot::PreLearnStateless)
    learning_ = true;

  learningIncomplete_ = false;

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

  if (!learningIncomplete_)
    slot->setJavaScript(result);

  collectJS(&statelessJS_);

  return result;
}

void WebRenderer::learningIncomplete()
{
  learningIncomplete_ = true;
}

std::string WebRenderer::headDeclarations() const
{
  const bool xhtml = session_.env().contentType() == WEnvironment::XHTML1;

  EscapeOStream result;

  if (session_.app()) {
    for (unsigned i = 0; i < session_.app()->metaHeaders_.size(); ++i) {
      const WApplication::MetaHeader& m = session_.app()->metaHeaders_[i];

      result << "<meta";

      if (!m.name.empty()) {
	if (m.type == MetaName)
	  result << " name=\"";
	else
	  result << " http-equiv=\"";
	result.pushEscape(EscapeOStream::HtmlAttribute);
	result << m.name;
	result.popEscape();
	result << '"';
      }

      if (!m.lang.empty()) {
	result << " lang=\"";
	result.pushEscape(EscapeOStream::HtmlAttribute);
	result << m.lang;
	result.popEscape();
	result << '"';
      }

      result << " content=\"";
      result.pushEscape(EscapeOStream::HtmlAttribute);
      result << m.content.toUTF8();
      result.popEscape();
      result << (xhtml ? "\"/>" : "\">");
    }
  } else
    if (session_.env().agentIsIE()
	&& session_.env().agent() <= WEnvironment::IE8)
      result << "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=7"
	     << (xhtml ? "\"/>" : "\">") << '\n';

  if (!session_.favicon().empty())
    result <<
      "<link rel=\"icon\" "
      "type=\"image/vnd.microsoft.icon\" "
      "href=\"" << session_.favicon() << (xhtml ? "\"/>" : "\">");

  return result.str();
}

}
