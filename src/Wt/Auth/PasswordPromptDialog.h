// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_PASSWORD_PROMPT_DIALOG_H_
#define WT_AUTH_PASSWORD_PROMPT_DIALOG_H_

#include <Wt/WDialog.h>

namespace Wt {

class WPushButton;
class WTemplateFormView;

  namespace Auth {

class AuthModel;
class Login;

/*! \class PasswordPromptDialog Wt/Auth/PasswordPromptDialog.h
 *  \brief A dialog that prompts for the user password.
 *
 * This is a simple dialog, useful for prompting the user to enter his
 * password. This may be convenient for example to let the user upgrade
 * from a weak authentication to a strong authentication.
 *
 * The dialog uses a Login object to get the currently identified
 * user, and also sets the result of the login process by calling
 * Login::login() on this object.
 *
 * The dialog renders the <tt>"Wt.Auth.template.password-prompt"</tt>
 * template.
 *
 * \ingroup auth
 */
class WT_API PasswordPromptDialog : public WDialog
{
public:
  /*! \brief Constructor.
   *
   * From the passed \p login object, the dialog obtains the User for
   * which a valid password needs to be entered. The result, if successful,
   * is signalled using Login::login().
   */
  PasswordPromptDialog(Login& login, const std::shared_ptr<AuthModel>& model);

protected:
  Login& login_;
  std::shared_ptr<AuthModel> model_;

  WTemplateFormView *impl_;

  void check();
};

  }
}

#endif // WT_AUTH_PASSWORD_PROMPT_DIALOG_H_
