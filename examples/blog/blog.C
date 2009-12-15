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

static const char *FeedUrl = "/feed/";

class BlogApplication : public WApplication
{
public:
  BlogApplication(const WEnvironment& env) 
    : WApplication(env)
  {
    root()->addWidget(new BlogView("/", "blog.db", FeedUrl));
    useStyleSheet("css/blogexample.css");
  }
};

WApplication *createApplication(const WEnvironment& env)
{
  return new BlogApplication(env);
}

int main(int argc, char **argv)
{
  WServer server(argv[0]);

  server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

  BlogRSSFeed rssFeed("blog.db", "Wt blog example",
		      "http://localhost:8080/",
		      "It's just an example.");

  server.addResource(&rssFeed, FeedUrl);
  server.addEntryPoint(Application, createApplication);

  if (server.start()) {
    WServer::waitForShutdown();
    server.stop();
  }
}

