// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HOME_H_
#define WT_HOME_H_

#include <Wt/WApplication.h>

#include "Home.h"

using namespace Wt;

class WtHome : public Home 
{
public:
  WtHome(const WEnvironment& env, Dbo::SqlConnectionPool& blogDb);

protected:
  virtual std::unique_ptr<WWidget> examples() override;
  virtual std::unique_ptr<WWidget> createQuoteForm() override;
  virtual std::unique_ptr<WWidget> sourceViewer(const std::string &internalPath) override;
  virtual std::string filePrefix() const override { return "wt-"; }

private:
  std::string wtExamplePath_;

  std::unique_ptr<WWidget> example(const char *textKey, const std::string& sourceDir);

  std::unique_ptr<WWidget> helloWorldExample();
  std::unique_ptr<WWidget> chartExample();
  std::unique_ptr<WWidget> homepageExample();
  std::unique_ptr<WWidget> treeviewExample();
  std::unique_ptr<WWidget> gitExample();
  std::unique_ptr<WWidget> chatExample();
  std::unique_ptr<WWidget> composerExample();
  std::unique_ptr<WWidget> widgetGalleryExample();
  std::unique_ptr<WWidget> hangmanExample();

  std::unique_ptr<WWidget> wrapView(std::unique_ptr<WWidget> (WtHome::*createFunction)());
};

std::unique_ptr<WApplication> createWtHomeApplication(const WEnvironment& env,
                                      Dbo::SqlConnectionPool *blogDb);

#endif // WT_HOME_H_
