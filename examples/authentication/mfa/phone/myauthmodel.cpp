#include "myauthmodel.h"

#include "Wt/Dbo/Transaction.h"
#include "authentry.h"

MyAuthModel::MyAuthModel(MySession& session)
  : Wt::Auth::AuthModel(session.auth(), session.users()),
    session_(session)
{
}

bool MyAuthModel::hasMfaStep(const Wt::Auth::User& user) const
{
  Wt::Dbo::Transaction t(session_);
  Wt::Dbo::Query<bool> result = session_.query<bool>("select requires_mfa from user join auth_info on auth_info.user_id = user.id where auth_info.id = ?")
                                .bind(user.id());
  return result.resultValue();
}
