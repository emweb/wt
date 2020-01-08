// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef QR_AUTH_SERVICE_H_
#define QR_AUTH_SERVICE_H_

#include <string>
#include <Wt/WGlobal.h>
#include <Wt/Http/Client.h>

// #include <system_error> for standalone Asio
// #include <boost/system/system_error.hpp> for Boost.Asio
#include <Wt/AsioWrapper/system_error.hpp>

using namespace Wt;

class QRTokenDatabase;

class QRAuthService
{
public:
  QRAuthService(const Auth::AuthService& baseAuth);

  const Auth::AuthService& baseAuth() const { return baseAuth_; }

  void setRedirectParameter(const std::string& parameter);
  std::string redirectParameter() const { return redirectParameter_; }

  std::string parseQRToken(const WEnvironment& env) const;

  std::unique_ptr<WResource> createLoginResource(QRTokenDatabase& database,
                                     Auth::AbstractUserDatabase& users,
                                     Auth::Login& login) const;

  std::string createQRToken(QRTokenDatabase& database,
                            WResource *resource) const;

  void remoteLogin(QRTokenDatabase& database, const Auth::User& user,
		   const std::string& token) const;

private:
  const Auth::AuthService& baseAuth_;
  std::string redirectParameter_;

  void handleHttpResponse(Wt::AsioWrapper::error_code err,
                          const Http::Message& response,
                          Http::Client *client) const;
};

#endif // QR_AUTH_SERVICE_H_
