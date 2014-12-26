/*
 * Copyright (C) 2009 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WServer>
#include <Wt/Dbo/SqlConnectionPool>

#include "model/BlogSession.h"
#include "model/Token.h"
#include "model/User.h"
#include "view/BlogView.h"
#include "BlogRSSFeed.h"

using namespace Wt;

//static const char *FeedUrl = "/Test/blog/feed/";
//static const char *BlogUrl = "/Test/blog";

static const char *FeedUrl = "/blog/feed/";
static const char *BlogUrl = "/blog";

class BlogApplication : public WApplication
{
public:
  BlogApplication(const WEnvironment& env, Wt::Dbo::SqlConnectionPool& blogDb) 
    : WApplication(env)
  {
    root()->addWidget(new BlogView("/", blogDb, FeedUrl));
    useStyleSheet("css/blogexample.css");
  }
};

WApplication *createApplication(const WEnvironment& env,
				Wt::Dbo::SqlConnectionPool *blogDb)
{
  return new BlogApplication(env, *blogDb);
}

int main(int argc, char **argv)
{
  try {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);

    BlogSession::configureAuth();

    Wt::Dbo::SqlConnectionPool *blogDb
      = BlogSession::createConnectionPool(server.appRoot() + "blog.db");

    BlogRSSFeed rssFeed(*blogDb, "Wt blog example", "", "It's just an example.");

    server.addResource(&rssFeed, FeedUrl);
    //When the blog application is deployed in ISAPI on the path "/blog"
    //the resources (css+images) are not fetched correctly
    server.addEntryPoint(Application,
			 boost::bind(&createApplication, _1, blogDb), BlogUrl);    

    std::cerr << "\n\n -- Warning: Example is deployed at '"
      << BlogUrl << "'\n\n";

    server.run();

    delete blogDb;
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}

