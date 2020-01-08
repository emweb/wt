/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/Identity.h"

namespace Wt {
  namespace Auth {

Identity::Identity()
{ }

Identity::Identity(const std::string& provider,
		   const std::string& id, const WT_USTRING& name,
		   const std::string& email, bool emailVerified)
  : provider_(provider),
    id_(id),
    email_(email),
    name_(name),
    emailVerified_(emailVerified)
{ }

const Identity Identity::Invalid;

const std::string Identity::LoginName = "loginname";

  }
}
