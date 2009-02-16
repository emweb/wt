// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HOME_H_
#define HOME_H_

#include <Wt/WApplication>

namespace Wt {
  class WMenu;
  class WStackedWidget;
  class WTabWidget;
  class WTreeNode;
  class WTable;
}

using namespace Wt;

class Home : public WApplication
{
public:
  Home(const WEnvironment& env);

  void refresh();

private:
  WStackedWidget *contents_;

  WWidget *introduction();
  WWidget *news();
  WWidget *status();
  WWidget *features();
  WWidget *documentation();
  WWidget *examples();
  WWidget *download();
  WWidget *community();

  WTable *recentNews_;
  WTable *historicalNews_;
  WTable *releases_;

  WMenu *mainMenu_;
  WTabWidget *examplesMenu_;

  int language_;

  void readNews(WTable *newsTable, const std::string newsfile);
  void readReleases(WTable *releaseTable, const std::string releasefile);
  static std::string href(const std::string url,
			  const std::string description);

  WTreeNode *makeTreeMap(const std::string name, WTreeNode *parent);
  WTreeNode *makeTreeFile(const std::string name, WTreeNode *parent);

  WWidget *helloWorldExample();
  WWidget *chartExample();
  WWidget *homepageExample();
  WWidget *treeviewExample();
  WWidget *gitExample();
  WWidget *chatExample();
  WWidget *composerExample();
  WWidget *widgetGalleryExample();

  WWidget *wrapViewOrDefer(WWidget *(Home::*createFunction)());

  void updateTitle();
  void logInternalPath();
  void changeLanguage(int language);
  void setLanguage(int language);
  void setLanguageFromPath(std::string prefix);

  static WString tr(const char *key);

  WContainerWidget *sideBarContent_;
};

#endif // HOME_H_
