/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WServer>

#include "WtHome.h"

int main(int argc, char **argv)
{
  WServer server(argv[0]);

  server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

  server.addEntryPoint(Application, createWtHomeApplication,
		       "", "/css/wt/favicon.ico");

  if (server.start()) {
    WServer::waitForShutdown();
    server.stop();
  }
}
