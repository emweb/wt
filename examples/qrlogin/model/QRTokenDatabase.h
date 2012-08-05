// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef QR_TOKEN_DATABASE_H_
#define QR_TOKEN_DATABASE_H_

#include <Wt/WGlobal>
#include <string>

class QRTokenDatabase
{
public:
  QRTokenDatabase(Wt::Dbo::Session& session);

  void addToken(const std::string& sessionId, const std::string& hash,
		const std::string& url);
  void removeToken(const std::string& sessionId);

  // returns the URL to the notification resource
  std::string setUser(const std::string& hash, const Wt::Auth::User& user);

  Wt::Auth::User findUser(const std::string& sessionId,
			  Wt::Auth::AbstractUserDatabase& db);

private:
  Wt::Dbo::Session& session_;
};

#endif // QR_TOKEN_DATABASE_H_
