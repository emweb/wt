#include "Wt/WApplication"
#include "Wt/Auth/GoogleService"
#include "Wt/Json/Object"
#include "Wt/Json/Parser"
#include "Wt/Http/Client"

#define ERROR_MSG(e) WString::tr("Wt.Auth.GoogleService." e)

namespace {
  const char *RedirectEndpointProperty = "google-oauth2-redirect-endpoint";
  const char *RedirectEndpointPathProperty = "google-oauth2-redirect-endpoint-"
    "path";
  const char *ClientIdProperty = "google-oauth2-client-id";
  const char *ClientSecretProperty = "google-oauth2-client-secret";

  const char *AuthUrl = "https://accounts.google.com/o/oauth2/auth";
  const char *TokenUrl = "https://accounts.google.com/o/oauth2/token";

  const char *ProfileScope = "https://www.googleapis.com/auth/userinfo.profile";
  const char *EmailScope = "https://www.googleapis.com/auth/userinfo.email";
}

namespace Wt {

LOGGER("Auth.GoogleService");

  namespace Auth {

class GoogleProcess : public OAuthProcess
{
public:
  GoogleProcess(const GoogleService& service, const std::string& scope)
    : OAuthProcess(service, scope)
  { }

  virtual void getIdentity(const OAuthAccessToken& token)
  {
    Http::Client *client = new Http::Client(this);
    client->setTimeout(15);
    client->setMaximumResponseSize(10 * 1024);

    client->done().connect(boost::bind(&GoogleProcess::handleMe, this, _1, _2));

    std::vector<Http::Message::Header> headers;
    headers.push_back(Http::Message::Header("Authorization", 
					    "OAuth " + token.value()));

    const char *UserInfoUrl = "https://www.googleapis.com/oauth2/v1/userinfo";
    client->get(UserInfoUrl, headers);

#ifndef WT_TARGET_JAVA
    WApplication::instance()->deferRendering();
#endif
  }

private:
  void handleMe(boost::system::error_code err, const Http::Message& response)
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
	std::string id = userInfo.get("id");
	WT_USTRING userName = userInfo.get("name");
	std::string email = userInfo.get("email").orIfNull("");
#ifndef WT_TARGET_JAVA
	bool emailVerified = userInfo.get("verified_email")
	  .toBool().orIfNull(false);
#else
	bool emailVerified = userInfo.get("verified_email")
	  .orIfNull(false);
#endif
	authenticated().emit(Identity(service().name(), id, userName,
				      email, emailVerified));
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
};

GoogleService::GoogleService(const AuthService& baseAuth)
  : OAuthService(baseAuth)
{ }

bool GoogleService::configured()
{
  try {
    configurationProperty(RedirectEndpointProperty);
    configurationProperty(ClientIdProperty);
    configurationProperty(ClientSecretProperty);

    return true;
  } catch (const std::exception& e) {
    LOG_INFO("not configured: " << e.what());

    return false;
  }
}

std::string GoogleService::name() const
{
  return "google";
}

WString GoogleService::description() const
{
  return "Google Account";
}

int GoogleService::popupWidth() const
{
  return 550;
}

int GoogleService::popupHeight() const
{
  return 400;
}

std::string GoogleService::authenticationScope() const
{
  return std::string(ProfileScope) + " " + EmailScope;
}

std::string GoogleService::redirectEndpoint() const
{
  return configurationProperty(RedirectEndpointProperty);
}

std::string GoogleService::redirectEndpointPath() const
{
  try {
    return configurationProperty(RedirectEndpointPathProperty);
  } catch (const std::exception& e) {
    return OAuthService::redirectEndpointPath();
  }
}

std::string GoogleService::authorizationEndpoint() const
{
 return AuthUrl;
}

std::string GoogleService::tokenEndpoint() const
{
  return TokenUrl;
}

std::string GoogleService::clientId() const
{
  return configurationProperty(ClientIdProperty);
}

std::string GoogleService::clientSecret() const
{
  return configurationProperty(ClientSecretProperty);
}

OAuthProcess *GoogleService::createProcess(const std::string& scope) const
{
  return new GoogleProcess(*this, scope);
}

  }
}
