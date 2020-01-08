/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PlannerApplication.h"

#include <cstdlib>

using namespace Wt;
using namespace Wt::Dbo;

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<PlannerApplication>(env);
}

int main(int argc, char **argv)
{
  std::srand(std::time(0));

  return WRun(argc, argv, &createApplication);
}

