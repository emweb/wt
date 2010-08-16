/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WtHome.h"

#include <Wt/WAnchor>
#include <Wt/WEnvironment>
#include <Wt/WLogger>
#include <Wt/WMenuItem>
#include <Wt/WTable>
#include <Wt/WTabWidget>
#include <Wt/WText>
#include <Wt/WTreeNode>
#include <Wt/WViewWidget>
#include <Wt/WWidget>

#include "ExampleSourceViewer.h"

WtHome::WtHome(const WEnvironment& env)
  : Home(env, "Wt, C++ Web Toolkit", "wt-home", "css/wt")
{
  addLanguage(Lang("en", "/", "en", "English"));
  addLanguage(Lang("cn", "/cn/", "汉语", "中文 (Chinese)"));

  char* wtExamplePath  = getenv("WT_EXAMPLE_PATH");
  if (wtExamplePath)
    wtExamplePath_ = wtExamplePath;
  else
    wtExamplePath_ = "../";

  init();
}

WWidget *WtHome::example(const char *textKey, const std::string& sourceDir)
{
  WContainerWidget *result = new WContainerWidget();
  new WText(tr(textKey), result);
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

WWidget *WtHome::examples()
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
  examplesMenu_->addTab(wrapView(&WtHome::helloWorldExample),
			tr("hello-world"))->setPathComponent("");
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
  examplesMenu_->addTab(wrapView(&WtHome::widgetGalleryExample),
			tr("widget-gallery"));

  // Enable internal paths for the example menu
  examplesMenu_->setInternalPathEnabled("/examples");
  examplesMenu_->currentChanged().connect(this, &Home::googleAnalyticsLogger);

  return result;
}

WWidget *WtHome::download()
{
  WContainerWidget *result = new WContainerWidget();
  result->addWidget(new WText(tr("home.download")));
  result->addWidget(new WText(tr("home.download.license")));
  result->addWidget(new WText(tr("home.download.packages")));

  releases_ = new WTable();
  readReleases(releases_);
  result->addWidget(releases_);
  
  result->addWidget
    (new WText("<p>Older releases are still available at "
	       + href("http://sourceforge.net/project/showfiles.php?"
		      "group_id=153710#files",
		      "sourceforge.net")
	       + "</p>"));

  result->addWidget(new WText(tr("home.download.requirements")));
  result->addWidget(new WText(tr("home.download.cvs")));

  return result;
}

WWidget *WtHome::sourceViewer(const std::string& deployPath)
{
  return new ExampleSourceViewer(deployPath, wtExamplePath_ + "/", "CPP");
}

WWidget *WtHome::wrapView(WWidget *(WtHome::*createWidget)())
{
  return makeStaticModel(boost::bind(createWidget, this));
}

WApplication *createWtHomeApplication(const WEnvironment& env)
{
  // support for old (< Wt-2.2) homepage URLS: redirect from "states"
  // to "internal paths"
  // this contains the initial "history state" in old Wt versions
  const std::string *historyKey = env.getParameter("historyKey");

  if (historyKey) {
    const char *mainStr[]
      = { "main:0", "/",
	  "main:1", "/news",
	  "main:2", "/features",
	  "main:4", "/examples",
	  "main:3", "/documentation",
	  "main:5", "/download",
	  "main:6", "/community" };

    const char *exampleStr[]
      = { "example:0", "/examples",
	  "example:1", "/examples/charts",
	  "example:2", "/examples/wt-homepage",
	  "example:3", "/examples/treelist",
	  "example:4", "/examples/hangman",
	  "example:5", "/examples/chat",
	  "example:6", "/examples/mail-composer",
	  "example:7", "/examples/drag-and-drop",
	  "example:8", "/examples/file-explorer",
	  "example:9", "/examples/calendar" };

    if (historyKey->find("main:4") != std::string::npos) {
      for (unsigned i = 0; i < 10; ++i)
	if (historyKey->find(exampleStr[i*2]) != std::string::npos) {
	  WApplication *app = new WApplication(env);
	  app->log("notice") << "redirecting old style URL '"
			     << *historyKey << "' to internal path: '"
			     << exampleStr[i*2+1] << "'";
	  app->redirect(app->bookmarkUrl(exampleStr[i*2+1]));
	  app->quit();
	  return app;
	}
    } else
      for (unsigned i = 0; i < 6; ++i)
	if (historyKey->find(mainStr[i*2]) != std::string::npos) {
	  WApplication *app = new WApplication(env);

	  app->log("notice") << "redirecting old style URL '"
			     << *historyKey << "' to internal path: '"
			     << mainStr[i*2+1] << "'";
	  app->redirect(app->bookmarkUrl(mainStr[i*2+1]));
	  app->quit();
	  return app;
	}

    // unknown history key, just continue
  }

  return new WtHome(env);
}
