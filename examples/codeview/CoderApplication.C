/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WEnvironment.h>
#include <Wt/WContainerWidget.h>

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
    root()->addWidget(cpp14::make_unique<CoderWidget>());
  else
    root()->addWidget(cpp14::make_unique<ObserverWidget>(path.substr(1)));
}

void CoderApplication::handlePathChange()
{
  root()->clear();

  createUI(internalPath());
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<CoderApplication>(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
