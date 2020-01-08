/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "UserDetailsModel.h"
#include "User.h"
#include "Session.h"


const WFormModel::Field
UserDetailsModel::FavouritePetField = "favourite-pet";

UserDetailsModel::UserDetailsModel(Session& session)
  : WFormModel(),
    session_(session)
{
  addField(FavouritePetField, WString::tr("favourite-pet-info"));
}

void UserDetailsModel::save(const Auth::User& authUser)
{
  Dbo::ptr<User> user = session_.user(authUser);
  user.modify()->favouritePet = valueText(FavouritePetField).toUTF8();
}
