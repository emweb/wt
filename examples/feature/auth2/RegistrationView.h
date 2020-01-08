// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef REGISTRATION_VIEW_H_
#define REGISTRATION_VIEW_H_

#include <Wt/Auth/RegistrationWidget.h>

using namespace Wt;

class Session;
class UserDetailsModel;

class RegistrationView : public Auth::RegistrationWidget
{
public:
  RegistrationView(Session& session, Auth::AuthWidget *authWidget = 0);

  /* specialize to create user details fields */
  virtual std::unique_ptr<WWidget> createFormWidget(WFormModel::Field field);

protected:
  /* specialize to also validate the user details */
  virtual bool validate();

  /* specialize to register user details */
  virtual void registerUserDetails(Auth::User& user);

private:
  Session& session_;

  std::unique_ptr<UserDetailsModel> detailsModel_;
};

#endif // REGISTRATION_VIEW_H_
