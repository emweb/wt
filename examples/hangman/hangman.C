/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanApplication.h"

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new HangmanApplication(env);
}


int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
