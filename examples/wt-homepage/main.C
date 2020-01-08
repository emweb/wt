/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WServer.h>
#include <Wt/Dbo/SqlConnectionPool.h>

#include "BlogRSSFeed.h"
#include "model/BlogSession.h"
#include "model/Token.h"
#include "model/User.h"
#include "WtHome.h"
#include "JWtHome.h"

using namespace Wt;

int main(int argc, char **argv)
{
  try {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);

    BlogSession::configureAuth();

    std::unique_ptr<Dbo::SqlConnectionPool> blogDb
      = BlogSession::createConnectionPool(server.appRoot() + "blog.db");

    BlogRSSFeed rssFeed(*blogDb, "Wt and JWt blog",
			"http://www.webtoolkit.eu/wt/blog",
			"We care about our webtoolkits.");

    server.addResource(&rssFeed, "/wt/blog/feed/");

    server.addEntryPoint(EntryPointType::Application,
                         std::bind(&createJWtHomeApplication, std::placeholders::_1, blogDb.get()),
                         "/jwt", "/css/jwt/favicon.ico");
    server.addEntryPoint(EntryPointType::Application,
                         std::bind(&createWtHomeApplication, std::placeholders::_1, blogDb.get()),
                         "", "/css/wt/favicon.ico");

    server.run();

  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
