#include "qrcodehandler.h"

#include "authentry.h"
#include "phonewidget.h"

QrCodeHandler::QrCodeHandler(MySession& session, PhoneWidget* phoneWidget)
  : session_(session),
    phoneWidget_(phoneWidget)
{
}

void QrCodeHandler::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
  std::string userAgent = request.headerValue("User-Agent");

  if (userAgent.find("Mobile") == std::string::npos) {
    response.setStatus(400);
    return;
  }

  const Wt::Auth::User& user = session_.login().user();

  if (!user.isValid()) {
    response.setStatus(400);
    return;
  }

  std::string query = request.queryString();
  bool isSetup = query.find("issetup") != std::string::npos;

  std::string host = request.headerValue("Host");
  std::string language = request.headerValue("Accept-Language");

  Wt::Dbo::Transaction t(session_);
  const Wt::Dbo::ptr<AuthInfo>& authInfo = session_.find<AuthInfo>("where id = ?").bind(user.id());
  Wt::Dbo::ptr<AuthEntry> entry = session_.find<AuthEntry>("where auth_info_id = ?").bind(authInfo);

  if (!entry && !isSetup) {
    response.setStatus(400);
    return;
  } else if (!entry && isSetup) {
    session_.addNew<AuthEntry>(authInfo, host, userAgent, language);
    phoneWidget_->setUpUserIdentity();
    response.setStatus(200);
    response.out() << "An entry was created.";
    allowSignin_.emit();
    return;
  } else if (entry && isSetup) {
    response.setStatus(400);
    response.out() << "An entry already exists. Cannot set up a new entry.";
    return;
  }

  if (entry->host() == host && entry->userAgent() == userAgent && entry->language() == language) {
    response.setStatus(200);
    response.out() << "Validation has been accepted.";
    allowSignin_.emit();
  } else {
    response.setStatus(400);
    response.out() << "Validation has been denied.";
  }
}
