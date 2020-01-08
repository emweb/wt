/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "OAuthAuthorizationEndpointProcess.h"

#include <string>
#include <set>

#include "Wt/Utils.h"
#include "Wt/WText.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WRandom.h"
#include "Wt/WDateTime.h"
#include "Wt/Utils.h"
#include "Wt/WContainerWidget.h"
#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/PasswordVerifier.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/OAuthClient.h"
#include "Wt/WLogger.h"

#define ERROR_MSG(e) WString::tr("Wt.Auth.OAuthAuthorizationEndpointProcess." e)

namespace Wt {

LOGGER("Auth.OAuthAuthorizationEndpointProcess");

namespace Auth {

OAuthAuthorizationEndpointProcess::OAuthAuthorizationEndpointProcess(
    Login& login,
    AbstractUserDatabase& db)
  : db_(&db),
    authCodeExpSecs_(600),
    validRequest_(false),
    login_(login)
{
}

void OAuthAuthorizationEndpointProcess::processEnvironment()
{
  const WEnvironment& env = WApplication::instance()->environment();
  const std::string *redirectUri = env.getParameter("redirect_uri");
  if (!redirectUri) {
    LOG_ERROR("The client application did not pass a redirection URI.");
    return;
  }
  redirectUri_ = *redirectUri;

  const std::string *clientId = env.getParameter("client_id");
  if (!clientId) {
    LOG_ERROR("Missing client_id parameter.");
    return;
  }
  client_ = db_->idpClientFindWithId(*clientId);
  if (!client_.checkValid()) {
    LOG_ERROR("Unknown or invalid client_id " << *clientId);
    return;
  }
  std::set<std::string> redirectUris = client_.redirectUris();
  if (redirectUris.find(redirectUri_) == redirectUris.end()) {
    LOG_ERROR("The client application passed the  unregistered redirection URI " << redirectUri_);
    return;
  }
  const std::string *scope = env.getParameter("scope");
  const std::string *responseType = env.getParameter("response_type");
  const std::string *state  = env.getParameter("state");
  if (!scope || !responseType || *responseType != "code") {
    sendResponse("error=invalid_request");
    LOG_INFO("error=invalid_request: "
      << " scope: " << (scope ? *scope : "NULL")
      << " response_type: " << (responseType ? *responseType : "NULL")
    );
    return;
  }
  validRequest_ = true;
  scope_ = *scope;

  if (state)
    state_ = *state;

  login_.changed().connect(this, &OAuthAuthorizationEndpointProcess::authEvent);
  const std::string *prompt = WApplication::instance()->environment().getParameter("prompt");
  if (login_.loggedIn()) {
    authorized_.emit(scope_);
    return;
  } else if (prompt && *prompt == "none") {
    sendResponse("error=login_required");
    LOG_INFO("error=login_required but prompt == none");
    return;
  }
}

void OAuthAuthorizationEndpointProcess::authorizeScope(const std::string& scope)
{
  if (validRequest_ && login_.loggedIn()) {
    std::string authCodeValue = WRandom::generateId();
    WDateTime expirationTime = WDateTime::currentDateTime().addSecs(authCodeExpSecs_);
    db_->idpTokenAdd(authCodeValue, expirationTime, "authorization_code", scope,
        redirectUri_, login_.user(), client_);
    sendResponse("code=" + authCodeValue);
    LOG_INFO("authorization_code created for " << login_.user().id() << "(" << login_.user().email() << ")"
	     << ", code = " + authCodeValue);
  } else {
    throw WException("Wt::Auth::OAuthAuthorizationEndpointProcess::authorizeScope: request isn't valid");
  }
}

void OAuthAuthorizationEndpointProcess::authEvent()
{
  if (login_.loggedIn()) {
    authorized_.emit(scope_);
  } else {
    sendResponse("error=login_required");
  }
}

void OAuthAuthorizationEndpointProcess::sendResponse(const std::string& param)
{
  std::string redirectParam = redirectUri_.find("?") != std::string::npos ? "&" : "?";
  redirectParam += param;
  if (!state_.empty())
    redirectParam += "&state=" + Utils::urlEncode(state_);
  WApplication::instance()->redirect(redirectUri_ + redirectParam);
  WApplication::instance()->quit();
}

void OAuthAuthorizationEndpointProcess::setAuthCodeExpSecs(int seconds)
{
  authCodeExpSecs_ = seconds;
}

}
}
