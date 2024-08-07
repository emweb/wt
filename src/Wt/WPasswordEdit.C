/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DomElement.h"

#include "Wt/WPasswordEdit.h"

namespace Wt {

WPasswordEdit::WPasswordEdit()
  : WLineEdit()
{
  setEchoMode(EchoMode::Password);
}

WPasswordEdit::WPasswordEdit(const WT_USTRING& content)
  : WPasswordEdit()
{
  setText(content);
}


void WPasswordEdit::updateDom(DomElement& element, bool all)
{
  if (all) {
    element.setAttribute("type", "password");
  }

  WLineEdit::updateDom(element, all);
}

}