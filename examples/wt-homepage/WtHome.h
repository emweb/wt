// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HOME_H_
#define WT_HOME_H_

#include <Wt/WApplication>

#include "Home.h"

using namespace Wt;

class WtHome : public Home 
{
public:
  WtHome(const WEnvironment& env, Wt::Dbo::SqlConnectionPool& blogDb);

protected:
  virtual WWidget *examples();
  virtual WWidget *createQuoteForm();
  virtual WWidget *sourceViewer(const std::string &internalPath);
  virtual std::string filePrefix() const { return "wt-"; }

private:
  std::string wtExamplePath_;

  WWidget *example(const char *textKey, const std::string& sourceDir);

  WWidget *helloWorldExample();
  WWidget *chartExample();
  WWidget *homepageExample();
  WWidget *treeviewExample();
  WWidget *gitExample();
  WWidget *chatExample();
  WWidget *composerExample();
  WWidget *widgetGalleryExample();
  WWidget *hangmanExample();

  WWidget *wrapView(WWidget *(WtHome::*createFunction)());
};

WApplication *createWtHomeApplication(const WEnvironment& env,
				      Wt::Dbo::SqlConnectionPool *blogDb);

#endif // WT_HOME_H_
