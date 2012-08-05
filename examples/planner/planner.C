/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PlannerApplication.h"

#include <stdlib.h>

using namespace Wt;
using namespace Wt::Dbo;

WApplication *createApplication(const WEnvironment& env) 
{
  return new PlannerApplication(env);
}

int main(int argc, char **argv)
{
  srand(time(0));

  return WRun(argc, argv, &createApplication);
}

