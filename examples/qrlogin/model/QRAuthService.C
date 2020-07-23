/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "QRAuthService.h"
#include "QRTokenDatabase.h"

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WRandom.h>
#include <Wt/WResource.h>
#include <Wt/WServer.h>
#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/User.h>
#include <Wt/Http/Client.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

class QRLoginResource : public WResource
{
public:
  QRLoginResource(QRTokenDatabase& database,
                  Auth::AbstractUserDatabase& users,
                  Auth::Login& login)
    : WResource(),
      database_(database),
      users_(users),
      login_(login)
  {
    setTakesUpdateLock(true);
  }

  virtual ~QRLoginResource()
  {
    beingDeleted();
  }

  virtual void handleRequest(const Http::Request &request,
                             Http::Response &response)
  {
    WApplication *app = WApplication::instance();

    app->environment().server()
      ->post(app->sessionId(), bindSafe(&QRLoginResource::doLogin));

    response.setMimeType("plain/text");
    response.out() << "ok";
  }

private:
  QRTokenDatabase& database_;
  Auth::AbstractUserDatabase& users_;
  Auth::Login& login_;

  void doLogin()
  {
    WApplication *app = WApplication::instance();

    Auth::User user = database_.findUser(app->sessionId(), users_);
    if (user.isValid())
      login_.login(user);
  }
};

QRAuthService::QRAuthService(const Auth::AuthService& baseAuth)
  : baseAuth_(baseAuth),
    redirectParameter_("qr")
{ }

std::string QRAuthService::createQRToken(QRTokenDatabase& database,
                                         WResource *loginResource) const
{
  WApplication* app = WApplication::instance();

  std::string token = WRandom::generateId(baseAuth_.randomTokenLength());
  std::string hash
    = baseAuth_.tokenHashFunction()->compute(token, std::string());

  database.addToken(app->sessionId(), hash,
		    app->makeAbsoluteUrl(loginResource->url()));

  return token;
}

std::string QRAuthService::parseQRToken(const WEnvironment& env) const
{
  const std::string *code = env.getParameter(redirectParameter_);

  if (code)
    return *code;
  else
    return std::string();
}

std::unique_ptr<WResource>
QRAuthService::createLoginResource(QRTokenDatabase& database,
				   Auth::AbstractUserDatabase& users,
				   Auth::Login& login) const
{
  return cpp14::make_unique<QRLoginResource>(database, users, login);
}

void QRAuthService::remoteLogin(QRTokenDatabase& database,
				const Auth::User& user,
				const std::string& token) const
{
  std::string hash
    = baseAuth_.tokenHashFunction()->compute(token, std::string());

  std::string url = database.setUser(hash, user);

  if (!url.empty()) {
    Http::Client *client = new Http::Client();
    client->setTimeout(std::chrono::seconds{15});
    client->setMaximumResponseSize(1024);
    client->done().connect(std::bind(&QRAuthService::handleHttpResponse,
                                       this, std::placeholders::_1, std::placeholders::_2, client));
    client->get(url);
  }
}

void QRAuthService::handleHttpResponse(Wt::AsioWrapper::error_code err,
				       const Http::Message& response,
				       Http::Client *client) const
{
  if (err || response.status() != 200)
    std::cerr << "Error: " << err.message() << ", " << response.status()
	      << std::endl;

  delete client;
}

