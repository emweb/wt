/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WText.h>
#include <Wt/WServer.h>

using namespace Wt;

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app = cpp14::make_unique<WApplication>(env);
  app->setTitle("Multiple servers @ " + env.hostName());
  app->root()->addWidget(cpp14::make_unique<WText>("Well hello there"));

  return app;
}

int main(int, char **)
{
  try {
    int argc = 5;
    auto argv1 = std::unique_ptr<char*[]>(new char*[argc]);

    argv1[0] = (char *) "multiple";
    argv1[1] = (char *) "--http-address=0.0.0.0";
    argv1[2] = (char *) "--http-port=8080";
    argv1[3] = (char *) "--deploy-path=/";
    argv1[4] = (char *) "--docroot=.";

    WServer server1(argc, argv1.get(), WTHTTP_CONFIGURATION);

    auto argv2 = std::unique_ptr<char*[]>(new char*[argc]);

    argv2[0] = (char *) "multiple";
    argv2[1] = (char *) "--http-address=0.0.0.0";
    argv2[2] = (char *) "--http-port=7070";
    argv2[3] = (char *) "--deploy-path=/";
    argv2[4] = (char *) "--docroot=.";

    WServer server2(argc, argv2.get(), WTHTTP_CONFIGURATION);

    server1.addEntryPoint(EntryPointType::Application, createApplication);
    server2.addEntryPoint(EntryPointType::Application, createApplication);

    if (server1.start()) {
      if (server2.start()) {
	WServer::waitForShutdown();
	server2.stop();
      }
      server1.stop();
    }
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
