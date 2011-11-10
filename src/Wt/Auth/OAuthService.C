/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/OAuthService"

#include "Wt/Json/Parser"
#include "Wt/Json/Object"

#include "Wt/Http/Client"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WLogger"
#include "Wt/WRandom"
#include "Wt/WResource"
#include "Wt/WStringStream"
#include "Wt/WServer"

#include "Utils.h"
#include "../../web/Utils.h"

#ifdef WT_THREADED
#include <boost/thread.hpp>
#endif // WT_THREADED

#include <boost/algorithm/string.hpp>

#define ERROR_MSG(e) WString::tr("Wt.Auth.OAuthService." e)

namespace Wt {
  namespace Auth {

class OAuthRedirectEndpoint : public WResource
{
public:
  OAuthRedirectEndpoint(OAuthProcess *process)
    : WResource(process),
      process_(process)
  {
    setInternalPath(process_->service().redirectInternalPath());
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
			     Http::Response& response)
  {
    if (!request.continuation()) {
      response.setMimeType("text/html; charset=UTF-8");

      const std::string *stateE = request.getParameter("state");
      if (!stateE || *stateE != process_->oAuthState_) {
	process_->setError(ERROR_MSG("invalid-state"));
	sendResponse(response);
	return;
      }

      const std::string *errorE = request.getParameter("error");
      if (errorE) {
	process_->setError(ERROR_MSG(+ *errorE));
	sendResponse(response);
	return;
      }

      const std::string *codeE = request.getParameter("code");
      if (!codeE) {
	process_->setError(ERROR_MSG("missing-code"));
	sendResponse(response);
	return;
      }

      const OAuthService& service = process_->service();

      /*
       * OAuth 2.0 draft says this should be a POST using
       * application/x-www-form-urlencoded but that's not what Facebook
       * does
       */
      std::string url = service.tokenEndpoint();

      WStringStream data;
      data << "grant_type=authorization_code"
	   << "&client_id=" << Wt::Utils::urlEncode(service.clientId())
	   << "&client_secret=" << Wt::Utils::urlEncode(service.clientSecret())
	   << "&redirect_uri="
	   << Wt::Utils::urlEncode(service.getRedirectEndpoint())
	   << "&code=" << *codeE;

      Http::Client *client = new Http::Client(this);
      client->done().connect
	(boost::bind(&OAuthRedirectEndpoint::handleToken, this, _1, _2));

      Http::Method m = service.tokenRequestMethod();
      if (m == Http::Get) {
	bool hasQuery = url.find('?') != std::string::npos;
	url += (hasQuery ? '&' : '?') + data.str();
	client->get(url);
      } else {
	Http::Message post;
	post.setHeader("Content-Type", "application/x-www-form-urlencoded");
	post.addBodyText(data.str());
	client->post(url, post);
      }

      Http::ResponseContinuation *cont = response.createContinuation();
      cont->waitForMoreData();
    } else
      sendResponse(response);
  }

  void sendResponse(Http::Response& response)
  {
#ifndef WT_TARGET_JAVA
    std::ostream& o = response.out();
#else
    std::ostream o(response.out());
#endif // WT_TARGET_JAVA

    std::string app = WApplication::instance()->javaScriptClass();

    o <<
      "<!DOCTYPE html>"
      "<html lang=\"en\" dir=\"ltr\">\n"
      "<head><title></title>\n"
      "<script type=\"text/javascript\">\n"
      "function load() { "
      """if (window.opener." << app << ") {"
      ""  "var " << app << "= window.opener." << app << ";"
      <<  process_->redirected_.createCall() << ";"
      ""  "window.close();"
      "}\n"
      "}\n"
      "</script></head>"
      "<body onload=\"load();\"></body></html>";
  }

private:
  OAuthProcess *process_;

  void handleToken(boost::system::error_code err, const Http::Message& response)
  {
    if (!err)
      process_->doParseTokenResponse(response);
    else
      process_->setError(WString::fromUTF8(err.message()));

    haveMoreData();
  }
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

const OAuthAccessToken OAuthAccessToken::Invalid;

OAuthProcess::OAuthProcess(const OAuthService& service,
			   const std::string& scope)
  : service_(service),
    scope_(scope),
    authenticate_(false),
    authorized_(this),
    authenticated_(this),
    redirected_(this, "redirected")
{
  redirectEndpoint_ = new OAuthRedirectEndpoint(this);

  WApplication *app = WApplication::instance();

  oAuthState_ = service_.encodeState(app->sessionId());

  WStringStream url;
  url << service_.authorizationEndpoint();
  bool hasQuery = url.str().find('?') != std::string::npos;

  url << (hasQuery ? '&' : '?')
      << "client_id=" << Wt::Utils::urlEncode(service_.clientId())
      << "&redirect_uri=" << Wt::Utils::urlEncode(service_.getRedirectEndpoint())
      << "&scope=" << Wt::Utils::urlEncode(scope)
      << "&response_type=code"
      << "&state=" << Wt::Utils::urlEncode(oAuthState_);

  WStringStream js;
  js << WT_CLASS ".authPopupWindow(" WT_CLASS
     << "," << WWebWidget::jsStringLiteral(url.str()) 
     << ", " << service.popupWidth()
     << ", " << service.popupHeight() << ");"; 

  redirected_.connect(this, &OAuthProcess::onOAuthDone);

  implementJavaScript(&OAuthProcess::startAuthorize, js.str());
  implementJavaScript(&OAuthProcess::startAuthenticate, js.str());
}

void OAuthProcess::startAuthorize()
{
  redirectEndpoint_->url(); // Make sure it is exposed

  if (!WApplication::instance()->environment().javaScript())
    throw WException("Not yet implemented");
}

void OAuthProcess::startAuthenticate()
{
  redirectEndpoint_->url(); // Make sure it is exposed

  if (!WApplication::instance()->environment().javaScript())
    throw WException("Not yet implemented");

  authenticate_ = true;
}

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

  if (authenticate_) {
    authenticate_ = false;
    getIdentity(token_);
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

OAuthAccessToken
OAuthProcess::parseTokenResponse(const Http::Message& response)
{
  if (response.status() == 200 || response.status() == 400) {
    /*
     * OAuth 2.0 states this should be application/json
     * but Facebook uses application/x-www-form-urlencoded body
     */
    const std::string *type = response.getHeader("Content-Type");

    if (type) {
      if (boost::starts_with(*type, "application/x-www-form-urlencoded"))
	return parseUrlEncodedToken(response);
      else if (boost::starts_with(*type, "application/json"))
	return parseJsonToken(response);
      else
	throw TokenError(ERROR_MSG("badresponse"));
    } else
      throw TokenError(ERROR_MSG("badresponse"));
  } else
    throw TokenError(ERROR_MSG("badresponse"));
}

OAuthAccessToken
OAuthProcess::parseUrlEncodedToken(const Http::Message& response)
{
  /* Facebook style */
  Http::ParameterMap params;
  Http::Request::parseFormUrlEncoded(response.body(), params);

  if (response.status() == 200) {
    const std::string *accessTokenE = Http::get(params, "access_token");
    if (accessTokenE) {
      std::string accessToken = *accessTokenE;

      WDateTime expires;
      const std::string *expiresE = Http::get(params, "expires");
      if (expiresE)
	expires = WDateTime::currentDateTime()
	  .addSecs(boost::lexical_cast<int>(*expiresE));

      // FIXME refresh token
      
      return OAuthAccessToken(accessToken, expires, std::string());
    } else
      throw TokenError(ERROR_MSG("badresponse"));
  } else {
    const std::string *errorE = Http::get(params, "error");
    
    if (errorE)
      throw TokenError(ERROR_MSG(+ *errorE));
    else
      throw TokenError(ERROR_MSG("badresponse"));
  }
}

OAuthAccessToken
OAuthProcess::parseJsonToken(const Http::Message& response)
{
  /* OAuth 2.0 style */
  Json::Object root;
  Json::ParseError e;
  bool ok = Json::parse(response.body(), root, e);

  if (!ok) {
    Wt::log("error") << e.what();
    throw TokenError(ERROR_MSG("badjson"));
  } else {
    if (response.status() == 200) {
      try {
	std::string accessToken = (std::string)root.get("access_token");
	int secs = root.get("expires_in").orIfNull(-1);
	WDateTime expires;
	if (secs > 0)
	  expires = WDateTime::currentDateTime().addSecs(secs);

	std::string refreshToken = root.get("refreshToken").orIfNull("");

	return OAuthAccessToken(accessToken, expires, refreshToken);
      } catch (std::exception& e) {
	std::cerr << e.what() << std::endl;
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
    : redirectResource_(0)
  { 
    secret_ = WRandom::generateId(32);
  }

#ifdef WT_THREADED
  boost::mutex mutex_;
#endif // WT_THREADED

  class RedirectEndpoint : public WResource
  {
  public:
    RedirectEndpoint(const OAuthService& service,
		     const std::string& redirectUrl)
      : service_(service),
	redirectUrl_(redirectUrl)
    { }

    virtual void handleRequest(const Http::Request& request,
			       Http::Response& response)
    {
      const std::string *stateE = request.getParameter("state");

      if (stateE) {
	std::string sessionId = service_.decodeState(*stateE);

	if (!sessionId.empty()) {
	  std::string redirectUrl = redirectUrl_;
	  
	  bool hasQuery = redirectUrl.find('?') != std::string::npos;
	  redirectUrl += (hasQuery ? '&' : '?');
	  redirectUrl += "wtd=" + Wt::Utils::urlEncode(sessionId)
	    + "&state=" + Wt::Utils::urlEncode(*stateE);

	  const std::string *errorE = request.getParameter("error");
	  if (errorE)
	    redirectUrl += "&error=" + Wt::Utils::urlEncode(*errorE);

	  const std::string *codeE = request.getParameter("code");
	  if (codeE)
	    redirectUrl += "&code=" + Wt::Utils::urlEncode(*codeE);

	  response.setStatus(301);
	  response.addHeader("Location", redirectUrl);
	  return;
	}
      }

      response.setStatus(400);
      response.setMimeType("text/html");
      response.out() << "<html><body><h1>Error</h1></body></html>";
    }

  private:
    const OAuthService& service_;
    std::string redirectUrl_;
  };

  RedirectEndpoint *redirectResource_;
  std::string secret_;
};

OAuthService::OAuthService(const AuthService& auth)
  : baseAuth_(auth),
    impl_(new Impl())
{ }

OAuthService::~OAuthService()
{ 
  delete impl_->redirectResource_;
  delete impl_;
}

std::string OAuthService::redirectInternalPath() const
{
  return "/auth/oauth/" + name() + "/redirect";
}

std::string OAuthService::getRedirectEndpoint() const
{
  std::string result = redirectEndpoint();
  configureRedirectEndpoint(result);
  return result;
}

std::string OAuthService::encodeState(const std::string& sessionId) const
{
  std::string msg = impl_->secret_ + sessionId;
  std::string hash = Auth::Utils::encodeAscii(Auth::Utils::md5(msg));
  return hash + "|" + sessionId;
}

std::string OAuthService::decodeState(const std::string& state) const
{
  std::size_t i = state.find('|');
  if (i != std::string::npos) {
    std::string sessionId = state.substr(i + 1);

    std::string check = encodeState(sessionId);
    if (check == state)
      return sessionId;
    else
      return std::string();
  } else
    return std::string();
}

void OAuthService::configureRedirectEndpoint(const std::string& endpoint) const
{
  if (!impl_->redirectResource_) {
#ifdef WT_THREADED
    boost::mutex::scoped_lock guard(impl_->mutex_);
#endif
    if (!impl_->redirectResource_) {
      /* Compute absolute URL for dynamic resource */
      WApplication *app = WApplication::instance();
      WServer *server = app->environment().server();

      std::string url = app->makeAbsoluteUrl(app->bookmarkUrl
					     (redirectInternalPath()));

      Impl::RedirectEndpoint *r = new Impl::RedirectEndpoint(*this, url);

      /* Compute deployment path for static resource */
      std::string protocol, host, path;
      int port;

      Http::Client::parseUrl(endpoint, protocol, host, port, path);

      Wt::log("notice") << "OAuth: deploying endpoint at " << path;
      server->addResource(r, path);

      impl_->redirectResource_ = r;
    }
  }
}

std::string OAuthService::configurationProperty(const std::string& property)
{
  WServer *instance = WServer::instance(); // Xx hmmmm...

  if (instance) {
    std::string result;

    if (!instance->readConfigurationProperty(property, result))
      throw WException("OAuth: no '" + property + "' property configured");

    return result;
  } else
    throw WException("OAuth: could not find a WServer instance");
}

Http::Method OAuthService::tokenRequestMethod() const
{
  return Http::Post;
}

  }
}
