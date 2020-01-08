/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/OAuthService.h"
#include "Wt/Auth/OAuthWidget.h"

namespace Wt {
  namespace Auth {

OAuthWidget::OAuthWidget(const OAuthService& oAuthService)
  : WImage("css/oauth-" + oAuthService.name() + ".png")
{
  setToolTip(oAuthService.description());
  setStyleClass("Wt-auth-icon");
  setVerticalAlignment(AlignmentFlag::Middle);

  process_ = oAuthService.createProcess(oAuthService.authenticationScope());
  clicked().connect(process_.get(), &OAuthProcess::startAuthenticate);

  process_->authenticated().connect(this, &OAuthWidget::oAuthDone);
}

void OAuthWidget::oAuthDone(const Identity& identity)
{
  authenticated_.emit(process_.get(), identity);
}

  }
}
