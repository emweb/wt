/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "User.h"

#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(User);

using namespace Wt::Dbo;

User::User()
  : gamesPlayed(0),
    score(0)
{ }
