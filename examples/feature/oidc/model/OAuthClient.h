#ifndef AUTH_CLIENT_H
#define AUTH_CLIENT_H

#include <string>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Auth/WAuthGlobal.h>

class OAuthClient
{
public:
  std::string clientId;
  bool confidential;
  std::string redirectUris;
  Wt::Auth::ClientSecretMethod authMethod;
  std::string secret;

  template<typename Action> void persist(Action& a) {
    Wt::Dbo::field(a, clientId, "client_id");
    Wt::Dbo::field(a, confidential, "confidential");
    Wt::Dbo::field(a, redirectUris, "redirect_uris");
    Wt::Dbo::field(a, authMethod, "auth_method");
    Wt::Dbo::field(a, secret, "secret");
  }
};

DBO_EXTERN_TEMPLATES(OAuthClient)

#endif // AUTH_CLIENT_H
