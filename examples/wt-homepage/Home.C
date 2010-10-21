/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <Wt/WAnchor>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WLogger>
#include <Wt/WMenu>
#include <Wt/WStackedWidget>
#include <Wt/WVBoxLayout>
#include <Wt/WTabWidget>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WText>
#include <Wt/WViewWidget>

#include "Home.h"
#include "view/BlogView.h"

static const std::string SRC_INTERNAL_PATH = "src";

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

Home::Home(const WEnvironment& env, const std::string& title,
	   const std::string& resourceBundle, const std::string& cssPath)
  : WApplication(env),
    releases_(0),
    homePage_(0),
    sourceViewer_(0)
{
  messageResourceBundle().use(appRoot() + resourceBundle, false);
  useStyleSheet(cssPath + "/wt.css");
  useStyleSheet(cssPath + "/wt_ie.css", "lt IE 7");
  useStyleSheet("css/home.css");
  useStyleSheet("css/sourceview.css");
  setTitle(title);

  setLocale("");
  language_ = 0;
}

void Home::init()
{
  internalPathChanged().connect(this, &Home::setup);
  internalPathChanged().connect(this, &Home::setLanguageFromPath);
  internalPathChanged().connect(this, &Home::logInternalPath);

  setup();

  setLanguageFromPath();
}

void Home::setup()
{
  /*
   * This function switches between the two major components of the homepage,
   * depending on the internal path:
   * /src -> source viewer
   * /... -> homepage
   */
  std::string base = internalPathNextPart("/");

  if (base == SRC_INTERNAL_PATH) {
    if (!sourceViewer_) {
      delete homePage_;
      homePage_ = 0;

      root()->clear();

      sourceViewer_ = sourceViewer("/" + SRC_INTERNAL_PATH + "/");
      WVBoxLayout *layout = new WVBoxLayout();
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(sourceViewer_);
      root()->setLayout(layout);
    }
  } else {
    if (!homePage_) {
      delete sourceViewer_;
      sourceViewer_ = 0;

      root()->clear();

      homePage_ = initHome();
      root()->addWidget(homePage_);
    }
  }
}

WWidget *Home::initHome()
{
  WContainerWidget *result = new WContainerWidget(root());
  Div *topWrapper = new Div(result, "top_wrapper");
  Div *topContent = new Div(topWrapper, "top_content");

  Div *languagesDiv = new Div(topContent, "top_languages");

  for (unsigned i = 0; i < languages.size(); ++i) {
    if (i != 0)
      new WText("- ", languagesDiv);

    const Lang& l = languages[i];

    WAnchor *a = new WAnchor("", WString::fromUTF8(l.longDescription_),
			     languagesDiv);
    a->setRefInternalPath(l.path_);
  }

  WText *topWt = new WText(tr("top_wt"), topContent);
  topWt->setInline(false);
  topWt->setId("top_wt");

  WText *bannerWt = new WText(tr("banner_wrapper"), result);
  bannerWt->setId("banner_wrapper");

  Div *mainWrapper = new Div(result, "main_wrapper");
  Div *mainContent = new Div(mainWrapper, "main_content");
  Div *mainMenu = new Div(mainContent, "main_menu");

  WStackedWidget *contents = new WStackedWidget();
  contents->setId("main_page");

  mainMenu_ = new WMenu(contents, Vertical, mainMenu);
  mainMenu_->setRenderAsList(true);

  mainMenu_->addItem
    (tr("introduction"), introduction())->setPathComponent("");

  mainMenu_->addItem
    (tr("blog"), deferCreate(boost::bind(&Home::blog, this)));

  mainMenu_->addItem
    (tr("features"), wrapView(&Home::features),  WMenuItem::PreLoading);

  mainMenu_->addItem
    (tr("documentation"), wrapView(&Home::documentation),
     WMenuItem::PreLoading);

  mainMenu_->addItem
    (tr("examples"), examples(),
     WMenuItem::PreLoading)->setPathComponent("examples/");

  mainMenu_->addItem
    (tr("download"), deferCreate(boost::bind(&Home::download, this)),
     WMenuItem::PreLoading);

  mainMenu_->addItem
    (tr("community"), wrapView(&Home::community), WMenuItem::PreLoading);

  mainMenu_->addItem
    (tr("other-language"), wrapView(&Home::otherLanguage),
     WMenuItem::PreLoading);

  mainMenu_->itemSelectRendered().connect(this, &Home::updateTitle);

  mainMenu_->itemSelected().connect(this, &Home::googleAnalyticsLogger);

  // Make the menu be internal-path aware.
  mainMenu_->setInternalPathEnabled("/");

  sideBarContent_ = new WContainerWidget(mainMenu);

  mainContent->addWidget(contents);
  WContainerWidget *clearAll = new WContainerWidget(mainContent);
  clearAll->setStyleClass("clearall");

  WText *footerWrapper = new WText(tr("footer_wrapper"), result);
  footerWrapper->setId("footer_wrapper");

  return result;
}

void Home::setLanguage(int index)
{
  if (homePage_) {
    const Lang& l = languages[index];

    setLocale(l.code_);

    std::string langPath = l.path_;
    mainMenu_->setInternalBasePath(langPath);
    examplesMenu_->setInternalBasePath(langPath + "examples");
    updateTitle();

    language_ = index;
  }
}

WWidget *Home::linkSourceBrowser(const std::string& example)
{
  WAnchor *a = new WAnchor("", tr("source-browser"));
  a->setRefInternalPath("/" + SRC_INTERNAL_PATH + "/" + example);
  return a;
}

void Home::setLanguageFromPath()
{
  std::string langPath = internalPathNextPart("/");

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

void Home::updateTitle()
{
  if (mainMenu_->currentItem()) {
    setTitle(tr("wt") + " - " + mainMenu_->currentItem()->text());
  }
}

void Home::logInternalPath(const std::string& path)
{
  // simulate an access log for the interal paths
  log("path") << path;

  // If this goes to /src, we need to invoke google analytics method too
  if (path.size() >= 4 && path.substr(0, 4) == "/src") {
    googleAnalyticsLogger();
  }
}

WWidget *Home::introduction()
{
  return new WText(tr("home.intro"));
}

WWidget *Home::blog()
{
  return new BlogView("/blog/", appRoot() + "blog.db", "/wt/blog/feed/");
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

WWidget *Home::otherLanguage()
{
  return new WText(tr("home.other-language"));
}

WWidget *Home::wrapView(WWidget *(Home::*createWidget)())
{
  return makeStaticModel(boost::bind(createWidget, this));
}

std::string Home::href(const std::string& url, const std::string& description)
{
  return "<a href=\"" + url + "\" target=\"_blank\">" + description + "</a>";
}

WWidget *Home::community()
{
  return new WText(tr("home.community"));
}

void Home::readReleases(WTable *releaseTable)
{
  std::ifstream f((filePrefix() + "releases.txt").c_str());

  releaseTable->clear();

  releaseTable->elementAt(0, 0)
    ->addWidget(new WText(tr("home.download.version")));
  releaseTable->elementAt(0, 1)
    ->addWidget(new WText(tr("home.download.date")));
  releaseTable->elementAt(0, 2)
    ->addWidget(new WText(tr("home.download.description")));

  releaseTable->elementAt(0, 0)->resize(WLength(15, WLength::FontEx),
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

      std::string fileName = *i;
      std::string description = *(++i);
      releaseTable->elementAt(row, 0)->addWidget
	(new WText(href("http://prdownloads.sourceforge.net/witty/" 
			+ fileName + "?download", description)));
      releaseTable->elementAt(row, 1)->addWidget(new WText(*(++i)));
      releaseTable->elementAt(row, 2)->addWidget(new WText(*(++i)));

      ++row;
    }
  }
}

WString Home::tr(const char *key)
{
  return WString::tr(key);
}

void Home::googleAnalyticsLogger()
{
  std::string googleCmd = 
    "if (window.pageTracker) {"
    """try {"
    ""  "window.pageTracker._trackPageview(\""
    + environment().deploymentPath() + internalPath() + "\");"
    """} catch (e) { }"
    "}";

  doJavaScript(googleCmd);
}

