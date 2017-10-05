#ifndef USER_H
#define USER_H

#include <string>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

class User;
typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;

class User
{
public:
  std::string name;
  std::string streetAddress;
  std::string locality;
  Wt::Dbo::weak_ptr<AuthInfo> authInfo;

  template<typename Action> void persist(Action& a)
  {
    Wt::Dbo::field(a, name, "name");
    Wt::Dbo::field(a, streetAddress, "street_address");
    Wt::Dbo::field(a, locality, "locality");
    Wt::Dbo::hasOne(a, authInfo, "user");
  }
};

DBO_EXTERN_TEMPLATES(User)

#endif // USER_H
