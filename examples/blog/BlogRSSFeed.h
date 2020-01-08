// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_RSS_FEED_H_
#define BLOG_RSS_FEED_H_

#include <Wt/WResource.h>

class BlogSession;

class BlogRSSFeed : public Wt::WResource
{
public:
  BlogRSSFeed(Wt::Dbo::SqlConnectionPool& connectionPool,
	      const std::string& title,
	      const std::string& url,
	      const std::string& description);
  virtual ~BlogRSSFeed();

protected:
  virtual void handleRequest(const Wt::Http::Request &request,
                             Wt::Http::Response &response);

private:
  Wt::Dbo::SqlConnectionPool& connectionPool_;
  std::string title_, url_, description_;
};

#endif // BLOG_RSS_FEED_H_
