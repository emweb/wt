#include "Wt/WApplication"
#include "Wt/Auth/GoogleAuth"
#include "Wt/Json/Object"
#include "Wt/Json/Parser"
#include "Wt/Http/Client"

#define ERROR_MSG(e) WString::tr("Wt.Auth.GoogleAuth." e)

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

class GoogleProcess : public OAuth::Process
{
public:
  GoogleProcess(const GoogleAuth& auth, const std::string& scope)
    : OAuth::Process(auth, scope)
  { }

  virtual void getIdentity(const OAuth::AccessToken& token)
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

      std::cerr << response.body() << std::endl;

      Json::Object userInfo;
      bool ok = Json::parse(response.body(), userInfo, e);

      if (!ok) {
	Wt::log("error") << "Could not parse Json: '" << response.body() << "'";
	setError(ERROR_MSG("badjson"));
	authenticated().emit(Identity::Invalid);
      } else {
	std::string id = "google:" + (std::string)userInfo.get("id");
	WT_USTRING userName = userInfo.get("name");
	std::string email = userInfo.get("email").orIfNull("");
	bool emailVerified = userInfo.get("verified_email").orIfNull(false);
	authenticated().emit(Identity(id, userName, email, emailVerified));
      }
    } else {
      setError(ERROR_MSG("badresponse"));
      authenticated().emit(Identity::Invalid);
    }
  }
};

GoogleAuth::GoogleAuth(const BaseAuth& baseAuth)
  : OAuth(baseAuth)
{ }

bool GoogleAuth::configured()
{
  try {
    configurationProperty(RedirectEndpointProperty);
    configurationProperty(ClientIdProperty);
    configurationProperty(ClientSecretProperty);

    return true;
  } catch (const std::exception& e) {
    Wt::log("notice") << "GoogleAuth not configured: " << e.what();

    return false;
  }
}

std::string GoogleAuth::name() const
{
  return "google";
}

WString GoogleAuth::description() const
{
  return "Google Account";
}

int GoogleAuth::popupWidth() const
{
  return 550;
}

int GoogleAuth::popupHeight() const
{
  return 350;
}

std::string GoogleAuth::authenticationScope() const
{
  return std::string(ProfileScope) + " " + EmailScope;
}

std::string GoogleAuth::redirectEndpoint() const
{
  return configurationProperty(RedirectEndpointProperty);
}

std::string GoogleAuth::authorizationEndpoint() const
{
 return AuthUrl;
}

std::string GoogleAuth::tokenEndpoint() const
{
  return TokenUrl;
}

std::string GoogleAuth::clientId() const
{
  return configurationProperty(ClientIdProperty);
}

std::string GoogleAuth::clientSecret() const
{
  return configurationProperty(ClientSecretProperty);
}

WLink GoogleAuth::icon() const
{
  return WLink("google.png");
}

OAuth::Process *GoogleAuth::createProcess(const std::string& scope) const
{
  return new GoogleProcess(*this, scope);
}

  }
}
