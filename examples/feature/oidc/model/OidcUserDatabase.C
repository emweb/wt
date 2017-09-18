#include "OidcUserDatabase.h"

#include <string>
#include <set>

#include <Wt/WDateTime.h>
#include <Wt/Auth/User.h>
#include <Wt/Auth/OAuthClient.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/WRandom.h>
#include <boost/algorithm/string.hpp>

#include "IssuedToken.h"
#include "OAuthClient.h"
#include "User.h"

namespace {
  long long stringToId(const std::string &s)
  {
    std::size_t pos = std::string::npos;
    auto result = std::stoll(s, &pos);
    if (pos != s.size())
      return -1;
    else
      return result;
  }
}

class InvalidObject : public std::runtime_error
{
public:
  InvalidObject(const std::string& obj, const std::string& id)
    : std::runtime_error("Invalid " + obj + ": " + id)
  { }
};

OidcUserDatabase::OidcUserDatabase(Wt::Dbo::Session& session)
  : UserDatabase(session),
    session_(session)
{
}

Wt::Json::Value OidcUserDatabase::idpJsonClaim(const Wt::Auth::User &user, const std::string &claim) const
{
  WithUser find(*this, user);
  if (claim == "name")
    return Wt::Json::Value(user_->user()->name);
  if (claim == "email") {
    if (user.email().empty())
      return Wt::Json::Value(user.unverifiedEmail());
    else
      return Wt::Json::Value(user.email());
  }
  if (claim == "email_verified") {
    if (user.email().empty())
      return Wt::Json::Value::False;
    else
      return Wt::Json::Value::True;
  }
  return Wt::Json::Value::Null;
}

Wt::Auth::IssuedToken OidcUserDatabase::idpTokenAdd(const std::string& value,
                                                    const Wt::WDateTime& expirationTime,
                                                    const std::string& purpose,
                                                    const std::string& scope,
                                                    const std::string& redirectUri,
                                                    const Wt::Auth::User& user,
                                                    const Wt::Auth::OAuthClient& authClient)
{
  WithUser findUser(*this, user);
  WithOAuthClient findOAuthClient(*this, authClient);
  issuedToken_ =
    session_.addNew<IssuedToken>(value, expirationTime, purpose, scope, redirectUri, user_->user(), authClient_);
  return Wt::Auth::IssuedToken(std::to_string(issuedToken_.id()), *this);
}

void OidcUserDatabase::idpTokenRemove(const Wt::Auth::IssuedToken& token)
{
  WithIssuedToken(*this, token);
  issuedToken_.remove();
}

Wt::Auth::IssuedToken OidcUserDatabase::idpTokenFindWithValue(const std::string& purpose, const std::string& value) const
{
  Wt::Dbo::Transaction t(session_);
  Wt::WDateTime cur = Wt::WDateTime::currentDateTime();
  std::cout << cur.toString() << std::endl;
  issuedToken_ = session_.find<IssuedToken>()
      .where("value = ?").bind(value)
      .where("purpose = ?").bind(purpose)
      .where("expires > ?").bind(cur);
  t.commit();
  if (issuedToken_)
    return Wt::Auth::IssuedToken(std::to_string(issuedToken_.id()), *this);
  else
    return Wt::Auth::IssuedToken();
}

Wt::WDateTime OidcUserDatabase::idpTokenExpirationTime(const Wt::Auth::IssuedToken& token) const
{
  WithIssuedToken find(*this, token);
  return issuedToken_->expires;
}

std::string OidcUserDatabase::idpTokenValue(const Wt::Auth::IssuedToken& token) const
{
  WithIssuedToken find(*this, token);
  return issuedToken_->value;
}

std::string OidcUserDatabase::idpTokenScope(const Wt::Auth::IssuedToken& token) const
{
  WithIssuedToken find(*this, token);
  return issuedToken_->scope;
}

std::string OidcUserDatabase::idpTokenPurpose(const Wt::Auth::IssuedToken& token) const
{
  WithIssuedToken find(*this, token);
  return issuedToken_->purpose;
}

std::string OidcUserDatabase::idpTokenRedirectUri(const Wt::Auth::IssuedToken& token) const
{
  WithIssuedToken find(*this, token);
  return issuedToken_->redirectUri;
}

Wt::Auth::User OidcUserDatabase::idpTokenUser(const Wt::Auth::IssuedToken& token) const
{
  WithIssuedToken find(*this, token);
  return Wt::Auth::User(std::to_string(issuedToken_->user.id()), *this);
}

Wt::Auth::OAuthClient OidcUserDatabase::idpTokenOAuthClient(const Wt::Auth::IssuedToken& token) const
{
  WithIssuedToken find(*this, token);
  return Wt::Auth::OAuthClient(std::to_string(issuedToken_->authClient.id()), *this);
}

Wt::Auth::OAuthClient OidcUserDatabase::idpClientFindWithId(const std::string& clientId) const
{
  Wt::Dbo::Transaction t(session_);
  authClient_ = session_.find<OAuthClient>()
      .where("client_id = ?").bind(clientId);
  t.commit();
  if (authClient_)
    return Wt::Auth::OAuthClient(std::to_string(authClient_.id()), *this);
  else
    return Wt::Auth::OAuthClient();
}

std::string OidcUserDatabase::idpClientSecret(const Wt::Auth::OAuthClient& client) const
{
  WithOAuthClient find(*this, client);
  return authClient_->secret;
}

bool OidcUserDatabase::idpVerifySecret(const Wt::Auth::OAuthClient& client,
                                       const std::string& secret) const
{
  return Wt::Auth::BCryptHashFunction(7).verify(secret,
                                                "",
                                                idpClientSecret(client));
}

std::set<std::string> OidcUserDatabase::idpClientRedirectUris(const Wt::Auth::OAuthClient& client) const
{
  WithOAuthClient find(*this, client);
  std::set<std::string> result;
  boost::split(result, authClient_->redirectUris, boost::is_any_of(" "));
  return result;
}

std::string OidcUserDatabase::idpClientId(const Wt::Auth::OAuthClient& client) const
{
  WithOAuthClient find(*this, client);
  return authClient_->clientId;
}

bool OidcUserDatabase::idpClientConfidential(const Wt::Auth::OAuthClient& client) const
{
  WithOAuthClient find(*this, client);
  return authClient_->confidential;
}

Wt::Auth::ClientSecretMethod OidcUserDatabase::idpClientAuthMethod(const Wt::Auth::OAuthClient &client) const
{
  WithOAuthClient find(*this, client);
  return authClient_->authMethod;
}

void OidcUserDatabase::getUser(const std::string& id) const
{
  if (!user_ || std::to_string(user_.id()) != id) {
    Wt::Dbo::Transaction t(session_);
    setUser(session_.load<Wt::Auth::Dbo::AuthInfo<User> >(stringToId(id)));
    t.commit();
  }
}

void OidcUserDatabase::setUser(Wt::Dbo::ptr<Wt::Auth::Dbo::AuthInfo<User> > user) const
{
  user_ = user;
}

void OidcUserDatabase::getIssuedToken(const std::string& id) const
{
  if (!issuedToken_ || std::to_string(issuedToken_->id()) != id) {
    Wt::Dbo::Transaction t(session_);
    issuedToken_ = session_.find<IssuedToken>()
        .where("id = ?").bind(stringToId(id));
    t.commit();
  }
}

void OidcUserDatabase::getOAuthClient(const std::string& id) const
{
  if (!authClient_ || std::to_string(authClient_.id()) != id) {
    Wt::Dbo::Transaction t(session_);
    authClient_ = session_.find<OAuthClient>()
        .where("id = ?").bind(stringToId(id));
    t.commit();
  }
}

OidcUserDatabase::WithUser::WithUser(const OidcUserDatabase& self,
                                     const Wt::Auth::User& user)
  : transaction(self.session_)
{
  self.getUser(user.id());

  if (!self.user_)
    throw InvalidObject("user", user.id());
}

OidcUserDatabase::WithUser::~WithUser()
{
  transaction.commit();
}

OidcUserDatabase::WithIssuedToken::WithIssuedToken(const OidcUserDatabase& self,
                                                   const Wt::Auth::IssuedToken& token)
  : transaction(self.session_)
{
  self.getIssuedToken(token.id());

  if (!self.issuedToken_)
    throw InvalidObject("token", token.id());
}

OidcUserDatabase::WithIssuedToken::~WithIssuedToken()
{
  transaction.commit();
}

OidcUserDatabase::WithOAuthClient::WithOAuthClient(const OidcUserDatabase& self,
                                                 const Wt::Auth::OAuthClient& authClient)
  : transaction(self.session_)
{
  self.getIssuedToken(authClient.id());

  if (!self.authClient_)
    throw InvalidObject("auth_client", authClient.id());
}

OidcUserDatabase::WithOAuthClient::~WithOAuthClient()
{
  transaction.commit();
}

Wt::Auth::OAuthClient OidcUserDatabase::idpClientAdd(const std::string &clientId,
                                                    bool confidential,
                                                    const std::set<std::string> &redirectUris,
                                                    Wt::Auth::ClientSecretMethod authMethod,
                                                    const std::string &secret)
{
  Wt::Dbo::Transaction t(session_);
  std::unique_ptr<OAuthClient> client{new OAuthClient};
  client->clientId = clientId;
  client->confidential = confidential;
  std::set<std::string>::iterator uri = redirectUris.begin();
  std::string uris = *(uri++);
  for (; uri != redirectUris.end();++uri) {
    uris = uris + " " + *uri;
  }
  client->redirectUris = uris;
  client->authMethod = authMethod;
  client->secret = Wt::Auth::BCryptHashFunction(7)
    .compute(secret,
             Wt::WRandom::generateId());
  Wt::Dbo::ptr<OAuthClient> client_ = session_.add(std::move(client));
  t.commit();
  return Wt::Auth::OAuthClient(std::to_string(client_.id()), *this);
}
