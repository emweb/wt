#include "Wt/WApplication.h"
#include "Wt/WLogger.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Parser.h"
#include "Wt/Http/Client.h"
#include "Wt/WException.h"
#include "Wt/Auth/OidcService.h"
#include "Wt/Utils.h"

#include <boost/algorithm/string.hpp>

#define ERROR_MSG(e) WString::tr("Wt.Auth.OidcService." e)

namespace Wt {

LOGGER("Auth.OidcService");

  namespace Auth {

  OidcProcess::OidcProcess(const OidcService& service, const std::string& scope)
    : OAuthProcess(service, scope)
  { }

  void OidcProcess::getIdentity(const OAuthAccessToken& token)
  {
    if (!token.idToken().empty()) {
      Identity id = parseIdToken(token.idToken());
      if (id.isValid()) { // else fall back to userinfo
        authenticated().emit(parseIdToken(token.idToken()));
        return;
      }
    }

    httpClient_.reset(new Http::Client());
    httpClient_->setTimeout(std::chrono::seconds(15));
    httpClient_->setMaximumResponseSize(10 * 1024);

    httpClient_->done().connect(this, std::bind(&OidcProcess::handleResponse,
                                       this, std::placeholders::_1, std::placeholders::_2));

    std::vector<Http::Message::Header> headers;
    headers.push_back(Http::Message::Header("Authorization",
					    "Bearer " + token.value()));

    httpClient_->get(service().userInfoEndpoint(), headers);

#ifndef WT_TARGET_JAVA
    WApplication::instance()->deferRendering();
#endif
  }

  Identity OidcProcess::parseClaims(const Json::Object& claims)
  {
    std::string id = claims.get("sub").orIfNull(""); // ifNull -> Invalid
    std::string name = claims.get("name").orIfNull("");
    std::string email = claims.get("email").orIfNull("");
    bool emailVerified = claims.get("email_verified").orIfNull(false);
    std::string providerName = this->service().name();
    return Identity(providerName, id, name, email, emailVerified);
  }

  void OidcProcess::handleResponse(AsioWrapper::error_code err, const Http::Message& response)
  {
#ifndef WT_TARGET_JAVA
    WApplication::instance()->resumeRendering();
#endif

    if (!err && response.status() == 200) {
      LOG_INFO("user info: " << response.body());

      Json::Object userInfo;

#ifndef WT_TARGET_JAVA
      Json::ParseError e;
      bool ok = Json::parse(response.body(), userInfo, e);
#else
      try {
	userInfo = (Json::Object)Json::Parser().parse(response.body());
      } catch (Json::ParseError pe) {
      }
      bool ok = userInfo.isNull();
#endif

      if (!ok) {
        LOG_ERROR("could not parse Json: '" << response.body() << "'");
	setError(ERROR_MSG("badjson"));
	authenticated().emit(Identity::Invalid);
      } else {
        authenticated().emit(parseClaims(userInfo));
      }
    } else {
      LOG_ERROR(ERROR_MSG("badresponse"));
      setError(ERROR_MSG("badresponse"));

      if (!err) {
        LOG_ERROR("user info request returned: " << response.status());
        LOG_ERROR("with: " << response.body());
      }

      authenticated().emit(Identity::Invalid);
    }
  }

Identity OidcProcess::parseIdToken(const std::string& idToken)
{
  std::vector<std::string> parts;
  boost::split(parts, idToken, boost::is_any_of("."));
  if (parts.size() != 3) {
    LOG_ERROR("malformed id_token: '" << idToken << "'");
    return Identity::Invalid;
  }
  Json::Object payloadJson;
#ifndef WT_TARGET_JAVA
  Json::ParseError err;
  bool ok = Json::parse(Utils::base64Decode(parts[1]), payloadJson, err, false);
#else
  try {
    payloadJson = (Json::Object)Json::Parser().parse(
        Utils::base64DecodeS(parts[1]));
  } catch (Json::ParseError pe) {
  }
  bool ok = payloadJson.isNull();
#endif
  if (!ok) {
    LOG_ERROR("could not parse Json: '" << parts[1] << "'");
    return Identity::Invalid;
  }
  return parseClaims(payloadJson);
}

OidcService::OidcService(const AuthService& baseAuth)
  : OAuthService(baseAuth),
    scope_("openid"),
    popupWidth_(670),
    popupHeight_(400),
    method_(HttpAuthorizationBasic),
    configured_(false)
{ }

bool OidcService::configure()
{
  configured_ = !redirectEndpoint_.empty()      &&
                !authorizationEndpoint_.empty() &&
                !tokenEndpoint_.empty()         &&
                !userInfoEndpoint_.empty()      &&
                !clientId_.empty()              &&
                !clientSecret_.empty()          &&
                !name_.empty()                  &&
                !scope_.empty();

  return configured_;
}

std::unique_ptr<OAuthProcess> OidcService::createProcess(const std::string& scope) const
{
  if (configured_)
    return std::unique_ptr<OAuthProcess>(new OidcProcess(*this, scope));
  else
    throw WException("OidcService not configured correctly");
}

void OidcService::setRedirectEndpoint(const std::string& url)
{ redirectEndpoint_ = url;
  configure();
}

void OidcService::setClientId(const std::string& id)
{
  clientId_ = id;
  configure();
}

void OidcService::setClientSecret(const std::string& secret)
{
  clientSecret_ = secret;
  configure();
}

void OidcService::setAuthEndpoint(const std::string& url)
{
  authorizationEndpoint_ = url;
  configure();
}

void OidcService::setTokenEndpoint(const std::string& url)
{
  tokenEndpoint_ = url;
  configure();
}

void OidcService::setUserInfoEndpoint(const std::string& url)
{
  userInfoEndpoint_ = url;
  configure();
}

void OidcService::setAuthenticationScope(const std::string& scope)
{
  scope_ = scope;
  configure();
}

void OidcService::setName(const std::string& name)
{
  name_ = name;
  configure();
}

void OidcService::setDescription(const std::string& description)
{
  description_ = description;
}

void OidcService::setClientSecretMethod(ClientSecretMethod method)
{
  method_ = method;
}

void OidcService::setPopupWidth(int width)
{
  popupWidth_ = width;
}

void OidcService::setPopupHeight(int height)
{
  popupHeight_ = height;
}

  }
}
