/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WText>
#include <Wt/WServer>

using namespace Wt;

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  app->setTitle("Multiple servers @ " + env.hostName());
  app->root()->addWidget(new WText("Well hello there"));

  return app;
}

int main(int, char **)
{
  try {
    int argc = 5;
    char** argv1 = new char*[argc];

    argv1[0] = (char *) "multiple";
    argv1[1] = (char *) "--http-address=0.0.0.0";
    argv1[2] = (char *) "--http-port=8080";
    argv1[3] = (char *) "--deploy-path=/";
    argv1[4] = (char *) "--docroot=.";

    WServer server1(argc, argv1, WTHTTP_CONFIGURATION);

    char** argv2 = new char*[argc];

    argv2[0] = (char *) "multiple";
    argv2[1] = (char *) "--http-address=0.0.0.0";
    argv2[2] = (char *) "--http-port=7070";
    argv2[3] = (char *) "--deploy-path=/";
    argv2[4] = (char *) "--docroot=.";

    WServer server2(argc, argv2, WTHTTP_CONFIGURATION);

    server1.addEntryPoint(Application, createApplication);
    server2.addEntryPoint(Application, createApplication);

    if (server1.start()) {
      if (server2.start()) {
	WServer::waitForShutdown();
	server2.stop();
      }
      server1.stop();
    }
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
