// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef AUTH_WIDGET_H_
#define AUTH_WIDGET_H_

#include <Wt/Auth/AuthWidget.h>

using namespace Wt;

class Session;

class AuthWidget : public Auth::AuthWidget
{
public:
  AuthWidget(Session& session);

  /* We will use a custom registration view */
  virtual std::unique_ptr<WWidget> createRegistrationView(const Auth::Identity& id) override;

private:
  Session& session_;
};

#endif // AUTH_WIDGET_H_
