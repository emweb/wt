#include "Wt/WApplication"
#include "Wt/Auth/GoogleService"
#include "Wt/Json/Object"
#include "Wt/Json/Parser"
#include "Wt/Http/Client"

#define ERROR_MSG(e) WString::tr("Wt.Auth.GoogleService." e)

namespace {
  const char *RedirectEndpointProperty = "google-oauth2-redirect-endpoint";
  const char *ClientIdProperty = "google-oauth2-client-id";
  const char *ClientSecretProperty = "google-oauth2-client-secret";

  const char *UserInfoUrl = "https://www.googleapis.com/oauth2/v1/userinfo";
  const char *AuthUrl = "https://accounts.google.com/o/oauth2/auth";
  const char *TokenUrl = "https://accounts.google.com/o/oauth2/token";

  const char *ProfileScope = "https://www.googleapis.com/auth/userinfo.profile";
  const char *EmailScope = "https://www.googleapis.com/auth/userinfo.email";
}

namespace Wt {
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

    Http::Message m;
    m.setHeader("Authorization", "OAuth " + token.value());

    client->request(Http::Get, UserInfoUrl, m);

    WApplication::instance()->deferRendering();
  }

private:
  void handleMe(boost::system::error_code err, const Http::Message& response)
  {
    WApplication::instance()->resumeRendering();

    if (!err && response.status() == 200) {
      Json::ParseError e;

      Wt::log("notice") << "User info: " << response.body();

      Json::Object userInfo;
      bool ok = Json::parse(response.body(), userInfo, e);

      if (!ok) {
	Wt::log("error") << "Could not parse Json: '" << response.body() << "'";
	setError(ERROR_MSG("badjson"));
	authenticated().emit(Identity::Invalid);
      } else {
	std::string id = userInfo.get("id");
	WT_USTRING userName = userInfo.get("name");
	std::string email = userInfo.get("email").orIfNull("");
	bool emailVerified = userInfo.get("verified_email").orIfNull(false);
	authenticated().emit(Identity(service().name(), id, userName,
				      email, emailVerified));
      }
    } else {
      setError(ERROR_MSG("badresponse"));
      if (!err) {
	Wt::log("error") << "user info request returned: " << response.status();
	Wt::log("error") << "with: " << response.body();
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
    Wt::log("notice") << "GoogleService not configured: " << e.what();

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
  return 350;
}

std::string GoogleService::authenticationScope() const
{
  return std::string(ProfileScope) + " " + EmailScope;
}

std::string GoogleService::redirectEndpoint() const
{
  return configurationProperty(RedirectEndpointProperty);
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
