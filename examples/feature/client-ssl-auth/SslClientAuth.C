/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WServer.h>
#include <Wt/WEnvironment.h>
#include <Wt/WSslInfo.h>
#include <Wt/WText.h>
#include "Wt/Utils.h"

#include <Wt/Auth/AuthModel.h>
#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/AbstractUserDatabase.h>

#include "model/Session.h"

using namespace Wt;

namespace {
  Auth::Identity createIdentity(const WSslInfo* sslInfo)
  {
    std::string name;
    auto clientSubjectDn = sslInfo->clientCertificate().subjectDn();
    for (auto &dn : clientSubjectDn) {
      if (dn.name() == WSslCertificate::CommonName) {
	name = dn.value();
	break;
      }
    }
    
    std::string der = sslInfo->clientCertificate().toDer();
    return Auth::Identity("CLIENT_SSL", Utils::hexEncode(Utils::sha1(der)),
			      name,
			      "",
			      false);
  }
}

class AuthApplication : public WApplication
{
public:
  AuthApplication(const WEnvironment& env)
    : WApplication(env),
      session_(appRoot() + "auth.db")
  {
    session_.login().changed().connect(this, &AuthApplication::authEvent);

    useStyleSheet("css/style.css");

    std::unique_ptr<Auth::AuthWidget> authWidget(new Auth::AuthWidget(
                          Session::auth(), session_.users(), session_.login()));

    authWidget->setRegistrationEnabled(true);

    WSslInfo *sslInfo = env.sslInfo();
    if (sslInfo) {
      Auth::Identity id = createIdentity(sslInfo);
      Auth::User u = session_.users().findWithIdentity(id.provider(),
							   id.id());
      if (!u.isValid()) 
	authWidget->registerNewUser(id);
      else
        session_.login().login(u, Auth::LoginState::Weak);

      root()->addWidget(std::move(authWidget));
    } else {
      root()->addWidget(std::make_unique<WText>("Not an SSL session, or no "
          "client certificate available. Please read the readme file in "
          "examples/feature/client-ssl-auth for more info."));
      quit();
    }

  }

  void authEvent() {
    if (session_.login().loggedIn())
      log("notice") << "User " << session_.login().user().id()
			<< " logged in.";
    else {
      log("notice") << "User logged out.";
      root()->clear();
      root()->addWidget(std::make_unique<WText>("You are logged out"));
      quit();
    }
  }

private:
  Session session_;
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<AuthApplication>(env);
}

int main(int argc, char **argv)
{
  try {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(EntryPointType::Application, createApplication);

    Session::configureAuth();

    server.run();
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
