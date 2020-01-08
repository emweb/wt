/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "QRTokenDatabase.h"

#include <Wt/Auth/User.h>
#include <Wt/Dbo/Dbo.h>

class QRToken
{
public:
  QRToken() { }

  QRToken(const std::string& aSessionId, const std::string& aHash,
	  const std::string& aUrl) :
    sessionId(aSessionId),
    hash(aHash),
    url(aUrl)
  { }

  std::string sessionId;
  std::string hash;
  std::string url;
  std::string userId;

  template<class Action>
  void persist(Action& a)
  {
    Dbo::field(a, sessionId, "session_id");
    Dbo::field(a, hash, "hash");
    Dbo::field(a, url, "url");
    Dbo::field(a, userId, "user_id");
  }
};

QRTokenDatabase::QRTokenDatabase(Dbo::Session& session)
  : session_(session)
{ 
  session_.mapClass<QRToken>("qr_token");
}

void QRTokenDatabase::addToken(const std::string& sessionId,
			       const std::string& hash,
			       const std::string& url)
{
  Dbo::Transaction t(session_);

  session_.add(Wt::cpp14::make_unique<QRToken>(sessionId, hash, url));
}

void QRTokenDatabase::removeToken(const std::string& sessionId)
{
  Dbo::Transaction t(session_);

  session_.execute("delete from qr_token where session_id = ?").bind(sessionId);
}

std::string QRTokenDatabase::setUser(const std::string& hash,
                                     const Auth::User& user)
{
  Dbo::Transaction t(session_);

  Dbo::ptr<QRToken> token
    = session_.find<QRToken>().where("hash = ?").bind(hash);

  if (token) {
    token.modify()->userId = user.id();
    token.modify()->hash.clear();

    return token->url;
  } else
    return std::string();
}

Auth::User QRTokenDatabase::findUser(const std::string& sessionId,
                                         Auth::AbstractUserDatabase& db)
{
  Dbo::Transaction t(session_);

    Dbo::ptr<QRToken> token
    = session_.find<QRToken>().where("session_id = ?").bind(sessionId);

  if (token) {
    return Auth::User(token->userId, db);
  } else
    return Auth::User();
}
