// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef OAUTH_AUTHORIZATIONENDPOINTPROCESS_H_
#define OAUTH_AUTHORIZATIONENDPOINTPROCESS_H_

#include <string>

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WSignal.h>

#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/OAuthClient.h>

namespace Wt {
namespace Auth {

/*! \class OAuthAuthorizationEndpointProcess Wt/Auth/OAuthAuthorizationEndpointProcess.h
 *  \brief Allows clients to authorize users according to the OAuth
 *  2.0 protocol.
 *
 * This class will process the environment and perform the
 * authorization of the user if this is possible. If this is
 * successful, an authorization code will be sent to the client.
 *
 * The following URL parameters are expected: "client_id", which
 * obviously has to contain a valid client ID. "redirect_uri", which
 * has to be a valid redirect URI where the user will be redirected to
 * when the authorization has been succesful. "scope", which has to be
 * set to the scope of the requested information. "response_type",
 * which has to be set to "code". If the "state" parameter has been
 * included, it will be passed on as a paremeter to the redirect URI.
 *
 * When the client ID and the redirect URI is valid but something else
 * went wrong, an "error=invalid_request" will be sent to the client.
 * If the user failed to log in correctly "error=login_required" will
 * be sent. If everything went OK, the "code" parameter is included
 * which can be used to obtain a token from a token endpoint.
 *
 * See https://tools.ietf.org/rfc/rfc6749.txt for more information.
 *
 * This class relies on a correct implementation of several function
 * in the AbstractUserDatabase. Namely
 * AbstractUserDatabase::idpClientFindWithId,
 * AbstractUserDatabase::idpTokenAdd, and
 * AbstractUserDatabase::idpClientRedirectUris.
 *
 * Must be deployed with TLS.
 *
 * Example:
 * \code
 * process = std::make_unique<OAuthAuthorizationEndpointProcess>(
 *     login,
 *     database);
 * process->authorized().connect(
 *     process.get(),
 *     &OAuthAuthorizationEndpointProcess::authorizeScope);
 * process->processEnvironment();
 * if (process->validRequest()) {
 *   root()->addWidget(std::move(authWidget));
 * } else
 *   root()->addWidget(std::make_unique<Wt::WText>(Wt::utf8("The request was invalid."));
 * \endcode
 *
 * \sa OAuthTokenEndpoint
 * \sa AbstractUserDatabase
 *
 */
class WT_API OAuthAuthorizationEndpointProcess : public WObject
{
public:
  /*! \brief Constructor.
   */
  OAuthAuthorizationEndpointProcess(Login& login,
                                    AbstractUserDatabase& db);

  /*! \brief Processes the environment and authorizes the user when
   * already logged in.
   *
   * The authorized() signal should be connected before calling this
   * function.
   */
  void processEnvironment();

  /*! \brief Returns true if the request was a valid OAuth request
   * with the correct parameters.
   */
  bool validRequest() const { return validRequest_; }

  /*! \brief This signal is emitted when the user has successfully
   * logged in.
   *
   * When the user has successfully logged in and the request is
   * valid, this signal will be emitted and the user can be redirected
   * to the redirect URI using authorizeScope.
   *
   * This signal supplies the scope as argument.
   *
   */
  Signal<std::string>& authorized() { return authorized_; }

  /*! \brief Authorize the given scope and redirect the user.
   *
   * If the user has successfully logged in this function will
   * redirect the user to the redirect URI with a valid "code"
   * parameter which is only valid for the given scope.
   *
   */
  void authorizeScope(const std::string& scope);

  /*! \brief Sets the amount of seconds after which generated
   * authorization codes expire.
   *
   * This defaults to 600 seconds.
   *
   */
  void setAuthCodeExpSecs(int seconds);

protected:
  AbstractUserDatabase *db_;
  void authEvent();
private:
  void sendResponse(const std::string& param);
  int authCodeExpSecs_;
  std::string redirectUri_;
  std::string state_;
  std::string scope_;
  OAuthClient client_;
  bool validRequest_;
  Login& login_;
  Signal<std::string> authorized_;
};

}
}
#endif // OAUTH_AUTHORIZATIONENDPOINTPROCESS_H_
