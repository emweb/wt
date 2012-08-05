// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef QR_AUTH_SERVICE_H_
#define QR_AUTH_SERVICE_H_

#include <string>
#include <Wt/WGlobal>
#include <Wt/Http/Client>

class QRTokenDatabase;

class QRAuthService
{
public:
  QRAuthService(const Wt::Auth::AuthService& baseAuth);

  const Wt::Auth::AuthService& baseAuth() const { return baseAuth_; }

  void setRedirectParameter(const std::string& parameter);
  std::string redirectParameter() const { return redirectParameter_; }

  std::string parseQRToken(const Wt::WEnvironment& env) const;

  Wt::WResource *createLoginResource(QRTokenDatabase& database,
				     Wt::Auth::AbstractUserDatabase& users,
				     Wt::Auth::Login& login,
				     Wt::WObject *parent = 0) const;

  std::string createQRToken(QRTokenDatabase& database,
			    Wt::WResource *resource) const;

  void remoteLogin(QRTokenDatabase& database, const Wt::Auth::User& user,
		   const std::string& token) const;

private:
  const Wt::Auth::AuthService& baseAuth_;
  std::string redirectParameter_;

  void handleHttpResponse(Wt::Http::Client *client,
			  boost::system::error_code err,
			  const Wt::Http::Message& response) const;
};

#endif // QR_AUTH_SERVICE_H_
