#include <Wt/WApplication>
#include <Wt/WText>

#include "DragExample.h"

using namespace Wt;

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  app->setTitle("Drag & drop");

  app->root()->setStyleClass("root");

  new WText("<h1>Wt Drag &amp; drop example.</h1>", app->root());

  new DragExample(app->root());

  app->useStyleSheet("dragdrop.css");

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

