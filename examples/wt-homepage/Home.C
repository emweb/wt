/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <Wt/WAnchor>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WIconPair>
#include <Wt/WImage>
#include <Wt/WLineEdit>
#include <Wt/WLogger>
#include <Wt/WMenu>
#include <Wt/WPushButton>
#include <Wt/WSignalMapper>
#include <Wt/WStackedWidget>
#include <Wt/WTabWidget>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WText>
#include <Wt/WTreeNode>
#include <Wt/WViewWidget>

#include "Home.h"

namespace {
  struct Lang {
    std::string code, path, shortDescription, longDescription;
  };

  Lang l[] = {
    { "en", "/", "en", "English" },
    { "cn", "/cn/", "汉语", "中文 (Chinese)" }
  };

  std::vector<Lang> languages(l, l + 2);
}

/*
 * A utility container widget which defers creation of its single
 * child widget until the container is loaded (which is done on-demand
 * by a WMenu). The constructor takes the create function for the
 * widget as a parameter.
 *
 * We use this to defer widget creation until needed.
 */
template <typename Function>
class DeferredWidget : public WContainerWidget
{
public:
  DeferredWidget(Function f)
    : f_(f) { }

private:
  void load() {
    WContainerWidget::load();
    addWidget(f_());
  }

  Function f_;
};

template <typename Function>
DeferredWidget<Function> *deferCreate(Function f)
{
  return new DeferredWidget<Function>(f);
}

/* Shortcut for a <div id=""> */
class Div : public WContainerWidget
{
public:
  Div(WContainerWidget *parent, const std::string& id)
    : WContainerWidget(parent)
  {
    setId(id);
  }
};

Home::Home(const WEnvironment& env)
  : WApplication(env),
    recentNews_(0),
    historicalNews_(0),
    releases_(0)
{
  messageResourceBundle().use("wt-home", false);
  useStyleSheet("images/wt.css");
  useStyleSheet("images/wt_ie.css", "lt IE 7");
  useStyleSheet("home.css");
  setTitle("Wt, C++ Web Toolkit");

  setLocale("");
  language_ = 0;

  Div *topWrapper = new Div(root(), "top_wrapper");
  Div *topContent = new Div(topWrapper, "top_content");

  Div *languagesDiv = new Div(topContent, "top_languages");

  WSignalMapper<int> *lmap = new WSignalMapper<int>(this);
  lmap->mapped.connect(SLOT(this, Home::changeLanguage));

  for (unsigned i = 0; i < languages.size(); ++i) {
    if (i != 0)
      new WText("- ", languagesDiv);

    const Lang& l = languages[i];

    WAnchor *a = new WAnchor(bookmarkUrl(l.path), l.longDescription,
			     languagesDiv);

    lmap->mapConnect(a->clicked, i);
  }

  WText *topWt = new WText(tr("top_wt"), topContent);
  topWt->setInline(false);
  topWt->setId("top_wt");

  WText *bannerWt = new WText(tr("banner_wrapper"), root());
  bannerWt->setId("banner_wrapper");

  Div *mainWrapper = new Div(root(), "main_wrapper");
  Div *mainContent = new Div(mainWrapper, "main_content");
  Div *mainMenu = new Div(mainContent, "main_menu");

  WStackedWidget *contents = new WStackedWidget();
  contents->setId("main_page");

  mainMenu_ = new WMenu(contents, Vertical, mainMenu);
  mainMenu_->setRenderAsList(true);

  // Use "/" instead of "/introduction/" as internal path
  mainMenu_->addItem
    (tr("introduction"), introduction())->setPathComponent("");
  mainMenu_->addItem
    (tr("news"), deferCreate(boost::bind(&Home::news, this)),
     WMenuItem::PreLoading);
  mainMenu_->addItem
    (tr("features"), wrapViewOrDefer(&Home::features),
     WMenuItem::PreLoading);
  mainMenu_->addItem
    (tr("documentation"), wrapViewOrDefer(&Home::documentation),
     WMenuItem::PreLoading);
  mainMenu_->addItem
    (tr("examples"), examples(),
     WMenuItem::PreLoading);
  mainMenu_->addItem
    (tr("download"), deferCreate(boost::bind(&Home::download, this)),
     WMenuItem::PreLoading);
  mainMenu_->addItem
    (tr("community"), wrapViewOrDefer(&Home::community),
     WMenuItem::PreLoading);

  mainMenu_->itemSelectRendered.connect(SLOT(this, Home::updateTitle));
  mainMenu_->itemSelected.connect(SLOT(this, Home::logInternalPath));
  mainMenu_->select((int)0);

  // Make the menu be internal-path aware.
  mainMenu_->setInternalPathEnabled();
  mainMenu_->setInternalBasePath("/");

  sideBarContent_ = new WContainerWidget(mainMenu);

  mainContent->addWidget(contents);
  WContainerWidget *clearAll = new WContainerWidget(mainContent);
  clearAll->setStyleClass("clearall");

  WText *footerWrapper = new WText(tr("footer_wrapper"), root());
  footerWrapper->setId("footer_wrapper");

  internalPathChanged.connect(SLOT(this, Home::setLanguageFromPath));
}

void Home::changeLanguage(int index)
{
  if (index == language_)
    return;

  int prevLanguage = language_;

  setLanguage(index);

  std::string langPath = languages[index].path;
  if (internalPath().empty())
    setInternalPath(langPath);
  else {
    std::string prevLangPath = languages[prevLanguage].path;
    std::string path = internalPath().substr(prevLangPath.length());
    setInternalPath(langPath + path);
  }
}

void Home::setLanguage(int index)
{
  const Lang& l = languages[index];

  setLocale(l.code);

  std::string langPath = l.path;
  mainMenu_->setInternalBasePath(langPath);
  examplesMenu_->setInternalBasePath(langPath + "examples");
  updateTitle();

  language_ = index;
}

void Home::setLanguageFromPath(std::string prefix)
{
  if (prefix == "/") {
    std::string langPath = internalPathNextPart(prefix);

    if (langPath.empty())
      langPath = '/';
    else
      langPath = '/' + langPath + '/';

    int newLanguage = 0;

    for (unsigned i = 0; i < languages.size(); ++i) {
      if (languages[i].path == langPath) {
	newLanguage = i;
	break;
      }
    }

    if (newLanguage != language_)
      setLanguage(newLanguage);
  }
}

void Home::updateTitle()
{
  setTitle(tr("wt") + " - " + mainMenu_->currentItem()->text());
}

void Home::logInternalPath()
{
  // simulate an access log for the interal paths
  log("path") << internalPath();
}

WWidget *Home::introduction()
{
  return new WText(tr("home.intro"));
}

void Home::refresh()
{
  if (recentNews_)
    readNews(recentNews_, "latest-news.txt");

  if (historicalNews_)
    readNews(historicalNews_, "historical-news.txt");

  if (releases_)
    readReleases(releases_, "releases.txt");

  WApplication::refresh();
}

WWidget *Home::news()
{
  WContainerWidget *result = new WContainerWidget();

  result->addWidget(new WText(tr("home.news")));

  result->addWidget(new WText(tr("home.latest-news")));
  recentNews_ = new WTable();
  readNews(recentNews_, "latest-news.txt");
  result->addWidget(recentNews_);

  result->addWidget(new WText(tr("home.historical-news")));
  historicalNews_ = new WTable();
  readNews(historicalNews_, "historical-news.txt");
  result->addWidget(historicalNews_);

  return result;
}

WWidget *Home::status()
{
  return new WText(tr("home.status"));
}

WWidget *Home::features()
{
  return new WText(tr("home.features"));
}

WWidget *Home::documentation()
{
  return new WText(tr("home.documentation"));
}

WWidget *Home::helloWorldExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.hello"), result);

  WTreeNode *tree = makeTreeMap("Hello world", 0);
  makeTreeFile("hello.C", tree);

  tree->expand();

  result->addWidget(tree);

  return result;
}

WWidget *Home::chartExample()
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

WWidget *Home::homepageExample()
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

WWidget *Home::treeviewExample()
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

WWidget *Home::gitExample()
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

WWidget *Home::chatExample()
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

WWidget *Home::composerExample()
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

WWidget *Home::widgetGalleryExample()
{
  WContainerWidget *result = new WContainerWidget();

  new WText(tr("home.examples.widgetgallery"), result);

  return result;
}

WWidget *Home::wrapViewOrDefer(WWidget *(Home::*createWidget)())
{
  /*
   * We can only create a view if we have javascript for the
   * client-side tree manipulation -- otherwise we require server-side
   * event handling which is not possible with a view since the
   * server-side widgets do not exist. Therefore, all we can do to
   * avoid unnecessary server-side resources when JavaScript is not
   * available is deferring creation until load time.
   */
  if (!environment().agentIEMobile() && environment().javaScript())
    return makeStaticModel(boost::bind(createWidget, this));
  else
    return deferCreate(boost::bind(createWidget, this));
}

WWidget *Home::examples()
{
  WContainerWidget *result = new WContainerWidget();

  result->addWidget(new WText(tr("home.examples")));

  examplesMenu_ = new WTabWidget(result);

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
  examplesMenu_->addTab(wrapViewOrDefer(&Home::helloWorldExample),
			tr("hello-world"))->setPathComponent("");

  examplesMenu_->addTab(wrapViewOrDefer(&Home::chartExample),
			tr("charts"));
  examplesMenu_->addTab(wrapViewOrDefer(&Home::homepageExample),
			tr("wt-homepage"));
  examplesMenu_->addTab(wrapViewOrDefer(&Home::treeviewExample),
			tr("treeview"));
  examplesMenu_->addTab(wrapViewOrDefer(&Home::gitExample),
			tr("git"));
  examplesMenu_->addTab(wrapViewOrDefer(&Home::chatExample),
			tr("chat"));
  examplesMenu_->addTab(wrapViewOrDefer(&Home::composerExample),
			tr("mail-composer"));
  examplesMenu_->addTab(wrapViewOrDefer(&Home::widgetGalleryExample),
			tr("widget-gallery"));

  examplesMenu_->currentChanged.connect(SLOT(this, Home::logInternalPath));

  // Enable internal paths for the example menu
  examplesMenu_->setInternalPathEnabled();
  examplesMenu_->setInternalBasePath("/examples");

  return result;
}

WWidget *Home::download()
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

std::string Home::href(const std::string url, const std::string description)
{
  return "<a href=\"" + url + "\" target=\"_blank\">" + description + "</a>";
}

WWidget *Home::community()
{
  return new WText(tr("home.community"));
}

void Home::readNews(WTable *newsTable, const std::string newsfile)
{
  std::ifstream f(newsfile.c_str());

  newsTable->clear();

  int row = 0;

  while (f) {
    std::string line;
    getline(f, line);

    if (f) {
      typedef boost::tokenizer<boost::escaped_list_separator<char> >
	CsvTokenizer;
      CsvTokenizer tok(line);

      CsvTokenizer::iterator i=tok.begin();

      newsTable->elementAt(row, 0)->
	addWidget(new WText("<p><b>" + *i + "</b></p>"));
      newsTable->elementAt(row, 0)
	->setContentAlignment(AlignCenter | AlignTop);
      newsTable->elementAt(row, 0)
	->resize(WLength(16, WLength::FontEx), WLength());
      newsTable
	->elementAt(row, 1)->addWidget(new WText("<p>" + *(++i) + "</p>"));

      ++row;
    }
  }
}

void Home::readReleases(WTable *releaseTable, const std::string releasefile)
{
  std::ifstream f(releasefile.c_str());

  releaseTable->clear();

  releaseTable->elementAt(0, 0)
    ->addWidget(new WText(tr("home.download.version")));
  releaseTable->elementAt(0, 1)
    ->addWidget(new WText(tr("home.download.date")));
  releaseTable->elementAt(0, 2)
    ->addWidget(new WText(tr("home.download.description")));

  releaseTable->elementAt(0, 0)->resize(WLength(10, WLength::FontEx),
					WLength());
  releaseTable->elementAt(0, 1)->resize(WLength(15, WLength::FontEx),
					WLength());

  int row = 1;

  while (f) {
    std::string line;
    getline(f, line);

    if (f) {
      typedef boost::tokenizer<boost::escaped_list_separator<char> >
	CsvTokenizer;
      CsvTokenizer tok(line);

      CsvTokenizer::iterator i=tok.begin();

      std::string version = *i;
      releaseTable->elementAt(row, 0)->addWidget
	(new WText(href("http://prdownloads.sourceforge.net/witty/wt-"
			+ version + ".tar.gz?download", "Wt " + version)));
      releaseTable->elementAt(row, 1)->addWidget(new WText(*(++i)));
      releaseTable->elementAt(row, 2)->addWidget(new WText(*(++i)));

      ++row;
    }
  }
}

WTreeNode *Home::makeTreeMap(const std::string name, WTreeNode *parent)
{
  WIconPair *labelIcon
    = new WIconPair("icons/yellow-folder-closed.png",
		    "icons/yellow-folder-open.png", false);

  WTreeNode *node = new WTreeNode(name, labelIcon, parent);
  node->label()->setFormatting(WText::PlainFormatting);

  if (!parent) {
    node->setImagePack("icons/");
    node->expand();
    node->setLoadPolicy(WTreeNode::NextLevelLoading);
  }

  return node;
}

WTreeNode *Home::makeTreeFile(const std::string name, WTreeNode *parent)
{
  WIconPair *labelIcon
    = new WIconPair("icons/document.png",
		    "icons/yellow-folder-open.png", false);

  return new WTreeNode("<a href=\"" + fixRelativeUrl("wt/src/" + name)
		       + "\" target=\"_blank\">"
		       + name + "</a>", labelIcon, parent);
}

WString Home::tr(const char *key)
{
  return WString::tr(key);
}

WApplication *createApplication(const WEnvironment& env)
{
  try {
    // support for old (< Wt-2.2) homepage URLS: redirect from "states"
    // to "internal paths"
    // this contains the initial "history state" in old Wt versions
    std::string historyKey = env.getArgument("historyKey")[0];

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

    if (historyKey.find("main:4") != std::string::npos) {
      for (unsigned i = 0; i < 10; ++i)
	if (historyKey.find(exampleStr[i*2]) != std::string::npos) {
	  WApplication *app = new WApplication(env);
	  app->log("notice") << "redirecting old style URL '"
			     << historyKey << "' to internal path: '"
			     << exampleStr[i*2+1] << "'";
	  app->redirect(app->bookmarkUrl(exampleStr[i*2+1]));
	  app->quit();
	  return app;
	}
    } else
      for (unsigned i = 0; i < 6; ++i)
	if (historyKey.find(mainStr[i*2]) != std::string::npos) {
	  WApplication *app = new WApplication(env);

	  app->log("notice") << "redirecting old style URL '"
			     << historyKey << "' to internal path: '"
			     << mainStr[i*2+1] << "'";
	  app->redirect(app->bookmarkUrl(mainStr[i*2+1]));
	  app->quit();
	  return app;
	}

    // unknown history key, just continue
  } catch (std::runtime_error) {
    // no "historyKey argument, simply continue
  }

  return new Home(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

