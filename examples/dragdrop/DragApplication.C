#include <Wt/WApplication.h>
#include <Wt/WText.h>

#include "DragExample.h"

using namespace Wt;

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  std::unique_ptr<WApplication> app
      = std::make_unique<WApplication>(env);
  app->setTitle("Drag & drop");

  app->root()->setStyleClass("root");

  app->root()->addWidget(std::make_unique<WText>("<h1>Wt Drag &amp; drop example.</h1>"));

  app->root()->addWidget(std::make_unique<DragExample>());

  app->useStyleSheet("dragdrop.css");

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

