/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include "ChartsExample.h"

using namespace Wt;

class ChartsApplication: public WApplication
{
public:
  ChartsApplication(const WEnvironment& env): WApplication(env) {
  setTitle("Charts example");

  messageResourceBundle().use("charts");

  root()->setPadding(10);

  new ChartsExample(root());


  /*
   * Set our style sheet last, so that it loaded after the ext stylesheets.
   */
  useStyleSheet("charts.css");
  }
};

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new ChartsApplication(env);

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
