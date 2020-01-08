// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef JWT_HOME_H_
#define JWT_HOME_H_

#include <Wt/WApplication.h>

#include "Home.h"

using namespace Wt;

class JWtHome : public Home 
{
public:
  JWtHome(const WEnvironment& env, Dbo::SqlConnectionPool& blogDb);

protected:
  virtual std::unique_ptr<WWidget> examples();
  virtual std::unique_ptr<WWidget> createQuoteForm();
  virtual std::unique_ptr<WWidget> sourceViewer(const std::string& deployPath);
  virtual std::string filePrefix() const { return "jwt-"; }

  std::unique_ptr<WWidget> wrapView(std::unique_ptr<WWidget> (JWtHome::*createFunction)());

private:
  std::unique_ptr<WWidget> example(const char *textKey, const std::string& sourceDir);

  std::unique_ptr<WWidget> helloWorldExample();
  std::unique_ptr<WWidget> chartExample();
  std::unique_ptr<WWidget> composerExample();
  std::unique_ptr<WWidget> treeviewExample();
  std::unique_ptr<WWidget> chatExample();
  std::unique_ptr<WWidget> figtreeExample();
  std::unique_ptr<WWidget> widgetGalleryExample();

  std::string jwtExamplePath_;
};

std::unique_ptr<WApplication> createJWtHomeApplication(const WEnvironment& env,
                                       Dbo::SqlConnectionPool *blogDb);

#endif // JWT_HOME_H_
