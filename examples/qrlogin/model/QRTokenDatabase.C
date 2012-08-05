/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "QRTokenDatabase.h"

#include <Wt/Auth/User>
#include <Wt/Dbo/Dbo>

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
    Wt::Dbo::field(a, sessionId, "session_id");
    Wt::Dbo::field(a, hash, "hash");
    Wt::Dbo::field(a, url, "url");
    Wt::Dbo::field(a, userId, "user_id");
  }
};

QRTokenDatabase::QRTokenDatabase(Wt::Dbo::Session& session)
  : session_(session)
{ 
  session_.mapClass<QRToken>("qr_token");
}

void QRTokenDatabase::addToken(const std::string& sessionId,
			       const std::string& hash,
			       const std::string& url)
{
  Wt::Dbo::Transaction t(session_);

  session_.add(new QRToken(sessionId, hash, url));
}

void QRTokenDatabase::removeToken(const std::string& sessionId)
{
  Wt::Dbo::Transaction t(session_);

  session_.execute("delete from qr_token where session_id = ?").bind(sessionId);
}

std::string QRTokenDatabase::setUser(const std::string& hash,
				     const Wt::Auth::User& user)
{
  Wt::Dbo::Transaction t(session_);

  Wt::Dbo::ptr<QRToken> token
    = session_.find<QRToken>().where("hash = ?").bind(hash);

  if (token) {
    token.modify()->userId = user.id();
    token.modify()->hash.clear();

    return token->url;
  } else
    return std::string();
}

Wt::Auth::User QRTokenDatabase::findUser(const std::string& sessionId,
					 Wt::Auth::AbstractUserDatabase& db)
{
  Wt::Dbo::Transaction t(session_);

    Wt::Dbo::ptr<QRToken> token
    = session_.find<QRToken>().where("session_id = ?").bind(sessionId);

  if (token) {
    return Wt::Auth::User(token->userId, db);
  } else
    return Wt::Auth::User();
}
