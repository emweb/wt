/*
 * Copyright (C) 2009 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WServer>

#include "view/BlogView.h"
#include "BlogRSSFeed.h"

using namespace Wt;

static const char *FeedUrl = "/blog/feed/";
static const char *BlogUrl = "/blog";

class BlogApplication : public WApplication
{
public:
  BlogApplication(const WEnvironment& env) 
    : WApplication(env)
  {
    root()->addWidget(new BlogView("/",
      WApplication::appRoot() + "blog.db", FeedUrl));
    useStyleSheet("css/blogexample.css");
  }
};

WApplication *createApplication(const WEnvironment& env)
{
  return new BlogApplication(env);
}

int main(int argc, char **argv)
{
  try {
    WServer server(argv[0]);

    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

    BlogRSSFeed rssFeed(server.appRoot() + "blog.db", "Wt blog example",
      "", "It's just an example.");

    server.addResource(&rssFeed, FeedUrl);
    server.addEntryPoint(Application, createApplication, BlogUrl);

    std::cerr << "\n\n -- Warning: Example is deployed at '"
      << BlogUrl << "'\n\n";

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

