// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_DETAILS_MODEL_H_
#define USER_DETAILS_MODEL_H_

#include <Wt/WFormModel.h>

using namespace Wt;

class Session;

class UserDetailsModel : public WFormModel
{
public:
  static const Field FavouritePetField;

  UserDetailsModel(Session& session);

  void save(const Auth::User& user);

private:
  Session& session_;
};

#endif // USER_DETAILS_MODEL_H_
