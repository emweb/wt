/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WServer>

#include "BlogRSSFeed.h"
#include "WtHome.h"
#include "JWtHome.h"

int main(int argc, char **argv)
{
  try {
    WServer server(argv[0]);

    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

    BlogRSSFeed rssFeed(server.appRoot() + "blog.db", "Wt and JWt blog",
      "http://www.webtoolkit.eu/wt/blog",
      "We care about our webtoolkits.");

    server.addResource(&rssFeed, "/wt/blog/feed/");

    server.addEntryPoint(Application, createJWtHomeApplication,
      "/jwt", "/css/jwt/favicon.ico");
    server.addEntryPoint(Application, createWtHomeApplication,
      "", "/css/wt/favicon.ico");

    if (server.start()) {
      WServer::waitForShutdown();
      server.stop();
    }
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
