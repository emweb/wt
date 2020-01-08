// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_VIEW_H_
#define BLOG_VIEW_H_

#include <Wt/WCompositeWidget.h>

namespace Wt {
  class WContainerWidget;
}

class BlogImpl;

class BlogView : public Wt::WCompositeWidget
{
public:
  BlogView(const std::string& basePath, Wt::Dbo::SqlConnectionPool& db,
           const std::string& rssFeedUrl);

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
