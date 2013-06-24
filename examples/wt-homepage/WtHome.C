/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WtHome.h"

#ifdef WT_EMWEB_BUILD
#include "QuoteForm.h"
#endif // WT_EMWEB_BUILD

#include <Wt/WAnchor>
#include <Wt/WEnvironment>
#include <Wt/WLogger>
#include <Wt/WMenuItem>
#include <Wt/WStackedWidget>
#include <Wt/WTable>
#include <Wt/WTabWidget>
#include <Wt/WText>
#include <Wt/WTreeNode>
#include <Wt/WViewWidget>
#include <Wt/WWidget>

#include "ExampleSourceViewer.h"

WtHome::WtHome(const WEnvironment& env, Wt::Dbo::SqlConnectionPool& blogDb)
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

WWidget *WtHome::example(const char *textKey, const std::string& sourceDir)
{
  WContainerWidget *result = new WContainerWidget();
  WText *w = new WText(tr(textKey), result);
  w->setInternalPathEncoding(true);
  result->addWidget(linkSourceBrowser(sourceDir));
  return result;
}

WWidget *WtHome::helloWorldExample()
{
  return example("home.examples.hello", "hello");
}

WWidget *WtHome::chartExample()
{
  return example("home.examples.chart", "charts");
}

WWidget *WtHome::homepageExample()
{
  return example("home.examples.wt", "wt-homepage");
}

WWidget *WtHome::treeviewExample()
{
  return example("home.examples.treeview", "treeview-dragdrop");
}

WWidget *WtHome::gitExample()
{
  return example("home.examples.git", "gitmodel");
}

WWidget *WtHome::chatExample()
{
  return example("home.examples.chat", "simplechat");
}

WWidget *WtHome::composerExample()
{
  return example("home.examples.composer", "composer");
}

WWidget *WtHome::widgetGalleryExample()
{
  return example("home.examples.widgetgallery", "widgetgallery");
}

WWidget *WtHome::hangmanExample()
{
  return example("home.examples.hangman", "hangman");
}

WWidget *WtHome::examples()
{
  WContainerWidget *result = new WContainerWidget();

  WText *intro = new WText(tr("home.examples"));
  intro->setInternalPathEncoding(true);
  result->addWidget(intro);

  examplesMenu_ = new WTabWidget(result);

  WAnimation animation(WAnimation::SlideInFromRight, WAnimation::EaseIn);
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

  return result;
}

WWidget *WtHome::createQuoteForm()
{
#ifdef WT_EMWEB_BUILD
  return new QuoteForm(QuoteForm::Wt);
#else
  return 0;
#endif
}

WWidget *WtHome::sourceViewer(const std::string& deployPath)
{
  return new ExampleSourceViewer(deployPath, wtExamplePath_ + "/", "CPP");
}

WWidget *WtHome::wrapView(WWidget *(WtHome::*createWidget)())
{
  return makeStaticModel(boost::bind(createWidget, this));
}

WApplication *createWtHomeApplication(const WEnvironment& env, 
				      Wt::Dbo::SqlConnectionPool *blogDb)
{
  return new WtHome(env, *blogDb);
}
