/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WServer.h>
#include <Wt/Dbo/SqlConnectionPool.h>

#include "model/BlogSession.h"
#include "model/Token.h"
#include "model/User.h"
#include "view/BlogView.h"
#include "BlogRSSFeed.h"

static const char *FeedUrl = "/blog/feed/";
static const char *BlogUrl = "/blog";

class BlogApplication : public Wt::WApplication
{
public:
  BlogApplication(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& blogDb)
    : Wt::WApplication(env)
  {
    root()->addWidget(std::make_unique<BlogView>("/", blogDb, FeedUrl));
    useStyleSheet("css/blogexample.css");
  }
};

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env,
                                Wt::Dbo::SqlConnectionPool *blogDb)
{
  return std::make_unique<BlogApplication>(env, *blogDb);
}

int main(int argc, char **argv)
{
  try {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    BlogSession::configureAuth();

    std::unique_ptr<Wt::Dbo::SqlConnectionPool> blogDb
      = BlogSession::createConnectionPool(server.appRoot() + "blog.db");

    BlogRSSFeed rssFeed(*blogDb, "Wt blog example", "", "It's just an example.");

    server.addResource(&rssFeed, FeedUrl);
    //When the blog application is deployed in ISAPI on the path "/blog"
    //the resources (css+images) are not fetched correctly
    server.addEntryPoint(Wt::EntryPointType::Application,
                         std::bind(&createApplication, std::placeholders::_1, blogDb.get()), BlogUrl);

    std::cerr << "\n\n -- Warning: Example is deployed at '"
      << BlogUrl << "'\n\n";

    server.run();

  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}

