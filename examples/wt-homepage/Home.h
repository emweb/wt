// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HOME_H_
#define HOME_H_

#include <Wt/WApplication>
#include <Wt/WContainerWidget>

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
DeferredWidget<Function> *deferCreate(Function f)
{
  return new DeferredWidget<Function>(f);
}

class Home : public WApplication
{
public:
  Home(const WEnvironment& env, Wt::Dbo::SqlConnectionPool& blogDb,
       const std::string& title,
       const std::string& resourceBundle, const std::string& cssPath);
  
  virtual ~Home();

  void googleAnalyticsLogger();

protected:
  virtual WWidget *examples() = 0;
  virtual WWidget *createQuoteForm() = 0;
  virtual WWidget *sourceViewer(const std::string &deployPath) = 0;
  virtual std::string filePrefix() const = 0;

  void init();
  
  void addLanguage(const Lang& l) { languages.push_back(l); }
  WWidget *linkSourceBrowser(const std::string& examplePath);

  WTabWidget *examplesMenu_;
  
  WString tr(const char *key);
  std::string href(const std::string& url, const std::string& description);

  WTable *releases_;
  void readReleases(WTable *releaseTable);

private:
  Wt::Dbo::SqlConnectionPool& blogDb_;
  WWidget *homePage_;
  WWidget *sourceViewer_;

  WStackedWidget *contents_;

  void createHome();

  WWidget *introduction();
  WWidget *blog();
  WWidget *status();
  WWidget *features();
  WWidget *documentation();
  WWidget *community();
  WWidget *otherLanguage();
  WWidget *download();
  WWidget *quoteForm();

  WMenu *mainMenu_;

  int language_;

  void readNews(WTable *newsTable, const std::string& newsfile);
  
  WWidget *wrapView(WWidget *(Home::*createFunction)());

  void updateTitle();
  void setLanguage(int language);
  void setLanguageFromPath();
  void setup();
  void logInternalPath(const std::string& path);
  void chatSetUser(const WString& name);

  WContainerWidget *sideBarContent_;
  
  std::vector<Lang> languages;
};

#endif // HOME_H_
