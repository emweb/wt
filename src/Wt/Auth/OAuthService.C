/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/OAuthService.h"

#include "Wt/Json/Parser.h"
#include "Wt/Json/Object.h"

#include "Wt/Http/Client.h"
#include "Wt/Http/Request.h"
#include "Wt/Http/Response.h"
#include "Wt/Http/HttpUtils.h"

#include "Wt/Utils.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WLogger.h"
#include "Wt/WRandom.h"
#include "Wt/WResource.h"
#include "Wt/WStringStream.h"
#include "Wt/WServer.h"

#include "Wt/Auth/AuthUtils.h"
#include "Wt/PopupWindow.h"

#include "WebUtils.h"
#include "WebSession.h"
#include "WebRequest.h"

#ifdef WT_THREADED
#include <mutex>
#endif // WT_THREADED

#include <boost/algorithm/string.hpp>

#define ERROR_MSG(e) WString::tr("Wt.Auth.OAuthService." e)

namespace Wt {

LOGGER("Auth.OAuthService");

  namespace Auth {

class OAuthRedirectEndpoint final : public WResource
{
public:
  OAuthRedirectEndpoint(OAuthProcess *process)
    : process_(process)
  { }

  virtual ~OAuthRedirectEndpoint()
  {
    beingDeleted();
  }

  void sendError(Http::Response& response)
  {
    response.setStatus(500);

#ifndef WT_TARGET_JAVA
    std::ostream& o = response.out();
#else
    std::ostream o(response.out());
#endif // WT_TARGET_JAVA

    o << "<html><body>OAuth error</body></html>";
  }

  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) override
  {
#ifndef WT_TARGET_JAVA
    if (!request.continuation()) {
#endif
      response.setMimeType("text/html; charset=UTF-8");

      const std::string *stateE = request.getParameter("state");
      if (!stateE || *stateE != process_->oAuthState_) {
	LOG_ERROR(ERROR_MSG("invalid-state") << 
		  ", state: " << (stateE ? *stateE : "(empty)"));
	process_->setError(ERROR_MSG("invalid-state"));
	sendError(response);
	return;
      }

      const std::string *errorE = request.getParameter("error");
      if (errorE) {
	LOG_ERROR(ERROR_MSG(+ *errorE));
	process_->setError(ERROR_MSG(+ *errorE));
	sendError(response);
	return;
      }

      const std::string *codeE = request.getParameter("code");
      if (!codeE) {
	LOG_ERROR(ERROR_MSG("missing-code"));
	process_->setError(ERROR_MSG("missing-code"));
	sendError(response);
	return;
      }

#ifndef WT_TARGET_JAVA
      Http::ResponseContinuation *cont = response.createContinuation();
      cont->waitForMoreData();
#endif

      process_->requestToken(*codeE); // Blocking in JWt, so no continuation necessary
#ifndef WT_TARGET_JAVA
    } else
#endif
      sendResponse(response);
  }

  void sendResponse(Http::Response& response)
  {
#ifndef WT_TARGET_JAVA
    std::ostream& o = response.out();
#else
    std::ostream o(response.out());
#endif // WT_TARGET_JAVA

    WApplication *app = WApplication::instance();
    if (app->environment().ajax()) {
      std::string appJs = app->javaScriptClass();
      o <<
        "<!DOCTYPE html>"
        "<html lang=\"en\" dir=\"ltr\">\n"
        "<head><title></title>\n"
        "<script type=\"text/javascript\">\n"
        "function load() { "
        """if (window.opener." << appJs << ") {"
        ""  "var " << appJs << "= window.opener." << appJs << ";"
#ifndef WT_TARGET_JAVA
        <<  process_->redirected_.createCall({}) << ";"
#else // WT_TARGET_JAVA
        <<  process_->redirected_.createCall() << ";"
#endif // WT_TARGET_JAVA
        ""  "window.close();"
        "}\n"
        "}\n"
        "</script></head>"
        "<body onload=\"load();\"></body></html>";
    } else {
      // FIXME: it would be way cleaner if we can send a 302 response, but at
      //        the moment there's no way to stall sending of status code and headers
      //        when using continuations
      std::string redirectTo = app->makeAbsoluteUrl(app->url(process_->startInternalPath_));
      o <<
	"<!DOCTYPE html>"
	"<html lang=\"en\" dir=\"ltr\">\n"
	"<head><meta http-equiv=\"refresh\" content=\"0; url="
	<< redirectTo << "\" /></head>\n"
	"<body><p><a href=\"" << redirectTo
	<< "\"> Click here to continue</a></p></body></html>";
    }
  }

private:
  OAuthProcess *process_;
};

OAuthAccessToken::OAuthAccessToken()
{ }

OAuthAccessToken::OAuthAccessToken(const std::string& accessToken, 
				   const WDateTime& expires,
				   const std::string& refreshToken)
  : accessToken_(accessToken),
    refreshToken_(refreshToken),
    expires_(expires)
{ }

OAuthAccessToken::OAuthAccessToken(const std::string& accessToken,
				   const WDateTime& expires,
				   const std::string& refreshToken,
                                   const std::string& idToken)
  : accessToken_(accessToken),
    refreshToken_(refreshToken),
    idToken_(idToken),
    expires_(expires)
{ }

const OAuthAccessToken OAuthAccessToken::Invalid;

OAuthProcess::OAuthProcess(const OAuthService& service,
			   const std::string& scope)
  : service_(service),
    scope_(scope),
    authenticate_(false),
    redirected_(this, "redirected")
{
  redirectEndpoint_.reset(new OAuthRedirectEndpoint(this));
  WApplication *app = WApplication::instance();

  PopupWindow::loadJavaScript(app);

  std::string url = app->makeAbsoluteUrl(redirectEndpoint_->url());
  oAuthState_ = service_.encodeState(url);

  redirected_.connect(this, &OAuthProcess::onOAuthDone);

#ifndef WT_TARGET_JAVA
  WStringStream js;
  js << WT_CLASS ".PopupWindow(" WT_CLASS
     << "," << WWebWidget::jsStringLiteral(authorizeUrl()) 
     << ", " << service.popupWidth()
     << ", " << service.popupHeight() << ");"; 

  implementJavaScript(&OAuthProcess::startAuthorize, js.str());
  implementJavaScript(&OAuthProcess::startAuthenticate, js.str());
#endif

#ifndef WT_TARGET_JAVA
  if (!app->environment().javaScript())
    authenticated().connect(this, &OAuthProcess::handleAuthComplete);
#endif // WT_TARGET_JAVA
}

OAuthProcess::~OAuthProcess()
{ }

std::string OAuthProcess::authorizeUrl() const
{
  WStringStream url;
  url << service_.authorizationEndpoint();
  bool hasQuery = url.str().find('?') != std::string::npos;

  url << (hasQuery ? '&' : '?')
      << "client_id=" << Wt::Utils::urlEncode(service_.clientId())
      << "&redirect_uri="
      << Wt::Utils::urlEncode(service_.generateRedirectEndpoint())
      << "&scope=" << Wt::Utils::urlEncode(scope_)
      << "&response_type=code"
      << "&state=" << Wt::Utils::urlEncode(oAuthState_);

  LOG_INFO("authorize URL: " << url.str());

  return url.str();
}

void OAuthProcess::startAuthorize()
{
  WApplication *app = WApplication::instance();
  if (!app->environment().javaScript()) {
    startInternalPath_ = app->internalPath();
    app->redirect(authorizeUrl());
  } else {
    redirectEndpoint_->url(); // Make sure it is exposed
  }
}

void OAuthProcess::startAuthenticate()
{
  authenticate_ = true;
  startAuthorize();
}

#ifdef WT_TARGET_JAVA
void OAuthProcess::connectStartAuthenticate(EventSignalBase &s)
{
  if (WApplication::instance()->environment().javaScript()) {
    WStringStream js;
    js << "function(object, event) {"
       << WT_CLASS ".PopupWindow(" WT_CLASS
       << "," << WWebWidget::jsStringLiteral(authorizeUrl()) 
       << ", " << service_.popupWidth()
       << ", " << service_.popupHeight() << ");"
       << "}";

    s.connect(js.str());
  }

  s.connect(this, &OAuthProcess::startAuthenticate);
}
#endif

void OAuthProcess::getIdentity(const OAuthAccessToken& token)
{
  throw WException("OAuth::Process::Identity(): not specialized");
}

void OAuthProcess::setError(const WString& error)
{
  error_ = error;
}

void OAuthProcess::onOAuthDone()
{
  bool success = error_.empty();

  authorized().emit(success ? token_ : OAuthAccessToken::Invalid);

  if (success && authenticate_) {
    authenticate_ = false;
    getIdentity(token_);
  }
#ifndef WT_TARGET_JAVA
  else if (!WApplication::instance()->environment().javaScript())
    redirectEndpoint_->haveMoreData();
#endif // WT_TARGET_JAVA
}

#ifndef WT_TARGET_JAVA
void OAuthProcess::handleAuthComplete()
{
  redirectEndpoint_->haveMoreData();
}
#endif // WT_TARGET_JAVA

void OAuthProcess::requestToken(const std::string& authorizationCode)
{
  /*
   * OAuth 2.0 draft says this should be a POST using
   * application/x-www-form-urlencoded but that's not what Facebook
   * does
   */
  std::string url = service_.tokenEndpoint();
  Http::Method m = service_.tokenRequestMethod();

  WStringStream ss;
  ss << "grant_type=authorization_code"
     << "&redirect_uri=" 
     << Wt::Utils::urlEncode(service_.generateRedirectEndpoint())
     << "&code=" << authorizationCode;

  httpClient_.reset(new Http::Client());
  httpClient_->setTimeout(std::chrono::seconds(15));
  httpClient_->done().connect
    (this, std::bind(&OAuthProcess::handleToken, this,
		     std::placeholders::_1, std::placeholders::_2));

  std::string clientId = Wt::Utils::urlEncode(service_.clientId());
  std::string clientSecret = Wt::Utils::urlEncode(service_.clientSecret());

  if (m == Http::Method::Get) {
    std::vector<Http::Message::Header> headers;
    if (service_.clientSecretMethod() == HttpAuthorizationBasic) {
      headers.push_back(Http::Message::Header("Authorization",
        "Basic " + Wt::Utils::base64Encode(
	  clientId + ":" + clientSecret, false)));
    } else if (service_.clientSecretMethod() == PlainUrlParameter) {
      ss << "&client_id=" << clientId
	<< "&client_secret=" << clientSecret;
    }

    bool hasQuery = url.find('?') != std::string::npos;
    url += (hasQuery ? '&' : '?') + ss.str();

    httpClient_->get(url, headers);
  } else {
    Http::Message post;
    post.setHeader("Content-Type", "application/x-www-form-urlencoded");
    if (service_.clientSecretMethod() == HttpAuthorizationBasic) {
      post.setHeader("Authorization",
		     "Basic " + Wt::Utils::base64Encode(clientId + ":" + clientSecret,
							false));
    } else if (service_.clientSecretMethod() == RequestBodyParameter) {
      ss << "&client_id=" << clientId
	<< "&client_secret=" << clientSecret;
    }
    post.addBodyText(ss.str());
    httpClient_->post(url, post);
  }
}

void OAuthProcess::handleToken(AsioWrapper::error_code err,
			       const Http::Message& response)
{
  if (!err)
    doParseTokenResponse(response);
  else {
    LOG_ERROR("handleToken(): " << err.message());
    setError(WString::fromUTF8(err.message()));
  }

  WApplication *app = WApplication::instance();

  if (app->environment().ajax()) {
#ifndef WT_TARGET_JAVA
    redirectEndpoint_->haveMoreData();
#endif
  } else {
    onOAuthDone();
  }
}

void OAuthProcess::doParseTokenResponse(const Http::Message& response)
{
  try {
    token_ = parseTokenResponse(response);
  } catch (TokenError& e) {
    error_ = e.error();
  }
}

OAuthAccessToken OAuthProcess::parseTokenResponse(const Http::Message& response)
{
  if (response.status() == 200 || response.status() == 400) {
    /*
     * OAuth 2.0 states this should be application/json
     * but Facebook uses text/plain; charset=UTF-8 body
     */
    const std::string *contenttype = response.getHeader("Content-Type");

    if (contenttype) {
      std::string mimetype = boost::trim_copy(*contenttype);
      std::vector<std::string> tokens;
      boost::split(tokens, mimetype, boost::is_any_of(";"));
      std::string combinedType; // type/subtype
      std::string params;
      if (tokens.size() > 0) {
	combinedType = tokens[0];
	boost::trim(combinedType);
      }
      if (tokens.size() > 1) {
	params = tokens[1];
	boost::trim(params);
      }
      if (combinedType == "text/plain") {
	if (boost::starts_with(params, "charset=UTF-8"))
	  return parseUrlEncodedToken(response);
	else
	  throw TokenError(ERROR_MSG("badresponse"));
      } else if (combinedType == "application/json")
	return parseJsonToken(response);
      else
	throw TokenError(ERROR_MSG("badresponse"));
    } else
      throw TokenError(ERROR_MSG("badresponse"));
  } else
    throw TokenError(ERROR_MSG("badresponse"));
}

OAuthAccessToken OAuthProcess::parseUrlEncodedToken(const Http::Message& response)
{
  /* Facebook style */
  Http::ParameterMap params;
  Http::Utils::parseFormUrlEncoded(response, params);

  if (response.status() == 200) {
    const std::string *accessTokenE 
      = Http::Utils::getParamValue(params, "access_token");
    if (accessTokenE) {
      std::string accessToken = *accessTokenE;

      WDateTime expires;
      const std::string *expiresE 
	= Http::Utils::getParamValue(params, "expires");
      if (expiresE)
	expires = WDateTime::currentDateTime().addSecs
	  (Wt::Utils::stoi(*expiresE));

      // FIXME refresh token
      
      return OAuthAccessToken(accessToken, expires, std::string());
    } else
      throw TokenError(ERROR_MSG("badresponse"));
  } else {
    const std::string *errorE = Http::Utils::getParamValue(params, "error");
    
    if (errorE)
      throw TokenError(ERROR_MSG(+ *errorE));
    else
      throw TokenError(ERROR_MSG("badresponse"));
  }
}

OAuthAccessToken OAuthProcess::parseJsonToken(const Http::Message& response)
{
  /* OAuth 2.0 style */
  Json::Object root;
  Json::ParseError pe;

#ifndef WT_TARGET_JAVA
  bool ok = Json::parse(response.body(), root, pe);
#else
  try {
    root = (Json::Object)Json::Parser().parse(response.body());
  } catch (Json::ParseError error) {
    pe = error;
  }
  bool ok = root.isNull();
#endif

  if (!ok) {
    LOG_ERROR("parseJsonToken(): " << pe.what());
    throw TokenError(ERROR_MSG("badjson"));
  } else {
    if (response.status() == 200) {
      try {
	std::string accessToken = root.get("access_token");
	int secs = root.get("expires_in").orIfNull(-1);
	WDateTime expires;
	if (secs > 0)
	  expires = WDateTime::currentDateTime().addSecs(secs);

        std::string refreshToken = root.get("refresh_token").orIfNull("");
        std::string idToken = root.get("id_token").orIfNull("");

	return OAuthAccessToken(accessToken, expires, refreshToken, idToken);
      } catch (std::exception& e) {
	LOG_ERROR("token response error: " << e.what());
	throw TokenError(ERROR_MSG("badresponse"));
      }
    } else {
      throw TokenError
	(ERROR_MSG(+ (root.get("error").orIfNull("missing error"))));
    }
  }
}

struct OAuthService::Impl
{ 
  Impl()
    : redirectResource_(nullptr)
  {
    try {
      secret_ = configurationProperty("oauth2-secret");
    } catch (std::exception& e) {    
      secret_ = WRandom::generateId(32);
    }
  }

#ifdef WT_THREADED
  std::mutex mutex_;
#endif // WT_THREADED

  class RedirectEndpoint final : public WResource
  {
  public:
    RedirectEndpoint(const OAuthService& service)
      : service_(service)
    { }

    virtual ~RedirectEndpoint()
    {
      beingDeleted();
    }

    virtual void handleRequest(const Http::Request& request,
			       Http::Response& response) override
    {
      const std::string *stateE = request.getParameter("state");

      if (stateE) {
	std::string redirectUrl = service_.decodeState(*stateE);

	if (!redirectUrl.empty()) {
	  bool hasQuery = redirectUrl.find('?') != std::string::npos;
	  redirectUrl += (hasQuery ? '&' : '?');
	  redirectUrl += "state=" + Wt::Utils::urlEncode(*stateE);

	  const std::string *errorE = request.getParameter("error");
	  if (errorE)
	    redirectUrl += "&error=" + Wt::Utils::urlEncode(*errorE);

	  const std::string *codeE = request.getParameter("code");
	  if (codeE)
	    redirectUrl += "&code=" + Wt::Utils::urlEncode(*codeE);

	  response.setStatus(302);
	  response.addHeader("Location", redirectUrl);
	  return;
	} else
	  LOG_ERROR("RedirectEndpoint: could not decode state " << *stateE);
      } else
	LOG_ERROR("RedirectEndpoint: missing state");

      response.setStatus(400);
      response.setMimeType("text/html");
      response.out() << "<html><body>"
		     << "<h1>OAuth Authentication error</h1>"
		     << "</body></html>";
    }

  private:
    const OAuthService& service_;
  };

  std::unique_ptr<RedirectEndpoint> redirectResource_;
  std::string secret_;
};

OAuthService::OAuthService(const AuthService& auth)
  : baseAuth_(auth),
    impl_(cpp14::make_unique<Impl>())
{ }

OAuthService::~OAuthService()
{ }

std::string OAuthService::redirectInternalPath() const
{
  return "/auth/oauth/" + name() + "/redirect";
}

std::string OAuthService::generateRedirectEndpoint() const
{
  configureRedirectEndpoint();

  std::string result = redirectEndpoint();
  return result;
}

std::string OAuthService::encodeState(const std::string& url) const
{
  std::string hash(Wt::Utils::base64Encode(Wt::Utils::hmac_sha1(url, impl_->secret_)));

  std::string b = Wt::Utils::base64Encode(hash + "|" + url, false);

  /* Variant of base64 encoding which is resistant to broken OAuth2 peers
   * that do not properly re-encode the state */
  b = Wt::Utils::replace(b, "+", "-");
  b = Wt::Utils::replace(b, "/", "_");  
  b = Wt::Utils::replace(b, "=", ".");

  return b;
}

std::string OAuthService::decodeState(const std::string& state) const
{
  std::string s = state;
  s = Wt::Utils::replace(s, "-", "+");
  s = Wt::Utils::replace(s, "_", "/");
  s = Wt::Utils::replace(s, ".", "=");

#ifndef WT_TARGET_JAVA
  s = Wt::Utils::base64Decode(s);
#else
  s = Wt::Utils::base64DecodeS(s);
#endif

  std::size_t i = s.find('|');
  if (i != std::string::npos) {
    std::string url = s.substr(i + 1);

    std::string check = encodeState(url);
    if (check == state)
      return url;
    else
      return std::string();
  } else
    return std::string();
}

std::string OAuthService::redirectEndpointPath() const
{
  /* Compute deployment path for static resource */
  Http::Client::URL parsedUrl;
  Http::Client::parseUrl(redirectEndpoint(), parsedUrl);

  std::string path = parsedUrl.path;

#ifndef WT_TARGET_JAVA
  /* Compute absolute URL for dynamic resource */
  WApplication *app = WApplication::instance();

  if (app) {
    // Attempt to equalize the path with our deployment configuration,
    // in case we are deployed using a reverse proxy
    std::string publicDeployPath = app->environment().deploymentPath();
    std::string deployPath = app->session()->deploymentPath();

    if (deployPath != publicDeployPath) {
      int diff = (int)publicDeployPath.length() - deployPath.length();
      if (diff > 0) {
	std::string prefix = publicDeployPath.substr(0, diff);
	if (boost::starts_with(path, prefix))
	  path = path.substr(prefix.length());
      }
    }
  }
#endif

  return path; 
}

void OAuthService::configureRedirectEndpoint() const
{
  if (!impl_->redirectResource_) {
#ifdef WT_THREADED
    std::unique_lock<std::mutex> guard(impl_->mutex_);
#endif
    if (!impl_->redirectResource_) {
      auto r = std::unique_ptr<Impl::RedirectEndpoint>(new Impl::RedirectEndpoint(*this));
      std::string path = redirectEndpointPath();

      LOG_INFO("deploying endpoint at " << path);
      WApplication *app = WApplication::instance();
      WServer *server;
      if (app)
	server = app->environment().server();
      else
	server = WServer::instance();

      server->addResource(r.get(), path);

      impl_->redirectResource_ = std::move(r);
    }
  }
}

std::string OAuthService::userInfoEndpoint() const
{
  throw WException("OAuth::Process::userInfoEndpoint(): not specialized");
}

std::string OAuthService::configurationProperty(const std::string& property)
{
  WServer *instance = WServer::instance(); // Xx hmmmm...

  if (instance) {
    std::string result;

      bool error;
#ifndef WT_TARGET_JAVA
      error = !instance->readConfigurationProperty(property, result);
#else
      std::string* v = instance->readConfigurationProperty(property, result);
      if (v != &result) {
        error = false;
        result = *v;
      } else {
        error = true;
      }
#endif

    if (error)
      throw WException("OAuth: no '" + property + "' property configured");

    return result;
  } else
    throw WException("OAuth: could not find a WServer instance");
}

Http::Method OAuthService::tokenRequestMethod() const
{
  return Http::Method::Post;
}

  }
}
