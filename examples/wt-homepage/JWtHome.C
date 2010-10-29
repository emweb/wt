/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "JWtHome.h"

#include <Wt/WText>
#include <Wt/WAnchor>
#include <Wt/WTreeNode>
#include <Wt/WWidget>
#include <Wt/WViewWidget>
#include <Wt/WTabWidget>
#include <Wt/WMenuItem>
#include <Wt/WTable>
#include <Wt/WEnvironment>
#include <Wt/WLogger>

#include "ExampleSourceViewer.h"

JWtHome::JWtHome(const WEnvironment& env)
  : Home(env, 
      "JWt, Java Web Toolkit",
      "jwt-home", "css/jwt")
{
  addLanguage(Lang("en", "/", "en", "English"));

  char* jwtExamplePath  = getenv("JWT_EXAMPLE_PATH");
  if (jwtExamplePath)
    jwtExamplePath_ = jwtExamplePath;
  else
    jwtExamplePath_ = "/home/pieter/projects/jwt/wt-port/java/examples/";

  init();
}

WWidget *JWtHome::examples()
{
  WContainerWidget *result = new WContainerWidget();

  result->addWidget(new WText(tr("home.examples")));

  examplesMenu_ = new WTabWidget(AlignTop | AlignJustify, result);

  /*
   * The following code is functionally equivalent to:
   *
   *   examplesMenu_->addTab(helloWorldExample(), "Hello world");
   *
   * However, we optimize here for memory consumption (it is a homepage
   * after all, and we hope to be slashdotted some day)
   *
   * Therefore, we wrap all the static content (including the tree
   * widgets), into WViewWidgets with static models. In this way the
   * widgets are not actually stored in memory on the server.
   */

  // The call ->setPathComponent() is to use "/examples/" instead of
  // "/examples/hello_world" as internal path
  examplesMenu_->addTab(wrapView(&JWtHome::helloWorldExample),
  			tr("hello-world"))->setPathComponent("");
  examplesMenu_->addTab(wrapView(&JWtHome::chartExample),
  			tr("charts"));
  examplesMenu_->addTab(wrapView(&JWtHome::treeviewExample),
			tr("treeview"));
  examplesMenu_->addTab(wrapView(&JWtHome::composerExample),
			tr("mail-composer"));
  examplesMenu_->addTab(wrapView(&JWtHome::chatExample),
			tr("chat"));
  examplesMenu_->addTab(wrapView(&JWtHome::figtreeExample),
			tr("figtree"));
  
  // Enable internal paths for the example menu
  examplesMenu_->setInternalPathEnabled("/examples");
  examplesMenu_->currentChanged().connect(this, &Home::googleAnalyticsLogger);

  return result;
}

WWidget *JWtHome::download()
{
  WContainerWidget *result = new WContainerWidget();
  result->addWidget(new WText(tr("home.download")));
  result->addWidget(new WText(tr("home.download.license")));
  result->addWidget(new WText(tr("home.download.packages")));

  releases_ = new WTable();
  readReleases(releases_);
  result->addWidget(releases_);
  result->addWidget(new WText(tr("home.download.requirements")));
  result->addWidget(new WText(tr("home.download.cvs")));

  return result;
}

WWidget *JWtHome::sourceViewer(const std::string &deployPath)
{
  return new ExampleSourceViewer(deployPath, jwtExamplePath_ + "/", "JAVA");
}

WWidget *JWtHome::example(const char *textKey, const std::string& sourceDir)
{
  WContainerWidget *result = new WContainerWidget();
  new WText(tr(textKey), result);
  result->addWidget(linkSourceBrowser(sourceDir));
  return result;
}

WWidget *JWtHome::helloWorldExample()
{
  return example("home.examples.hello", "hello");
}

WWidget *JWtHome::chartExample()
{
  return example("home.examples.chart", "charts");
}

WWidget *JWtHome::treeviewExample()
{
  return example("home.examples.treeview", "treeviewdragdrop");
}

WWidget *JWtHome::composerExample()
{
  return example("home.examples.composer", "composer");
}

WWidget *JWtHome::chatExample()
{
  return example("home.examples.chat", "simplechat");
}

WWidget *JWtHome::figtreeExample()
{
  WContainerWidget *result = new WContainerWidget();
  new WText(tr("home.examples.figtree"), result);
  return result;
}

WWidget *JWtHome::wrapView(WWidget *(JWtHome::*createWidget)())
{
  return makeStaticModel(boost::bind(createWidget, this));
}

WApplication *createJWtHomeApplication(const WEnvironment& env)
{
  return new JWtHome(env);
}
