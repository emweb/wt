/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Form/WFormDelegate.h"

#include <Wt/WLineEdit.h>

namespace Wt {
  namespace Form {
WFormDelegate<Wt::WString, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<Wt::WString, void>::createFormWidget()
{
  return std::make_unique<Wt::WLineEdit>();
}
  }
}
