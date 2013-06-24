// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef JWT_HOME_H_
#define JWT_HOME_H_

#include <Wt/WApplication>

#include "Home.h"

using namespace Wt;

class JWtHome : public Home 
{
public:
  JWtHome(const WEnvironment& env, Wt::Dbo::SqlConnectionPool& blogDb);

protected:
  virtual WWidget *examples();
  virtual WWidget *createQuoteForm();
  virtual WWidget *sourceViewer(const std::string& deployPath);
  virtual std::string filePrefix() const { return "jwt-"; }

  WWidget *wrapView(WWidget *(JWtHome::*createFunction)());

private:
  WWidget *example(const char *textKey, const std::string& sourceDir);

  WWidget *helloWorldExample();
  WWidget *chartExample();
  WWidget *composerExample();
  WWidget *treeviewExample();
  WWidget *chatExample();
  WWidget *figtreeExample();
  WWidget *widgetGalleryExample();

  std::string jwtExamplePath_;
};

WApplication *createJWtHomeApplication(const WEnvironment& env,
				       Wt::Dbo::SqlConnectionPool *blogDb);

#endif // JWT_HOME_H_
