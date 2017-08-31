#ifndef OIDCUSERDATABASE_H_
#define OIDCUSERDATABASE_H_

#include <string>
#include <set>

#include <Wt/WDateTime.h>
#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/IssuedToken.h>
#include <Wt/Auth/OAuthClient.h>
#include <Wt/Auth/OAuthService.h>

#include "User.h"
#include "IssuedToken.h"
#include "OAuthClient.h"

class OidcUserDatabase : public Wt::Auth::Dbo::UserDatabase<Wt::Auth::Dbo::AuthInfo<User> >
{
public:
  OidcUserDatabase(Wt::Dbo::Session &session);

  virtual Wt::Json::Value idpJsonClaim(const Wt::Auth::User& user, const std::string& claim) const;

  virtual Wt::Auth::IssuedToken idpTokenAdd(const std::string& value,
                                            const Wt::WDateTime& expirationTime,
                                            const std::string& purpose,
                                            const std::string& scope,
                                            const std::string& redirectUri,
                                            const Wt::Auth::User& user,
                                            const Wt::Auth::OAuthClient& authClient);

  virtual void idpTokenRemove(const Wt::Auth::IssuedToken& token);
  virtual Wt::Auth::IssuedToken idpTokenFindWithValue(const std::string& purpose, const std::string& value) const;
  virtual Wt::WDateTime idpTokenExpirationTime(const Wt::Auth::IssuedToken& token) const;
  virtual std::string idpTokenValue(const Wt::Auth::IssuedToken& token) const;
  virtual std::string idpTokenPurpose(const Wt::Auth::IssuedToken& token) const;
  virtual std::string idpTokenScope(const Wt::Auth::IssuedToken& token) const;
  virtual std::string idpTokenRedirectUri(const Wt::Auth::IssuedToken& token) const;
  virtual Wt::Auth::User idpTokenUser(const Wt::Auth::IssuedToken& token) const;
  virtual Wt::Auth::OAuthClient idpTokenOAuthClient(const Wt::Auth::IssuedToken& token) const;

  virtual Wt::Auth::OAuthClient idpClientFindWithId(const std::string& clientId) const;
  virtual std::string idpClientSecret(const Wt::Auth::OAuthClient& client) const;
  virtual bool idpVerifySecret(const Wt::Auth::OAuthClient& client, const std::string& secret) const;
  virtual std::set<std::string> idpClientRedirectUris(const Wt::Auth::OAuthClient& client) const;
  virtual std::string idpClientId(const Wt::Auth::OAuthClient& client) const;
  virtual bool idpClientConfidential(const Wt::Auth::OAuthClient& client) const;
  virtual Wt::Auth::ClientSecretMethod idpClientAuthMethod(const Wt::Auth::OAuthClient& client) const;

  virtual Wt::Auth::OAuthClient idpClientAdd(const std::string& clientId,
                                             bool confidential,
                                             const std::set<std::string> &redirectUris,
                                             Wt::Auth::ClientSecretMethod authMethod,
                                             const std::string &secret);

private:
  Wt::Dbo::Session& session_;
  mutable Wt::Dbo::ptr<Wt::Auth::Dbo::AuthInfo<User> > user_;
  mutable Wt::Dbo::ptr<IssuedToken> issuedToken_;
  mutable Wt::Dbo::ptr<OAuthClient> authClient_;

  struct WithUser
  {
    WithUser(const OidcUserDatabase& self, const Wt::Auth::User& user);
    ~WithUser();

    Wt::Dbo::Transaction transaction;
  };

  struct WithIssuedToken
  {
    WithIssuedToken(const OidcUserDatabase& self, const Wt::Auth::IssuedToken& token);
    ~WithIssuedToken();

    Wt::Dbo::Transaction transaction;
  };

  struct WithOAuthClient
  {
    WithOAuthClient(const OidcUserDatabase& self, const Wt::Auth::OAuthClient& authClient);
    ~WithOAuthClient();

    Wt::Dbo::Transaction transaction;
  };

  void getUser(const std::string& id) const;
  void setUser(Wt::Dbo::ptr<Wt::Auth::Dbo::AuthInfo<User> > user) const;
  void getIssuedToken(const std::string& id) const;
  void getOAuthClient(const std::string& id) const;
};

#endif // OIDCUSERDATABASE_H_
