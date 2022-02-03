/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Widget.h"

#include "Wt/Auth/Identity.h"

#include "Process.h"
#include "Service.h"

namespace Wt {
  namespace Auth {
    namespace Saml {

Widget::Widget(const Service &samlService)
  : WImage("css/saml-" + samlService.name() + ".png") {
  setToolTip(samlService.description());
  setStyleClass("Wt-auth-icon");
  setVerticalAlignment(AlignmentFlag::Middle);

  process_ = samlService.createProcess();
  clicked().connect(process_.get(), &Process::startAuthenticate);
#ifdef WT_TARGET_JAVA
  process_->connectStartAuthenticate(clicked());
#endif // WT_TARGET_JAVA

  process_->authenticated().connect(this, &Widget::samlDone);
}

void Widget::samlDone(const Identity &identity) {
  authenticated_.emit(process_.get(), identity);
}

    }
  }
}


