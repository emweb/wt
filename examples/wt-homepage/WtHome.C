/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WtHome.h"

#include <Wt/WText>
#include <Wt/WTreeNode>
#include <Wt/WWidget>
#include <Wt/WViewWidget>
#include <Wt/WTabWidget>
#include <Wt/WMenuItem>
#include <Wt/WTable>
#include <Wt/WEnvironment>
#include <Wt/WLogger>

WtHome::WtHome(const WEnvironment& env)
  : Home(env, "wt-home", "css/wt")
{
  addLanguage(Lang("en", "/", "en", "English"));
  addLanguage(Lang("cn", "/cn/", "汉语", "中文 (Chinese)"));

  init();
}

WWidget *WtHome::helloWorldExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.hello"), result);

  WTreeNode *tree = makeTreeMap("Hello world", 0);
  makeTreeFile("hello.C", tree);

  tree->expand();

  result->addWidget(tree);

  return result;
}

WWidget *WtHome::chartExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.chart"), result);

  WTreeNode *tree = makeTreeMap("Chart example", 0);
  WTreeNode *chartsExample = makeTreeMap("class ChartsExample", tree);
  makeTreeFile("ChartsExample.h", chartsExample);
  makeTreeFile("ChartsExample.C", chartsExample);
  WTreeNode *chartConfig = makeTreeMap("class ChartConfig", tree);
  makeTreeFile("ChartConfig.h", chartConfig);
  makeTreeFile("ChartConfig.C", chartConfig);
  WTreeNode *panelList = makeTreeMap("class PanelList", tree);
  makeTreeFile("PanelList.h", panelList);
  makeTreeFile("PanelList.C", panelList);
  makeTreeFile("CsvUtil.C", tree);
  makeTreeFile("charts.xml", tree);
  makeTreeFile("charts.css", tree);

  tree->expand();

  result->addWidget(tree);

  return result;
}

WWidget *WtHome::homepageExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.wt"), result);

  WTreeNode *tree = makeTreeMap("Wt Homepage", 0);
  WTreeNode *home = makeTreeMap("class Home", tree);
  makeTreeFile("Home.h", home);
  makeTreeFile("Home.C", home);
  WTreeNode *treeexample = makeTreeMap("class TreeListExample", tree);
  makeTreeFile("TreeListExample.h", treeexample);
  makeTreeFile("TreeListExample.C", treeexample);
  makeTreeFile("wt-home.xml", tree);

  tree->expand();

  result->addWidget(tree);

  return result;
}

WWidget *WtHome::treeviewExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.treeview"), result);

  WTreeNode *tree = makeTreeMap("Treeview example", 0);

  WTreeNode *classMap;
  classMap = makeTreeMap("class FolderView", tree);
  makeTreeFile("FolderView.h", classMap);
  makeTreeFile("FolderView.C", classMap);
  makeTreeFile("TreeViewDragDrop.C", tree);
  makeTreeFile("CsvUtil.C", tree);
  makeTreeFile("about.xml", tree);
  makeTreeFile("styles.css", tree);

  tree->expand();

  result->addWidget(tree);

  return result;
}

WWidget *WtHome::gitExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.git"), result);

  WTreeNode *tree = makeTreeMap("Git example", 0);

  WTreeNode *classMap;
  classMap = makeTreeMap("class GitModel", tree);
  makeTreeFile("GitModel.h", classMap);
  makeTreeFile("GitModel.C", classMap);
  classMap = makeTreeMap("class Git", tree);
  makeTreeFile("Git.h", classMap);
  makeTreeFile("Git.C", classMap);
  makeTreeFile("GitView.C", tree);
  makeTreeFile("gitview.css", tree);

  tree->expand();

  result->addWidget(tree);

  return result;
}

WWidget *WtHome::chatExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.chat"), result);

  WTreeNode *tree = makeTreeMap("Chat example", 0);

  WTreeNode *classMap;
  classMap = makeTreeMap("class SimpleChatWidget", tree);
  makeTreeFile("SimpleChatWidget.h", classMap);
  makeTreeFile("SimpleChatWidget.C", classMap);
  classMap = makeTreeMap("class SimpleChatServer", tree);
  makeTreeFile("SimpleChatServer.h", classMap);
  makeTreeFile("SimpleChatServer.C", classMap);
  makeTreeFile("simpleChat.C", tree);
  makeTreeFile("simplechat.css", tree);
  makeTreeFile("simplechat.xml", tree);

  tree->expand();

  result->addWidget(tree);

  return result;
}

WWidget *WtHome::composerExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.composer"), result);

  WTreeNode *tree = makeTreeMap("Mail composer example", 0);

  WTreeNode *classMap;
  classMap = makeTreeMap("class AddresseeEdit", tree);
  makeTreeFile("AddresseeEdit.h", classMap);
  makeTreeFile("AddresseeEdit.C", classMap);
  classMap = makeTreeMap("class AttachmentEdit", tree);
  makeTreeFile("AttachmentEdit.h", classMap);
  makeTreeFile("AttachmentEdit.C", classMap);
  classMap = makeTreeMap("class ComposeExample", tree);
  makeTreeFile("ComposeExample.h", classMap);
  makeTreeFile("ComposeExample.C", classMap);
  classMap = makeTreeMap("class Composer", tree);
  makeTreeFile("Composer.h", classMap);
  makeTreeFile("Composer.C", classMap);
  classMap = makeTreeMap("class ContactSuggestions", tree);
  makeTreeFile("ContactSuggestions.h", classMap);
  makeTreeFile("ContactSuggestions.C", classMap);
  classMap = makeTreeMap("class Label", tree);
  makeTreeFile("Label.h", classMap);
  makeTreeFile("Label.C", classMap);
  classMap = makeTreeMap("class Option", tree);
  makeTreeFile("Option.h", classMap);
  makeTreeFile("Option.C", classMap);
  classMap = makeTreeMap("class OptionList", tree);
  makeTreeFile("OptionList.h", classMap);
  makeTreeFile("OptionList.C", classMap);
  makeTreeFile("Contact.h", tree);
  makeTreeFile("Attachment.h", tree);
  makeTreeFile("composer.xml", tree);
  makeTreeFile("composer.css", tree);

  tree->expand();

  result->addWidget(tree);

  return result;
}

WWidget *WtHome::widgetGalleryExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.widgetgallery"), result);

  return result;
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
   *
   * For the tree list example (for which we cannot use a view with a
   * static model, since we allow the tree to be manipulated) we use
   * the defer utility function to defer its creation until it is
   * loaded.
   */

  // The call ->setPathComponent() is to use "/examples" instead of
  // "/examples/hello_world" as internal path
  examplesMenu_->addTab(wrapViewOrDefer(&WtHome::helloWorldExample),
			tr("hello-world"))->setPathComponent("");

  examplesMenu_->addTab(wrapViewOrDefer(&WtHome::chartExample),
			tr("charts"));
  examplesMenu_->addTab(wrapViewOrDefer(&WtHome::homepageExample),
			tr("wt-homepage"));
  examplesMenu_->addTab(wrapViewOrDefer(&WtHome::treeviewExample),
			tr("treeview"));
  examplesMenu_->addTab(wrapViewOrDefer(&WtHome::gitExample),
			tr("git"));
  examplesMenu_->addTab(wrapViewOrDefer(&WtHome::chatExample),
			tr("chat"));
  examplesMenu_->addTab(wrapViewOrDefer(&WtHome::composerExample),
			tr("mail-composer"));
  examplesMenu_->addTab(wrapViewOrDefer(&WtHome::widgetGalleryExample),
			tr("widget-gallery"));

  examplesMenu_->currentChanged().connect(SLOT(this, Home::logInternalPath));

  // Enable internal paths for the example menu
  examplesMenu_->setInternalPathEnabled();
  examplesMenu_->setInternalBasePath("/examples");

  return result;
}

WWidget *WtHome::download()
{
  WContainerWidget *result = new WContainerWidget();
  result->addWidget(new WText(tr("home.download")));
  result->addWidget(new WText(tr("home.download.license")));
  result->addWidget(new WText(tr("home.download.requirements")));
  result->addWidget(new WText(tr("home.download.cvs")));
  result->addWidget(new WText(tr("home.download.packages")));

  releases_ = new WTable();
  readReleases(releases_, "releases.txt");
  result->addWidget(releases_);

  result->addWidget
    (new WText("<p>Older releases are still available at "
	       + href("http://sourceforge.net/project/showfiles.php?"
		      "group_id=153710#files",
		      "sourceforge.net")
	       + "</p>"));

  return result;
}

WWidget *WtHome::wrapViewOrDefer(WWidget *(WtHome::*createWidget)())
{
  /*
   * We can only create a view if we have javascript for the
   * client-side tree manipulation -- otherwise we require server-side
   * event handling which is not possible with a view since the
   * server-side widgets do not exist. Therefore, all we can do to
   * avoid unnecessary server-side resources when JavaScript is not
   * available is deferring creation until load time.
   */
  if (!environment().agentIsIEMobile() && environment().javaScript())
    return makeStaticModel(boost::bind(createWidget, this));
  else
    return deferCreate(boost::bind(createWidget, this));
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
