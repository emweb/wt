/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WEnvironment>
#include <Wt/WContainerWidget>

#include "CoderApplication.h"
#include "CoderWidget.h"
#include "ObserverWidget.h"

using namespace Wt;

CoderApplication::CoderApplication(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("Watch that coding.");
  useStyleSheet("coder.css");

  createUI(env.internalPath());

  internalPathChanged().connect(this, &CoderApplication::handlePathChange);
}

void CoderApplication::createUI(const std::string& path)
{
  if (path.length() <= 1)
    root()->addWidget(new CoderWidget());
  else
    root()->addWidget(new ObserverWidget(path.substr(1)));
}

void CoderApplication::handlePathChange()
{
  root()->clear();

  createUI(internalPath());
}

WApplication *createApplication(const WEnvironment& env)
{
  return new CoderApplication(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
