/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "QRAuthWidget.h"
#include "model/QRAuthService.h"
#include "model/QRTokenDatabase.h"

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/Utils>
#include <Wt/Auth/Login>

const std::string GOOGLE_QR_CHART_URL
  = "http://chart.apis.google.com/chart?cht=qr&chs=300x300&chld=H|0&&chl=";

QRAuthWidget::QRAuthWidget(Wt::Auth::Login& login, Wt::WContainerWidget *parent)
  : Wt::Auth::AuthWidget(login, parent),
    dialog_(0)
{ }

void QRAuthWidget::configureQRAuth(const QRAuthService& service,
				   QRTokenDatabase& database)
{
  qrService_ = &service;
  qrDatabase_ = &database;
}

void QRAuthWidget::processEnvironment()
{
  Wt::Auth::AuthWidget::processEnvironment();

  const Wt::WEnvironment& env = Wt::WApplication::instance()->environment();

  qrToken_ = qrService_->parseQRToken(env);

  if (!qrToken_.empty()) {
    if (login().loggedIn())
      confirmRemoteLogin();
    else
      login().changed().connect(this, &QRAuthWidget::confirmRemoteLogin);
  }
}

void QRAuthWidget::confirmRemoteLogin()
{
  if (!qrToken_.empty() && login().loggedIn()) {
    Wt::WMessageBox *mb
      = new Wt::WMessageBox("Remote login",
			    "<p>Do you want to login the desktop user too ?</p>"
			    "<p><b>WARNING !</b><br/>"
			    "You should only do this if you arrived here "
			    "by scanning a QR code.</p>",
			    Wt::NoIcon, Wt::Yes | Wt::No);
    mb->animateShow
      (Wt::WAnimation(Wt::WAnimation::Pop | Wt::WAnimation::Fade,
		      Wt::WAnimation::Linear, 300));
    mb->setWidth("70%");
    mb->buttonClicked().connect(this, &QRAuthWidget::doRemoteLogin);
    dialog_ = mb;
  }
}

void QRAuthWidget::doRemoteLogin(Wt::StandardButton button)
{
  if (button == Wt::Yes)
    qrService_->remoteLogin(*qrDatabase_, login().user(), qrToken_);

  qrToken_.clear();

  delete dialog_;
  dialog_ = 0;
}

void QRAuthWidget::createLoginView()
{
  Wt::Auth::AuthWidget::createLoginView();

  Wt::WImage *button = new Wt::WImage("css/QRcode.png");
  button->setToolTip("Sign in using your mobile phone");
  button->setVerticalAlignment(Wt::AlignMiddle);
  button->clicked().connect(this, &QRAuthWidget::showQRDialog);

  bindWidget("qrauth", button);
}

void QRAuthWidget::showQRDialog()
{
  delete dialog_;
  dialog_ = new Wt::WDialog("Sign in with your mobile phone.");
  dialog_->animateShow
    (Wt::WAnimation(Wt::WAnimation::Pop | Wt::WAnimation::Fade,
		    Wt::WAnimation::Linear, 300));
  dialog_->contents()->setContentAlignment(Wt::AlignCenter);

  Wt::WResource *resource
    = qrService_->createLoginResource(*qrDatabase_, model()->users(),
				      login(), dialog_);
  std::string qrToken = qrService_->createQRToken(*qrDatabase_, resource);

  Wt::WApplication *app = Wt::WApplication::instance();

  std::string qrUrl = app->bookmarkUrl("/");
  if (qrUrl.find("?") != std::string::npos)
    qrUrl += "&";
  else
    qrUrl += "?";

  qrUrl += qrService_->redirectParameter()
    + "=" + Wt::Utils::urlEncode(qrToken);

  qrUrl = app->makeAbsoluteUrl(qrUrl);

  new Wt::WText("Use the barcode scanner to scan the QR code below.",
		dialog_->contents());

  Wt::WImage *image = new Wt::WImage(GOOGLE_QR_CHART_URL
				     + Wt::Utils::urlEncode(qrUrl),
				     dialog_->contents());
  image->resize(300, 300);
  image->setMargin(20);
  image->setInline(false);

  Wt::WPushButton *cancel = new Wt::WPushButton("Cancel", dialog_->contents());
  cancel->clicked().connect(dialog_, &Wt::WDialog::reject);

  dialog_->finished().connect(this, &QRAuthWidget::dialogDone);
  login().changed().connect(this, &QRAuthWidget::dialogDone);

  app->enableUpdates(true);
}

void QRAuthWidget::dialogDone()
{
  if (dialog_) {
    delete dialog_;
    dialog_ = 0;

    Wt::WApplication *app = Wt::WApplication::instance();
    qrDatabase_->removeToken(app->sessionId());

    app->triggerUpdate();
    app->enableUpdates(false);
  }
}
