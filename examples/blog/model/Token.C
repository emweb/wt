/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Token.h"
#include "User.h"

#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(Token)

Token::Token()
{ }

Token::Token(const std::string& v, const Wt::WDateTime& e)
  : value(v),
    expires(e)
{ }
