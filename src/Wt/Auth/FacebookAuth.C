#include "Wt/WApplication"
#include "Wt/Auth/FacebookAuth"
#include "Wt/Json/Object"
#include "Wt/Json/Parser"
#include "Wt/Http/Client"

#define ERROR_MSG(e) WString::tr("Wt.Auth.FacebookAuth." e)

namespace Wt {
  namespace Auth {

class FacebookProcess : public OAuth::Process
{
public:
  FacebookProcess(const FacebookAuth& auth, const std::string& scope)
    : Process(auth, scope)
  { }

  virtual void getIdentity(const OAuth::AccessToken& token)
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
	std::string id = "facebook:" + (std::string)me.get("id");
	WT_USTRING userName = me.get("name");
	std::string email = me.get("email");
	bool emailVerified = me.get("verified").orIfNull(false);

	authenticated().emit(Identity(id, userName, email, emailVerified));
      }
    } else {
      setError(ERROR_MSG("badresponse"));
      authenticated().emit(Identity::Invalid);
    }
  }
};

FacebookAuth::FacebookAuth(const BaseAuth& baseAuth)
  : OAuth(baseAuth)
{ }

std::string FacebookAuth::name() const
{
  return "facebook";
}

WString FacebookAuth::description() const
{
  return "Facebook Account";
}

std::string FacebookAuth::authenticationScope() const
{
  return "email";
}

int FacebookAuth::popupWidth() const
{
  return 550;
}

int FacebookAuth::popupHeight() const
{
  return 350;
}

std::string FacebookAuth::redirectEndpoint() const
{
  return configurationProperty("facebook-oauth2-redirect-endpoint");
}

std::string FacebookAuth::authorizationEndpoint() const
{
  return "https://www.facebook.com/dialog/oauth";
}

std::string FacebookAuth::tokenEndpoint() const
{
  return "https://graph.facebook.com/oauth/access_token";
}

std::string FacebookAuth::clientId() const
{
  return configurationProperty("facebook-oauth2-app-id");
}

std::string FacebookAuth::clientSecret() const
{
  return configurationProperty("facebook-oauth2-app-secret");
}

Http::Method FacebookAuth::tokenRequestMethod() const
{
  return Http::Get;
}

WLink FacebookAuth::icon() const
{
  return WLink("facebook.png");
}

OAuth::Process *FacebookAuth::createProcess(const std::string& scope) const
{
  return new FacebookProcess(*this, scope);
}

  }
}
