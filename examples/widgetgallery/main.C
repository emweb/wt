/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WHBoxLayout>

#include "WidgetGallery.h"

using namespace Wt;

WApplication *createApplication(const WEnvironment& env)
{
  WApplication* app = new WApplication(env);
 
  app->setCssTheme("polished");
 
  WHBoxLayout *layout = new WHBoxLayout(app->root());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(new WidgetGallery());

  app->setTitle("Wt widgets demo");

  app->addMetaHeader("viewport", "width=700, height=1200");

  // load text bundles (for the tr() function)
  app->messageResourceBundle().use(app->appRoot() + "text");
  app->messageResourceBundle().use(app->appRoot() + "charts");
  app->messageResourceBundle().use(app->appRoot() + "treeview");

  app->useStyleSheet("style/everywidget.css");
  app->useStyleSheet("style/dragdrop.css");
  app->useStyleSheet("style/combostyle.css");

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
