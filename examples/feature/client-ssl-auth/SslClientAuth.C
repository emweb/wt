/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WServer>
#include <Wt/WEnvironment>
#include <Wt/WSslInfo>
#include <Wt/WText>
#include "Wt/Utils"

#include <Wt/Auth/AuthModel>
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/Identity>
#include <Wt/Auth/AbstractUserDatabase>

#include "model/Session.h"

namespace {
  Wt::Auth::Identity createIdentity(const Wt::WSslInfo* sslInfo)
  {
    std::string name;
    std::vector<Wt::WSslCertificate::DnAttribute> clientSubjectDn
      = sslInfo->clientCertificate().subjectDn();
    for (unsigned i = 0; i < clientSubjectDn.size(); ++i) {
      if (clientSubjectDn[i].name() == Wt::WSslCertificate::CommonName) {
	name = clientSubjectDn[i].value();
	break;
      }
    }
    
    std::string der = sslInfo->clientCertificate().toDer();
    return Wt::Auth::Identity("CLIENT_SSL", 
			      Wt::Utils::hexEncode(Wt::Utils::sha1(der)), 
			      name, 
			      "",  
			      false);
  }
}

class AuthApplication : public Wt::WApplication
{
public:
  AuthApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env),
      session_(appRoot() + "auth.db")
  {
    session_.login().changed().connect(this, &AuthApplication::authEvent);

    useStyleSheet("css/style.css");

    Wt::Auth::AuthWidget *authWidget
      = new Wt::Auth::AuthWidget(Session::auth(), session_.users(),
				 session_.login());

    authWidget->setRegistrationEnabled(true);

    Wt::WSslInfo *sslInfo = env.sslInfo();
    if (sslInfo) {
      Wt::Auth::Identity id = createIdentity(sslInfo);
      Wt::Auth::User u = session_.users().findWithIdentity(id.provider(), 
							   id.id());
      if (!u.isValid()) 
	authWidget->registerNewUser(id);
      else
	session_.login().login(u, Wt::Auth::WeakLogin);

      root()->addWidget(authWidget);
    } else {
      new Wt::WText("Not an SSL session, or no client certificate available. "
          "Please read the readme file in examples/feature/client-ssl-auth "
          "for more info.",
          root());
      quit();
    }

  }

  void authEvent() {
    if (session_.login().loggedIn())
      Wt::log("notice") << "User " << session_.login().user().id()
			<< " logged in.";
    else {
      Wt::log("notice") << "User logged out.";
      root()->clear();
      new Wt::WText("You are logged out", root());
      quit();
    }
  }

private:
  Session session_;
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new AuthApplication(env);
}

int main(int argc, char **argv)
{
  try {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::Application, createApplication);

    Session::configureAuth();

    server.run();
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
