/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WStackedWidget.h>

#include <Wt/WCssTheme.h>

#include "WidgetGallery.h"

using namespace Wt;

std::unique_ptr<WApplication> createApplication(const Wt::WEnvironment& env)
{
  auto app = cpp14::make_unique<WApplication>(env);

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
    theme = "bootstrap3";
  else
    theme = *themePtr;

  if (theme == "bootstrap3") {
    auto bootstrapTheme = std::make_shared<WBootstrapTheme>();
    bootstrapTheme->setVersion(BootstrapVersion::v3);
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);

    // load the default bootstrap3 (sub-)theme
    app->useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");
  } else if (theme == "bootstrap2") {
    auto bootstrapTheme = std::make_shared<WBootstrapTheme>();
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);
  } else
    app->setTheme(std::make_shared<WCssTheme>(theme));


  // load text bundles (for the tr() function)
  app->messageResourceBundle().use(app->appRoot() + "report");
  app->messageResourceBundle().use(app->appRoot() + "text");
  app->messageResourceBundle().use(app->appRoot() + "src");
 

  auto layout =
      app->root()->setLayout(cpp14::make_unique<WHBoxLayout>());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(cpp14::make_unique<WidgetGallery>());

  app->setTitle("Wt Widget Gallery");

  app->useStyleSheet("style/everywidget.css");
  app->useStyleSheet("style/dragdrop.css");
  app->useStyleSheet("style/combostyle.css");
  app->useStyleSheet("style/pygments.css");
  app->useStyleSheet("style/layout.css");
  app->useStyleSheet("style/filedrop.css");

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
