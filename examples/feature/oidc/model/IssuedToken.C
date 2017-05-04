#include "IssuedToken.h"

#include <string>
#include <Wt/WDateTime>
#include <Wt/Dbo/Impl>
#include <Wt/Dbo/WtSqlTraits>

#include "User.h"
#include "OAuthClient.h"

IssuedToken::IssuedToken()
{
}

IssuedToken::IssuedToken(std::string value,
                         Wt::WDateTime expires,
                         std::string purpose,
                         std::string scope,
                         std::string redirectUri,
                         Wt::Dbo::ptr<User> user,
                         Wt::Dbo::ptr<OAuthClient> authClient)
  : value(value),
    expires(expires),
    purpose(purpose),
    scope(scope),
    redirectUri(redirectUri),
    user(user),
    authClient(authClient)
{
}

DBO_INSTANTIATE_TEMPLATES(IssuedToken)
