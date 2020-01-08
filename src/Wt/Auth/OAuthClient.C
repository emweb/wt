/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "OAuthClient.h"

#include <string>
#include <set>

#include "Wt/WException.h"

namespace {
  const char *INVALID_ERROR = "Wt::Auth::OAuthClient invalid";
}

namespace Wt {
namespace Auth {

OAuthClient::OAuthClient()
  : db_(0)
{
}

OAuthClient::OAuthClient(const std::string& id, const AbstractUserDatabase& db)
  : db_(const_cast<AbstractUserDatabase *>(&db)),
    id_(id)
{
}

bool OAuthClient::checkValid() const
{
  return db_;
}

std::string OAuthClient::id() const
{
  return id_;
}

std::string OAuthClient::clientId() const
{
  if (db_)
    return db_->idpClientId(*this);
  else
    throw WException(INVALID_ERROR);
}

bool OAuthClient::verifySecret(const std::string &secret) const
{
  if (db_)
    return db_->idpVerifySecret(*this, secret);
  else
    throw WException(INVALID_ERROR);
}

std::set<std::string> OAuthClient::redirectUris() const
{
  if (db_)
    return db_->idpClientRedirectUris(*this);
  else
    throw WException(INVALID_ERROR);
}

bool OAuthClient::confidential() const
{
  if (db_)
    return db_->idpClientConfidential(*this);
  else
    throw WException(INVALID_ERROR);
}

ClientSecretMethod OAuthClient::authMethod() const
{
  if (db_)
    return db_->idpClientAuthMethod(*this);
  else
    throw WException(INVALID_ERROR);
}

}
}
