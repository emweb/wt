/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WInPlaceEdit>

using namespace Wt;

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);

  new WText("Name: ", app->root());
  WInPlaceEdit *edit = new WInPlaceEdit("Bob Smith", app->root());
  edit->setStyleClass("inplace");
  edit->setEmptyText("Empty: click to edit");

  app->styleSheet().addRule("*.inplace span:hover", "background-color: gray");

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
