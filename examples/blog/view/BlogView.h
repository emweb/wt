// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_VIEW_H_
#define BLOG_VIEW_H_

#include <Wt/WCompositeWidget>

namespace Wt {
  class WContainerWidget;
}

class BlogImpl;

class BlogView : public Wt::WCompositeWidget
{
public:
  BlogView(const std::string& basePath, const std::string& sqliteDb,
	   const std::string& rssFeedUrl, Wt::WContainerWidget *parent = 0);

  void setInternalBasePath(const std::string& basePath);

  Wt::WString user();
  void login(const std::string& user);
  void logout();

  Wt::Signal<Wt::WString>& userChanged() { return userChanged_; }

private:
  BlogImpl *impl_;
  Wt::Signal<Wt::WString> userChanged_;
};

#endif // BLOG_VIEW_H_
