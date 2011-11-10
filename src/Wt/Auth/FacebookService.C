#include "Wt/WApplication"
#include "Wt/Auth/FacebookService"
#include "Wt/Json/Object"
#include "Wt/Json/Parser"
#include "Wt/Http/Client"

#define ERROR_MSG(e) WString::tr("Wt.Auth.FacebookService." e)

namespace Wt {
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

    WApplication::instance()->deferRendering();
  }

private:
  void handleMe(boost::system::error_code err, const Http::Message& response)
  {
    WApplication::instance()->resumeRendering();

    if (!err && response.status() == 200) {
      Json::ParseError e;
      Json::Object me;
      bool ok = Json::parse(response.body(), me, e);

      if (!ok) {
	Wt::log("error") << "Could not parse Json: '" << response.body() << "'";
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
      setError(ERROR_MSG("badresponse"));
      authenticated().emit(Identity::Invalid);
    }
  }
};

FacebookService::FacebookService(const AuthService& baseAuth)
  : OAuthService(baseAuth)
{ }

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
  return "email";
}

int FacebookService::popupWidth() const
{
  return 550;
}

int FacebookService::popupHeight() const
{
  return 350;
}

std::string FacebookService::redirectEndpoint() const
{
  return configurationProperty("facebook-oauth2-redirect-endpoint");
}

std::string FacebookService::authorizationEndpoint() const
{
  return "https://www.facebook.com/dialog/oauth";
}

std::string FacebookService::tokenEndpoint() const
{
  return "https://graph.facebook.com/oauth/access_token";
}

std::string FacebookService::clientId() const
{
  return configurationProperty("facebook-oauth2-app-id");
}

std::string FacebookService::clientSecret() const
{
  return configurationProperty("facebook-oauth2-app-secret");
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
