/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLogger.h>
#include <Wt/WMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WViewWidget.h>
#include <Wt/WVBoxLayout.h>

#include "Home.h"
#include "view/BlogView.h"

using namespace Wt;

static const std::string SRC_INTERNAL_PATH = "src";

Home::~Home() 
{
}

Home::Home(const WEnvironment& env,
	   Dbo::SqlConnectionPool& blogDb,
	   const std::string& title, const std::string& resourceBundle,
	   const std::string& cssPath)
  : WApplication(env),
    blogDb_(blogDb),
    homePage_(0),
    sourceViewer_(0)
{
  messageResourceBundle().use(appRoot() + resourceBundle, false);

  useStyleSheet(cssPath + "/wt.css");
  useStyleSheet(cssPath + "/wt_ie.css", "lt IE 7", "all");
  useStyleSheet("css/home.css");
  useStyleSheet("css/sourceview.css");
  useStyleSheet("css/chatwidget.css");
  useStyleSheet("css/chatwidget_ie6.css", "lt IE 7", "all");
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
   *
   * FIXME: we should take into account language /cn/src ...
   */
  std::string base = internalPathNextPart("/");

  if (base == SRC_INTERNAL_PATH) {
    if (!sourceViewer_) {
      root()->removeChild(homePage_);
      homePage_ = nullptr;

      root()->clear();

      auto layout = cpp14::make_unique<WVBoxLayout>();
      layout->setContentsMargins(0, 0, 0, 0);
      auto source = sourceViewer("/" + SRC_INTERNAL_PATH + "/");
      sourceViewer_ = source.get();
      layout->addWidget(std::move(source));

      root()->setLayout(std::move(layout));
    }
  } else {
    if (!homePage_) {
      root()->removeChild(sourceViewer_);
      sourceViewer_ = nullptr;
      root()->clear();

      createHome();

      setLanguageFromPath();
    }
  }
}

void Home::createHome()
{
  WTemplate *result = root()->addWidget(cpp14::make_unique<WTemplate>(tr("template")));
  homePage_ = result;

  auto languagesDiv = cpp14::make_unique<WContainerWidget>();
  languagesDiv->setId("top_languages");

  for (unsigned i = 0; i < languages.size(); ++i) {
    if (i != 0)
      languagesDiv->addWidget(cpp14::make_unique<WText>("- "));

    const Lang& l = languages[i];

    languagesDiv->addWidget(cpp14::make_unique<WAnchor>(WLink(LinkType::InternalPath, l.path_), l.longDescription_));
  }

  auto contents = cpp14::make_unique<WStackedWidget>();
  WAnimation fade(AnimationEffect::Fade, TimingFunction::Linear, 250);
  contents->setTransitionAnimation(fade);
  contents->setId("main_page");

  auto mainMenu = cpp14::make_unique<WMenu>(contents.get());
  mainMenu_ = mainMenu.get();
  mainMenu_->addItem
    (tr("introduction"), introduction())->setPathComponent("");

  mainMenu_->addItem
    (tr("blog"), deferCreate(std::bind(&Home::blog, this)));

  mainMenu_->addItem
    (tr("features"), wrapView(&Home::features),
     ContentLoading::Eager);

  mainMenu_->addItem
    (tr("documentation"), wrapView(&Home::documentation),
     ContentLoading::Eager);

  mainMenu_->addItem
    (tr("examples"), examples(),
     ContentLoading::Eager)->setPathComponent("examples/");

  mainMenu_->addItem
    (tr("download"), deferCreate(std::bind(&Home::download, this)),
     ContentLoading::Eager);

  mainMenu_->addItem
    (tr("community"), wrapView(&Home::community),
     ContentLoading::Eager);

  mainMenu_->addItem
    (tr("other-language"), wrapView(&Home::otherLanguage),
     ContentLoading::Eager);

  mainMenu_->itemSelectRendered().connect(this, &Home::updateTitle);

  mainMenu_->itemSelected().connect(this, &Home::googleAnalyticsLogger);

  // Make the menu be internal-path aware.
  mainMenu_->setInternalPathEnabled("/");

  sideBarContent_ = cpp14::make_unique<WContainerWidget>();

  result->bindWidget("languages", std::move(languagesDiv));
  result->bindWidget("menu", std::move(mainMenu));
  result->bindWidget("contents", std::move(contents));
  result->bindWidget("sidebar", std::move(sideBarContent_));
}

void Home::setLanguage(int index)
{
  if (homePage_) {
    const Lang& l = languages[index];

    setLocale(l.code_);

    std::string langPath = l.path_;
    mainMenu_->setInternalBasePath(langPath);
    examplesMenu_->setInternalBasePath(langPath + "examples");
    BlogView *blog = dynamic_cast<BlogView *>(findWidget("blog"));
    if (blog)
      blog->setInternalBasePath(langPath + "blog/");
    updateTitle();

    language_ = index;
  }
}

std::unique_ptr<WWidget> Home::linkSourceBrowser(const std::string& example)
{
  /*
   * Instead of using a WAnchor, which will not progress properly because
   * it is wrapped with wrapView() (-- should we not fix that?), we use
   * a WText which contains an anchor, and enable internal path encoding.
   */
  std::string path = "#/" + SRC_INTERNAL_PATH + "/" + example;
  std::unique_ptr<WText> a(cpp14::make_unique<WText>(tr("source-browser-link").arg(path)));
  a->setInternalPathEncoding(true);
  return std::move(a);
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

std::unique_ptr<WWidget> Home::introduction()
{
  return cpp14::make_unique<WText>(tr("home.intro"));
}

std::unique_ptr<WWidget> Home::blog()
{
  const Lang& l = languages[language_];
  std::string langPath = l.path_;
  std::unique_ptr<BlogView> blog
      = cpp14::make_unique<BlogView>(langPath + "blog/",
                                blogDb_, "/wt/blog/feed/");
  blog->setObjectName("blog");

  if (!blog->user().empty())
    chatSetUser(blog->user());

  blog->userChanged().connect(std::bind(&Home::chatSetUser, this, std::placeholders::_1));

  return std::move(blog);
}

void Home::chatSetUser(const WString& userName)
{
  WApplication::instance()->doJavaScript
    ("if (window.chat && window.chat.emit) {"
     """try {"
     ""  "window.chat.emit(window.chat, 'login', "
     ""                    "" + userName.jsStringLiteral() + "); "
     """} catch (e) {"
     ""  "window.chatUser=" + userName.jsStringLiteral() + ";"
     """}"
     "} else "
     """window.chatUser=" + userName.jsStringLiteral() + ";");
}

std::unique_ptr<WWidget> Home::status()
{
  return cpp14::make_unique<WText>(tr("home.status"));
}

std::unique_ptr<WWidget> Home::features()
{
  return std::unique_ptr<WText>(cpp14::make_unique<WText>(tr("home.features")));
}

std::unique_ptr<WWidget> Home::documentation()
{
  std::unique_ptr<WText> result
      = cpp14::make_unique<WText>(tr("home.documentation"));
  result->setInternalPathEncoding(true);
  return std::move(result);
}

std::unique_ptr<WWidget> Home::otherLanguage()
{
  return std::unique_ptr<WText>(cpp14::make_unique<WText>(tr("home.other-language")));
}

std::unique_ptr<WWidget> Home::wrapView(std::unique_ptr<WWidget> (Home::*createWidget)())
{
  return makeStaticModel(std::bind(createWidget, this));
}

std::string Home::href(const std::string& url, const std::string& description)
{
  return "<a href=\"" + url + "\" target=\"_blank\">" + description + "</a>";
}

std::unique_ptr<WWidget> Home::community()
{
  return cpp14::make_unique<WText>(tr("home.community"));
}

void Home::readReleases(WTable *releaseTable)
{
  std::ifstream f((filePrefix() + "releases.txt").c_str());

  releaseTable->clear();

  releaseTable->elementAt(0, 0)
    ->addWidget(cpp14::make_unique<WText>(tr("home.download.version")));
  releaseTable->elementAt(0, 1)
    ->addWidget(cpp14::make_unique<WText>(tr("home.download.date")));
  releaseTable->elementAt(0, 2)
    ->addWidget(cpp14::make_unique<WText>(tr("home.download.description")));

  releaseTable->elementAt(0, 0)->resize(WLength(15, LengthUnit::FontEx),
					WLength::Auto);
  releaseTable->elementAt(0, 1)->resize(WLength(15, LengthUnit::FontEx),
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
      releaseTable->elementAt(row, 1)->addWidget(cpp14::make_unique<WText>(*(++i)));
      releaseTable->elementAt(row, 2)->addWidget(cpp14::make_unique<WText>(*(++i)));

      ++i;
      std::string url = "http://prdownloads.sourceforge.net/witty/" 
	+ fileName + "?download";
      if (i != tok.end())
	url = *i;
	
      releaseTable->elementAt(row, 0)->addWidget
        (cpp14::make_unique<WText>(href(url, description)));

      ++row;
    }
  }
}

#ifdef WT_EMWEB_BUILD
std::unique_ptr<WWidget> Home::quoteForm()
{
  auto result = cpp14::make_unique<WContainerWidget>();
  result->setStyleClass("quote");

  WTemplate *requestTemplate =
      result->addWidget(cpp14::make_unique<WTemplate>(tr("quote.request")));

  auto quoteButton = cpp14::make_unique<WPushButton>(tr("quote.requestbutton"));
  auto quoteButtonPtr = requestTemplate->bindWidget("button", std::move(quoteButton));

  auto quoteForm = result->addWidget(std::move(createQuoteForm()));

  quoteButtonPtr->clicked().connect(quoteForm, &WWidget::show);
  quoteButtonPtr->clicked().connect(requestTemplate, &WWidget::hide);

  quoteForm->hide();

  return result;
}
#endif // WT_EMWEB_BUILD

std::unique_ptr<WWidget> Home::download()
{
  auto result = cpp14::make_unique<WContainerWidget>();
  result->addWidget(cpp14::make_unique<WText>(tr("home.download")));

  result->addWidget(cpp14::make_unique<WText>(tr("home.download.license")));

#ifdef WT_EMWEB_BUILD
  result->addWidget(std::move(quoteForm()));
#endif // WT_EMWEB_BUILD

  result->addWidget(cpp14::make_unique<WText>(tr("home.download.packages")));

  auto releases = cpp14::make_unique<WTable>();
  readReleases(releases.get());
  releases_ = result->addWidget(std::move(releases));

  result->addWidget(cpp14::make_unique<WText>(tr("home.download.other")));

  return std::move(result);
}


WString Home::tr(const char *key)
{
  return WString::tr(key);
}

void Home::googleAnalyticsLogger()
{
  doJavaScript("if (window.ga) ga('send','pageview',"
	       + WWebWidget::jsStringLiteral(environment().deploymentPath() 
					     + internalPath()) + ");");
}

