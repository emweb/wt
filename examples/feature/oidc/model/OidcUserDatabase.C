#include "OidcUserDatabase.h"

#include <string>
#include <set>

#include <Wt/WDateTime>
#include <Wt/Auth/User>
#include <Wt/Auth/OAuthClient>
#include <Wt/Dbo/Dbo>
#include <Wt/Auth/HashFunction>
#include <Wt/WRandom>
#include <boost/algorithm/string.hpp>

#include "IssuedToken.h"
#include "OAuthClient.h"
#include "User.h"

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
  IssuedToken *token = new IssuedToken(value, expirationTime, purpose, scope, redirectUri, user_->user(), authClient_);
  issuedToken_ = session_.add(token);
  return Wt::Auth::IssuedToken(boost::lexical_cast<std::string>(issuedToken_.id()), *this);
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
    return Wt::Auth::IssuedToken(boost::lexical_cast<std::string>(issuedToken_.id()), *this);
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
  return Wt::Auth::User(boost::lexical_cast<std::string>(issuedToken_->user.id()), *this);
}

Wt::Auth::OAuthClient OidcUserDatabase::idpTokenOAuthClient(const Wt::Auth::IssuedToken& token) const
{
  WithIssuedToken find(*this, token);
  return Wt::Auth::OAuthClient(boost::lexical_cast<std::string>(issuedToken_->authClient.id()), *this);
}

Wt::Auth::OAuthClient OidcUserDatabase::idpClientFindWithId(const std::string& clientId) const
{
  Wt::Dbo::Transaction t(session_);
  authClient_ = session_.find<OAuthClient>()
      .where("client_id = ?").bind(clientId);
  t.commit();
  if (authClient_)
    return Wt::Auth::OAuthClient(boost::lexical_cast<std::string>(authClient_.id()), *this);
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
  if (!user_ || boost::lexical_cast<std::string>(user_.id()) != id) {
    Wt::Dbo::Transaction t(session_);
    setUser(session_.load<Wt::Auth::Dbo::AuthInfo<User> >(boost::lexical_cast<long long>(id)));
    t.commit();
  }
}

void OidcUserDatabase::setUser(Wt::Dbo::ptr<Wt::Auth::Dbo::AuthInfo<User> > user) const
{
  user_ = user;
}

void OidcUserDatabase::getIssuedToken(const std::string& id) const
{
  if (!issuedToken_ || boost::lexical_cast<std::string>(issuedToken_->id()) != id) {
    Wt::Dbo::Transaction t(session_);
    issuedToken_ = session_.find<IssuedToken>()
        .where("id = ?").bind(boost::lexical_cast<long long>(id));
    t.commit();
  }
}

void OidcUserDatabase::getOAuthClient(const std::string& id) const
{
  if (!authClient_ || boost::lexical_cast<std::string>(authClient_.id()) != id) {
    Wt::Dbo::Transaction t(session_);
    authClient_ = session_.find<OAuthClient>()
        .where("id = ?").bind(boost::lexical_cast<long long>(id));
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
  OAuthClient *client = new OAuthClient;
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
  Wt::Dbo::ptr<OAuthClient> client_ = session_.add(client);
  t.commit();
  return Wt::Auth::OAuthClient(boost::lexical_cast<std::string>(client_.id()),
                               *this);
}
