// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_REGISTRATION_WIDGET_H_
#define WT_AUTH_REGISTRATION_WIDGET_H_

#include <Wt/WTemplateFormView.h>
#include <Wt/Auth/RegistrationModel.h>

namespace Wt {
  namespace Auth {

class AuthWidget;
class Login;
class OAuthProcess;

/*! \class RegistrationWidget Wt/Auth/RegistrationWidget.h
 *  \brief A registration widget.
 *
 * This implements a widget which allows a new user to register.  The
 * widget renders the <tt>"Wt.Auth.template.registration"</tt>
 * template. and uses a RegistrationModel for the actual registration
 * logic.
 *
 * Typically, you may want to specialize this widget to ask for other
 * information.
 *
 * \ingroup auth
 */
class WT_API RegistrationWidget : public WTemplateFormView
{
public:
  /*! \brief Constructor
   *
   * Creates a new authentication.
   */
  RegistrationWidget(AuthWidget *authWidget = nullptr);

  /*! \brief Sets the registration model.
   */
  void setModel(std::unique_ptr<RegistrationModel> model);

  /*! \brief Returns the registration model.
   *
   * This returns the model that is used by the widget to do the actual
   * registration.
   */
  RegistrationModel *model() const { return model_.get(); }

  /*! \brief Updates the user-interface.
   *
   * This updates the user-interface to reflect the current state of the
   * model.
   */
  virtual void update();

protected:
  /*! \brief Validates the current information.
   *
   * The default implementation simply calls
   * RegistrationModel::validate() on the model.
   *
   * You may want to reimplement this method if you've added other
   * information to the registration form that need validation.
   */
  virtual bool validate();

  /*! \brief Performs the registration.
   *
   * The default implementation checks if the information is valid
   * with validate(), and then calls
   * RegistrationModel::doRegister(). If registration was successful,
   * it calls registerUserDetails() and subsequently logs the user in.
   */
  virtual void doRegister();

  /*! \brief Closes the registration widget.
   *
   * The default implementation simply deletes the widget.
   */
  virtual void close();

  /*! \brief Registers more user information.
   *
   * This method is called when a new user has been successfully
   * registered.
   *
   * You may want to reimplement this method if you've added other
   * information to the registration form which needs to be annotated
   * to the user.
   */
  virtual void registerUserDetails(User& user);

  virtual void render(WFlags<RenderFlag> flags) override;

protected:
  virtual std::unique_ptr<WWidget> createFormWidget
    (RegistrationModel::Field field) override;

private:
  AuthWidget *authWidget_;
  std::unique_ptr<RegistrationModel> model_;

  bool created_;
  std::unique_ptr<Login> confirmPasswordLogin_;

  void checkLoginName();
  void checkPassword();
  void checkPassword2();
  void confirmIsYou();
  void confirmedIsYou();
  void oAuthDone(OAuthProcess *oauth, const Identity& identity);
};

  }
}

#endif // WT_AUTH_REGISTRATION_WIDGET_H_
