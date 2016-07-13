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
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>
#include <Wt/WTabWidget>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WViewWidget>
#include <Wt/WVBoxLayout>

#include "Home.h"
#include "view/BlogView.h"

static const std::string SRC_INTERNAL_PATH = "src";

Home::~Home() 
{
}

Home::Home(const WEnvironment& env,
	   Wt::Dbo::SqlConnectionPool& blogDb,
	   const std::string& title, const std::string& resourceBundle,
	   const std::string& cssPath)
  : WApplication(env),
    releases_(0),
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

      createHome();
      root()->addWidget(homePage_);

      setLanguageFromPath();
    }
  }
}

void Home::createHome()
{
  WTemplate *result = new WTemplate(tr("template"), root());
  homePage_ = result;

  WContainerWidget *languagesDiv = new WContainerWidget();
  languagesDiv->setId("top_languages");

  for (unsigned i = 0; i < languages.size(); ++i) {
    if (i != 0)
      new WText("- ", languagesDiv);

    const Lang& l = languages[i];

    new WAnchor(WLink(WLink::InternalPath, l.path_),
		WString::fromUTF8(l.longDescription_), languagesDiv);
  }

  WStackedWidget *contents = new WStackedWidget();
  WAnimation fade(WAnimation::Fade, WAnimation::Linear, 250);
  contents->setTransitionAnimation(fade);
  contents->setId("main_page");

  mainMenu_ = new WMenu(contents, Vertical);

  mainMenu_->addItem
    (tr("introduction"), introduction())->setPathComponent("");

  mainMenu_->addItem
    (tr("blog"), deferCreate(boost::bind(&Home::blog, this)));

  mainMenu_->addItem
    (tr("features"), wrapView(&Home::features), WMenuItem::PreLoading);

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

  sideBarContent_ = new WContainerWidget();

  result->bindWidget("languages", languagesDiv);
  result->bindWidget("menu", mainMenu_);
  result->bindWidget("contents", contents);
  result->bindWidget("sidebar", sideBarContent_);
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

WWidget *Home::linkSourceBrowser(const std::string& example)
{
  /*
   * Instead of using a WAnchor, which will not progress properly because
   * it is wrapped with wrapView() (-- should we not fix that?), we use
   * a WText which contains an anchor, and enable internal path encoding.
   */
  std::string path = "#/" + SRC_INTERNAL_PATH + "/" + example;
  WText *a = new WText(tr("source-browser-link").arg(path));
  a->setInternalPathEncoding(true);
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
  const Lang& l = languages[language_];
  std::string langPath = l.path_;
  BlogView *blog = new BlogView(langPath + "blog/",
				blogDb_, "/wt/blog/feed/");
  blog->setObjectName("blog");

  if (!blog->user().empty())
    chatSetUser(blog->user());

  blog->userChanged().connect(this, &Home::chatSetUser);

  return blog;
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
  WText *result = new WText(tr("home.documentation"));
  result->setInternalPathEncoding(true);
  return result;
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
      releaseTable->elementAt(row, 1)->addWidget(new WText(*(++i)));
      releaseTable->elementAt(row, 2)->addWidget(new WText(*(++i)));

      ++i;
      std::string url = "http://prdownloads.sourceforge.net/witty/" 
	+ fileName + "?download";
      if (i != tok.end())
	url = *i;
	
      releaseTable->elementAt(row, 0)->addWidget
	(new WText(href(url, description)));

      ++row;
    }
  }
}

#ifdef WT_EMWEB_BUILD
WWidget *Home::quoteForm()
{
  WContainerWidget *result = new WContainerWidget();
  result->setStyleClass("quote");

  WTemplate *requestTemplate = new WTemplate(tr("quote.request"), result);

  WPushButton *quoteButton = new WPushButton(tr("quote.requestbutton"));
  requestTemplate->bindWidget("button", quoteButton);

  WWidget *quoteForm = createQuoteForm();
  result->addWidget(quoteForm);

  quoteButton->clicked().connect(quoteForm, &WWidget::show);
  quoteButton->clicked().connect(requestTemplate, &WWidget::hide);

  quoteForm->hide();

  return result;
}
#endif // WT_EMWEB_BUILD

WWidget *Home::download()
{
  WContainerWidget *result = new WContainerWidget();
  result->addWidget(new WText(tr("home.download")));

  result->addWidget(new WText(tr("home.download.license")));

#ifdef WT_EMWEB_BUILD
  result->addWidget(quoteForm());
#endif // WT_EMWEB_BUILD

  result->addWidget(new WText(tr("home.download.packages")));

  releases_ = new WTable();
  readReleases(releases_);
  result->addWidget(releases_);

  result->addWidget(new WText(tr("home.download.other")));

  return result;
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

