/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTemplate.h>
#include <Wt/WContainerWidget.h>

#include "WidgetFunction.h"

using namespace Wt;

WidgetFunction widgetFunction; 

std::unique_ptr<WWidget> createLineEdit(const std::vector<WString>& args)
{
  return std::make_unique<WLineEdit>();
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app = std::make_unique<WApplication>(env);

  WTemplate *t =
      app->root()->addWidget(std::make_unique<WTemplate>("${widget:line-edit}"));
  t->addFunction("widget", widgetFunction);
  return app;
}

int main(int argc, char **argv)
{
  widgetFunction.registerType("line-edit", createLineEdit);

  return WRun(argc, argv, &createApplication);
}
