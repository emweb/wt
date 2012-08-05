// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef QR_AUTH_WIDGET
#define QR_AUTH_WIDGET

#include <Wt/Auth/AuthWidget>

class QRAuthService;
class QRTokenDatabase;

class QRAuthWidget : public Wt::Auth::AuthWidget
{
public:
  QRAuthWidget(Wt::Auth::Login& login, Wt::WContainerWidget *parent = 0);

  void configureQRAuth(const QRAuthService& service, QRTokenDatabase& database);

  virtual void processEnvironment();
  virtual void createLoginView();

private:
  const QRAuthService *qrService_;
  QRTokenDatabase *qrDatabase_;

  std::string qrToken_;
  Wt::WDialog *dialog_;

  void confirmRemoteLogin();
  void doRemoteLogin(Wt::StandardButton button);
  void showQRDialog();
  void dialogDone();
};

#endif // QR_AUTH_WIDGET
