#ifndef ISSUED_TOKEN_H
#define ISSUED_TOKEN_H

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDateTime.h>

#include "User.h"
#include "OAuthClient.h"

class IssuedToken : public Wt::Dbo::Dbo<IssuedToken>
{
public:
  IssuedToken();
  IssuedToken(std::string value,
              Wt::WDateTime expires,
              std::string purpose,
              std::string scope,
              std::string redirectUri,
              Wt::Dbo::ptr<User> user,
              Wt::Dbo::ptr<OAuthClient> authClient);

  std::string   value;
  Wt::WDateTime expires;
  std::string   purpose;
  std::string   scope;
  std::string   redirectUri;
  Wt::Dbo::ptr<User> user;
  Wt::Dbo::ptr<OAuthClient> authClient;

  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, value, "value");
    Wt::Dbo::field(a, expires, "expires");
    Wt::Dbo::field(a, purpose, "purpose");
    Wt::Dbo::field(a, scope, "scope");
    Wt::Dbo::field(a, redirectUri, "redirect_uri");
    Wt::Dbo::belongsTo(a, user, "user");
    Wt::Dbo::belongsTo(a, authClient, "auth_client");
  }
};

DBO_EXTERN_TEMPLATES(IssuedToken)

#endif // ISSUED_TOKEN_H
