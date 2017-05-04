#include "IssuedToken.h"

#include <string>
#include "Wt/WDateTime.h"
#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/User.h"
#include "Wt/Auth/OAuthClient.h"

namespace {
  const char *INVALID_ERROR = "Wt::Auth::IssuedToken invalid";
}

namespace Wt {
namespace Auth {

IssuedToken::IssuedToken()
  : db_(0)
{
}

IssuedToken::IssuedToken(const std::string& id, const AbstractUserDatabase& userDatabase)
  : id_(id),
    db_(const_cast<AbstractUserDatabase *>(&userDatabase))
{
}

bool IssuedToken::checkValid() const
{
  return db_;
}

const std::string IssuedToken::value() const
{
  if (db_)
    return db_->idpTokenValue(*this);
  else
    throw WException(INVALID_ERROR);
}

const WDateTime IssuedToken::expirationTime() const
{
  if (db_)
    return db_->idpTokenExpirationTime(*this);
  else
    throw WException(INVALID_ERROR);
}

const std::string IssuedToken::purpose() const
{
  if (db_)
    return db_->idpTokenPurpose(*this);
  else
    throw WException(INVALID_ERROR);
}

const std::string IssuedToken::scope() const
{
  if (db_)
    return db_->idpTokenScope(*this);
  else
    throw WException(INVALID_ERROR);
}

const std::string IssuedToken::redirectUri() const
{
  if (db_)
    return db_->idpTokenRedirectUri(*this);
  else
    throw WException(INVALID_ERROR);
}

const User IssuedToken::user() const
{
  if (db_)
    return db_->idpTokenUser(*this);
  else
    throw WException(INVALID_ERROR);
}

const OAuthClient IssuedToken::authClient() const
{
  if (db_)
    return db_->idpTokenOAuthClient(*this);
  else
    throw WException(INVALID_ERROR);
}

}
}
