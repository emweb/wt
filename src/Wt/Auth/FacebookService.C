#include "Wt/Auth/FacebookService.h"

#include "Wt/WApplication.h"
#include "Wt/WLogger.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Parser.h"
#include "Wt/Http/Client.h"

#define ERROR_MSG(e) WString::tr("Wt.Auth.FacebookService." e)

namespace {
  const char *RedirectEndpointProperty = "facebook-oauth2-redirect-endpoint";
  const char *RedirectEndpointPathProperty = "facebook-oauth2-redirect"
    "-endpoint-path";
  const char *ClientIdProperty = "facebook-oauth2-app-id";
  const char *ClientSecretProperty = "facebook-oauth2-app-secret";

  const char *AuthUrl = "https://www.facebook.com/dialog/oauth";
  const char *TokenUrl = "https://graph.facebook.com/oauth/access_token";

  const char *EmailScope = "email";
}

namespace Wt {

LOGGER("Auth.FacebookService");

  namespace Auth {

class FacebookProcess final : public OAuthProcess
{
public:
  FacebookProcess(const FacebookService& auth, const std::string& scope)
    : OAuthProcess(auth, scope)
  { }

  virtual void getIdentity(const OAuthAccessToken& token) override
  {
    httpClient_.reset(new Http::Client());
    httpClient_->setTimeout(std::chrono::seconds(15));
    httpClient_->setMaximumResponseSize(10*1024);

    httpClient_->done().connect
      (this, std::bind(&FacebookProcess::handleMe, this,
		       std::placeholders::_1,
		       std::placeholders::_2));
    httpClient_->get("https://graph.facebook.com/me?fields=name,id,email&access_token="
		     + token.value());

#ifndef WT_TARGET_JAVA
    WApplication::instance()->deferRendering();
#endif
  }

private:
  std::unique_ptr<Http::Client> httpClient_;

  void handleMe(AsioWrapper::error_code err, const Http::Message& response)
  {
#ifndef WT_TARGET_JAVA
    WApplication::instance()->resumeRendering();
#endif

    if (!err && response.status() == 200) {
#ifndef WT_TARGET_JAVA
      Json::ParseError e;
      Json::Object me;
      bool ok = Json::parse(response.body(), me, e);
#else
      Json::Object me;
      try {
	me = (Json::Object)Json::Parser().parse(response.body());
      } catch (Json::ParseError pe) {
      }
      bool ok = me.isNull();
#endif

      if (!ok) {
	LOG_ERROR("could not parse Json: '" << response.body() << "'");
	setError(ERROR_MSG("badjson"));
	authenticated().emit(Identity::Invalid);
      } else {
	std::string id = me.get("id");
	WT_USTRING userName = me.get("name");
	std::string email = me.get("email").orIfNull("");
        bool emailVerified = !me.get("email").isNull();

	authenticated().emit(Identity(service().name(), id, userName,
				      email, emailVerified));
      }
    } else {
      if (!err) {
	LOG_ERROR("user info request returned: " << response.status());
	LOG_ERROR("with: " << response.body());
      } else
	LOG_ERROR("handleMe(): " << err.message());

      setError(ERROR_MSG("badresponse"));

      authenticated().emit(Identity::Invalid);
    }
  }
};

FacebookService::FacebookService(const AuthService& baseAuth)
  : OAuthService(baseAuth)
{ }

bool FacebookService::configured()
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

std::string FacebookService::name() const
{
  return "facebook";
}

WString FacebookService::description() const
{
  return "Facebook Account";
}

std::string FacebookService::authenticationScope() const
{
  return EmailScope;
}

int FacebookService::popupWidth() const
{
  return 1000;
}

int FacebookService::popupHeight() const
{
  return 600;
}

std::string FacebookService::redirectEndpoint() const
{
  return configurationProperty(RedirectEndpointProperty);
}

std::string FacebookService::redirectEndpointPath() const
{
  try {
    return configurationProperty(RedirectEndpointPathProperty);
  } catch (const std::exception& e) {
    return OAuthService::redirectEndpointPath();
  }
}

std::string FacebookService::authorizationEndpoint() const
{
  return AuthUrl;
}

std::string FacebookService::tokenEndpoint() const
{
  return TokenUrl;
}

std::string FacebookService::clientId() const
{
  return configurationProperty(ClientIdProperty);
}

std::string FacebookService::clientSecret() const
{
  return configurationProperty(ClientSecretProperty);
}

ClientSecretMethod FacebookService::clientSecretMethod() const
{
  return PlainUrlParameter;
}

Http::Method FacebookService::tokenRequestMethod() const
{
  return Http::Method::Get;
}

std::unique_ptr<OAuthProcess> FacebookService
::createProcess(const std::string& scope) const
{
  return std::unique_ptr<OAuthProcess>(new FacebookProcess(*this, scope));
}

  }
}
