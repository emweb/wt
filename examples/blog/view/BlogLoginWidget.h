// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_LOGIN_WIDGET_H_
#define BLOG_LOGIN_WIDGET_H_

#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/RegistrationModel.h>

class BlogSession;

/*
 * Displays login, logout and registration options
 */
class BlogLoginWidget : public Wt::Auth::AuthWidget
{
public:
  BlogLoginWidget(BlogSession& session, const std::string& basePath);

  virtual void createLoginView() override;
  virtual void createLoggedInView() override;
};

#endif // BLOG_LOGIN_WIDGET_H_
