/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WRandom"
#include "Wt/WRegExp"
#include "Wt/WWebWidget"
#include "Wt/WStringStream"
#include "Wt/WTheme"
#include "Wt/Utils"

#include "Configuration.h"
#include "DomElement.h"
#include "EscapeOStream.h"
#include "FileServe.h"
#include "WebController.h"
#include "WebRenderer.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WebUtils.h"

#ifdef WT_WIN32
#include <process.h> // for getpid()
#ifdef min
#undef min
#endif
#endif

#ifndef WT_TARGET_JAVA
#define DESCRIBE(w) typeid(*(w)).name()
#else
#define DESCRIBE(w) "(fixme)"
#endif

namespace {

  bool isAbsoluteUrl(const std::string& url) {
    return url.find("://") != std::string::npos;
  }

  void appendAttribute(Wt::EscapeOStream& eos,
		       const std::string& name, 
		       const std::string& value) {
    eos << ' ' << name << "=\"";
    eos.pushEscape(Wt::EscapeOStream::HtmlAttribute);
    eos << value;
    eos.popEscape();
    eos << '"';
  }

  void closeSpecial(Wt::WStringStream& s) {
    s << ">\n";
  }

  void closeSpecial(Wt::EscapeOStream& s) {
    s << ">\n";
  }
}

namespace skeletons {
  extern const char *Boot_html1;
  extern const char *Plain_html1;
  extern const char *Hybrid_html1;
  extern const char *Wt_js1;
  extern const char *Boot_js1;
  extern const char *JQuery_js1;

  extern std::vector<const char *> JQuery_js();
  extern std::vector<const char *> Wt_js();
}

namespace Wt {

LOGGER("WebRenderer");

WebRenderer::CookieValue::CookieValue()
  : secure(false)
{ }

WebRenderer::CookieValue::CookieValue(const std::string& v,
				      const std::string& p,
				      const std::string& d,
				      const WDateTime& e,
				      bool s)
  : value(v),
    path(p),
    domain(d),
    expires(e),
    secure(s)
{ }

WebRenderer::WebRenderer(WebSession& session)
  : session_(session),
    visibleOnly_(true),
    rendered_(false),
    initialStyleRendered_(false),
    twoPhaseThreshold_(5000),
    pageId_(0),
    expectedAckId_(0),
    scriptId_(0),
    ackErrs_(0),
    linkedCssCount_(-1),
    currentStatelessSlotIsActuallyStateless_(true),
    formObjectsChanged_(true),
    updateLayout_(false),
    learning_(false)
{ }

void WebRenderer::setRendered(bool how)
{
  if (rendered_ != how) {
    LOG_DEBUG("setRendered: " << how);
    rendered_ = how;
  }
}

void WebRenderer::setTwoPhaseThreshold(int bytes)
{
  twoPhaseThreshold_ = bytes;
}

void WebRenderer::needUpdate(WWidget *w, bool laterOnly)
{
  LOG_DEBUG("needUpdate: " << w->id() << " (" << DESCRIBE(w) << ")");

  updateMap_.insert(w);

  if (!laterOnly)
    moreUpdates_ = true;
}

void WebRenderer::doneUpdate(WWidget *w)
{
  LOG_DEBUG("doneUpdate: " << w->id() << " (" << DESCRIBE(w) << ")");

  updateMap_.erase(w);
}

bool WebRenderer::isDirty() const
{
  return !updateMap_.empty()
    || formObjectsChanged_
    || session_.app()->isQuited()
    || !session_.app()->afterLoadJavaScript_.empty()
    || session_.app()->serverPushChanged_
    || session_.app()->styleSheetsAdded_
    || !session_.app()->styleSheetsToRemove_.empty()
    || session_.app()->styleSheet().isDirty()
    || session_.app()->internalPathIsChanged_
    || !collectedJS1_.empty()
    || !collectedJS2_.empty()
    || !invisibleJS_.empty();
}

const WebRenderer::FormObjectsMap& WebRenderer::formObjects() const
{
  return currentFormObjects_;
}

std::string WebRenderer::bodyClassRtl() const
{
  if (session_.app()) {
    std::string s = session_.app()->bodyClass_;
    if (!s.empty())
      s += ' ';

    s += session_.app()->layoutDirection() == LeftToRight
      ? "Wt-ltr" : "Wt-rtl";

    session_.app()->bodyHtmlClassChanged_ = false;

    return s;
  } else
    return std::string();
}

void WebRenderer::saveChanges()
{
  collectJS(&collectedJS1_);
}

void WebRenderer::discardChanges()
{
  collectJS(0);
}

bool WebRenderer::ackUpdate(int updateId)
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
    LOG_DEBUG("jsSynced(false) after ackUpdate okay");
    setJSSynced(false);
    ackErrs_ = 0;
    return true;
  } else if ((updateId < expectedAckId_ && expectedAckId_ - updateId < 5)
	     || (expectedAckId_ - 5 < updateId)) {
    ++ackErrs_;
    return ackErrs_ < 3; // That's still acceptible but no longer plausible
  } else
    return false;
}

void WebRenderer::letReloadJS(WebResponse& response, bool newSession,
			      bool embedded)
{
  if (!embedded) {
    setCaching(response, false);
    setHeaders(response, "text/javascript; charset=UTF-8");
  }

  // FIXME: we should foresee something independent of app->javaScriptClass()
  response.out() <<
    "if (window.Wt) window.Wt._p_.quit(null); window.location.reload(true);";
}

void WebRenderer::letReloadHTML(WebResponse& response, bool newSession)
{
  setCaching(response, false);
  setHeaders(response, "text/html; charset=UTF-8");

  response.out() << "<html><script type=\"text/javascript\">";
  letReloadJS(response, newSession, true);
  response.out() << "</script><body></body></html>";
}

void WebRenderer::streamRedirectJS(WStringStream& out,
				   const std::string& redirect)
{
  if (session_.app() && session_.app()->internalPathIsChanged_)
    out << "if (window." << session_.app()->javaScriptClass() << ") "
	<< session_.app()->javaScriptClass()
	<< "._p_.setHash("
	<< WWebWidget::jsStringLiteral(session_.app()->newInternalPath_)
	<< ", false);\n";
  out <<
    "if (window.location.replace)"
    " window.location.replace('" << redirect << "');"
    "else"
    " window.location.href='" << redirect << "';\n";
}

void WebRenderer::serveResponse(WebResponse& response)
{
  session_.setTriggerUpdate(false);

  switch (response.responseType()) {
  case WebResponse::Update:
    serveJavaScriptUpdate(response);
    break;
  case WebResponse::Page:
    initialStyleRendered_ = false;
    ++pageId_;
    if (session_.app())
      serveMainpage(response);
    else
      serveBootstrap(response);
    break;
  case WebResponse::Script:
    bool hybridPage = session_.progressiveBoot() || session_.env().ajax();
    if (!hybridPage)
      setRendered(false);
    serveMainscript(response);
    break;
  }
}

void WebRenderer::setPageVars(FileServe& page)
{
  WApplication *app = session_.app();

  page.setVar("DOCTYPE", session_.docType());

  std::string htmlAttr;
  if (app && !app->htmlClass_.empty()) {
    htmlAttr = " class=\"" + app->htmlClass_ + "\"";
  }

  if (session_.env().agentIsIE())
    page.setVar("HTMLATTRIBUTES",
		"xmlns:v=\"urn:schemas-microsoft-com:vml\""
		" lang=\"en\" dir=\"ltr\"" + htmlAttr);
  else
    page.setVar("HTMLATTRIBUTES", "lang=\"en\" dir=\"ltr\"" + htmlAttr);
  page.setVar("METACLOSE", ">");

  std::string attr = bodyClassRtl();

  if (!attr.empty())
    attr = " class=\"" + attr + "\"";

  if (app && app->layoutDirection() == RightToLeft)
    attr += " dir=\"RTL\"";

  page.setVar("BODYATTRIBUTES", attr);

  page.setVar("HEADDECLARATIONS", headDeclarations());

  page.setCondition("FORM", !session_.env().agentIsSpiderBot()
		    && !session_.env().ajax());
  page.setCondition("BOOT_STYLE", true);
}

void WebRenderer::streamBootContent(WebResponse& response, 
				    FileServe& boot, bool hybrid)
{
  Configuration& conf = session_.controller()->configuration();

  WStringStream out(response.out());

  FileServe bootJs(skeletons::Boot_js1);

  boot.setVar("BLANK_HTML",
	      session_.bootstrapUrl(response, WebSession::ClearInternalPath)
	      + "&amp;request=resource&amp;resource=blank");
  boot.setVar("SESSION_ID", session_.sessionId());
  //TODO remove APP_CLASS, will later only be used in the javascript
  boot.setVar("APP_CLASS", "Wt");

  bootJs.setVar("SELF_URL",
		safeJsStringLiteral
		(session_.bootstrapUrl(response, 
				       WebSession::ClearInternalPath)));
  bootJs.setVar("SESSION_ID", session_.sessionId());

  expectedAckId_ = scriptId_ = WRandom::get();
  ackErrs_ = 0;

  bootJs.setVar("SCRIPT_ID", scriptId_);
  bootJs.setVar("RANDOMSEED", WRandom::get());
  bootJs.setVar("RELOAD_IS_NEWSESSION", conf.reloadIsNewSession());
  bootJs.setVar("USE_COOKIES",
		conf.sessionTracking() == Configuration::CookiesURL);
  bootJs.setVar("AJAX_CANONICAL_URL",
		safeJsStringLiteral(session_.ajaxCanonicalUrl(response)));
  bootJs.setVar("APP_CLASS", "Wt");
  bootJs.setVar("PATH_INFO", safeJsStringLiteral
		(session_.pagePathInfo_));

  bootJs.setCondition("COOKIE_CHECKS", conf.cookieChecks());
  bootJs.setCondition("SPLIT_SCRIPT", conf.splitScript());
  bootJs.setCondition("HYBRID", hybrid);
  bootJs.setCondition("PROGRESS", hybrid && !session_.env().ajax());
  bootJs.setCondition("DEFER_SCRIPT", true);
  bootJs.setCondition("WEBGL_DETECT", conf.webglDetect());

  std::string internalPath
    = hybrid ? session_.app()->internalPath() : session_.env().internalPath();
  bootJs.setVar("INTERNAL_PATH", safeJsStringLiteral(internalPath));

  boot.streamUntil(out, "BOOT_JS");
  bootJs.stream(out);

  out.spool(response.out());
}

void WebRenderer::serveLinkedCss(WebResponse& response)
{
  response.setContentType("text/css");

  if (!initialStyleRendered_) {
    WApplication *app = session_.app();
    
    WStringStream out(response.out());

    if (app->theme())
      app->theme()->serveCss(out);

    for (unsigned i = 0; i < app->styleSheets_.size(); ++i)
      app->styleSheets_[i].cssText(out, true);

    app->styleSheetsAdded_ = 0;

    initialStyleRendered_ = true;
    linkedCssCount_ = app->styleSheets_.size();

    out.spool(response.out());
  } else if (linkedCssCount_ > -1) {
    /*
     * Make sure we serve the same response again, since a 'GET' must be
     *idempotent. This is used by e.g. browser-side tools like 'usersnap'
     */
    WApplication *app = session_.app();

    WStringStream out(response.out());

    if (app->theme())
      app->theme()->serveCss(out);

    unsigned count
      = std::min((std::size_t)linkedCssCount_, app->styleSheets_.size());

    for (unsigned i = 0; i < count; ++i)
      app->styleSheets_[i].cssText(out, true);

    out.spool(response.out());
  }
}

void WebRenderer::serveBootstrap(WebResponse& response)
{
  Configuration& conf = session_.controller()->configuration();

  FileServe boot(skeletons::Boot_html1);
  setPageVars(boot);

  WStringStream noJsRedirectUrl;
  DomElement::htmlAttributeValue
    (noJsRedirectUrl,
     session_.bootstrapUrl(response, WebSession::KeepInternalPath) + "&js=no");

  boot.setVar("REDIRECT_URL", noJsRedirectUrl.str());
  boot.setVar("AUTO_REDIRECT",
	      "<noscript><meta http-equiv=\"refresh\" content=\"0; url="
	      + noJsRedirectUrl.str() + "\"></noscript>");
  boot.setVar("NOSCRIPT_TEXT", conf.redirectMessage());

  WStringStream bootStyleUrl;
  DomElement::htmlAttributeValue
    (bootStyleUrl,
     session_.bootstrapUrl(response, WebSession::ClearInternalPath)
     + "&request=style&page="
     + boost::lexical_cast<std::string>(pageId_));

  boot.setVar("BOOT_STYLE_URL", bootStyleUrl.str());

  setCaching(response, false);

  std::string contentType = "text/html; charset=UTF-8";

  setHeaders(response, contentType);

  WStringStream out(response.out());
  streamBootContent(response, boot, false);
  boot.stream(out);

  setRendered(false);

  out.spool(response.out());
}

void WebRenderer::serveError(int status, WebResponse& response,
			     const std::string& message)
{
  bool js = response.responseType() != WebResponse::Page;

  WApplication *app = session_.app();
  if (!js || !app) {
    response.setStatus(status);
    response.setContentType("text/html");
    response.out()
      << "<title>Error occurred.</title>"
      << "<h2>Error occurred.</h2>"
      << WWebWidget::escapeText(WString(message), true).toUTF8()
      << '\n';
  } else {
    response.out() << app->javaScriptClass()
		   << "._p_.quit(null);"
		   << "document.title = 'Error occurred.';"
		   << "document.body.innerHtml='<h2>Error occurred.</h2>' +"
		   <<  WWebWidget::jsStringLiteral(message)
		   << ';';
  }
}

void WebRenderer::setCookie(const std::string name, const std::string value,
			    const WDateTime& expires,
			    const std::string domain, const std::string path,
			    bool secure)
{
  cookiesToSet_[name] = CookieValue(value, path, domain, expires, secure);
}

void WebRenderer::setCaching(WebResponse& response, bool allowCache)
{
  if (allowCache)
    response.addHeader("Cache-Control", "max-age=2592000,private");
  else {
    response.addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response.addHeader("Pragma", "no-cache");
    response.addHeader("Expires", "0");
  }
}

void WebRenderer::setHeaders(WebResponse& response, const std::string mimeType)
{
  for (std::map<std::string, CookieValue>::const_iterator
	 i = cookiesToSet_.begin(); i != cookiesToSet_.end(); ++i) {
    const CookieValue& cookie = i->second;

    WStringStream header;

    std::string value = cookie.value;
    if (value.empty())
      value = "deleted";

    header << Utils::urlEncode(i->first) << '=' << Utils::urlEncode(value)
	   << "; Version=1;";

    if (!cookie.expires.isNull()) {
#ifndef WT_TARGET_JAVA
      std::string formatString = "ddd, dd-MMM-yyyy hh:mm:ss 'GMT'";
#else
      std::string formatString = "EEE, dd-MMM-yyyy hh:mm:ss 'GMT'";
#endif

      std::string d
	= cookie.expires.toString
	(WString::fromUTF8(formatString), false).toUTF8();

      header << "Expires=" << d << ';';
    }

    if (!cookie.domain.empty())
      header << " Domain=" << cookie.domain << ';';

    if (cookie.path.empty())
      header << " Path=" << session_.env().deploymentPath() << ';';
    else
      header << " Path=" << cookie.path << ';';

    // a httponly cookie cannot be set using JavaScript
    if (!response.isWebSocketMessage())
      header << " httponly;";

    if (cookie.secure)
      header << " secure;";

    response.addHeader("Set-Cookie", header.str());
  }
  cookiesToSet_.clear();

#ifndef WT_TARGET_JAVA
  const WServer *s = session_.controller()->server();
  if (s->dedicatedSessionProcess()) {
    response.addHeader("X-Wt-Session", session_.sessionId());
  }
#endif // WT_TARGET_JAVA

  response.setContentType(mimeType);
}

void WebRenderer::renderSetServerPush(WStringStream& out)
{
  if (session_.app()->serverPushChanged_) {
    out << session_.app()->javaScriptClass()
	<< "._p_.setServerPush("
	<< session_.app()->updatesEnabled()
	<< ");";

    session_.app()->serverPushChanged_ = false;
  }
}

std::string WebRenderer::sessionUrl() const
{
  std::string result = session_.applicationUrl();
  if (isAbsoluteUrl(result))
    return session_.appendSessionQuery(result);
  else {
    // Wt.js will prepand the correct deployment path
    return session_.appendSessionQuery(".").substr(1);
  }
}

void WebRenderer::serveJavaScriptUpdate(WebResponse& response)
{
  setCaching(response, false);
  setHeaders(response, "text/javascript; charset=UTF-8");

  if (session_.sessionIdChanged_) {
    collectedJS1_ << session_.app()->javaScriptClass()
		  << "._p_.setSessionUrl("
		  << WWebWidget::jsStringLiteral(sessionUrl())
		  << ");";
  }

  WStringStream out(response.out());

  if (!rendered_) {
    serveMainAjax(out);
  } else {
    collectJavaScript();

    addResponseAckPuzzle(out);
    renderSetServerPush(out);

    LOG_DEBUG("js: " << collectedJS1_.str() << collectedJS2_.str());

    out << collectedJS1_.str() << collectedJS2_.str();

    if (response.isWebSocketMessage()) {
      LOG_DEBUG("jsSynced(false) after rendering websocket message");
      setJSSynced(false);
    }
  }

  out.spool(response.out());
}

void WebRenderer::addContainerWidgets(WWebWidget *w,
				      std::vector<WContainerWidget *>& result)
{
  for (unsigned i = 0; i < w->children().size(); ++i) {
    WWidget *c = w->children()[i];

    if (!c->isRendered())
      return;

    if (!c->isHidden())
      addContainerWidgets(c->webWidget(), result);

    WContainerWidget *wc = dynamic_cast<WContainerWidget *>(c);
    if (wc)
      result.push_back(wc);
  }
}

void WebRenderer::addResponseAckPuzzle(WStringStream& out)
{
  std::string puzzle;

  Configuration& conf = session_.controller()->configuration();
  if (conf.ajaxPuzzle() && expectedAckId_ == scriptId_) {
    /*
     * We need to pick a random WContainerWidget. Let's be dumb for now.
     */
    std::vector<WContainerWidget *> widgets;

    WApplication *app = session_.app();
    addContainerWidgets(app->domRoot_, widgets);
    if (app->domRoot2_)
      addContainerWidgets(app->domRoot2_, widgets);
    
    unsigned r = WRandom::get() % widgets.size();

    WContainerWidget *wc = widgets[r];

    puzzle = '"' + wc->id() + '"';

    std::string l;
    for (WWidget *w = wc->parent(); w; w = w->parent()) {
      if (w->id().empty())
	continue;
      if (w->id() == l)
	continue;

      l = w->id();

      if (!solution_.empty())
	solution_ += ',';

      solution_ += l;
    }
  }

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
  out << session_.app()->javaScriptClass()
      << "._p_.response(" << ++expectedAckId_;
  if (!puzzle.empty())
    out << "," << puzzle;
  out << ");";
}

bool WebRenderer::checkResponsePuzzle(const WebRequest& request)
{
  if (!solution_.empty()) {
    const std::string *ackPuzzleE = request.getParameter("ackPuzzle");

    if (!ackPuzzleE) {
      LOG_SECURE("Ajax puzzle fail: solution missing");
      return false;
    }

    std::string ackPuzzle = *ackPuzzleE;

    Utils::SplitVector answer, solution;

    boost::split(solution, solution_, boost::is_any_of(","));
    boost::split(answer, ackPuzzle, boost::is_any_of(","));

    unsigned j = 0;
    bool fail = false;

    for (unsigned i = 0; i < solution.size(); ++i) {
      for (; j < answer.size(); ++j) {
	if (solution[i] == answer[j])

	  break;
	else {
	  /* Verify that answer[j] is not a valid widget id */
	}
      }

      if (j == answer.size()) {
	fail = true;
	break;
      }
    }

    if (j < answer.size() - 1)
      fail = true;
   
    if (fail) {
      LOG_SECURE("Ajax puzzle fail: '" << ackPuzzle << "' vs '"
		 << solution_ << '\'');

      solution_.clear();

      return false;
    } else {
      solution_.clear();
      return true;
    }
  } else
    return true;
}

void WebRenderer::collectJavaScript()
{
  WApplication *app = session_.app();
  Configuration& conf = session_.controller()->configuration();

  /*
   * Pending invisible changes are also collected into JS1. This is
   * also done in ackUpdate(), but just in case an update was not
   * acknowledged.
   *
   * This is also used to render JavaScript that was rendered in asHtml()
   * in a hybrid page.
   */
  LOG_DEBUG("Rendering invisible: " << invisibleJS_.str());
  
  collectedJS1_ << invisibleJS_.str();
  invisibleJS_.clear();

  if (conf.inlineCss())
    app->styleSheet().javaScriptUpdate(app, collectedJS1_, false);

  loadStyleSheets(collectedJS1_, app);

  if (app->bodyHtmlClassChanged_) {
    bool widgetset = session_.type() == WidgetSet;
    std::string op = widgetset ? "+=" : "=";
    collectedJS1_ << "document.body.parentNode.className" << op << '\''
		  << app->htmlClass_ << "';"
		  << "document.body.className" << op << '\'' << bodyClassRtl() << "';"
		  << "document.body.setAttribute('dir', '";
    if (app->layoutDirection() == LeftToRight)
      collectedJS1_ << "LTR";
    else
      collectedJS1_ << "RTL";
    collectedJS1_ << "');";
  }

  /*
   * This opens scopes, waiting for new libraries to be loaded.
   */
  int librariesLoaded = loadScriptLibraries(collectedJS1_, app);

  /*
   * This closes the same scopes.
   */
  loadScriptLibraries(collectedJS2_, app, librariesLoaded);

  /*
   * Everything else happens inside JS1: after libraries have been loaded.
   */
  app->streamBeforeLoadJavaScript(collectedJS1_, false);

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

	if (invisibleJS_.length() < (unsigned)twoPhaseThreshold_) {
	  collectedJS1_ << invisibleJS_.str();
	  invisibleJS_.clear();
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

  bool serveSkeletons = !conf.splitScript() 
    || response.getParameter("skeleton");
  bool serveRest = !conf.splitScript() || !serveSkeletons;

  session_.sessionIdChanged_ = false;

  setCaching(response, conf.splitScript() && serveSkeletons);
  setHeaders(response, "text/javascript; charset=UTF-8");

  WStringStream out(response.out());

  if (!widgetset) {
    // FIXME: this cannot be replayed
    std::string redirect = session_.getRedirect();

    if (!redirect.empty()) {
      streamRedirectJS(out, redirect);
      return;
    }
  } else {
    expectedAckId_ = scriptId_ = WRandom::get();
    ackErrs_ = 0;
  }

  WApplication *app = session_.app();

  const bool innerHtml = true;

  if (serveSkeletons) {
    bool haveJQuery = app->customJQuery();

    if (!haveJQuery) {
      out << "if (typeof window.$ === 'undefined') {";
#ifndef WT_TARGET_JAVA
      std::vector<const char *> parts = skeletons::JQuery_js();
      for (std::size_t i = 0; i < parts.size(); ++i)
	out << const_cast<char *>(parts[i]);
#else
      out << const_cast<char *>(skeletons::JQuery_js1);
#endif
      out << '}';
    }

#ifndef WT_TARGET_JAVA
    std::vector<const char *> parts = skeletons::Wt_js();
#else
    std::vector<const char *> parts = std::vector<const char *>();
#endif
    std::string Wt_js_combined;
    if (parts.size() > 1)
      for (std::size_t i = 0; i < parts.size(); ++i)
	Wt_js_combined += parts[i];

    FileServe script(parts.size() > 1
		     ? Wt_js_combined.c_str() : skeletons::Wt_js1);

    script.setCondition
      ("CATCH_ERROR", conf.errorReporting() != Configuration::NoErrors);
    script.setCondition
      ("SHOW_ERROR", conf.errorReporting() == Configuration::ErrorMessage);
    script.setCondition
      ("UGLY_INTERNAL_PATHS", session_.useUglyInternalPaths());

#ifdef WT_DEBUG_JS
    script.setCondition("DYNAMIC_JS", true);
#else
    script.setCondition("DYNAMIC_JS", false);
#endif // WT_DEBUG_JS

    script.setVar("WT_CLASS", WT_CLASS);
    script.setVar("APP_CLASS", app->javaScriptClass());
    script.setCondition("STRICTLY_SERIALIZED_EVENTS", conf.serializedEvents());
    script.setCondition("WEB_SOCKETS", conf.webSockets());
    script.setVar("INNER_HTML", innerHtml);
    script.setVar("ACK_UPDATE_ID", expectedAckId_);
    script.setVar("SESSION_URL", WWebWidget::jsStringLiteral(sessionUrl()));
    script.setVar("QUITTED_STR",
		  WString::tr("Wt.QuittedMessage").jsStringLiteral());

    std::string deployPath = session_.env().publicDeploymentPath_;
    if (deployPath.empty())
      deployPath = session_.deploymentPath();

    script.setVar("DEPLOY_PATH", WWebWidget::jsStringLiteral(deployPath));

    int keepAlive;
    if (conf.sessionTimeout() == -1)
      keepAlive = 1000000;
    else
      keepAlive = conf.sessionTimeout() / 2;
    script.setVar("KEEP_ALIVE", boost::lexical_cast<std::string>(keepAlive));

    script.setVar("INDICATOR_TIMEOUT", conf.indicatorTimeout());
    script.setVar("SERVER_PUSH_TIMEOUT", conf.serverPushTimeout() * 1000);

    /*
     * Was in honor of Mozilla Bugzilla #246651
     */
    script.setVar("CLOSE_CONNECTION", false);

    /*
     * Set the original script params for a widgetset session, so that any
     * Ajax update request has all the information to reload the session.
     */
    std::string params;
    if (session_.type() == WidgetSet) {
      const Http::ParameterMap *m = &session_.env().getParameterMap();
      Http::ParameterMap::const_iterator it = m->find("Wt-params");
      Http::ParameterMap wtParams;
      if (it != m->end()) {
	// Parse and reencode Wt-params, so it's definitely safe
	Http::Request::parseFormUrlEncoded(it->second[0], wtParams);
	m = &wtParams;
      }
      for (Http::ParameterMap::const_iterator i = m->begin();
	   i != m->end(); ++i) {
	if (!params.empty())
	  params += '&';
	params
	  += Utils::urlEncode(i->first) + '=' + Utils::urlEncode(i->second[0]);
      }
    }
    script.setVar("PARAMS", params);

    script.stream(out);
  }

  if (!serveRest) {
    out.spool(response.out());
    return;
  }

  out << app->javaScriptClass() << "._p_.setPage(" << pageId_ << ");";

  formObjectsChanged_ = true;
  app->autoJavaScriptChanged_ = true;

  if (session_.type() == WidgetSet) {
    out << app->javaScriptClass() << "._p_.update(null, 'load', null, false);";
  } else if (!rendered_) {
    serveMainAjax(out);
  } else {
    bool enabledAjax = app->enableAjax_;

    if (app->enableAjax_) {
      // Before-load JavaScript of libraries that were loaded directly
      // in HTML
      collectedJS1_ << "var form = " WT_CLASS ".getElement('Wt-form'); "
	"if (form) {" << beforeLoadJS_.str();

      beforeLoadJS_.clear();

      collectedJS1_
	<< "var domRoot=" << app->domRoot_->jsRef() << ';'
	<< WT_CLASS ".progressed(domRoot);";

      // Load JavaScript libraries that were added during enableAjax()
      int librariesLoaded = loadScriptLibraries(collectedJS1_, app);

      app->streamBeforeLoadJavaScript(collectedJS1_, false);

      collectedJS2_
	<< WT_CLASS ".resolveRelativeAnchors();"
	<< "domRoot.style.visibility = 'visible';"
	<< app->javaScriptClass() << "._p_.doAutoJavaScript();";

      loadScriptLibraries(collectedJS2_, app, librariesLoaded);

      collectedJS2_ << '}';

      app->enableAjax_ = false;
    } else
      app->streamBeforeLoadJavaScript(out, true);

    out << "window." << app->javaScriptClass()
	<< "LoadWidgetTree = function(){\n";

    if (app->internalPathsEnabled_)
      out << app->javaScriptClass() << "._p_.enableInternalPaths("
	  << WWebWidget::jsStringLiteral(app->renderedInternalPath_)
	  << ");\n";

    visibleOnly_ = false;

    formObjectsChanged_ = true;
    currentFormObjectsList_.clear();
    collectJavaScript();
    updateLoadIndicator(collectedJS1_, app, true);

    LOG_DEBUG("js: " << collectedJS1_.str() << collectedJS2_.str());

    out << collectedJS1_.str();

    addResponseAckPuzzle(out);

    out << app->javaScriptClass()
	<< "._p_.setHash("
	<< WWebWidget::jsStringLiteral(app->newInternalPath_)
	<< ", false);\n";

    if (!app->environment().internalPathUsingFragments())
      session_.setPagePathInfo(app->newInternalPath_);

    out << app->javaScriptClass()
	<< "._p_.update(null, 'load', null, false);"
	<< collectedJS2_.str()
	<< "};"; // LoadWidgetTree = function() { ... }

    session_.app()->serverPushChanged_ = true;
    renderSetServerPush(out);

    if (enabledAjax)
      out
	/*
	 * Firefox < 3.5 doesn't have this and in that case it could be
	 * that we are already ready and jqeury doesn't fire the callback.
	 */
	<< "\nif (typeof document.readyState === 'undefined')"
	<< " setTimeout(function() { "
	<<              app->javaScriptClass() << "._p_.load(true);"
	<<   "}, 400);"
	<< "else ";

    out << "$(document).ready(function() { "
	<< app->javaScriptClass() << "._p_.load(true);});\n";
  }

  out.spool(response.out());
}

void WebRenderer::serveMainAjax(WStringStream& out)
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

  app->scriptLibrariesAdded_ = app->scriptLibraries_.size();
  int librariesLoaded = loadScriptLibraries(out, app);

  out << app->javaScriptClass() << "._p_.autoJavaScript=function(){"
      << app->autoJavaScript_ << "};\n";
  app->autoJavaScriptChanged_ = false;

  app->streamBeforeLoadJavaScript(out, true);

  if (!widgetset)
    out << "window." << app->javaScriptClass()
	<< "LoadWidgetTree = function(){\n";

  if (!initialStyleRendered_) {
    /*
     * In case we have not yet served the bootstyle for this page:
     */
    if (app->theme()) {
      std::vector<WCssStyleSheet> styleSheets = app->theme()->styleSheets();
      for (unsigned i = 0; i < styleSheets.size(); ++i)
	loadStyleSheet(out, app, styleSheets[i]);
    }

    app->styleSheetsAdded_ = app->styleSheets_.size();
    loadStyleSheets(out, app);

    initialStyleRendered_ = true;
  }

  /*
   * Need to do this after createSDomElement, since additional CSS/JS
   * may be made during rendering, e.g. from WViewWidget::render()
   */
  if (conf.inlineCss())
    app->styleSheet().javaScriptUpdate(app, out, true);

  if (app->bodyHtmlClassChanged_) {
    std::string op = widgetset ? "+=" : "=";
    out << "document.body.parentNode.className" << op << '\'' << app->htmlClass_ << "';"
	<< "document.body.className" << op << '\'' << bodyClassRtl() << "';"
	<< "document.body.setAttribute('dir', '";
    if (app->layoutDirection() == LeftToRight)
      out << "LTR";
    else
      out << "RTL";
    out << "');";
  }

#ifdef WT_DEBUG_ENABLED
  WStringStream s;
#else
  WStringStream& s = out;
#endif // WT_DEBUG_ENABLED

  mainElement->addToParent(s, "document.body", widgetset ? 0 : -1, app);
  delete mainElement;

  addResponseAckPuzzle(s);

  if (app->hasQuit())
    s << app->javaScriptClass() << "._p_.quit("
      << (app->quittedMessage_.empty() ? "null" :
	  app->quittedMessage_.jsStringLiteral()) + ");";

  if (widgetset)
    app->domRoot2_->rootAsJavaScript(app, s, true);

#ifdef WT_DEBUG_ENABLED
  LOG_DEBUG("js: " << s.str());
  out << s.str();
#endif // WT_DEBUG_ENABLED

  currentFormObjectsList_ = createFormObjectsList(app);
  out << app->javaScriptClass()
      << "._p_.setFormObjects([" << currentFormObjectsList_ << "]);\n";
  formObjectsChanged_ = false;

  setRendered(true);
  setJSSynced(true);

  preLearnStateless(app, collectedJS1_);

  LOG_DEBUG("js: " << collectedJS1_.str());

  out << collectedJS1_.str();
  collectedJS1_.clear();

  updateLoadIndicator(out, app, true);

  if (widgetset) {
    const std::string *historyE = app->environment().getParameter("Wt-history");
    if (historyE) {
      out << WT_CLASS << ".history.initialize('"
	  << (*historyE)[0] << "-field', '"
	  << (*historyE)[0] << "-iframe', '');\n";
    }
  }

  app->streamAfterLoadJavaScript(out);
  out << "{var o=null,e=null;"
      << app->hideLoadingIndicator_.javaScript()
      << '}';

  if (!widgetset) {
    if (!app->isQuited())
      out << session_.app()->javaScriptClass()
	  << "._p_.update(null, 'load', null, false);\n";
    out << "};\n";
  }

  renderSetServerPush(out);

  out << "$(document).ready(function() { "
      << app->javaScriptClass() << "._p_.load(" << !widgetset << ");});\n";

  loadScriptLibraries(out, app, librariesLoaded);
}

void WebRenderer::setJSSynced(bool invisibleToo)
{
  LOG_DEBUG("setJSSynced: " << invisibleToo);

  collectedJS1_.clear();
  collectedJS2_.clear();

  if (!invisibleToo)
    collectedJS1_ << invisibleJS_.str();

  invisibleJS_.clear();
}

std::string WebRenderer::safeJsStringLiteral(const std::string& value)
{
  std::string s = WWebWidget::jsStringLiteral(value);
  return Wt::Utils::replace(s, "<", "<'+'");
}

void WebRenderer::updateLoadIndicator(WStringStream& out, WApplication *app,
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

void WebRenderer::renderStyleSheet(WStringStream& out,
				   const WCssStyleSheet& sheet,
				   WApplication *app)
{
  out << "<link href=\"";
  DomElement::htmlAttributeValue(out, sheet.link().resolveUrl(app));
  out << "\" rel=\"stylesheet\" type=\"text/css\"";

  if (!sheet.media().empty() && sheet.media() != "all")
    out << " media=\"" << sheet.media() << '"';
  
  closeSpecial(out);
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
  ++expectedAckId_;
  session_.sessionIdChanged_ = false;

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
		&& app->renderedInternalPath_ != app->newInternalPath_))) {
    app->renderedInternalPath_ = app->newInternalPath_;

    if (session_.state() == WebSession::JustCreated &&
	conf.progressiveBoot(app->environment().internalPath())) {
      session_.redirect
	(session_.fixRelativeUrl
	 (session_.bookmarkUrl(app->newInternalPath_)));
      session_.kill();
    } else {
      session_.redirect
	(session_.fixRelativeUrl
	 (session_.mostRelativeUrl(app->newInternalPath_)));
    }
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
  setRendered(true);
  setJSSynced(true);

  WStringStream styleSheets;

  if (app->theme()) {
    std::vector<WCssStyleSheet> sheets = app->theme()->styleSheets();

    for (unsigned i = 0; i < sheets.size(); ++i)
      renderStyleSheet(styleSheets, sheets[i], app);
  }

  for (unsigned i = 0; i < app->styleSheets_.size(); ++i)
    renderStyleSheet(styleSheets, app->styleSheets_[i], app);

  app->styleSheetsAdded_ = 0;
  initialStyleRendered_ = true;

  beforeLoadJS_.clear();
  for (unsigned i = 0; i < app->scriptLibraries_.size(); ++i) {
    std::string url = app->scriptLibraries_[i].uri;
    styleSheets << "<script src=";
    DomElement::htmlAttributeValue(styleSheets, session_.fixRelativeUrl(url));
    styleSheets << "></script>\n";

    beforeLoadJS_ << app->scriptLibraries_[i].beforeLoadJS;
  }
  app->scriptLibrariesAdded_ = 0;

  app->newBeforeLoadJavaScript_ = app->beforeLoadJavaScript_.length();

  bool hybridPage = session_.progressiveBoot() || session_.env().ajax();
  FileServe page(hybridPage ? skeletons::Hybrid_html1 : skeletons::Plain_html1);

  setPageVars(page);
  page.setVar("SESSION_ID", session_.sessionId());

  std::string url
    = (app->environment().agentIsSpiderBot() || !session_.useUrlRewriting())
    ? session_.bookmarkUrl(app->newInternalPath_)
    : session_.mostRelativeUrl(app->newInternalPath_);

  url = session_.fixRelativeUrl(url);

  url = Wt::Utils::replace(url, '&', "&amp;");
  page.setVar("RELATIVE_URL", url);

  if (conf.inlineCss()) {
    WStringStream css;
    app->styleSheet().cssText(css, true);
    page.setVar("STYLESHEET", css.str());
  } else
    page.setVar("STYLESHEET", "");

  page.setVar("STYLESHEETS", styleSheets.str());

  page.setVar("TITLE", WWebWidget::escapeText(app->title()).toUTF8());

  app->titleChanged_ = false;

  std::string contentType = "text/html; charset=UTF-8";

  setCaching(response, false);
  response.addHeader("X-Frame-Options", "SAMEORIGIN");
  setHeaders(response, contentType);

  currentFormObjectsList_ = createFormObjectsList(app);

  if (hybridPage)
    streamBootContent(response, page, true);

  WStringStream out(response.out());
  page.streamUntil(out, "HTML");

  DomElement::TimeoutList timeouts;
  {
    EscapeOStream js;
    EscapeOStream eout(out);
    mainElement->asHTML(eout, js, timeouts);

    /*
     * invisibleJS_ is being streamed as the first JavaScript inside
     * collectJavaScript(). This is where this belongs since between
     * the HTML and the script there may already be changes (because
     * of server push) that delete elements that were rendered.
     */
    invisibleJS_ << js.str();
    delete mainElement;

    app->domRoot_->doneRerender();
  }

  int refresh;
  if (app->environment().ajax()) {
    WStringStream str;
    DomElement::createTimeoutJs(str, timeouts, app);
    app->doJavaScript(str.str());

    refresh = 1000000;
  } else {
    if (app->isQuited() || conf.sessionTimeout() == -1)
      refresh = 1000000;
    else {
      refresh = conf.sessionTimeout() / 3;
      for (unsigned i = 0; i < timeouts.size(); ++i)
	refresh = std::min(refresh, 1 + timeouts[i].msec/1000);
    }
  }
  page.setVar("REFRESH", boost::lexical_cast<std::string>(refresh));

  page.stream(out);

  app->internalPathIsChanged_ = false;

  out.spool(response.out());
}

int WebRenderer::loadScriptLibraries(WStringStream& out,
				     WApplication *app, int count)
{
  if (count == -1) {
    int first = app->scriptLibraries_.size() - app->scriptLibrariesAdded_;

    for (unsigned i = first; i < app->scriptLibraries_.size(); ++i) {
      std::string uri = session_.fixRelativeUrl(app->scriptLibraries_[i].uri);

      out << app->scriptLibraries_[i].beforeLoadJS
	  << app->javaScriptClass() << "._p_.loadScript('" << uri << "',";
      DomElement::jsStringLiteral(out, app->scriptLibraries_[i].symbol, '\'');
      out << ");\n";
      out << app->javaScriptClass() << "._p_.onJsLoad(\""
	  << uri << "\",function() {\n";
    }

    count = app->scriptLibrariesAdded_;
    app->scriptLibrariesAdded_ = 0;

    return count;
  } else {
    if (count) {
      out << app->javaScriptClass() << "._p_.doAutoJavaScript();";
      for (int i = 0; i < count; ++i)
	out << "});";
    }

    return 0;
  }
}

void WebRenderer::loadStyleSheet(WStringStream& out, WApplication *app,
				 const WCssStyleSheet& sheet)
{
  out << WT_CLASS << ".addStyleSheet('"
      << sheet.link().resolveUrl(app) << "', '"
      << sheet.media() << "');\n ";
}

void WebRenderer::removeStyleSheets(WStringStream& out, WApplication *app)
{
  for (int i = (int)app->styleSheetsToRemove_.size() - 1; i > -1; --i){
    out << WT_CLASS << ".removeStyleSheet('"
        << app->styleSheetsToRemove_[i].link().resolveUrl(app) << "');\n ";
    app->styleSheetsToRemove_.erase(app->styleSheetsToRemove_.begin() + i);
  }
}

void WebRenderer::loadStyleSheets(WStringStream& out, WApplication *app)
{
  int first = app->styleSheets_.size() - app->styleSheetsAdded_;

  for (unsigned i = first; i < app->styleSheets_.size(); ++i)
    loadStyleSheet(out, app, app->styleSheets_[i]);

  removeStyleSheets(out, app);

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
	LOG_DEBUG("ignoring: " << ww->id() << " (" << DESCRIBE(ww) << ") " <<
		  w->id() << " (" << DESCRIBE(w) << ")");

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

	LOG_DEBUG("updating: " << w->id() << " (" << DESCRIBE(w) << ")");

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
	  } else {
	    LOG_DEBUG("Ignoring: " << w->id());
	  }
	} else {
	  w->getSDomChanges(changes, app);
	}
      }
    }
  } while (!learning_ && moreUpdates_);
}

void WebRenderer::collectJavaScriptUpdate(WStringStream& out)
{
  WApplication *app = session_.app();

  out << '{';

  if (session_.sessionIdChanged_) {
    if (session_.hasSessionIdInUrl()) {
      if (app->environment().ajax() &&
	  !app->environment().internalPathUsingFragments()) {
	streamRedirectJS(out, app->url(app->internalPath()));
	// better would be to use HTML5 history in this case but that would
	// need some minor JavaScript reorganizations
      } else {
	streamRedirectJS(out, app->url(app->internalPath()));
      }
      out << '}';
      return;
    }

    out << session_.app()->javaScriptClass()
	<< "._p_.setSessionUrl("
	<< WWebWidget::jsStringLiteral(sessionUrl())
	<< ");";
    session_.sessionIdChanged_ = false;
  }

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

  app->streamAfterLoadJavaScript(out);

  if (app->isQuited())
    out << app->javaScriptClass() << "._p_.quit("
	<< (app->quittedMessage_.empty() ? "null" :
	    app->quittedMessage_.jsStringLiteral()) + ");";

  if (updateLayout_) {
    out << "window.onresize();";
    updateLayout_ = false;
  }

  app->renderedInternalPath_ = app->newInternalPath_;

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

void WebRenderer::collectJS(WStringStream* js)
{
  std::vector<DomElement *> changes;

  collectChanges(changes);

  WApplication *app = session_.app();

  if (js) {
    if (!preLearning())
      app->streamBeforeLoadJavaScript(*js, false);

    Configuration& conf = session_.controller()->configuration();
    if (conf.inlineCss())
      app->styleSheet().javaScriptUpdate(app, *js, false);

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
    
	if (app->localeChanged_) {
      *js << app->javaScriptClass()
	  << "._p_.setLocale(" << WString(app->locale().name()).jsStringLiteral()
	  << ");\n";
    }
  }

  app->titleChanged_ = false;
  app->closeMessageChanged_ = false;
  app->localeChanged_ = false;

  if (js) {
    int librariesLoaded = loadScriptLibraries(*js, app);

    app->streamAfterLoadJavaScript(*js);

    if (app->internalPathIsChanged_) {
      *js << app->javaScriptClass()
	  << "._p_.setHash("
	  << WWebWidget::jsStringLiteral(app->newInternalPath_)
	  << ", false);\n";
      if (!preLearning() && !app->environment().internalPathUsingFragments())
	session_.setPagePathInfo(app->newInternalPath_);
    }

    loadScriptLibraries(*js, app, librariesLoaded);
  } else
    app->afterLoadJavaScript_.clear();

  app->internalPathIsChanged_ = false;
  app->renderedInternalPath_ = app->newInternalPath_;
}

void WebRenderer::preLearnStateless(WApplication *app, WStringStream& out)
{
  if (!session_.env().ajax())
    return;

  collectJS(&out);

  // TODO optimize this so that only signals which require an update
  //      are processed instead of looping through all signals.

  WApplication::SignalMap& ss = session_.app()->exposedSignals();

  for (WApplication::SignalMap::iterator i = ss.begin();
       i != ss.end(); ) {
    Wt::EventSignalBase* s = i->second;

    if (s->sender() == app)
      s->processPreLearnStateless(this);
    else if (s->canAutoLearn()) {
      WWidget *ww = static_cast<WWidget *>(s->sender());
      if (ww && ww->isRendered())
	s->processPreLearnStateless(this);
    }

    ++i;
  }

  out << statelessJS_.str();
  statelessJS_.clear();
}

std::string WebRenderer::learn(WStatelessSlot* slot)
{
  if (slot->invalidated())
    return std::string();

  if (slot->type() == WStatelessSlot::PreLearnStateless)
    learning_ = true;

  learningIncomplete_ = false;

  currentStatelessSlotIsActuallyStateless_ = true;

  slot->trigger();

  WStringStream js;

  collectJS(&js);

  std::string result = js.str();

  LOG_DEBUG("learned: " << result);

  if (slot->type() == WStatelessSlot::PreLearnStateless) {
    slot->undoTrigger();
    collectJS(0);

    learning_ = false;
  } else { // AutoLearnStateless
    statelessJS_ << result;
  }

  if (currentStatelessSlotIsActuallyStateless_ && !learningIncomplete_) {
    slot->setJavaScript(result);
  } else if (!currentStatelessSlotIsActuallyStateless_) {
    slot->invalidate();
  }

  collectJS(&statelessJS_);

  return result;
}

void WebRenderer::learningIncomplete()
{
  learningIncomplete_ = true;
}

std::string WebRenderer::headDeclarations() const
{
  EscapeOStream result;
 
  const Configuration& conf = session_.env().server()->configuration();

  const std::vector<MetaHeader>& confMetaHeaders = conf.metaHeaders();
  std::vector<MetaHeader> metaHeaders;

  for (unsigned i = 0; i < confMetaHeaders.size(); ++i) {
    const MetaHeader& m = confMetaHeaders[i];

    bool add = true;
    if (!m.userAgent.empty()) {
      WT_USTRING s = WT_USTRING::fromUTF8(session_.env().userAgent());
      WRegExp expr(WT_USTRING::fromUTF8(m.userAgent));
      if (!expr.exactMatch(s))
	add = false;
    }

    if (add)
      metaHeaders.push_back(confMetaHeaders[i]);
  }

  if (session_.app()) {
    const std::vector<MetaHeader>& appMetaHeaders
      = session_.app()->metaHeaders_;

    for (unsigned i = 0; i < appMetaHeaders.size(); ++i) {
      const MetaHeader& m = appMetaHeaders[i];

      bool add = true;
      for (unsigned j = 0; j < metaHeaders.size(); ++j) {
	MetaHeader& m2 = metaHeaders[j];

	if (m.type == m2.type && m.name == m2.name) {
	  m2.content = m.content;
	  add = false;
	  break;
	}
      }

      if (add)
	metaHeaders.push_back(m);
    }
  }

  for (unsigned i = 0; i < metaHeaders.size(); ++i) {
    const MetaHeader& m = metaHeaders[i];

    result << "<meta";

    if (!m.name.empty()) {
      std::string attribute;
      switch (m.type) {
      case MetaName: attribute = "name"; break;
      case MetaProperty: attribute = "property"; break;
      case MetaHttpHeader: attribute = "http-equiv"; break;
      }

      appendAttribute(result, attribute, m.name);
    }

    if (!m.lang.empty())
      appendAttribute(result, "lang", m.lang);

    appendAttribute(result, "content", m.content.toUTF8());

    closeSpecial(result);
  }

  if (session_.app()) {
    for (unsigned i = 0; i < session_.app()->metaLinks_.size(); ++i) {
      const WApplication::MetaLink& ml = session_.app()->metaLinks_[i];

      result << "<link";

      appendAttribute(result, "href", ml.href); 
      appendAttribute(result, "rel", ml.rel);
      if (!ml.media.empty())
	appendAttribute(result, "media", ml.media);
      if (!ml.hreflang.empty())
	appendAttribute(result, "hreflang", ml.hreflang);
      if (!ml.type.empty())
	appendAttribute(result, "type", ml.type);
      if (!ml.sizes.empty())
	appendAttribute(result, "sizes", ml.sizes);
      if (ml.disabled)
	appendAttribute(result, "disabled", "");

      closeSpecial(result);
    }
  } else
    if (session_.env().agentIsIE()) {
      /*
       * WARNING: Similar code in WApplication.C must be kept in sync for 
       *          progressive boot.
       */
      if (session_.env().agent() < WEnvironment::IE9) {
	bool selectIE7 = conf.uaCompatible().find("IE8=IE7")
	  != std::string::npos;

	if (selectIE7) {
	  result << "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=7\"";
	  closeSpecial(result);
	}
      } else if (session_.env().agent() == WEnvironment::IE9) {
	result << "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=9\"";
	closeSpecial(result);
      } else if (session_.env().agent() == WEnvironment::IE10) {
	result << "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=10\"";
	closeSpecial(result);
      } else {
	result << "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=11\"";
	closeSpecial(result);
      }
    }

  if (!session_.favicon().empty()) {
    result <<
      "<link rel=\"shortcut icon\" href=\"" << session_.favicon() << '"';
    closeSpecial(result);
  }

  std::string baseUrl;
  WApplication::readConfigurationProperty("baseURL", baseUrl);

  if (!baseUrl.empty()) {
    result << "<base href=\"" << baseUrl << '"';
    closeSpecial(result);
  }

  return result.str();
}

}
