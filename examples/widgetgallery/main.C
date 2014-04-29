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

  // app->setLayoutDirection(Wt::RightToLeft);

  // Choice of theme: defaults to bootstrap3 but can be overridden using
  // a theme parameter (for testing)
  const std::string *themePtr = env.getParameter("theme");
  std::string theme;
  if (!themePtr)
    theme = "bootstrap3";
  else
    theme = *themePtr;

  if (theme == "bootstrap3") {
    Wt::WBootstrapTheme *bootstrapTheme = new Wt::WBootstrapTheme(app);
    bootstrapTheme->setVersion(Wt::WBootstrapTheme::Version3);
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);

    // load the default bootstrap3 (sub-)theme
    app->useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");
  } else if (theme == "bootstrap2") {
    Wt::WBootstrapTheme *bootstrapTheme = new Wt::WBootstrapTheme(app);
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);
  } else
    app->setTheme(new Wt::WCssTheme(theme));


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
  app->useStyleSheet("style/layout.css");

  return app;
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
