/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "QRAuthService.h"
#include "QRTokenDatabase.h"

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WRandom>
#include <Wt/WResource>
#include <Wt/WServer>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/Login>
#include <Wt/Auth/User>
#include <Wt/Http/Client>
#include <Wt/Http/Request>
#include <Wt/Http/Response>

class QRLoginResource : public Wt::WResource
{
public:
  QRLoginResource(QRTokenDatabase& database,
		  Wt::Auth::AbstractUserDatabase& users,
		  Wt::Auth::Login& login,
		  Wt::WObject *parent)
    : Wt::WResource(parent),
      database_(database),
      users_(users),
      login_(login)
  { }

  virtual ~QRLoginResource()
  {
    beingDeleted();
  }

  virtual void handleRequest(const Wt::Http::Request &request,
			     Wt::Http::Response &response)
  {
    Wt::WApplication *app = Wt::WApplication::instance();

    app->environment().server()
      ->post(app->sessionId(),
	     app->bind(boost::bind(&QRLoginResource::doLogin, this))); 

    response.setMimeType("plain/text");
    response.out() << "ok";
  }

private:
  QRTokenDatabase& database_;
  Wt::Auth::AbstractUserDatabase& users_;
  Wt::Auth::Login& login_;

  void doLogin()
  {
    Wt::WApplication *app = Wt::WApplication::instance();

    Wt::Auth::User user = database_.findUser(app->sessionId(), users_);
    if (user.isValid())
      login_.login(user);
  }
};

QRAuthService::QRAuthService(const Wt::Auth::AuthService& baseAuth)
  : baseAuth_(baseAuth),
    redirectParameter_("qr")
{ }

std::string QRAuthService::createQRToken(QRTokenDatabase& database,
					 Wt::WResource *loginResource) const
{
  Wt::WApplication* app = Wt::WApplication::instance();

  std::string token = Wt::WRandom::generateId(baseAuth_.randomTokenLength());
  std::string hash
    = baseAuth_.tokenHashFunction()->compute(token, std::string());

  database.addToken(app->sessionId(), hash,
		    app->makeAbsoluteUrl(loginResource->url()));

  return token;
}

std::string QRAuthService::parseQRToken(const Wt::WEnvironment& env) const
{
  const std::string *code = env.getParameter(redirectParameter_);

  if (code)
    return *code;
  else
    return std::string();
}

Wt::WResource *
QRAuthService::createLoginResource(QRTokenDatabase& database,
				   Wt::Auth::AbstractUserDatabase& users,
				   Wt::Auth::Login& login,
				   Wt::WObject *parent) const
{
  return new QRLoginResource(database, users, login, parent);
}

void QRAuthService::remoteLogin(QRTokenDatabase& database,
				const Wt::Auth::User& user,
				const std::string& token) const
{
  std::string hash
    = baseAuth_.tokenHashFunction()->compute(token, std::string());

  std::string url = database.setUser(hash, user);

  if (!url.empty()) {
    Wt::Http::Client *client = new Wt::Http::Client();
    client->setTimeout(15);
    client->setMaximumResponseSize(1024);
    client->done().connect(boost::bind(&QRAuthService::handleHttpResponse,
				       this, client, _1, _2));
    client->get(url);
  }
}

void QRAuthService::handleHttpResponse(Wt::Http::Client *client,
				       boost::system::error_code err,
				       const Wt::Http::Message& response) const
{
  if (err || response.status() != 200)
    std::cerr << "Error: " << err.message() << ", " << response.status()
	      << std::endl;

  delete client;
}

