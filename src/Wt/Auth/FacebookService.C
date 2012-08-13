#include "Wt/WApplication"
#include "Wt/Auth/FacebookService"
#include "Wt/Json/Object"
#include "Wt/Json/Parser"
#include "Wt/Http/Client"

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

class FacebookProcess : public OAuthProcess
{
public:
  FacebookProcess(const FacebookService& auth, const std::string& scope)
    : OAuthProcess(auth, scope)
  { }

  virtual void getIdentity(const OAuthAccessToken& token)
  {
    Http::Client *client = new Http::Client(this);
    client->setTimeout(15);
    client->setMaximumResponseSize(10*1024);

    client->done().connect
      (boost::bind(&FacebookProcess::handleMe, this, _1, _2));
    client->get("https://graph.facebook.com/me?access_token=" + token.value());

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
	std::string email = me.get("email");
	bool emailVerified = me.get("verified").orIfNull(false);

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

Http::Method FacebookService::tokenRequestMethod() const
{
  return Http::Get;
}

OAuthProcess *FacebookService::createProcess(const std::string& scope) const
{
  return new FacebookProcess(*this, scope);
}

  }
}
