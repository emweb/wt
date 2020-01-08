// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef QR_AUTH_WIDGET
#define QR_AUTH_WIDGET

#include <Wt/Auth/AuthWidget.h>

using namespace Wt;

class QRAuthService;
class QRTokenDatabase;

class QRAuthWidget : public Auth::AuthWidget
{
public:
  QRAuthWidget(Auth::Login& login);

  void configureQRAuth(const QRAuthService& service, QRTokenDatabase& database);

  virtual void processEnvironment();
  virtual void createLoginView();

private:
  const QRAuthService      *qrService_;
  QRTokenDatabase          *qrDatabase_;

  std::string               qrToken_;
  std::unique_ptr<WDialog>  dialog_;

  void confirmRemoteLogin();
  void doRemoteLogin(StandardButton button);
  void showQRDialog();
  void dialogDone();
};

#endif // QR_AUTH_WIDGET
