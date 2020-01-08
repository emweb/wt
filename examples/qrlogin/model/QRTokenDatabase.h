// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef QR_TOKEN_DATABASE_H_
#define QR_TOKEN_DATABASE_H_

#include <Wt/WGlobal.h>
#include <string>

using namespace Wt;

class QRTokenDatabase
{
public:
  QRTokenDatabase(Dbo::Session& session);

  void addToken(const std::string& sessionId, const std::string& hash,
		const std::string& url);
  void removeToken(const std::string& sessionId);

  // returns the URL to the notification resource
  std::string setUser(const std::string& hash, const Auth::User& user);

  Auth::User findUser(const std::string& sessionId,
                          Auth::AbstractUserDatabase& db);

private:
  Dbo::Session& session_;
};

#endif // QR_TOKEN_DATABASE_H_
