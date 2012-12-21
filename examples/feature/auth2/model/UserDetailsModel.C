/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "UserDetailsModel.h"
#include "User.h"
#include "Session.h"

const Wt::WFormModel::Field
UserDetailsModel::FavouritePetField = "favourite-pet";

UserDetailsModel::UserDetailsModel(Session& session, Wt::WObject *parent)
  : Wt::WFormModel(parent),
    session_(session)
{
  addField(FavouritePetField, Wt::WString::tr("favourite-pet-info"));
}

void UserDetailsModel::save(const Wt::Auth::User& authUser)
{
  Wt::Dbo::ptr<User> user = session_.user(authUser);
  user.modify()->favouritePet = valueText(FavouritePetField).toUTF8();
}
