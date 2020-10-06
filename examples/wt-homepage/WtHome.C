/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WtHome.h"

#ifdef WT_EMWEB_BUILD
#include "QuoteForm.h"
#endif // WT_EMWEB_BUILD

#include <Wt/WAnchor.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLogger.h>
#include <Wt/WMenuItem.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTable.h>
#include <Wt/WTabWidget.h>
#include <Wt/WText.h>
#include <Wt/WTreeNode.h>
#include <Wt/WViewWidget.h>
#include <Wt/WWidget.h>

#include "ExampleSourceViewer.h"

using namespace Wt;

WtHome::WtHome(const WEnvironment& env, Dbo::SqlConnectionPool& blogDb)
  : Home(env, blogDb, "Wt, C++ Web Toolkit", "wt-home", "css/wt")
{
  addLanguage(Lang("en", "/", "en", "English"));
  addLanguage(Lang("cn", "/cn/", "汉语", "中文 (Chinese)"));
  addLanguage(Lang("ru", "/ru/", "ру", "Русский (Russian)"));

  char* wtExamplePath  = getenv("WT_EXAMPLE_PATH");
  if (wtExamplePath)
    wtExamplePath_ = wtExamplePath;
  else
    wtExamplePath_ = "..";

  init();
}

std::unique_ptr<WWidget> WtHome::example(const char *textKey, const std::string& sourceDir)
{
  auto result = std::make_unique<WContainerWidget>();
  WText *w = result->addWidget(std::make_unique<WText>(tr(textKey)));
  w->setInternalPathEncoding(true);
  result->addWidget(linkSourceBrowser(sourceDir));
  return std::move(result);
}

std::unique_ptr<WWidget> WtHome::helloWorldExample()
{
  return example("home.examples.hello", "hello");
}

std::unique_ptr<WWidget> WtHome::chartExample()
{
  return example("home.examples.chart", "charts");
}

std::unique_ptr<WWidget> WtHome::homepageExample()
{
  return example("home.examples.wt", "wt-homepage");
}

std::unique_ptr<WWidget> WtHome::treeviewExample()
{
  return example("home.examples.treeview", "treeview-dragdrop");
}

std::unique_ptr<WWidget> WtHome::gitExample()
{
  return example("home.examples.git", "gitmodel");
}

std::unique_ptr<WWidget> WtHome::chatExample()
{
  return example("home.examples.chat", "simplechat");
}

std::unique_ptr<WWidget> WtHome::composerExample()
{
  return example("home.examples.composer", "composer");
}

std::unique_ptr<WWidget> WtHome::widgetGalleryExample()
{
  return example("home.examples.widgetgallery", "widgetgallery");
}

std::unique_ptr<WWidget> WtHome::hangmanExample()
{
  return example("home.examples.hangman", "hangman");
}

std::unique_ptr<WWidget> WtHome::examples()
{
  auto result = std::make_unique<WContainerWidget>();

  auto intro = std::make_unique<WText>(tr("home.examples"));
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
  examplesMenu_->addTab(wrapView(&WtHome::helloWorldExample),
			tr("hello-world"))->setPathComponent("");
  examplesMenu_->addTab(wrapView(&WtHome::widgetGalleryExample),
			tr("widget-gallery"));
  examplesMenu_->addTab(wrapView(&WtHome::chartExample),
  			tr("charts"));
  examplesMenu_->addTab(wrapView(&WtHome::homepageExample),
			tr("wt-homepage"));
  examplesMenu_->addTab(wrapView(&WtHome::treeviewExample),
			tr("treeview"));
  examplesMenu_->addTab(wrapView(&WtHome::gitExample),
			tr("git"));
  examplesMenu_->addTab(wrapView(&WtHome::chatExample),
			tr("chat"));
  examplesMenu_->addTab(wrapView(&WtHome::composerExample),
			tr("mail-composer"));
  examplesMenu_->addTab(wrapView(&WtHome::hangmanExample),
			tr("hangman"));

  // Enable internal paths for the example menu
  examplesMenu_->setInternalPathEnabled("/examples");
  examplesMenu_->currentChanged().connect(this, &Home::googleAnalyticsLogger);

  return std::move(result);
}

std::unique_ptr<WWidget> WtHome::createQuoteForm()
{
#ifdef WT_EMWEB_BUILD
  return std::make_unique<QuoteForm>(QuoteForm::Wt);
#else
  return nullptr;
#endif
}

std::unique_ptr<WWidget> WtHome::sourceViewer(const std::string& deployPath)
{
  return std::make_unique<ExampleSourceViewer>(deployPath, wtExamplePath_ + "/", "CPP");
}

std::unique_ptr<WWidget> WtHome::wrapView(std::unique_ptr<WWidget> (WtHome::*createWidget)())
{
  return makeStaticModel(std::bind(createWidget, this));
}

std::unique_ptr<WApplication> createWtHomeApplication(const WEnvironment& env,
                                      Dbo::SqlConnectionPool *blogDb)
{
  return std::make_unique<WtHome>(env, *blogDb);
}
