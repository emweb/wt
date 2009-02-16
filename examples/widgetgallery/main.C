/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include "WidgetGallery.h"

using namespace Wt;

WApplication *createApplication(const WEnvironment& env)
{
  return new WidgetGallery(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
