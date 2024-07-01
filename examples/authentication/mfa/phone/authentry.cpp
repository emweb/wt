#include "authentry.h"

#include "Wt/Dbo/Impl.h"

AuthEntry::AuthEntry(const Wt::Dbo::ptr<AuthInfo>& authInfo, const std::string& host, const std::string& userAgent, const std::string& language)
  : authInfo_(authInfo),
    host_(host),
    userAgent_(userAgent),
    language_(language)
{
}
