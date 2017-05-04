#include "UserDetailsModel.h"
#include "User.h"
#include "Session.h"

const Wt::WFormModel::Field UserDetailsModel::NameField = "name";

UserDetailsModel::UserDetailsModel(Session& session)
  : session_(session)
{
  addField(NameField, Wt::WString::tr("name-info"));
}

void UserDetailsModel::save(const Wt::Auth::User& authUser)
{
  Wt::Dbo::ptr<User> user = session_.user(authUser);
  user.modify()->name = valueText(NameField).toUTF8();
}
