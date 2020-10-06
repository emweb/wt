/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WServer.h>

#include "HangmanGame.h"
#include "Session.h"

using namespace Wt;

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app = std::make_unique<WApplication>(env);
  
  app->setTitle("Hangman");

  app->messageResourceBundle().use(app->appRoot() + "strings");
  app->messageResourceBundle().use(app->appRoot() + "templates");

  app->useStyleSheet("css/hangman.css");

  app->root()->addWidget(std::make_unique<HangmanGame>());

  return app;
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
