/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "JWtHome.h"

#ifdef WT_EMWEB_BUILD
#include "QuoteForm.h"
#endif // WT_EMWEB_BUILD

#include <Wt/WText.h>
#include <Wt/WAnchor.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTreeNode.h>
#include <Wt/WWidget.h>
#include <Wt/WViewWidget.h>
#include <Wt/WTabWidget.h>
#include <Wt/WMenuItem.h>
#include <Wt/WTable.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLogger.h>

#include "ExampleSourceViewer.h"

JWtHome::JWtHome(const WEnvironment& env, Dbo::SqlConnectionPool& blogDb)
  : Home(env, blogDb,
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

std::unique_ptr<WWidget> JWtHome::examples()
{
  std::unique_ptr<WContainerWidget> result(std::make_unique<WContainerWidget>());

  std::unique_ptr<WText> intro(std::make_unique<WText>(tr("home.examples")));
  intro->setInternalPathEncoding(true);
  result->addWidget(std::move(intro));

  examplesMenu_ = result->addWidget(std::make_unique<WTabWidget>());
  WAnimation animation(AnimationEffect::SlideInFromRight, TimingFunction::EaseIn);
  examplesMenu_->contentsStack()->setTransitionAnimation(animation, true);

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
  examplesMenu_->addTab(std::move(wrapView(&JWtHome::helloWorldExample)),
  			tr("hello-world"))->setPathComponent("");
  examplesMenu_->addTab(std::move(wrapView(&JWtHome::widgetGalleryExample)),
			tr("widget-gallery"));
  examplesMenu_->addTab(std::move(wrapView(&JWtHome::chartExample)),
  			tr("charts"));
  examplesMenu_->addTab(std::move(wrapView(&JWtHome::treeviewExample)),
			tr("treeview"));
  examplesMenu_->addTab(std::move(wrapView(&JWtHome::composerExample)),
			tr("mail-composer"));
  examplesMenu_->addTab(std::move(wrapView(&JWtHome::chatExample)),
			tr("chat"));
  examplesMenu_->addTab(std::move(wrapView(&JWtHome::figtreeExample)),
			tr("figtree"));
  
  // Enable internal paths for the example menu
  examplesMenu_->setInternalPathEnabled("/examples");
  examplesMenu_->currentChanged().connect(this, &Home::googleAnalyticsLogger);

  return std::move(result);
}

std::unique_ptr<WWidget> JWtHome::createQuoteForm()
{
#ifdef WT_EMWEB_BUILD
  return std::make_unique<QuoteForm>(QuoteForm::JWt);
#else
  return nullptr;
#endif
}

std::unique_ptr<WWidget> JWtHome::sourceViewer(const std::string &deployPath)
{
  return std::make_unique<ExampleSourceViewer>(deployPath, jwtExamplePath_ + "/", "JAVA");
}

std::unique_ptr<WWidget> JWtHome::example(const char *textKey, const std::string& sourceDir)
{
  std::unique_ptr<WContainerWidget> result = std::make_unique<WContainerWidget>();
  result->addWidget(std::make_unique<WText>(tr(textKey)));
  result->addWidget(linkSourceBrowser(sourceDir));
  return std::move(result);
}

std::unique_ptr<WWidget> JWtHome::helloWorldExample()
{
  return std::move(example("home.examples.hello", "hello"));
}

std::unique_ptr<WWidget> JWtHome::chartExample()
{
  return std::move(example("home.examples.chart", "charts"));
}

std::unique_ptr<WWidget> JWtHome::treeviewExample()
{
  return std::move(example("home.examples.treeview", "treeviewdragdrop"));
}

std::unique_ptr<WWidget> JWtHome::composerExample()
{
  return std::move(example("home.examples.composer", "composer"));
}

std::unique_ptr<WWidget> JWtHome::chatExample()
{
  return std::move(example("home.examples.chat", "simplechat"));
}

std::unique_ptr<WWidget> JWtHome::figtreeExample()
{
  std::unique_ptr<WContainerWidget> result(std::make_unique<WContainerWidget>());
  WText *text = result->addWidget(std::make_unique<WText>(tr("home.examples.figtree")));
  text->setInternalPathEncoding(true);
  return std::move(result);
}

std::unique_ptr<WWidget> JWtHome::widgetGalleryExample()
{
  return std::move(example("home.examples.widgetgallery", "widgetgallery"));
}

std::unique_ptr<WWidget> JWtHome::wrapView(std::unique_ptr<WWidget> (JWtHome::*createWidget)())
{
  return makeStaticModel(std::bind(createWidget, this));
}

std::unique_ptr<WApplication> createJWtHomeApplication(const WEnvironment& env,
                                       Dbo::SqlConnectionPool *blogDb)
{
  return std::make_unique<JWtHome>(env, *blogDb);
}
