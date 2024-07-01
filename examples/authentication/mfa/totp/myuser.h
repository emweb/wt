#pragma once

#include <Wt/Auth/Dbo/AuthInfo.h>

class MyUser;

using AuthInfo = Wt::Auth::Dbo::AuthInfo<MyUser>;

class MyUser : public Wt::Dbo::Dbo<MyUser> {
public:
  MyUser() = default;
  explicit MyUser(const std::string& name);

  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, name_, "name");
    Wt::Dbo::hasOne(a, m_authInfo, "user");
  }

private:
  std::string name_;
  Wt::Dbo::weak_ptr<AuthInfo> m_authInfo;

};

DBO_EXTERN_TEMPLATES(MyUser)
