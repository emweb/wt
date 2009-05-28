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
#include <Wt/WStackedWidget>
#include <Wt/WTabWidget>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WText>
#include <Wt/WTreeNode>
#include <Wt/WViewWidget>

#include "Home.h"

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

Home::~Home() 
{
}

Home::Home(const WEnvironment& env, 
    const std::string& resourceBundle, const std::string& cssPath)
  : WApplication(env),
    recentNews_(0),
    historicalNews_(0),
    releases_(0)
{
  messageResourceBundle().use(resourceBundle, false);
  useStyleSheet(cssPath + "/wt.css");
  useStyleSheet(cssPath + "/wt_ie.css", "lt IE 7");
  useStyleSheet("home.css");
  setTitle("Wt, C++ Web Toolkit");

  setLocale("");
  language_ = 0;
}

void Home::init()
{
  Div *topWrapper = new Div(root(), "top_wrapper");
  Div *topContent = new Div(topWrapper, "top_content");

  Div *languagesDiv = new Div(topContent, "top_languages");

  for (unsigned i = 0; i < languages.size(); ++i) {
    if (i != 0)
      new WText("- ", languagesDiv);

    const Lang& l = languages[i];

    WAnchor *a = new WAnchor("", l.longDescription_, languagesDiv);
    a->setRefInternalPath(l.path_);
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

  mainMenu_->itemSelectRendered().connect(SLOT(this, Home::updateTitle));
  mainMenu_->itemSelected().connect(SLOT(this, Home::logInternalPath));
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

  internalPathChanged().connect(SLOT(this, Home::setLanguageFromPath));
}

void Home::setLanguage(int index)
{
  const Lang& l = languages[index];

  setLocale(l.code_);

  std::string langPath = l.path_;
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
      if (languages[i].path_ == langPath) {
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
  if (!environment().agentIsIEMobile() && environment().javaScript())
    return makeStaticModel(boost::bind(createWidget, this));
  else
    return deferCreate(boost::bind(createWidget, this));
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
	->resize(WLength(16, WLength::FontEx), WLength::Auto);
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
					WLength::Auto);
  releaseTable->elementAt(0, 1)->resize(WLength(15, WLength::FontEx),
					WLength::Auto);

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
  node->label()->setTextFormat(PlainText);

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
