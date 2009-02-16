// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include <vector>
#include <utility>

#include "ControlsWidget.h"

namespace Wt {
  class WFormWidget;
  class WText;
}

class Validators : public ControlsWidget
{
public:
  Validators(EventDisplayer *ed);

private:
  void validateServerside();
  std::vector<std::pair<Wt::WFormWidget *, Wt::WText *> > fields_;
};

#endif

