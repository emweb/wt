/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WFitLayout.h>
#include <Wt/WBootstrap2Theme.h>
#include <Wt/WBootstrap3Theme.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WLayout.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WStackedWidget.h>

#include <Wt/WCssTheme.h>

#include "Theme.h"
#include "WidgetGallery.h"

using namespace Wt;

std::unique_ptr<WApplication> createApplication(const Wt::WEnvironment& env)
{
  auto app = std::make_unique<WApplication>(env);

  if (app->appRoot().empty()) {
    std::cerr << "!!!!!!!!!!" << std::endl
              << "!! Warning: read the README.md file for hints on deployment,"
              << " the approot looks suspect!" << std::endl
              << "!!!!!!!!!!" << std::endl;
  }

  // app->setLayoutDirection(LayoutDirection::RightToLeft);

  // Choice of theme: defaults to bootstrap3 but can be overridden using
  // a theme parameter (for testing)
  const std::string *themePtr = env.getParameter("theme");
  std::string theme;
  if (!themePtr)
    theme = "wt";
  else
    theme = *themePtr;

  if (theme == "wt" || theme == "jwt") {
    auto wtTheme = std::make_shared<Theme>(theme);
    app->setTheme(wtTheme);
  } else if (theme == "bootstrap5") {
    auto bootstrapTheme = std::make_shared<WBootstrap5Theme>();
    app->setTheme(bootstrapTheme);
  } else if (theme == "bootstrap3") {
    auto bootstrapTheme = std::make_shared<WBootstrap3Theme>();
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);

    // load the default bootstrap3 (sub-)theme
    app->useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");
  } else if (theme == "bootstrap2") {
    auto bootstrapTheme = std::make_shared<WBootstrap2Theme>();
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);
  } else
    app->setTheme(std::make_shared<WCssTheme>(theme));


  // load text bundles (for the tr() function)
  app->messageResourceBundle().use(app->appRoot() + "report");
  app->messageResourceBundle().use(app->appRoot() + "text");
  app->messageResourceBundle().use(app->appRoot() + "tpl");
  app->messageResourceBundle().use(app->appRoot() + "src");

  app->root()->addWidget(std::make_unique<WidgetGallery>());

  app->setTitle("Wt Widget Gallery");

  app->useStyleSheet("resources/font-awesome/css/font-awesome.min.css");
  app->useStyleSheet("style/widgetgallery.css");
  app->useStyleSheet("style/everywidget.css");
  app->useStyleSheet("style/pygments.css");
  app->useStyleSheet("style/layout.css");
  app->useStyleSheet("style/filedrop.css");

  return app;
}

int main(int argc, char **argv)
{
  //Wt::WLayout::setDefaultImplementation(Wt::LayoutImplementation::JavaScript);
  return WRun(argc, argv, &createApplication);
}
