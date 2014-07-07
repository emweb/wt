/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WServer>
#include <Wt/Dbo/SqlConnectionPool>

#include "BlogRSSFeed.h"
#include "model/BlogSession.h"
#include "model/Token.h"
#include "model/User.h"
#include "WtHome.h"
#include "JWtHome.h"

int main(int argc, char **argv)
{
  try {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);

    BlogSession::configureAuth();

    Wt::Dbo::SqlConnectionPool *blogDb
      = BlogSession::createConnectionPool(server.appRoot() + "blog.db");

    BlogRSSFeed rssFeed(*blogDb, "Wt and JWt blog",
			"http://www.webtoolkit.eu/wt/blog",
			"We care about our webtoolkits.");

    server.addResource(&rssFeed, "/wt/blog/feed/");

    server.addEntryPoint(Application,
			 boost::bind(&createJWtHomeApplication, _1, blogDb),
			 "/jwt", "/css/jwt/favicon.ico");
    server.addEntryPoint(Application,
			 boost::bind(&createWtHomeApplication, _1, blogDb),
			 "", "/css/wt/favicon.ico");

    server.run();

    delete blogDb;
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
