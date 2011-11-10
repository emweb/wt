// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_LOGIN_WIDGET_H_
#define BLOG_LOGIN_WIDGET_H_

#include <Wt/Auth/AuthWidget>

class BlogSession;

/*
 * Displays login, logout and registration options
 */
class BlogLoginWidget : public Wt::Auth::AuthWidget
{
public:
  BlogLoginWidget(BlogSession& session, const std::string& basePath,
		  Wt::WContainerWidget *parent = 0);

  virtual void createLoginView();
  virtual void createLoggedInView();
};

#endif // BLOG_LOGIN_WIDGET_H_
