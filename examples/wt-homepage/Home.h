// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HOME_H_
#define HOME_H_

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>

namespace Wt {
  class WMenu;
  class WStackedWidget;
  class WTabWidget;
  class WTreeNode;
  class WTable;
}

using namespace Wt;

struct Lang {
  Lang(const std::string& code, const std::string& path,
       const std::string& shortDescription,
       const std::string& longDescription) :
    code_(code),
    path_(path),
    shortDescription_(shortDescription),
    longDescription_(longDescription) {
  }

  std::string code_, path_, shortDescription_, longDescription_;
};

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
    if (count() == 0)
      addWidget(f_());
  }

  Function f_;
};

template <typename Function>
std::unique_ptr<DeferredWidget<Function>> deferCreate(Function f)
{
  return cpp14::make_unique<DeferredWidget<Function>>(f);
}

class Home : public WApplication
{
public:
  Home(const WEnvironment& env, Dbo::SqlConnectionPool& blogDb,
       const std::string& title,
       const std::string& resourceBundle, const std::string& cssPath);
  
  virtual ~Home();

  void googleAnalyticsLogger();

protected:
  virtual std::unique_ptr<WWidget> examples() = 0;
  virtual std::unique_ptr<WWidget> createQuoteForm() = 0;
  virtual std::unique_ptr<WWidget> sourceViewer(const std::string &deployPath) = 0;
  virtual std::string filePrefix() const = 0;

  void init();
  
  void addLanguage(const Lang& l) { languages.push_back(l); }
  std::unique_ptr<WWidget> linkSourceBrowser(const std::string& examplePath);

  WTabWidget *examplesMenu_;
  
  WString tr(const char *key);
  std::string href(const std::string& url, const std::string& description);

  WTable *releases_;
  void readReleases(WTable *releaseTable);

private:
  Dbo::SqlConnectionPool& blogDb_;
  WWidget *homePage_;
  WWidget *sourceViewer_;

  WStackedWidget *contents_;

  void createHome();

  std::unique_ptr<WWidget> introduction();
  std::unique_ptr<WWidget> blog();
  std::unique_ptr<WWidget> status();
  std::unique_ptr<WWidget> features();
  std::unique_ptr<WWidget> documentation();
  std::unique_ptr<WWidget> community();
  std::unique_ptr<WWidget> otherLanguage();
  std::unique_ptr<WWidget> download();
  std::unique_ptr<WWidget> quoteForm();

  WMenu *mainMenu_;

  int language_;

  void readNews(WTable *newsTable, const std::string& newsfile);
  
  std::unique_ptr<WWidget> wrapView(std::unique_ptr<WWidget> (Home::*createFunction)());

  void updateTitle();
  void setLanguage(int language);
  void setLanguageFromPath();
  void setup();
  void logInternalPath(const std::string& path);
  void chatSetUser(const WString& name);

  std::unique_ptr<WContainerWidget> sideBarContent_;
  
  std::vector<Lang> languages;
};

#endif // HOME_H_
