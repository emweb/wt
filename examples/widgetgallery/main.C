/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WHBoxLayout>
#include <Wt/WBootstrapTheme>
#include <Wt/WCssTheme>

#include "WidgetGallery.h"

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  Wt::WApplication* app = new Wt::WApplication(env);

  if (app->appRoot().empty()) {
    std::cerr << "!!!!!!!!!!" << std::endl
	      << "!! Warning: read the README.md file for hints on deployment,"
	      << " the approot looks suspect!" << std::endl
	      << "!!!!!!!!!!" << std::endl;
  }

  const std::string *theme = env.getParameter("theme");
  if (theme)
    app->setTheme(new Wt::WCssTheme(*theme));
  else 
    app->setTheme(new Wt::WBootstrapTheme(app));

  // load text bundles (for the tr() function)
  app->messageResourceBundle().use(app->appRoot() + "report");
  app->messageResourceBundle().use(app->appRoot() + "text");
  app->messageResourceBundle().use(app->appRoot() + "src");
 
  Wt::WHBoxLayout *layout = new Wt::WHBoxLayout(app->root());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(new WidgetGallery());

  app->setTitle("Wt Widget Gallery");

  app->useStyleSheet("style/everywidget.css");
  app->useStyleSheet("style/dragdrop.css");
  app->useStyleSheet("style/combostyle.css");
  app->useStyleSheet("style/pygments.css");

  return app;
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
