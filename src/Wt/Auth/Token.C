/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Token.h"

namespace Wt {
  namespace Auth {

Token::Token()
{ }

Token::Token(const std::string& hash, const WDateTime& expirationTime)
  : hash_(hash),
    expirationTime_(expirationTime)
{ }

  }
}
