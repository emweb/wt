// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_LOST_PASSWORD_WIDGET_H_
#define WT_AUTH_LOST_PASSWORD_WIDGET_H_

#include <Wt/WTemplate.h>

namespace Wt {
  namespace Auth {

class AbstractUserDatabase;
class AuthService;

/*! \class LostPasswordWidget Auth/LostPasswordWidget
 *  \brief A widget which initiates a lost-password email.
 *
 * The widget renders the <tt>"Wt.Auth.template.lost-password"</tt>
 * template. It prompts for an email address and then invokes
 * AuthService::lostPassword() with the given email address.
 *
 * \sa AuthWidget::createLostPasswordView()
 *
 * \ingroup auth
 */ 
class WT_API LostPasswordWidget : public WTemplate
{
public:
  /*! \brief Constructor
   */
  LostPasswordWidget(AbstractUserDatabase& users,
		     const AuthService& auth);

protected:
  void send();
  void cancel();

private:
  AbstractUserDatabase& users_;
  const AuthService& baseAuth_;

  static void deleteBox(Wt::WMessageBox *box);
};

  }
}

#endif // WT_AUTH_LOST_PASSWORD_WIDGET_H_
