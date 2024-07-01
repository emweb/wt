#pragma once

#include "myuser.h"

#include "Wt/Dbo/Dbo.h"

class AuthEntry : public Wt::Dbo::Dbo<AuthEntry>
{
public:
  AuthEntry() = default;
  AuthEntry(const Wt::Dbo::ptr<AuthInfo>& authInfo, const std::string& host, const std::string& userAgent, const std::string& language);

  const std::string& host() const { return host_; }
  const std::string& userAgent() const { return userAgent_; }
  const std::string& language() const { return language_; }

  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::belongsTo(a, authInfo_, "auth_info");

    Wt::Dbo::field(a, host_, "host");
    Wt::Dbo::field(a, userAgent_, "userAgent");
    Wt::Dbo::field(a, language_, "language");
  }

private:
  Wt::Dbo::ptr<AuthInfo> authInfo_;

  std::string host_;
  std::string userAgent_;
  std::string language_;
};
