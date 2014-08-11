/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WLineEdit>
#include <Wt/WTemplate>

#include "WidgetFunction.h"

WidgetFunction widgetFunction; 

Wt::WWidget *createLineEdit(const std::vector<Wt::WString>& args)
{
  return new Wt::WLineEdit();
}

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  Wt::WApplication *app = new Wt::WApplication(env);

  Wt::WTemplate *t = new Wt::WTemplate("${widget:line-edit}", app->root());
  t->addFunction("widget", widgetFunction);
  return app;
}

int main(int argc, char **argv)
{
  widgetFunction.registerType("line-edit", createLineEdit);

  return WRun(argc, argv, &createApplication);
}
