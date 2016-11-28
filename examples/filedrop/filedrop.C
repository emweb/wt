/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "FileDropApplication.h"

using namespace Wt;

WApplication *createApplication(const WEnvironment& env)
{
  return new FileDropApplication(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

