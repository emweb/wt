/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "QRAuthWidget.h"
#include "model/QRAuthService.h"
#include "model/QRTokenDatabase.h"

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WImage.h>
#include <Wt/WText.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/Utils.h>
#include <Wt/Auth/Login.h>
#include <Wt/WResource.h>

const std::string GOOGLE_QR_CHART_URL
  = "http://chart.apis.google.com/chart?cht=qr&chs=300x300&chld=H|0&&chl=";

QRAuthWidget::QRAuthWidget(Auth::Login& login)
  : Auth::AuthWidget(login),
    dialog_(nullptr)
{ }

void QRAuthWidget::configureQRAuth(const QRAuthService& service,
				   QRTokenDatabase& database)
{
  qrService_ = &service;
  qrDatabase_ = &database;
}

void QRAuthWidget::processEnvironment()
{
  Auth::AuthWidget::processEnvironment();

  const WEnvironment& env = WApplication::instance()->environment();

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
    auto mb
      = cpp14::make_unique<WMessageBox>("Remote login",
			    "<p>Do you want to login the desktop user too ?</p>"
			    "<p><b>WARNING !</b><br/>"
			    "You should only do this if you arrived here "
			    "by scanning a QR code.</p>",
			    Icon::None, StandardButton::Yes | StandardButton::No);
    mb->animateShow
      (WAnimation(AnimationEffect::Pop | AnimationEffect::Fade,
                      TimingFunction::Linear, 300));
    mb->setWidth("70%");
    mb->buttonClicked().connect(this, &QRAuthWidget::doRemoteLogin);
    dialog_ = std::move(mb);
  }
}

void QRAuthWidget::doRemoteLogin(StandardButton button)
{
  if (button == StandardButton::Yes)
    qrService_->remoteLogin(*qrDatabase_, login().user(), qrToken_);

  qrToken_.clear();

  dialog_.reset();
}

void QRAuthWidget::createLoginView()
{
  Auth::AuthWidget::createLoginView();

  auto button = cpp14::make_unique<WImage>("css/QRcode.png");
  button->setToolTip("Sign in using your mobile phone");
  button->setVerticalAlignment(AlignmentFlag::Middle);
  button->clicked().connect(this, &QRAuthWidget::showQRDialog);

  bindWidget("qrauth", std::move(button));
}

void QRAuthWidget::showQRDialog()
{
  dialog_ = cpp14::make_unique<WDialog>("Sign in with your mobile phone.");
  dialog_->animateShow
    (WAnimation(AnimationEffect::Pop | AnimationEffect::Fade,
                    TimingFunction::Linear, 300));
  dialog_->contents()->setContentAlignment(AlignmentFlag::Center);

  std::unique_ptr<WResource> resource
    = qrService_->createLoginResource(*qrDatabase_, model()->users(),
                                      login());
  std::string qrToken = qrService_->createQRToken(*qrDatabase_, resource.get());
  dialog_->addChild(std::move(resource));

  WApplication *app = Wt::WApplication::instance();

  std::string qrUrl = app->bookmarkUrl("/");
  if (qrUrl.find("?") != std::string::npos)
    qrUrl += "&";
  else
    qrUrl += "?";

  qrUrl += qrService_->redirectParameter()
    + "=" + Utils::urlEncode(qrToken);

  qrUrl = app->makeAbsoluteUrl(qrUrl);

  dialog_->contents()->addWidget(cpp14::make_unique<WText>("Use the barcode scanner to scan the QR code below."));

  auto image = dialog_->contents()->addWidget(cpp14::make_unique<WImage>(GOOGLE_QR_CHART_URL
                                     + Utils::urlEncode(qrUrl)));
  image->resize(300, 300);
  image->setMargin(20);
  image->setInline(false);

  auto cancel = dialog_->contents()->addWidget(cpp14::make_unique<WPushButton>("Cancel"));
  cancel->clicked().connect(dialog_.get(), &WDialog::reject);

  dialog_->finished().connect(this, &QRAuthWidget::dialogDone);
  login().changed().connect(this, &QRAuthWidget::dialogDone);

  app->enableUpdates(true);
}

void QRAuthWidget::dialogDone()
{
  if (dialog_) {
    dialog_.reset();

    WApplication *app = WApplication::instance();
    qrDatabase_->removeToken(app->sessionId());

    app->triggerUpdate();
    app->enableUpdates(false);
  }
}
